// PFResults.cpp
//
// FResult helpers and the JSON report writer behind PF.ExportTestReport.
// Reports go to Saved/AutomationReports/PF_<label>_<utc>_<guid>.json (schemaVersion 2)
// with an overall status: Failed if any run has errors, NeedsAttention if any run is
// not a clean pass, otherwise Passed. History: ccbad56 (foundation).

#include "PFResults.h"

#include "Dom/JsonObject.h"
#include "HAL/FileManager.h"
#include "Misc/App.h"
#include "Misc/EngineVersion.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonSerializer.h"

DEFINE_LOG_CATEGORY(LogPrimalAgentTools);

namespace PF::AgentTools
{
FResult::FResult(FString InCommand, FString InScope)
    : Command(MoveTemp(InCommand)), Scope(MoveTemp(InScope)), StartedUtc(FDateTime::UtcNow().ToIso8601()) {}

void FResult::Add(const TCHAR* Severity, const TCHAR* Code, const FString& Asset, const FString& Message)
{
    Issues.Add({Severity, Code, Asset, Message});
}

bool FResult::HasErrors() const
{
    return Issues.ContainsByPredicate([](const FIssue& I) { return I.Severity == TEXT("Error"); });
}

FString FResult::Status() const
{
    if (HasErrors()) { return TEXT("Failed"); }
    if (bNotImplemented) { return TEXT("NOT IMPLEMENTED"); }
    if (Issues.ContainsByPredicate([](const FIssue& I) { return I.Severity == TEXT("Warning"); }))
    {
        return TEXT("NeedsAttention");
    }
    return TEXT("Passed");
}

bool IsSafeLabel(const FString& Label)
{
    if (Label.IsEmpty() || Label.Len() > 64) { return false; }
    for (TCHAR C : Label)
    {
        if (!((C >= 'a' && C <= 'z') || (C >= 'A' && C <= 'Z') ||
            (C >= '0' && C <= '9') || C == '_' || C == '-')) { return false; }
    }
    return true;
}

FString ReportsDirectory()
{
    return FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() / TEXT("AutomationReports"));
}

FString NewArtifactPath(const FString& Label, const FString& Extension)
{
    if (!IsSafeLabel(Label) || (Extension != TEXT("json") && Extension != TEXT("png"))) { return FString(); }
    return ReportsDirectory() / FString::Printf(TEXT("PF_%s_%s_%s.%s"), *Label,
        *FDateTime::UtcNow().ToString(TEXT("%Y%m%dT%H%M%S")),
        *FGuid::NewGuid().ToString(EGuidFormats::Digits), *Extension);
}

