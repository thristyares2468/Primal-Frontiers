// PFPlayerSurvivalComponent.h
//
// Server-authoritative survival vitals for one survivor pawn: Health, Stamina,
// food reserve (Hunger), water reserve (Thirst) and environmental Exposure.
//
// Only the server changes these values; clients receive one replicated snapshot
// (FPFPlayerVitals) and react through OnVitalsChanged/OnDeath. Every mutator
// returns false (and changes nothing) when called without authority, on a dead
// survivor, or with non-finite/out-of-range input.
//
// History: M1 (d32fbaf, f7ed11d) added Health/Stamina and death.
//          M2 (8be2a14) added Hunger/Thirst drain, threshold damage and Exposure.
// Docs:    Docs/SURVIVAL_M1.md, Docs/SURVIVAL_M2.md

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "PFPlayerSurvivalComponent.generated.h"

/** One replicated snapshot of every survival attribute. Replicating it as a single
 *  struct keeps clients from ever seeing a half-updated set of values. */
USTRUCT(BlueprintType)
struct FPFPlayerVitals
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly) float Health = 100.f;
    UPROPERTY(BlueprintReadOnly) float Stamina = 100.f;
    UPROPERTY(BlueprintReadOnly) float MaxHealth = 100.f;
    UPROPERTY(BlueprintReadOnly) float MaxStamina = 100.f;

    // Higher values mean better fed/hydrated; zero causes damage.
    UPROPERTY(BlueprintReadOnly) float Hunger = 100.f;   // food reserve, 0..100
    UPROPERTY(BlueprintReadOnly) float Thirst = 100.f;   // water reserve, 0..100
    UPROPERTY(BlueprintReadOnly) float Exposure = 0.f;   // hazard intensity, 0..1
};

/** Fired on server and clients whenever the vitals snapshot changes (or when a survivor dies). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPFSurvivalEvent);

/** Shared log category for all Primal Frontier gameplay systems (survival, inventory,
 *  crafting, building, creatures, world). Lines are prefixed with [PrimalXxx] tags. */
DECLARE_LOG_CATEGORY_EXTERN(LogPFSurvival, Log, All);

/** Survival vitals component. Lives on APFSurvivorCharacter; a new one is created
 *  with every respawned pawn, so vitals reset on respawn. */
