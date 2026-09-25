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
    APFSurvivalHazard();
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UBoxComponent> Bounds;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Survival", meta=(ClampMin="0", ClampMax="1")) float Intensity = 1.f;
};
