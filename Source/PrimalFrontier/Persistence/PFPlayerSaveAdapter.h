#pragma once
#include "Persistence/PFPlayerSaveFormat.h"
class APFSurvivalPlayerController;
class UPFInventoryComponent;

/** Game-thread server adapters. These APIs are never client RPCs. */
class PRIMALFRONTIER_API FPFPlayerSaveAdapter
{
public:
    static TArray<FPFSavedItemStack> CaptureInventory(const UPFInventoryComponent& Inventory);
    static bool Capture(APFSurvivalPlayerController* PC, FPFPlayerSaveData& Out, FString& Error);
    static bool CanRestore(APFSurvivalPlayerController* PC, const FPFPlayerSaveData& Data,
        double Age, FString& Error);
    static bool Restore(APFSurvivalPlayerController* PC, const FPFPlayerSaveData& Data,
        double Age, FString& Error);
};
