#pragma once

#include "CoreMinimal.h"
#include "PFProgressionRecord.generated.h"

class UPFProgressionCatalog;
class UPFCraftingCatalog;
class UPFItemCatalog;

/** Bounded player-owned record. No clocks, UObject paths, cached level/points or client claims. */
USTRUCT(BlueprintType)
struct PRIMALFRONTIER_API FPFProgressionRecord
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadOnly) int32 Experience=0;
    UPROPERTY(BlueprintReadOnly) TArray<FName> Knowledge;
    UPROPERTY(BlueprintReadOnly) TArray<FName> CreditedCrafts;
};

/** Pure transactions for trusted server callers, not networking authority or proof of an event.
 *  No Blueprint grant function or RPC. Mutating methods validate and commit a complete candidate.
 *  Persistence, successful-event hooks, gather windows, building/discovery and UI are separate gates. */
class PRIMALFRONTIER_API FPFProgressionTransactions
{
public:
    static constexpr int32 MaximumExperience=2700;
    static constexpr int32 FirstCraftExperience=20;
    static int32 LevelForExperience(int32 Experience);
    static bool Validate(const FPFProgressionRecord& Record,const UPFProgressionCatalog& Catalog,
        const UPFCraftingCatalog& Crafting,const UPFItemCatalog& Items,FString& Error);
    static int32 AvailablePoints(const FPFProgressionRecord& Record,const UPFProgressionCatalog& Catalog);
    static bool AwardExperience(FPFProgressionRecord& Record,int32 Amount,const UPFProgressionCatalog& Catalog,
        const UPFCraftingCatalog& Crafting,const UPFItemCatalog& Items,FString& Error);
    static bool CreditFirstCraft(FPFProgressionRecord& Record,FName Recipe,const UPFProgressionCatalog& Catalog,
        const UPFCraftingCatalog& Crafting,const UPFItemCatalog& Items,FString& Error);
    static bool Purchase(FPFProgressionRecord& Record,FName Knowledge,const UPFProgressionCatalog& Catalog,
        const UPFCraftingCatalog& Crafting,const UPFItemCatalog& Items,FString& Error);
};