FResult ExportResults(const TArray<FResult>& Results, const FString& Label)
{
    FResult Result(TEXT("PF.ExportResults"), ReportsDirectory());
    const FString Path = NewArtifactPath(Label, TEXT("json"));
    if (Path.IsEmpty())
    {
        Result.Add(TEXT("Error"), TEXT("InvalidLabel"), Label, TEXT("Use 1-64 ASCII letters, digits, underscore or hyphen; paths are not accepted."));
        return Result;
    }
    TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
    Root->SetNumberField(TEXT("schemaVersion"), 2);
    Root->SetStringField(TEXT("project"), FApp::GetProjectName());
    Root->SetStringField(TEXT("engineVersion"), FEngineVersion::Current().ToString());
    Root->SetStringField(TEXT("exportedUtc"), FDateTime::UtcNow().ToIso8601());
    FString Overall = Results.IsEmpty() ? TEXT("NoResults") : TEXT("Passed");
    TArray<TSharedPtr<FJsonValue>> Runs;
    for (const FResult& Run : Results)
    {
        if (Run.HasErrors()) { Overall = TEXT("Failed"); }
        else if (Overall == TEXT("Passed") && Run.Status() != TEXT("Passed")) { Overall = TEXT("NeedsAttention"); }
        TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
        Object->SetStringField(TEXT("command"), Run.Command);
        Object->SetStringField(TEXT("testName"), Run.Command);
        Object->SetStringField(TEXT("timestamp"), Run.StartedUtc);
        Object->SetStringField(TEXT("map"), Run.Map);
        Object->SetStringField(TEXT("result"), Run.Status());
        Object->SetStringField(TEXT("scope"), Run.Scope);
        Object->SetStringField(TEXT("startedUtc"), Run.StartedUtc);
        Object->SetStringField(TEXT("status"), Run.Status());
        TSharedRef<FJsonObject> Counts = MakeShared<FJsonObject>();
        TArray<FString> Keys;
        Run.Counts.GetKeys(Keys);
        Keys.Sort();
        for (const FString& Key : Keys) { Counts->SetNumberField(Key, Run.Counts[Key]); }
        Object->SetObjectField(TEXT("counts"), Counts);
        TArray<TSharedPtr<FJsonValue>> Issues;
        for (const FIssue& Issue : Run.Issues)
        {
            TSharedRef<FJsonObject> Entry = MakeShared<FJsonObject>();
            Entry->SetStringField(TEXT("severity"), Issue.Severity);
            Entry->SetStringField(TEXT("code"), Issue.Code);
            Entry->SetStringField(TEXT("asset"), Issue.Asset);
            Entry->SetStringField(TEXT("message"), Issue.Message);
            Issues.Add(MakeShared<FJsonValueObject>(Entry));
        }
        Object->SetArrayField(TEXT("issues"), Issues);
        Object->SetArrayField(TEXT("details"), Issues);
        TArray<TSharedPtr<FJsonValue>> Artifacts;
        for (const FString& Artifact : Run.Artifacts) { Artifacts.Add(MakeShared<FJsonValueString>(Artifact)); }
        Object->SetArrayField(TEXT("artifacts"), Artifacts);
        Runs.Add(MakeShared<FJsonValueObject>(Object));
    }
    Root->SetStringField(TEXT("status"), Overall);
    Root->SetArrayField(TEXT("results"), Runs);
    FString Json;
    const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Json);
    if (!FJsonSerializer::Serialize(Root, Writer) ||
        !IFileManager::Get().MakeDirectory(*ReportsDirectory(), true) ||
        !FFileHelper::SaveStringToFile(Json, *Path, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
    {
        Result.Add(TEXT("Error"), TEXT("ReportWriteFailed"), Path, TEXT("Could not write the JSON report."));
        return Result;
    }
    Result.Counts.Add(TEXT("exportedResults"), Results.Num());
    Result.Artifacts.Add(Path);
    if (Results.IsEmpty()) { Result.Add(TEXT("Warning"), TEXT("NoResults"), FString(), TEXT("No commands have run in this editor session.")); }
    return Result;
}

void LogResult(const FResult& Result)
{
    UE_LOG(LogPrimalAgentTools, Display, TEXT("[PrimalAgentTools] %s: %s (scope=%s, issues=%d)"),
        *Result.Command, *Result.Status(), *Result.Scope, Result.Issues.Num());
    for (const FIssue& Issue : Result.Issues)
    {
        if (Issue.Severity == TEXT("Error"))
        {
            UE_LOG(LogPrimalAgentTools, Error, TEXT("[PrimalAgentTools] %s %s: %s"), *Issue.Code, *Issue.Asset, *Issue.Message);
        }
        else if (Issue.Severity == TEXT("Warning"))
        {
            UE_LOG(LogPrimalAgentTools, Warning, TEXT("[PrimalAgentTools] %s %s: %s"), *Issue.Code, *Issue.Asset, *Issue.Message);
        }
        else { UE_LOG(LogPrimalAgentTools, Display, TEXT("[PrimalAgentTools] %s %s: %s"), *Issue.Code, *Issue.Asset, *Issue.Message); }
    }
    for (const FString& Artifact : Result.Artifacts)
    {
        UE_LOG(LogPrimalAgentTools, Display, TEXT("[PrimalAgentTools] Artifact: %s"), *Artifact);
    }
}
}
