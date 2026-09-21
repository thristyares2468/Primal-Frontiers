#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFSurvivalHUD.h"

APFSurvivalPlayerController::APFSurvivalPlayerController() { SurvivalHUDClass = UPFSurvivalHUD::StaticClass(); }
void APFSurvivalPlayerController::BeginPlay()
{
    Super::BeginPlay();
    if (IsLocalController() && GetLocalPlayer() && SurvivalHUDClass)
    {
        SurvivalHUD = CreateWidget<UPFSurvivalHUD>(this, SurvivalHUDClass);
        if (SurvivalHUD) { SurvivalHUD->AddToPlayerScreen(); }
    }
}
