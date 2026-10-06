// PFCreatureCatalog.h
//
// Data asset of creature definitions (Content/PrimalFrontier/Creatures/
// DA_CreatureCatalog). Two original greybox creatures exist:
//  - Creature_Forager: passive, flees from survivors.
//  - Creature_Prowler: hostile, chases and attacks.
// Defaults are built in the C++ constructor; Find() fails closed on bad data.
//
// History: M6 (0c2d935). Docs: Docs/CREATURES_M6.md, Docs/CREATURES.md

#pragma once
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "PFCreatureCatalog.generated.h"

USTRUCT(BlueprintType)
struct FPFCreatureDefinition
{
    GENERATED_BODY()
    /** Stable ID, e.g. "Creature_Prowler" (used by spawners and PF.SpawnCreature). */
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FName Id;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FText Name;
    /** Hostile creatures chase and attack; passive ones flee. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly) bool bHostile = false;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float Health = 60;      // 1..1000
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float Speed = 170;      // cm/s, max walk speed (1..500)
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float SightRange = 700; // cm (100..2000)
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float Damage = 8;       // per landed attack (1..100)
    /** Found food dropped on death (1..10), using the item catalog's shelf life. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 FoodLoot = 2;
};

UCLASS(BlueprintType)
class PRIMALFRONTIER_API UPFCreatureCatalog : public UDataAsset
{
    GENERATED_BODY()

public:
    /** Adds the Forager and Prowler defaults. */
    UPFCreatureCatalog();
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<FPFCreatureDefinition> Creatures;
    /** Validated lookup; nullptr for unknown/duplicate/out-of-range definitions. */
    const FPFCreatureDefinition* Find(FName Id) const;
};
