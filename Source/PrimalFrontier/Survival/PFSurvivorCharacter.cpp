#include "Survival/PFSurvivorCharacter.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Survival/PFSurvivalGameMode.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"

APFSurvivorCharacter::APFSurvivorCharacter()
{
    bReplicates = true;
    Survival = CreateDefaultSubobject<UPFPlayerSurvivalComponent>(TEXT("Survival"));
}
void APFSurvivorCharacter::BeginPlay()
{
    Super::BeginPlay();
    Survival->OnDeath.AddDynamic(this, &APFSurvivorCharacter::HandleDeath);
    if (Survival->IsDead()) { HandleDeath(); }
}
float APFSurvivorCharacter::TakeDamage(float Amount, const FDamageEvent& Event, AController* InstigatorController, AActor* Causer)
{
    if (!HasAuthority() || !FMath::IsFinite(Amount) || Amount <= 0.f || Survival->IsDead() || !CanBeDamaged()) { return 0.f; }
    const float Before = Survival->GetVitals().Health;
    // Keep Unreal's damage-type and event pipeline; attributes remain authoritative.
    const float Accepted = Super::TakeDamage(Amount, Event, InstigatorController, Causer);
    Survival->ApplyDamage(Accepted);
    return Before - Survival->GetVitals().Health;
}
bool APFSurvivorCharacter::CanJumpInternal_Implementation() const
{
    return Survival->CanSpendStamina(JumpStaminaCost) && Super::CanJumpInternal_Implementation();
}
void APFSurvivorCharacter::OnJumped_Implementation()
{
    Super::OnJumped_Implementation();
    if (HasAuthority()) { Survival->SpendStamina(JumpStaminaCost); }
}
void APFSurvivorCharacter::DoMove(float Right, float Forward)
{
    if (!Survival->IsDead()) { Super::DoMove(Right, Forward); }
}
void APFSurvivorCharacter::HandleDeath()
{
    StopJumping();
    GetCharacterMovement()->StopMovementImmediately();
    GetCharacterMovement()->DisableMovement();
    SetActorEnableCollision(false);
    if (HasAuthority())
    {
        if (APFSurvivalGameMode* Mode = GetWorld()->GetAuthGameMode<APFSurvivalGameMode>()) { Mode->ScheduleRespawn(GetController()); }
    }
}
