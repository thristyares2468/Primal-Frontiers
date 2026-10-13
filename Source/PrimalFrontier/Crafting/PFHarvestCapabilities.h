#pragma once
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "PFHarvestCapabilities.generated.h"

/** Material grade is access capability, independent of speed and weapon damage. */
UENUM(BlueprintType)
enum class EPFHarvestGrade : uint8
{
    None, Wood, Stone, Copper, Iron, Diamond, Titanium, Deepforge, Tech
};

/** Empty/None is the explicit legacy unrestricted node default. */
USTRUCT(BlueprintType)
struct FPFHarvestRequirement
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere,BlueprintReadOnly) FGameplayTag Family;
    UPROPERTY(EditAnywhere,BlueprintReadOnly) EPFHarvestGrade MinimumGrade=EPFHarvestGrade::None;
};

namespace PFHarvest
{
    PRIMALFRONTIER_API FGameplayTag Pick();
    PRIMALFRONTIER_API FGameplayTag Axe();
    PRIMALFRONTIER_API FGameplayTag Sickle();
    PRIMALFRONTIER_API bool KnownFamily(FGameplayTag Family);
    PRIMALFRONTIER_API bool ValidRequirement(const FPFHarvestRequirement& Requirement);
    PRIMALFRONTIER_API bool ValidTool(const FGameplayTagContainer& Families,EPFHarvestGrade Grade,int32 Hits);
    PRIMALFRONTIER_API FString RequirementText(const FPFHarvestRequirement& Requirement);
}
