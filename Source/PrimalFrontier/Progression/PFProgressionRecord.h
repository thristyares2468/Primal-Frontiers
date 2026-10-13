#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "PFProgressionRecord.generated.h"

class UPFProgressionCatalog;
class UPFCraftingCatalog;
class UPFItemCatalog;

UENUM(BlueprintType)
enum class EPFPlayerStat : uint8 { Attack, Stamina, Weight, CraftingSpeed };

/** Allocations, not claimed bonuses or point balances. Technology uses its own ledger. */
USTRUCT(BlueprintType)
struct PRIMALFRONTIER_API FPFPlayerStatRanks
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadOnly) int32 Attack=0;
    UPROPERTY(BlueprintReadOnly) int32 Stamina=0;
    UPROPERTY(BlueprintReadOnly) int32 Weight=0;
    UPROPERTY(BlueprintReadOnly) int32 CraftingSpeed=0;
    bool operator==(const FPFPlayerStatRanks& Other) const
    {return Attack==Other.Attack && Stamina==Other.Stamina && Weight==Other.Weight && CraftingSpeed==Other.CraftingSpeed;}
};

/** Remaining active-server duration, never an absolute/offline clock or a client timestamp. */
USTRUCT(BlueprintType)
struct PRIMALFRONTIER_API FPFGatherRewardWindow
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadOnly) FGameplayTag Category;
    UPROPERTY(BlueprintReadOnly) int32 Rewards=0;
    UPROPERTY(BlueprintReadOnly) double RemainingSeconds=0;
    bool operator==(const FPFGatherRewardWindow& Other) const
    {return Category==Other.Category && Rewards==Other.Rewards && RemainingSeconds==Other.RemainingSeconds;}
};

/** Bounded player-owned record. No absolute clocks, UObject paths, cached level/points or client claims. */
USTRUCT(BlueprintType)
struct PRIMALFRONTIER_API FPFProgressionRecord
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadOnly) int32 Experience=0;
    UPROPERTY(BlueprintReadOnly) TArray<FName> Knowledge;
    UPROPERTY(BlueprintReadOnly) TArray<FName> CreditedCrafts;
    UPROPERTY(BlueprintReadOnly) TArray<FPFGatherRewardWindow> GatherWindows;
    UPROPERTY(BlueprintReadOnly) FPFPlayerStatRanks Stats;
};

/** Pure transactions for trusted server callers, not networking authority or proof of an event.
 *  No Blueprint grant function or RPC. Mutating methods validate and commit a complete candidate.
 *  Persistence, successful-event hooks, gather windows, building/discovery and UI are separate gates. */
class PRIMALFRONTIER_API FPFProgressionTransactions
{
public:
    static constexpr int32 MaximumExperience=2700;
    static constexpr int32 FirstCraftExperience=20;
    static constexpr int32 GatherExperience=5;
    static constexpr int32 MaximumGatherCategories=5;
    static constexpr int32 MaximumGatherRewards=5;
    static constexpr double GatherWindowSeconds=1800;
    /** Exact native tags only; unknown child tags are not reward categories. */
    static TArray<FGameplayTag> GatherCategories();
    static FGameplayTag GatherCategoryForItem(FName Item);
    /** Trusted future server success-event caller only, no gameplay hook/RPC implied. */
    static bool CreditGather(FPFProgressionRecord& Record,FGameplayTag Category,const UPFProgressionCatalog& Catalog,
        const UPFCraftingCatalog& Crafting,const UPFItemCatalog& Items,FString& Error);
    /** Atomic trusted elapsed-active-time operation. Offline time/time-of-day must never feed this. */
    static bool AdvanceGatherWindows(FPFProgressionRecord& Record,double ActiveSeconds,const UPFProgressionCatalog& Catalog,
        const UPFCraftingCatalog& Crafting,const UPFItemCatalog& Items,FString& Error);
    static int32 LevelForExperience(int32 Experience);
    /** Cumulative threshold; clamped to the supported ten-level slice. */
    static int32 ExperienceForLevel(int32 Level);
    static bool Validate(const FPFProgressionRecord& Record,const UPFProgressionCatalog& Catalog,
        const UPFCraftingCatalog& Crafting,const UPFItemCatalog& Items,FString& Error);
    static int32 AvailablePoints(const FPFProgressionRecord& Record,const UPFProgressionCatalog& Catalog);
    static int32 AvailableStatPoints(const FPFProgressionRecord& Record);
    static float AttackMultiplier(const FPFProgressionRecord& R);
    static float StaminaLimit(const FPFProgressionRecord& R);
    static float WeightLimit(const FPFProgressionRecord& R);
    static float CraftingMultiplier(const FPFProgressionRecord& R);
    static bool PurchaseStat(FPFProgressionRecord& Record,EPFPlayerStat Stat,const UPFProgressionCatalog& Catalog,
        const UPFCraftingCatalog& Crafting,const UPFItemCatalog& Items,FString& Error);
    static bool AwardExperience(FPFProgressionRecord& Record,int32 Amount,const UPFProgressionCatalog& Catalog,
        const UPFCraftingCatalog& Crafting,const UPFItemCatalog& Items,FString& Error);
    static bool CreditFirstCraft(FPFProgressionRecord& Record,FName Recipe,const UPFProgressionCatalog& Catalog,
        const UPFCraftingCatalog& Crafting,const UPFItemCatalog& Items,FString& Error);
    static bool Purchase(FPFProgressionRecord& Record,FName Knowledge,const UPFProgressionCatalog& Catalog,
        const UPFCraftingCatalog& Crafting,const UPFItemCatalog& Items,FString& Error);
};
