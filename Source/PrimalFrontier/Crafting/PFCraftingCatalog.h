#pragma once
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "PFCraftingCatalog.generated.h"
class UPFItemCatalog;

USTRUCT(BlueprintType)
struct FPFCraftIngredient
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FName ItemId;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 Quantity=1;
};
USTRUCT(BlueprintType)
struct FPFRecipeDefinition
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FName Id;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FText DisplayName;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FGameplayTag Category;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<FPFCraftIngredient> Ingredients;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FName Output;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 OutputQuantity=1;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float Duration=5;
};
USTRUCT(BlueprintType)
struct FPFResourceDefinition
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FName Id;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FText DisplayName;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FName YieldItem;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 YieldPerHit=2;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 Hits=3;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float RespawnSeconds=20;
};
UCLASS(BlueprintType)
class PRIMALFRONTIER_API UPFCraftingCatalog : public UDataAsset
{
    GENERATED_BODY()
public:
    UPFCraftingCatalog();
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<FPFRecipeDefinition> Recipes;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<FPFResourceDefinition> Resources;
    const FPFRecipeDefinition* Recipe(FName Id, const UPFItemCatalog* Items) const;
    const FPFResourceDefinition* Resource(FName Id, const UPFItemCatalog* Items) const;
};
