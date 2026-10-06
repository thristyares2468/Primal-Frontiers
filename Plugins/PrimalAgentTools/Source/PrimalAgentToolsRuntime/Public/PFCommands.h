// PFCommands.h
//
// Public API of the PF.* developer command layer (implemented in PFCommands.cpp).
// Used by the console registration, the editor module and the automation tests.
// Commands are compiled out of Shipping builds. History: ccbad56 (foundation).
#pragma once
#include "PFResults.h"

class UWorld;
namespace PF::AgentTools
{
/** One row of the command table returned by CommandSpecs(). */
struct FCommandSpec
{
    // Name: console name ("PF.GiveItem"). Help: usage text shown by PF.Help.
    // Blocker: if non-empty, the command is NOT IMPLEMENTED and this is the reason.
    FString Name, Help, Blocker;
    // Allowed argument count (inclusive).
    int32 MinArgs = 0, MaxArgs = 0;
    // bEditor: runs only through the editor backend. bMutation: changes game state,
    // so it is refused on network clients.
    bool bEditor = false, bMutation = false;
};
/** Handler the editor module installs to run bEditor commands. */
using FEditorCommand = TFunction<FResult(const FString&, const TArray<FString>&, UWorld*)>;
/** Installs (or, with an empty handler, removes) the editor command backend. */
PRIMALAGENTTOOLSRUNTIME_API void SetEditorCommand(FEditorCommand Handler);
/** The full, fixed command table. */
PRIMALAGENTTOOLSRUNTIME_API const TArray<FCommandSpec>& CommandSpecs();
/** Returns an empty string when Args are valid for Spec, otherwise the reason. */
PRIMALAGENTTOOLSRUNTIME_API FString ValidateArguments(const FCommandSpec& Spec, const TArray<FString>& Args);
/** Runs a command against World (may be null) and records it in CommandHistory(). */
PRIMALAGENTTOOLSRUNTIME_API FResult ExecuteCommand(const FString& Name, const TArray<FString>& Args, UWorld* World, bool bLog = true);
/** Every command executed in this process so far, oldest first. */
PRIMALAGENTTOOLSRUNTIME_API const TArray<FResult>& CommandHistory();
}
