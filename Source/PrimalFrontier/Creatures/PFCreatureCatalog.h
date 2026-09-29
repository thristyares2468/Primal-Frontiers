#pragma once
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "PFCreatureCatalog.generated.h"

USTRUCT(BlueprintType)
struct FPFCreatureDefinition
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FName Id;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FText Name;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) bool bHostile = false;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float Health = 60;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float Speed = 170;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float SightRange = 700;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float Damage = 8;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 FoodLoot = 2;
};

UCLASS(BlueprintType)
class PRIMALFRONTIER_API UPFCreatureCatalog : public UDataAsset
{
    GENERATED_BODY()
public:
    UPFCreatureCatalog();
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<FPFCreatureDefinition> Creatures;
    const FPFCreatureDefinition* Find(FName Id) const;
};
