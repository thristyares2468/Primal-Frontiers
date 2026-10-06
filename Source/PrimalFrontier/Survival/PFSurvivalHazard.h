// PFSurvivalHazard.h
//
// Static exposure zone placed in maps (e.g. the M7 arena's northern danger area).
// It does no work itself: each survivor's UPFPlayerSurvivalComponent samples the
// hazards it overlaps on the server and takes the strongest Intensity as its
// Exposure, which causes damage over time and blocks stamina recovery.
// This is a generic danger interface, not a temperature/weather simulation.
//
// History: M2 (8be2a14). Docs: Docs/SURVIVAL_M2.md

#pragma once
#include "GameFramework/Actor.h"
#include "PFSurvivalHazard.generated.h"
class UBoxComponent;

// Static server-owned exposure region. Overlapping regions use the strongest intensity.
UCLASS(Blueprintable)
class PRIMALFRONTIER_API APFSurvivalHazard : public AActor
{
    GENERATED_BODY()

public:
    /** Creates the overlap box (pawns only), a floor marker and a "DANGER" label. */
    APFSurvivalHazard();

    /** Overlap volume; resize it in the level to shape the zone. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UBoxComponent> Bounds;

    /** Exposure applied while inside, 0..1 (1 = full ExposureDamagePerSecond). */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Survival", meta=(ClampMin="0", ClampMax="1")) float Intensity = 1.f;
};
