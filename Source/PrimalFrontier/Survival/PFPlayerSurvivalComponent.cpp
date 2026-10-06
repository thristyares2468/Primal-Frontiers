// PFPlayerSurvivalComponent.cpp
//
// Implementation of the authoritative survival vitals. See the header for the
// overall contract. Key rules enforced here:
//  - Every mutation goes through CanMutate() (server + alive) and rejects NaN/Inf.
//  - Values are clamped to their legal ranges; a dead survivor can't be revived.
//  - Publish() is the single place that replicates changes and fires events.

#include "Survival/PFPlayerSurvivalComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "NativeGameplayTags.h"
#include "Net/UnrealNetwork.h"
#include "Survival/PFSurvivalHazard.h"

DEFINE_LOG_CATEGORY(LogPFSurvival);

// Life-state and attribute tags. Survivor death uses State.Survival.Dead; creatures
// use a separate Creature.State.Dead tag (see PFCreature.cpp).
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PF_Alive, "State.Survival.Alive");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PF_Dead, "State.Survival.Dead");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PF_Health, "Attribute.Survival.Health");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PF_Stamina, "Attribute.Survival.Stamina");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PF_Hunger, "Attribute.Survival.Hunger");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PF_Thirst, "Attribute.Survival.Thirst");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PF_Exposure, "Attribute.Survival.Exposure");

UPFPlayerSurvivalComponent::UPFPlayerSurvivalComponent()
{
    SetIsReplicatedByDefault(true);
    // 10 Hz is plenty for needs drain and keeps server cost low with many players.
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
}

void UPFPlayerSurvivalComponent::BeginPlay()
{
    Super::BeginPlay();
    // Only the server simulates; clients just receive replicated snapshots.
    SetComponentTickEnabled(GetOwner()->HasAuthority());
    if (GetOwner()->HasAuthority())
    {
        // Fresh pawn => full vitals. Invalid designer values fall back to 100.
        const FPFPlayerVitals Previous = Vitals;
        Vitals.MaxHealth = FMath::IsFinite(InitialMaxHealth) ? FMath::Max(1.f, InitialMaxHealth) : 100.f;
        Vitals.MaxStamina = FMath::IsFinite(InitialMaxStamina) ? FMath::Max(1.f, InitialMaxStamina) : 100.f;
        Vitals.Health = Vitals.MaxHealth;
        Vitals.Stamina = Vitals.MaxStamina;
        Vitals.Hunger = 100.f;
        Vitals.Thirst = 100.f;
        Vitals.Exposure = 0.f;
        Publish(Previous);
    }
}

void UPFPlayerSurvivalComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    // Replicated to everyone: remote players' health is visible game state.
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
    // A dead survivor has no stamina; this also blocks jumping/attacks immediately.
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
        // Spending stamina postpones recovery by StaminaRecoveryDelay seconds.
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

    // 1) Exposure: strongest overlapping hazard wins; the developer override acts as a floor.
    if (CanMutate())
    {
        TArray<AActor*> Regions;
        GetOwner()->GetOverlappingActors(Regions, APFSurvivalHazard::StaticClass());
        float Exposure = TestExposure;
        for (const AActor* Actor : Regions)
        {
            const float Intensity = CastChecked<APFSurvivalHazard>(Actor)->Intensity;
            if (FMath::IsFinite(Intensity)) { Exposure = FMath::Max(Exposure,FMath::Clamp(Intensity,0.f,1.f)); }
        }
        if (Exposure != Vitals.Exposure)
        {
            const auto Previous = Vitals;
            Vitals.Exposure = Exposure;
            Publish(Previous);
            UE_LOG(LogPFSurvival,Display,TEXT("[PrimalSurvival] %s exposure changed to %.2f (authority)"),*GetOwner()->GetName(),Exposure);
        }
    }

    // 2) Needs drain and threshold damage (no-op when dead or not authority).
    AdvanceNeeds(DeltaTime);

    // 3) Stamina recovery: only after the delay, with food and water left and no exposure.
    if (CanMutate() && Vitals.Stamina < Vitals.MaxStamina && GetWorld()->GetTimeSeconds() >= RecoveryStartsAt &&
        Vitals.Hunger > 0.f && Vitals.Thirst > 0.f && Vitals.Exposure == 0.f &&
        FMath::IsFinite(StaminaRecoveryPerSecond) && StaminaRecoveryPerSecond > 0.f)
    {
        ChangeStamina(StaminaRecoveryPerSecond * DeltaTime);
    }
}

FGameplayTag UPFPlayerSurvivalComponent::GetLifeState() const { return IsDead() ? TAG_PF_Dead : TAG_PF_Alive; }

bool UPFPlayerSurvivalComponent::SupportsStat(FGameplayTag Stat) const
{
    return Stat == TAG_PF_Health || Stat == TAG_PF_Stamina || Stat == TAG_PF_Hunger || Stat == TAG_PF_Thirst || Stat == TAG_PF_Exposure;
}

