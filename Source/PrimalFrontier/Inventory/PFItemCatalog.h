#pragma once
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "PFItemCatalog.generated.h"
class UTexture2D;

USTRUCT(BlueprintType)
struct FPFItemDefinition
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FName Id;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FText DisplayName;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FGameplayTag Category;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="1",ClampMax="1000")) int32 StackLimit = 20;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0.001")) float Weight = 0.5f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TSoftObjectPtr<UTexture2D> Icon;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float ShelfLifeSeconds = 0;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float FoodRecovery = 0;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float WaterRecovery = 0;
};

UCLASS(BlueprintType)
class PRIMALFRONTIER_API UPFItemCatalog : public UDataAsset
{
    GENERATED_BODY()
public:
    UPFItemCatalog();
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<FPFItemDefinition> Items;
    const FPFItemDefinition* Find(FName Id) const;
};
