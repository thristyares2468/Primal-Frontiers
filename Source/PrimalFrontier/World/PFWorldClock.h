// PFWorldClock.h
//
// The map's single day/night clock. The server advances Hour (0..24) so that one
// full day lasts DayLengthSeconds (15 minutes by default) and tags the phase as
// World.Time.Day (06:00-18:00) or World.Time.Night. Hour and Phase replicate to
// every client (always relevant); each client then rotates/dims its own sun light.
// Clients cannot change the time; the server-only PF.SetTimeOfDay command can.
//
// Two movable directional lights keep the greybox arena readable without sky art:
// a Sun that follows the hour and a dim, shadowless NightFill.
//
// History: M7 (b5a3165). Docs: Docs/WORLD_M7.md, Docs/DECISIONS.md (2026-09-30)

#pragma once
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "PFWorldClock.generated.h"
class UDirectionalLightComponent;

UCLASS(Blueprintable)
class PRIMALFRONTIER_API APFWorldClock : public AActor
{
    GENERATED_BODY()

public:
    APFWorldClock();

    /** Real seconds per in-game day (60..86400). */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="World", meta=(ClampMin="60",ClampMax="86400")) float DayLengthSeconds=900;
    /** Current hour, 0 <= Hour < 24. Starts at 09:00. */
    UPROPERTY(ReplicatedUsing=OnRep_Hour, BlueprintReadOnly) float Hour=9;
    /** World.Time.Day or World.Time.Night. */
    UPROPERTY(Replicated, BlueprintReadOnly) FGameplayTag Phase;

    /** Server: jump to NewHour (0 <= NewHour < 24). False on clients or invalid input. */
    bool SetHour(float NewHour);
    /** Server: advance the clock by Seconds of real time, wrapping past midnight. */
    void Advance(float Seconds);
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
    virtual void BeginPlay() override;
    /** 4 Hz: Advance on the server (no-op on clients). */
    virtual void Tick(float DeltaSeconds) override;

private:
    UPROPERTY() TObjectPtr<UDirectionalLightComponent> Sun;
    UPROPERTY() TObjectPtr<UDirectionalLightComponent> NightFill;
    /** Rendering clients: point and dim the sun for the current hour. */
    UFUNCTION() void OnRep_Hour();
    /** Recompute Phase from Hour (logs on change). */
    void UpdatePhase();
};
