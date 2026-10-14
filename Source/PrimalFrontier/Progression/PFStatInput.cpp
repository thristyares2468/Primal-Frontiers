#include "Survival/PFSurvivalPlayerController.h"
#include "Progression/PFProgressionComponent.h"
#include "Progression/PFPlayerStatsMenu.h"
#include "Inventory/PFInventoryPlayerState.h"
#include "Building/PFBuildingComponent.h"
#include "GameFramework/PlayerInput.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Inventory/PFInventoryHUD.h"

void APFSurvivalPlayerController::ServerUpgradeStat_Implementation(EPFPlayerStat Stat)
{auto* PS=GetPlayerState<APFInventoryPlayerState>();FString Error;if(PS && PS->Progression){PS->Progression->RequestStat(Stat,GetPawn(),Error);}}
void APFSurvivalPlayerController::ToggleStatMenu(){if(!bPauseMenuOpen){SetStatMenuOpen(!bStatMenuOpen);}}
void APFSurvivalPlayerController::SetStatMenuOpen(bool bOpen)
{
    if(!IsLocalController() || bOpen==bStatMenuOpen || (bOpen && bPauseMenuOpen)){return;}
    if(bOpen)
    {
        SetInventoryOpen(true);if(!bInventoryOpen){return;}
    }
    bStatMenuOpen=bOpen;
    if(InventoryHUD && GetLocalPlayer() && InventoryHUD->IsInViewport()){InventoryHUD->SetPage(bOpen);}
}
