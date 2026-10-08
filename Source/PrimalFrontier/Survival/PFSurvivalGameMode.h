// PFSurvivalGameMode.h
//
// Server-only GameMode for every Primal Frontier survival map. Chooses the
// survivor pawn, survival controller and inventory PlayerState, and owns the
// death -> delay -> respawn-at-PlayerStart cycle.
//
// Inventory and crafting live on APFInventoryPlayerState, which outlives the pawn,
// so items survive respawn while vitals reset with the new pawn.
//
// History: M1 (f7ed11d) respawn; M3 (340c538) inventory PlayerState.
// Docs:    Docs/SURVIVAL_M1.md

#pragma once
#include "GameFramework/GameModeBase.h"
#include "PFSurvivalGameMode.generated.h"

UCLASS()
class PRIMALFRONTIER_API APFSurvivalGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    APFSurvivalGameMode();

    virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
    virtual void StartPlay() override;
    virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
    virtual FString InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId,
        const FString& Options, const FString& Portal = TEXT("")) override;
    virtual void PostLogin(APlayerController* NewPlayer) override;

    /** Seconds between death and automatic respawn (minimum 0.1). */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Survival", meta=(ClampMin="0.1")) float RespawnDelay = 3.f;

    /** Called by a dying survivor. Respawns after RespawnDelay only if the controller
     *  still possesses that same dead pawn (avoids double respawns). */
    void ScheduleRespawn(AController* Controller);

    /** Immediately replace a dead survivor with a fresh one. If spawning fails the dead
     *  pawn is re-possessed so a later retry is possible. Living pawns are refused. */
    bool RespawnPlayer(AController* Controller);

    /** Requires a real APlayerStart; refuses (and logs) instead of spawning at the origin. */
    virtual void RestartPlayer(AController* NewPlayer) override;
};
