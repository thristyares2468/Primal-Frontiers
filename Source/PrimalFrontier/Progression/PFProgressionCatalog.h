#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "PFProgressionCatalog.generated.h"

class UPFCraftingCatalog;
class UPFItemCatalog;

/** Optional learned recipes. Baseline survival recipes may never be assigned here. */
USTRUCT(BlueprintType)
struct FPFKnowledgeDefinition
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FName Id;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FText DisplayName;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FGameplayTag Category;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="2",ClampMax="10")) int32 MinimumLevel=2;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="1",ClampMax="27")) int32 PointCost=2;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<FName> Prerequisites;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<FName> Recipes;
};

UCLASS(BlueprintType)
class PRIMALFRONTIER_API UPFProgressionCatalog : public UDataAsset
{
    GENERATED_BODY()
public:
    UPFProgressionCatalog();
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<FPFKnowledgeDefinition> Knowledge;
    static constexpr int32 MaximumKnowledge=32;
    static constexpr int32 MaximumCraftRecords=128;
    bool Validate(const UPFCraftingCatalog& Crafting,const UPFItemCatalog& Items,FString& Error) const;
    const FPFKnowledgeDefinition* Find(FName Id) const;
};
