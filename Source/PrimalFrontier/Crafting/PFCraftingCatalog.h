// PFCraftingCatalog.h
//
// Data asset holding crafting recipes and gatherable resource-node definitions.
// Loaded from Content/PrimalFrontier/Crafting/DA_CraftingCatalog.
//
// As with the item catalog, defaults are built in the C++ constructor and the
// lookups Recipe()/Resource() "fail closed": any duplicate ID, unknown item
// reference or out-of-range number returns nullptr.
//
// History: M4 (e7ffd71). Docs: Docs/GATHERING_CRAFTING_M4.md, Docs/ITEMS_AND_CRAFTING.md

#pragma once
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "PFCraftingCatalog.generated.h"
class UPFItemCatalog;

/** One recipe input: Quantity of ItemId. */
USTRUCT(BlueprintType)
struct FPFCraftIngredient
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FName ItemId;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 Quantity=1;
};

/** A timed recipe: Ingredients -> OutputQuantity x Output after Duration seconds. */
USTRUCT(BlueprintType)
struct FPFRecipeDefinition
{
    GENERATED_BODY()
    /** Stable ID (e.g. "Recipe_Tool"); the only thing a client sends when crafting. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FName Id;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FText DisplayName;
    /** Recipe.Category.Tool / Food / Material / Weapon. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FGameplayTag Category;
    /** 1..8 distinct items; none may equal Output (prevents refresh loops). */
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<FPFCraftIngredient> Ingredients;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FName Output;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 OutputQuantity=1;
    /** Seconds, 0.1..600. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float Duration=5;
};

/** A gatherable world node type (wood pile, stone outcrop, forage patch). */
USTRUCT(BlueprintType)
struct FPFResourceDefinition
{
    GENERATED_BODY()
    /** Stable ID referenced by APFResourceNode::ResourceId (e.g. "Node_Wood"). */
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FName Id;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FText DisplayName;
    /** Item granted per hit. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FName YieldItem;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 YieldPerHit=2;
    /** Hits before the node is depleted (finite total yield = Hits * YieldPerHit). */
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 Hits=3;
    /** Seconds after depletion before the node refills. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float RespawnSeconds=20;
};

UCLASS(BlueprintType)
class PRIMALFRONTIER_API UPFCraftingCatalog : public UDataAsset
{
    GENERATED_BODY()

public:
    /** Fills Recipes and Resources with the greybox defaults. */
    UPFCraftingCatalog();
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<FPFRecipeDefinition> Recipes;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<FPFResourceDefinition> Resources;

    /** Validated recipe lookup; Items is needed to check that every referenced item exists. */
    const FPFRecipeDefinition* Recipe(FName Id, const UPFItemCatalog* Items) const;
    /** Validated resource lookup. */
    const FPFResourceDefinition* Resource(FName Id, const UPFItemCatalog* Items) const;
};
