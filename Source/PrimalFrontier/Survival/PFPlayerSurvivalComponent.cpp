#include "Survival/PFPlayerSurvivalComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "NativeGameplayTags.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY(LogPFSurvival);
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PF_Alive, "State.Survival.Alive");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PF_Dead, "State.Survival.Dead");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PF_Health, "Attribute.Survival.Health");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PF_Stamina, "Attribute.Survival.Stamina");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PF_Hunger, "Attribute.Survival.Hunger");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PF_Thirst, "Attribute.Survival.Thirst");

UPFPlayerSurvivalComponent::UPFPlayerSurvivalComponent()
{
    SetIsReplicatedByDefault(true);
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
}
void UPFPlayerSurvivalComponent::BeginPlay()
{
    Super::BeginPlay();
    SetComponentTickEnabled(GetOwner()->HasAuthority());
    if (GetOwner()->HasAuthority())
    {
        const FPFPlayerVitals Previous = Vitals;
        Vitals.MaxHealth = FMath::IsFinite(InitialMaxHealth) ? FMath::Max(1.f, InitialMaxHealth) : 100.f;
        Vitals.MaxStamina = FMath::IsFinite(InitialMaxStamina) ? FMath::Max(1.f, InitialMaxStamina) : 100.f;
        Vitals.Health = Vitals.MaxHealth;
        Vitals.Stamina = Vitals.MaxStamina;
        Publish(Previous);
    }
}
void UPFPlayerSurvivalComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UPFPlayerSurvivalComponent, Vitals);
}
bool UPFPlayerSurvivalComponent::CanMutate() const
{
    return GetOwner() && GetOwner()->HasAuthority() && !IsDead();
}
bool UPFPlayerSurvivalComponent::ApplyDamage(float Amount)
{
    if (!CanMutate() || !FMath::IsFinite(Amount) || Amount <= 0.f) { return false; }
    return SetHealth(Vitals.Health - FMath::Min(Amount, Vitals.Health));
}
bool UPFPlayerSurvivalComponent::SetHealth(float Value)
{
    if (!CanMutate() || !FMath::IsFinite(Value)) { return false; }
    const FPFPlayerVitals Previous = Vitals;
    Vitals.Health = FMath::Clamp(Value, 0.f, Vitals.MaxHealth);
    if (IsDead()) { Vitals.Stamina = 0.f; }
    Publish(Previous);
    UE_LOG(LogPFSurvival, Display, TEXT("[PrimalSurvival] %s Health=%.1f Stamina=%.1f State=%s (authority)"),
        *GetNameSafe(GetOwner()), Vitals.Health, Vitals.Stamina, *GetLifeState().ToString());
    return true;
}
bool UPFPlayerSurvivalComponent::ChangeStamina(float Delta)
{
    if (!CanMutate() || !FMath::IsFinite(Delta)) { return false; }
    const FPFPlayerVitals Previous = Vitals;
    // Bound the delta before addition so even a finite FLT_MAX cannot overflow.
    Vitals.Stamina = FMath::Clamp(Vitals.Stamina + FMath::Clamp(Delta, -Vitals.MaxStamina, Vitals.MaxStamina), 0.f, Vitals.MaxStamina);
    if (Delta < 0.f)
    {
        const float Delay = FMath::IsFinite(StaminaRecoveryDelay) ? FMath::Max(0.f, StaminaRecoveryDelay) : 1.5f;
        RecoveryStartsAt = GetWorld()->GetTimeSeconds() + Delay;
    }
    Publish(Previous);
    return true;
}
bool UPFPlayerSurvivalComponent::CanSpendStamina(float Amount) const
{
    return !IsDead() && FMath::IsFinite(Amount) && Amount > 0.f && Vitals.Stamina >= Amount;
}
bool UPFPlayerSurvivalComponent::SpendStamina(float Amount)
{
    return CanMutate() && CanSpendStamina(Amount) && ChangeStamina(-Amount);
}
void UPFPlayerSurvivalComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (CanMutate() && Vitals.Stamina < Vitals.MaxStamina && GetWorld()->GetTimeSeconds() >= RecoveryStartsAt &&
        FMath::IsFinite(StaminaRecoveryPerSecond) && StaminaRecoveryPerSecond > 0.f)
    {
        ChangeStamina(StaminaRecoveryPerSecond * DeltaTime);
    }
}
FGameplayTag UPFPlayerSurvivalComponent::GetLifeState() const { return IsDead() ? TAG_PF_Dead : TAG_PF_Alive; }
bool UPFPlayerSurvivalComponent::SupportsStat(FGameplayTag Stat) const { return Stat == TAG_PF_Health || Stat == TAG_PF_Stamina; }
void UPFPlayerSurvivalComponent::OnRep_Vitals(FPFPlayerVitals Previous) { Publish(Previous); }
void UPFPlayerSurvivalComponent::Publish(const FPFPlayerVitals& Previous)
{
    if (GetOwner()->HasAuthority()) { GetOwner()->ForceNetUpdate(); }
    OnVitalsChanged.Broadcast();
    if (Previous.Health > 0.f && IsDead()) { OnDeath.Broadcast(); }
}
