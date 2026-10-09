#pragma once
#include "Persistence/PFWorldSaveData.h"
#include "Persistence/PFPlayerSaveFormat.h"
class UPFItemCatalog;
class UPFBuildingCatalog;
class UPFCraftingCatalog;
class UPFCreatureCatalog;
class UPFProgressionCatalog;
struct FPFProgressionRecord;

/** Pure bounded metadata codec. No actor spawning, file I/O, auth or object-path imports. */
class PRIMALFRONTIER_API FPFWorldSaveFormat
{
public:
    static bool PackPlayer(const FPFPlayerSaveData& Data, const UPFItemCatalog& Items, double Weight, FString& Out, FString& Error);
    static bool UnpackPlayer(const FString& Text, const UPFItemCatalog& Items, double Weight, FPFPlayerSaveData& Out, FString& Error);
    static bool PackProgression(FGuid Owner,const FPFProgressionRecord& Record,const UPFProgressionCatalog& Catalog,
        const UPFCraftingCatalog& Crafting,const UPFItemCatalog& Items,FString& Out,FString& Error);
    static bool UnpackProgression(const FString& Text,FGuid Owner,const UPFProgressionCatalog& Catalog,
        const UPFCraftingCatalog& Crafting,const UPFItemCatalog& Items,FPFProgressionRecord& Out,FString& Error);
    static bool Validate(const FPFWorldSaveData& Data, const UPFItemCatalog& Items,
        const UPFBuildingCatalog& Buildings, const UPFCraftingCatalog& Crafting, const UPFCreatureCatalog& Creatures, FString& Error,
        const UPFProgressionCatalog* Progression=nullptr);
    static bool DecodeValidated(const TArray<uint8>& Bytes,const UPFItemCatalog& Items,const UPFBuildingCatalog& Buildings,
        const UPFCraftingCatalog& Crafting,const UPFCreatureCatalog& Creatures,FPFWorldSaveData& Out,FString& Error,
        const UPFProgressionCatalog* Progression=nullptr);
    static bool Encode(const FPFWorldSaveData& Data, TArray<uint8>& Out, FString& Error);
    static bool Decode(const TArray<uint8>& Bytes, FPFWorldSaveData& Out, FString& Error);
};