bool UPFPlayerSurvivalComponent::SetHunger(float Value)
{
    if (!CanMutate() || !FMath::IsFinite(Value)) { return false; }
    const FPFPlayerVitals Previous = Vitals;
    Vitals.Hunger = FMath::Clamp(Value, 0.f, 100.f);
    Publish(Previous);
    return true;
}

bool UPFPlayerSurvivalComponent::SetThirst(float Value)
{
    if (!CanMutate() || !FMath::IsFinite(Value)) { return false; }
    const FPFPlayerVitals Previous = Vitals;
    Vitals.Thirst = FMath::Clamp(Value, 0.f, 100.f);
    Publish(Previous);
    return true;
}

bool UPFPlayerSurvivalComponent::SetExposure(float Value)
{
    if (!CanMutate() || !FMath::IsFinite(Value) || Value < 0.f || Value > 1.f) { return false; }
    const FPFPlayerVitals Previous = Vitals;
    Vitals.Exposure = Value;
    TestExposure = Value; // persists as a floor for the per-tick hazard scan
    Publish(Previous);
    return true;
}

bool UPFPlayerSurvivalComponent::RecoverNeeds(float Food, float Water)
{
    // Bounded, positive recovery only: there is no "free food" path for clients.
    if (!CanMutate() || !FMath::IsFinite(Food) || !FMath::IsFinite(Water) || Food < 0.f || Water < 0.f ||
        Food > 100.f || Water > 100.f || (Food == 0.f && Water == 0.f)) { return false; }
    const FPFPlayerVitals Previous = Vitals;
    Vitals.Hunger = FMath::Min(100.f, Vitals.Hunger + Food);
    Vitals.Thirst = FMath::Min(100.f, Vitals.Thirst + Water);
    Publish(Previous);
    return true;
}

bool UPFPlayerSurvivalComponent::AdvanceNeeds(float Seconds)
{
    if (!CanMutate() || !FMath::IsFinite(Seconds) || Seconds <= 0.f) { return false; }

    // Sanitise designer rates: non-finite becomes 0, everything clamped to 0..1000.
    const auto Rate = [](float Value) -> double { return FMath::IsFinite(Value) ? FMath::Clamp(Value, 0.f, 1000.f) : 0.f; };
    const FPFPlayerVitals Previous = Vitals;
    const double FoodRate = Rate(HungerDrainPerSecond), WaterRate = Rate(ThirstDrainPerSecond);

    // How many of the elapsed Seconds the value stays above Threshold while draining at Drain/s.
    const auto TimeAbove = [Seconds](float Value, float Threshold, double Drain)
    {
        return Value <= Threshold ? 0.0 : (Drain > 0 ? FMath::Min(double(Seconds), (Value - Threshold) / Drain) : double(Seconds));
    };
    const double StarvingTime = Seconds - TimeAbove(Vitals.Hunger, 0.f, FoodRate);
    const double DehydratedTime = Seconds - TimeAbove(Vitals.Thirst, 0.f, WaterRate);
    // Optional regeneration only while both reserves are above 25 and there is no exposure.
    const double FedTime = Vitals.Exposure == 0.f ? FMath::Min(TimeAbove(Vitals.Hunger, 25.f, FoodRate), TimeAbove(Vitals.Thirst, 25.f, WaterRate)) : 0;

    Vitals.Hunger = float(FMath::Max(0.0, Vitals.Hunger - FoodRate * Seconds));
    Vitals.Thirst = float(FMath::Max(0.0, Vitals.Thirst - WaterRate * Seconds));

    // Integrate the fraction of time actually below each threshold, including long frames.
    const double Damage = StarvingTime * Rate(StarvationDamagePerSecond) + DehydratedTime * Rate(DehydrationDamagePerSecond)
        + Vitals.Exposure * Rate(ExposureDamagePerSecond) * Seconds;
    // Heal first (capped at max), then subtract damage, then clamp to the legal range.
    Vitals.Health = float(FMath::Clamp(FMath::Min(double(Vitals.MaxHealth), Vitals.Health + FedTime * Rate(FedHealthRecoveryPerSecond)) - Damage, 0.0, double(Vitals.MaxHealth)));
    if (IsDead()) { Vitals.Stamina = 0.f; }

    // Avoid network traffic when nothing observable changed (e.g. drain rates of 0).
    if (Vitals.Health != Previous.Health || Vitals.Hunger != Previous.Hunger || Vitals.Thirst != Previous.Thirst) { Publish(Previous); }
    return true;
}

void UPFPlayerSurvivalComponent::OnRep_Vitals(FPFPlayerVitals Previous) { Publish(Previous); }

void UPFPlayerSurvivalComponent::Publish(const FPFPlayerVitals& Previous)
{
    // Server: send the new snapshot promptly instead of waiting for the next net update.
    if (GetOwner()->HasAuthority()) { GetOwner()->ForceNetUpdate(); }
    OnVitalsChanged.Broadcast();
    // Death fires exactly once, on the transition from alive to dead.
    if (Previous.Health > 0.f && IsDead()) { OnDeath.Broadcast(); }
}
