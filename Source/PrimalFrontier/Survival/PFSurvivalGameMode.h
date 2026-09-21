#pragma once
#include "GameFramework/GameModeBase.h"
#include "PFSurvivalGameMode.generated.h"

UCLASS()
class PRIMALFRONTIER_API APFSurvivalGameMode : public AGameModeBase
{
    GENERATED_BODY()
public:
    APFSurvivalGameMode();
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Survival", meta=(ClampMin="0.1")) float RespawnDelay = 3.f;
    void ScheduleRespawn(AController* Controller);
    bool RespawnPlayer(AController* Controller);
    virtual void RestartPlayer(AController* NewPlayer) override;
};
