// PFBuildingCatalog.h
//
// Data asset of placeable structure pieces (Content/PrimalFrontier/Building/
// DA_BuildingCatalog). Each piece has a stable ID, a kind (which decides its
// shape and snapping rules), a wood cost and max health. Defaults are created
// in the C++ constructor; Find() fails closed on invalid or duplicate data.
//
// History: M5 (5702d4b). Docs: Docs/BUILDING_M5.md, Docs/BUILDING_SYSTEM.md

#pragma once
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "PFBuildingCatalog.generated.h"

/** Shape/snapping family. Order matters: the constructor maps index N to kind N. */
UENUM(BlueprintType)
enum class EPFBuildKind : uint8 { Foundation, Wall, Floor, Ceiling, Door, Storage };

USTRUCT(BlueprintType)
struct FPFBuildingDefinition
{
    GENERATED_BODY()
    /** Stable ID, e.g. "Build_Foundation". */
    UPROPERTY(EditAnywhere,BlueprintReadOnly) FName Id;
    UPROPERTY(EditAnywhere,BlueprintReadOnly) FText Name;
    UPROPERTY(EditAnywhere,BlueprintReadOnly) EPFBuildKind Kind=EPFBuildKind::Foundation;
    UPROPERTY(EditAnywhere,BlueprintReadOnly) FGameplayTag Category;
    /** Wood consumed on placement (1..20). Not refunded on demolition. */
    UPROPERTY(EditAnywhere,BlueprintReadOnly) int32 WoodCost=2;
    UPROPERTY(EditAnywhere,BlueprintReadOnly) float MaxHealth=100;
};

UCLASS(BlueprintType)
class PRIMALFRONTIER_API UPFBuildingCatalog : public UDataAsset
{
    GENERATED_BODY()

public:
    /** Creates the six greybox pieces (2 wood, 100 health each). */
    UPFBuildingCatalog();
    UPROPERTY(EditAnywhere,BlueprintReadOnly) TArray<FPFBuildingDefinition> Pieces;
    /** Validated lookup; nullptr for unknown/duplicate/invalid pieces. */
    const FPFBuildingDefinition* Find(FName Id) const;
    /** Full size (cm) of a kind's collision box: platforms 400x400x20, walls/doors 20x380x300, storage 70x70x80. */
    static FVector Size(EPFBuildKind Kind);
};
