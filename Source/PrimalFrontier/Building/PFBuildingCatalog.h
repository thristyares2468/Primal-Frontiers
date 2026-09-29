#pragma once
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "PFBuildingCatalog.generated.h"

UENUM(BlueprintType)
enum class EPFBuildKind : uint8 { Foundation, Wall, Floor, Ceiling, Door, Storage };
USTRUCT(BlueprintType)
struct FPFBuildingDefinition
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere,BlueprintReadOnly) FName Id;
    UPROPERTY(EditAnywhere,BlueprintReadOnly) FText Name;
    UPROPERTY(EditAnywhere,BlueprintReadOnly) EPFBuildKind Kind=EPFBuildKind::Foundation;
    UPROPERTY(EditAnywhere,BlueprintReadOnly) FGameplayTag Category;
    UPROPERTY(EditAnywhere,BlueprintReadOnly) int32 WoodCost=2;
    UPROPERTY(EditAnywhere,BlueprintReadOnly) float MaxHealth=100;
};
UCLASS(BlueprintType)
class PRIMALFRONTIER_API UPFBuildingCatalog : public UDataAsset
{
    GENERATED_BODY()
public:
    UPFBuildingCatalog();
    UPROPERTY(EditAnywhere,BlueprintReadOnly) TArray<FPFBuildingDefinition> Pieces;
    const FPFBuildingDefinition* Find(FName Id) const;
    static FVector Size(EPFBuildKind Kind);
};
