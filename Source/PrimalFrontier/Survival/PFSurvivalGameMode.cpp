#include "Survival/PFSurvivalGameMode.h"
#include "Survival/PFSurvivorCharacter.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Inventory/PFInventoryPlayerState.h"
#include "GameFramework/PlayerStart.h"
#include "TimerManager.h"
#include "Engine/World.h"

APFSurvivalGameMode::APFSurvivalGameMode()
{
    DefaultPawnClass = APFSurvivorCharacter::StaticClass();
    PlayerControllerClass = APFSurvivalPlayerController::StaticClass();
    PlayerStateClass = APFInventoryPlayerState::StaticClass();
}
void APFSurvivalGameMode::RestartPlayer(AController* Controller)
{
    if (!Controller || !HasAuthority()) { return; }
    APlayerStart* Start = Cast<APlayerStart>(FindPlayerStart(Controller));
    if (!Start)
    {
        UE_LOG(LogPFSurvival, Error, TEXT("[PrimalSurvival] Respawn refused: no valid PlayerStart."));
        return;
    }
    RestartPlayerAtPlayerStart(Controller, Start);
}
void APFSurvivalGameMode::ScheduleRespawn(AController* Controller)
{
    if (!Controller || !HasAuthority()) { return; }
    const TWeakObjectPtr<AController> WeakController(Controller);
    const TWeakObjectPtr<APawn> DeadPawn(Controller->GetPawn());
    FTimerHandle Timer;
    const float Delay = FMath::IsFinite(RespawnDelay) ? FMath::Max(0.1f, RespawnDelay) : 3.f;
    GetWorldTimerManager().SetTimer(Timer, FTimerDelegate::CreateWeakLambda(this, [this, WeakController, DeadPawn]
    {
        if (WeakController.IsValid() && DeadPawn.IsValid() && WeakController->GetPawn() == DeadPawn.Get()) { RespawnPlayer(WeakController.Get()); }
    }), Delay, false);
}
bool APFSurvivalGameMode::RespawnPlayer(AController* Controller)
{
    APFSurvivorCharacter* Dead = Controller ? Cast<APFSurvivorCharacter>(Controller->GetPawn()) : nullptr;
    if (!HasAuthority() || !Dead || !Dead->Survival->IsDead()) { return false; }
    Controller->UnPossess();
    RestartPlayer(Controller);
    APFSurvivorCharacter* Replacement = Cast<APFSurvivorCharacter>(Controller->GetPawn());
    if (!Replacement)
    {
        Controller->Possess(Dead);
        UE_LOG(LogPFSurvival, Error, TEXT("[PrimalSurvival] Respawn failed; dead pawn retained for retry."));
        return false;
    }
    Dead->Destroy();
    UE_LOG(LogPFSurvival, Display, TEXT("[PrimalSurvival] Respawned %s at %s Health=%.0f Stamina=%.0f"),
        *Replacement->GetName(), *Replacement->GetActorLocation().ToString(), Replacement->Survival->GetVitals().Health, Replacement->Survival->GetVitals().Stamina);
    return true;
}