UCLASS(ClassGroup=(PrimalFrontier), BlueprintType, meta=(BlueprintSpawnableComponent))
class PRIMALFRONTIER_API UPFPlayerSurvivalComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPFPlayerSurvivalComponent();

    // ---- Read-only queries (safe on server and client) ----

    /** Current replicated snapshot of all attributes. */
    UFUNCTION(BlueprintPure, Category="Survival") FPFPlayerVitals GetVitals() const { return Vitals; }

    /** Death is derived from Health; there is no separate dead flag to desync. */
    UFUNCTION(BlueprintPure, Category="Survival") bool IsDead() const { return Vitals.Health <= 0.f; }

    /** State.Survival.Alive or State.Survival.Dead. */
    UFUNCTION(BlueprintPure, Category="Survival") FGameplayTag GetLifeState() const;

    /** True for the Attribute.Survival.* tags this component simulates. */
    UFUNCTION(BlueprintPure, Category="Survival") bool SupportsStat(FGameplayTag Stat) const;

    // ---- Server-only needs mutators (M2) ----

    /** Set the food reserve, clamped to 0..100. */
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Survival") bool SetHunger(float Value);

    /** Set the water reserve, clamped to 0..100. */
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Survival") bool SetThirst(float Value);

    /** Developer/test exposure override in 0..1. It is remembered as a floor for the
     *  hazard-volume scan in TickComponent; 0 restores volume-only exposure. */
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Survival") bool SetExposure(float Value);

    /** Add food/water (each 0..100, not both zero). Used by eating and rations. */
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Survival") bool RecoverNeeds(float Food, float Water);

    // Authoritative simulation entry point shared by tick and deterministic tests.
    /** Drain food/water for Seconds and apply starvation/dehydration/exposure damage
     *  (and optional fed regeneration). Damage counts only the part of the interval
     *  actually spent below each threshold, so one long frame isn't over-charged. */
    bool AdvanceNeeds(float Seconds);

    // ---- Server-only health/stamina mutators (M1) ----

    /** Subtract positive damage from Health. Characters route through TakeDamage first. */
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Survival") bool ApplyDamage(float Amount);

    /** Set Health (clamped). Cannot revive a dead survivor; respawn creates a new pawn instead. */
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Survival") bool SetHealth(float Value);

    /** Add or subtract stamina (clamped). Any decrease restarts the recovery delay. */
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Survival") bool ChangeStamina(float Delta);

    /** Spend stamina only if enough is available (all-or-nothing). */
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Survival") bool SpendStamina(float Amount);

    /** True if the survivor is alive and has at least Amount stamina. Usable on clients for prediction checks. */
    UFUNCTION(BlueprintPure, Category="Survival") bool CanSpendStamina(float Amount) const;

    // ---- Events ----

    UPROPERTY(BlueprintAssignable, Category="Survival") FPFSurvivalEvent OnVitalsChanged;
    UPROPERTY(BlueprintAssignable, Category="Survival") FPFSurvivalEvent OnDeath;

    // ---- Designer tuning (defaults are greybox test values, not final balance) ----

    UPROPERTY(EditDefaultsOnly, Category="Survival", meta=(ClampMin="1")) float InitialMaxHealth = 100.f;
    UPROPERTY(EditDefaultsOnly, Category="Survival", meta=(ClampMin="1")) float InitialMaxStamina = 100.f;
    UPROPERTY(EditDefaultsOnly, Category="Survival", meta=(ClampMin="0")) float StaminaRecoveryPerSecond = 10.f;
    /** Seconds after spending stamina before recovery begins. */
    UPROPERTY(EditDefaultsOnly, Category="Survival", meta=(ClampMin="0")) float StaminaRecoveryDelay = 1.5f;
    UPROPERTY(EditDefaultsOnly, Category="Survival|Needs", meta=(ClampMin="0", ClampMax="1000")) float HungerDrainPerSecond = 0.2f;
    UPROPERTY(EditDefaultsOnly, Category="Survival|Needs", meta=(ClampMin="0", ClampMax="1000")) float ThirstDrainPerSecond = 0.3f;
    /** Health lost per second while the food reserve is empty. */
    UPROPERTY(EditDefaultsOnly, Category="Survival|Needs", meta=(ClampMin="0", ClampMax="1000")) float StarvationDamagePerSecond = 2.f;
    /** Health lost per second while the water reserve is empty. */
    UPROPERTY(EditDefaultsOnly, Category="Survival|Needs", meta=(ClampMin="0", ClampMax="1000")) float DehydrationDamagePerSecond = 3.f;
    /** Health lost per second at full (1.0) exposure; scales linearly with intensity. */
    UPROPERTY(EditDefaultsOnly, Category="Survival|Needs", meta=(ClampMin="0", ClampMax="1000")) float ExposureDamagePerSecond = 5.f;
    // Opt-in health regeneration; default off preserves the M1 damage contract.
    /** When > 0, heals while both reserves are above 25 and exposure is zero. */
    UPROPERTY(EditDefaultsOnly, Category="Survival|Needs", meta=(ClampMin="0", ClampMax="1000")) float FedHealthRecoveryPerSecond = 0.f;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
    /** Server: initialise vitals from the Initial* tuning. Clients never tick. */
    virtual void BeginPlay() override;

    /** Server, 10 Hz: sample hazard volumes, advance needs, recover stamina. */
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    UPROPERTY(ReplicatedUsing=OnRep_Vitals) FPFPlayerVitals Vitals;

    /** Client: re-broadcast events (and detect death) from the previous snapshot. */
    UFUNCTION() void OnRep_Vitals(FPFPlayerVitals Previous);

    /** Push a changed snapshot to clients and fire OnVitalsChanged / OnDeath. */
    void Publish(const FPFPlayerVitals& Previous);

    /** Authority and alive: the precondition for every mutation. */
    bool CanMutate() const;

    /** World time at which stamina recovery may resume. */
    double RecoveryStartsAt = 0;

    /** Developer exposure floor set by SetExposure (see PF.SetExposure). */
    float TestExposure = 0.f;
};
