#pragma once
#include "Persistence/PFWorldSaveData.h"
class UPFItemCatalog;
class UPFBuildingCatalog;
class UPFCraftingCatalog;
class UPFCreatureCatalog;
class UPFProgressionCatalog;

/** Trusted reviewed manifest entry, never a client request or proximity match. */
struct FPFResourceLayoutEntry
{
    FString Name;
    FString Definition;
};

/** Offline additive value transform only. No registered live edge or file/actor mutation. */
class PRIMALFRONTIER_API FPFResourceLayoutTransform
{
public:
    static bool Prepare(const FString& ExpectedMap,const TArray<FPFResourceLayoutEntry>& Previous,
        const TArray<FPFResourceLayoutEntry>& Added,const FPFWorldSaveData& Source,
        const UPFItemCatalog& Items,const UPFBuildingCatalog& Buildings,const UPFCraftingCatalog& Crafting,
        const UPFCreatureCatalog& Creatures,const UPFProgressionCatalog& Progression,
        FPFWorldSaveData& Out,FString& Error);
};
