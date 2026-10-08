// PFSurvivalGameMode.cpp
//
// See PFSurvivalGameMode.h. Respawn never reuses the dead pawn: a new
// APFSurvivorCharacter (with fresh vitals) is spawned at a PlayerStart.

#include "Survival/PFSurvivalGameMode.h"
#include "Survival/PFSurvivorCharacter.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Inventory/PFInventoryPlayerState.h"
#include "Persistence/PFWorldPersistence.h"
#include "GameFramework/PlayerStart.h"
#include "TimerManager.h"
#include "Engine/World.h"

APFSurvivalGameMode::APFSurvivalGameMode()
{
    // Native defaults; BP_SurvivalGameMode overrides these with the Blueprint presentation classes.
    DefaultPawnClass = APFSurvivorCharacter::StaticClass();
    PlayerControllerClass = APFSurvivalPlayerController::StaticClass();
    PlayerStateClass = APFInventoryPlayerState::StaticClass();
}

void APFSurvivalGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
    Super::InitGame(MapName, Options, ErrorMessage);
    GetWorld()->GetSubsystem<UPFWorldPersistence>()->ConfigureStartup();
}

void APFSurvivalGameMode::StartPlay()
{
    Super::StartPlay(); // Actors/catalogs/components must finish BeginPlay before restoration.
    GetWorld()->GetSubsystem<UPFWorldPersistence>()->ApplyStartup();
}

void APFSurvivalGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
    Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
    if (ErrorMessage.IsEmpty()) { GetWorld()->GetSubsystem<UPFWorldPersistence>()->CheckLogin(Options, ErrorMessage); }
}

FString APFSurvivalGameMode::InitNewPlayer(APlayerController* PC, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal)
{
    FString Error = Super::InitNewPlayer(PC, UniqueId, Options, Portal);
    auto* SurvivorPC = Cast<APFSurvivalPlayerController>(PC);
    auto* Persistence = GetWorld()->GetSubsystem<UPFWorldPersistence>();
    if (Error.IsEmpty() && !Persistence->CheckLogin(Options, Error)) { return Error; }
    if (Error.IsEmpty() && SurvivorPC) { Persistence->Login(SurvivorPC, Options); }
    return Error;
}

void APFSurvivalGameMode::PostLogin(APlayerController* PC)
{
    Super::PostLogin(PC);
    const TWeakObjectPtr<APFSurvivalPlayerController> WeakPC(Cast<APFSurvivalPlayerController>(PC));
    GetWorldTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this, WeakPC]
    {
        if (WeakPC.IsValid() && !GetWorld()->GetSubsystem<UPFWorldPersistence>()->RestorePlayer(WeakPC.Get()))
        {
            WeakPC->ClientReturnToMainMenuWithTextReason(FText::FromString(TEXT("Saved player restoration failed; previous record preserved. Check server log.")));
            WeakPC->Destroy();
        }
    }));
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
    // Weak pointers: the controller may disconnect or the pawn may be replaced before the timer fires.
    const TWeakObjectPtr<AController> WeakController(Controller);
    const TWeakObjectPtr<APawn> DeadPawn(Controller->GetPawn());
    FTimerHandle Timer;
    const float Delay = FMath::IsFinite(RespawnDelay) ? FMath::Max(0.1f, RespawnDelay) : 3.f;
    GetWorldTimerManager().SetTimer(Timer, FTimerDelegate::CreateWeakLambda(this, [this, WeakController, DeadPawn]
    {
        // Only respawn if nothing else (e.g. PF.Respawn) already replaced the dead pawn.
        if (WeakController.IsValid() && DeadPawn.IsValid() && WeakController->GetPawn() == DeadPawn.Get()) { RespawnPlayer(WeakController.Get()); }
    }), Delay, false);
}

bool APFSurvivalGameMode::RespawnPlayer(AController* Controller)
{
    APFSurvivorCharacter* Dead = Controller ? Cast<APFSurvivorCharacter>(Controller->GetPawn()) : nullptr;
    if (!HasAuthority() || !Dead || !Dead->Survival->IsDead()) { return false; }

    // Unpossess first so RestartPlayer spawns a new pawn instead of reusing the old one.
    Controller->UnPossess();
    RestartPlayer(Controller);
    APFSurvivorCharacter* Replacement = Cast<APFSurvivorCharacter>(Controller->GetPawn());
    if (!Replacement)
    {
        // Spawn failed (no PlayerStart, blocked, ...): keep the corpse possessed for a retry.
        Controller->Possess(Dead);
        UE_LOG(LogPFSurvival, Error, TEXT("[PrimalSurvival] Respawn failed; dead pawn retained for retry."));
        return false;
    }
    Dead->Destroy();
    UE_LOG(LogPFSurvival, Display, TEXT("[PrimalSurvival] Respawned %s at %s Health=%.0f Stamina=%.0f"),
        *Replacement->GetName(), *Replacement->GetActorLocation().ToString(), Replacement->Survival->GetVitals().Health, Replacement->Survival->GetVitals().Stamina);
    return true;
}
