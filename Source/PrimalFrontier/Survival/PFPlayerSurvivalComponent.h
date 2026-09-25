#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "PFPlayerSurvivalComponent.generated.h"

USTRUCT(BlueprintType)
struct FPFPlayerVitals
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadOnly) float Health = 100.f;
    UPROPERTY(BlueprintReadOnly) float Stamina = 100.f;
    UPROPERTY(BlueprintReadOnly) float MaxHealth = 100.f;
    UPROPERTY(BlueprintReadOnly) float MaxStamina = 100.f;
    // Higher values mean better fed/hydrated; zero causes damage.
    UPROPERTY(BlueprintReadOnly) float Hunger = 100.f;
    UPROPERTY(BlueprintReadOnly) float Thirst = 100.f;
    UPROPERTY(BlueprintReadOnly) float Exposure = 0.f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPFSurvivalEvent);
DECLARE_LOG_CATEGORY_EXTERN(LogPFSurvival, Log, All);

UCLASS(ClassGroup=(PrimalFrontier), BlueprintType, meta=(BlueprintSpawnableComponent))
class PRIMALFRONTIER_API UPFPlayerSurvivalComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UPFPlayerSurvivalComponent();
    UFUNCTION(BlueprintPure, Category="Survival") FPFPlayerVitals GetVitals() const { return Vitals; }
    UFUNCTION(BlueprintPure, Category="Survival") bool IsDead() const { return Vitals.Health <= 0.f; }
    UFUNCTION(BlueprintPure, Category="Survival") FGameplayTag GetLifeState() const;
    UFUNCTION(BlueprintPure, Category="Survival") bool SupportsStat(FGameplayTag Stat) const;
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Survival") bool SetHunger(float Value);
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Survival") bool SetThirst(float Value);
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Survival") bool SetExposure(float Value);
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Survival") bool RecoverNeeds(float Food, float Water);
    // Authoritative simulation entry point shared by tick and deterministic tests.
    bool AdvanceNeeds(float Seconds);
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Survival") bool ApplyDamage(float Amount);
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Survival") bool SetHealth(float Value);
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Survival") bool ChangeStamina(float Delta);
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Survival") bool SpendStamina(float Amount);
    UFUNCTION(BlueprintPure, Category="Survival") bool CanSpendStamina(float Amount) const;

    UPROPERTY(BlueprintAssignable, Category="Survival") FPFSurvivalEvent OnVitalsChanged;
    UPROPERTY(BlueprintAssignable, Category="Survival") FPFSurvivalEvent OnDeath;

    UPROPERTY(EditDefaultsOnly, Category="Survival", meta=(ClampMin="1")) float InitialMaxHealth = 100.f;
    UPROPERTY(EditDefaultsOnly, Category="Survival", meta=(ClampMin="1")) float InitialMaxStamina = 100.f;
    UPROPERTY(EditDefaultsOnly, Category="Survival", meta=(ClampMin="0")) float StaminaRecoveryPerSecond = 10.f;
    UPROPERTY(EditDefaultsOnly, Category="Survival", meta=(ClampMin="0")) float StaminaRecoveryDelay = 1.5f;
    UPROPERTY(EditDefaultsOnly, Category="Survival|Needs", meta=(ClampMin="0", ClampMax="1000")) float HungerDrainPerSecond = 0.2f;
    UPROPERTY(EditDefaultsOnly, Category="Survival|Needs", meta=(ClampMin="0", ClampMax="1000")) float ThirstDrainPerSecond = 0.3f;
    UPROPERTY(EditDefaultsOnly, Category="Survival|Needs", meta=(ClampMin="0", ClampMax="1000")) float StarvationDamagePerSecond = 2.f;
    UPROPERTY(EditDefaultsOnly, Category="Survival|Needs", meta=(ClampMin="0", ClampMax="1000")) float DehydrationDamagePerSecond = 3.f;
    UPROPERTY(EditDefaultsOnly, Category="Survival|Needs", meta=(ClampMin="0", ClampMax="1000")) float ExposureDamagePerSecond = 5.f;
    // Opt-in health regeneration; default off preserves the M1 damage contract.
    UPROPERTY(EditDefaultsOnly, Category="Survival|Needs", meta=(ClampMin="0", ClampMax="1000")) float FedHealthRecoveryPerSecond = 0.f;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
private:
    UPROPERTY(ReplicatedUsing=OnRep_Vitals) FPFPlayerVitals Vitals;
    UFUNCTION() void OnRep_Vitals(FPFPlayerVitals Previous);
    void Publish(const FPFPlayerVitals& Previous);
    bool CanMutate() const;
    double RecoveryStartsAt = 0;
    float TestExposure = 0.f;
};
