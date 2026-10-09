// PFItemCatalog.h
//
// Data asset listing every item the game knows about (ID, name, category tag,
// stack size, weight, icon, freshness and nutrition). Loaded from
// Content/PrimalFrontier/Items/DA_ItemCatalog.
//
// NOTE: the default entries are created in the C++ constructor; the saved data
// asset currently stores no overrides, so editing the constructor changes the
// game's items. Designers can still override values in the asset in the Editor.
//
// Find() is the only way gameplay reads definitions, and it "fails closed":
// duplicate IDs or out-of-range values return nullptr so bad data can never
// grant impossible items.
//
// History: M3 (340c538) wood/stone/food; M4 (e7ffd71) tool, cooked and dried food.
// Docs:    Docs/INVENTORY_M3.md, Docs/ITEMS_AND_CRAFTING.md

#pragma once
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "PFItemCatalog.generated.h"
class UTexture2D;

/** Static description of one item type. Runtime quantities live in FPFItemStack. */
USTRUCT(BlueprintType)
struct FPFItemDefinition
{
    GENERATED_BODY()

    /** Stable identifier used everywhere (e.g. "Item_Wood"). Never rename once saved data exists. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FName Id;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FText DisplayName;
    /** Item.Category.Resource / Food / Tool / Weapon / Protection. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FGameplayTag Category;
    /** Maximum quantity in one inventory slot (1..1000). */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="1",ClampMax="1000")) int32 StackLimit = 20;
    /** Kilograms per unit; counts against UPFInventoryComponent::WeightLimit. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0.001")) float Weight = 0.5f;
    /** Soft reference so the catalog doesn't force-load every icon. Placeholder white square for now. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TSoftObjectPtr<UTexture2D> Icon;
    /** 0 = never spoils; otherwise seconds from acquisition until the batch expires (max 86400). */
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float ShelfLifeSeconds = 0;
    /** Food/water restored when eaten (0..100). Both 0 means the item isn't edible. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float FoodRecovery = 0;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float WaterRecovery = 0;
    /** Performance selected from the server's owned bag; weapons cannot grant gathering. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0",ClampMax="8")) int32 GatheringHits = 0;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0",ClampMax="100")) float MeleeDamage = 0;
    /** Best carried Protection item mitigates creature-caused hits only, never needs/hazards. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0",ClampMax="0.5")) float CreatureHitReduction = 0;
};

UCLASS(BlueprintType)
class PRIMALFRONTIER_API UPFItemCatalog : public UDataAsset
{
    GENERATED_BODY()

public:
    /** Fills Items with the greybox defaults (see PFItemCatalog.cpp). */
    UPFItemCatalog();

    UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<FPFItemDefinition> Items;

    /** Validated lookup. Returns nullptr for unknown, duplicated or invalid definitions. */
    const FPFItemDefinition* Find(FName Id) const;
};
