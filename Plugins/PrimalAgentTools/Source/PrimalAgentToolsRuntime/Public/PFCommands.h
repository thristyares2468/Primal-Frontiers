#pragma once
#include "PFResults.h"

class UWorld;
namespace PF::AgentTools
{
struct FCommandSpec
{
    FString Name, Help, Blocker;
    int32 MinArgs = 0, MaxArgs = 0;
    bool bEditor = false, bMutation = false;
};
using FEditorCommand = TFunction<FResult(const FString&, const TArray<FString>&, UWorld*)>;
PRIMALAGENTTOOLSRUNTIME_API void SetEditorCommand(FEditorCommand Handler);
PRIMALAGENTTOOLSRUNTIME_API const TArray<FCommandSpec>& CommandSpecs();
PRIMALAGENTTOOLSRUNTIME_API FString ValidateArguments(const FCommandSpec& Spec, const TArray<FString>& Args);
PRIMALAGENTTOOLSRUNTIME_API FResult ExecuteCommand(const FString& Name, const TArray<FString>& Args, UWorld* World, bool bLog = true);
PRIMALAGENTTOOLSRUNTIME_API const TArray<FResult>& CommandHistory();
}
