// PFResults.h
//
// Structured result type shared by every PF.* command and asset check, plus the
// JSON report export to Saved/AutomationReports. History: ccbad56 (foundation).
#pragma once

#include "CoreMinimal.h"

PRIMALAGENTTOOLSRUNTIME_API DECLARE_LOG_CATEGORY_EXTERN(LogPrimalAgentTools, Log, All);

namespace PF::AgentTools
{
/** One finding. Severity is "Error", "Warning" or "Info"; Code is a stable ID such as "NotAuthority". */
struct FIssue
{
    FString Severity;
    FString Code;
    FString Asset;
    FString Message;
};

/** Outcome of one command: metadata, counters, issues and any files produced. */
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
    /** True if any issue has Error severity. */
    bool HasErrors() const;
    /** "Failed", "NOT IMPLEMENTED", "NeedsAttention" (warnings) or "Passed". */
    FString Status() const;
};

// Labels, never paths: all artifacts stay directly inside Saved/AutomationReports.
PRIMALAGENTTOOLSRUNTIME_API bool IsSafeLabel(const FString& Label);
PRIMALAGENTTOOLSRUNTIME_API FString ReportsDirectory();
/** Unique "PF_<label>_<utc>_<guid>.<ext>" path; empty if the label or extension (json/png) is invalid. */
PRIMALAGENTTOOLSRUNTIME_API FString NewArtifactPath(const FString& Label, const FString& Extension);
/** Writes Results as a JSON report and returns a result describing the export. */
PRIMALAGENTTOOLSRUNTIME_API FResult ExportResults(const TArray<FResult>& Results, const FString& Label);

/** Logs a summary line, then one line per issue (at its severity) and per artifact. */
PRIMALAGENTTOOLSRUNTIME_API void LogResult(const FResult& Result);
}
