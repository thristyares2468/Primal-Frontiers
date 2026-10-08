#pragma once
#include "Persistence/PFWorldSaveData.h"
#include "Persistence/PFPlayerSaveFormat.h"
class UPFItemCatalog;
class UPFBuildingCatalog;
class UPFCraftingCatalog;
class UPFCreatureCatalog;

/** Pure bounded metadata codec. No actor spawning, file I/O, auth or object-path imports. */
class PRIMALFRONTIER_API FPFWorldSaveFormat
{
public:
    static bool PackPlayer(const FPFPlayerSaveData& Data, const UPFItemCatalog& Items, double Weight, FString& Out, FString& Error);
    static bool UnpackPlayer(const FString& Text, const UPFItemCatalog& Items, double Weight, FPFPlayerSaveData& Out, FString& Error);
    static bool Validate(const FPFWorldSaveData& Data, const UPFItemCatalog& Items,
        const UPFBuildingCatalog& Buildings, const UPFCraftingCatalog& Crafting, const UPFCreatureCatalog& Creatures, FString& Error);
    static bool Encode(const FPFWorldSaveData& Data, TArray<uint8>& Out, FString& Error);
    static bool Decode(const TArray<uint8>& Bytes, FPFWorldSaveData& Out, FString& Error);
};
