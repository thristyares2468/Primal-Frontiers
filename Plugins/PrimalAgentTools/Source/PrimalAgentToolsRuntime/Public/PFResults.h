#pragma once

#include "CoreMinimal.h"

PRIMALAGENTTOOLSRUNTIME_API DECLARE_LOG_CATEGORY_EXTERN(LogPrimalAgentTools, Log, All);

namespace PF::AgentTools
{
struct FIssue
{
    FString Severity;
    FString Code;
    FString Asset;
    FString Message;
};

struct PRIMALAGENTTOOLSRUNTIME_API FResult
{
    explicit FResult(FString InCommand, FString InScope = FString());
    FString Command;
    FString Scope;
    FString StartedUtc;
    FString Map = TEXT("Unavailable");
    bool bNotImplemented = false;
    TMap<FString, int32> Counts;
    TArray<FIssue> Issues;
    TArray<FString> Artifacts;

    void Add(const TCHAR* Severity, const TCHAR* Code, const FString& Asset, const FString& Message);
    bool HasErrors() const;
    FString Status() const;
};

// Labels, never paths: all artifacts stay directly inside Saved/AutomationReports.
PRIMALAGENTTOOLSRUNTIME_API bool IsSafeLabel(const FString& Label);
PRIMALAGENTTOOLSRUNTIME_API FString ReportsDirectory();
PRIMALAGENTTOOLSRUNTIME_API FString NewArtifactPath(const FString& Label, const FString& Extension);
PRIMALAGENTTOOLSRUNTIME_API FResult ExportResults(const TArray<FResult>& Results, const FString& Label);

PRIMALAGENTTOOLSRUNTIME_API void LogResult(const FResult& Result);
}
