#pragma once
#include "CoreMinimal.h"
class UPFItemCatalog;
class UPFCraftingCatalog;
class UPFProgressionCatalog;

/** Append-only home-world content. No dimensional materials or powered-Tech claims. */
namespace PFEryndor
{
    void AddItems(UPFItemCatalog& Catalog);
    void AddCrafting(UPFCraftingCatalog& Catalog);
    void AddKnowledge(UPFProgressionCatalog& Catalog);
    bool IsMineral(FName Item);
}
