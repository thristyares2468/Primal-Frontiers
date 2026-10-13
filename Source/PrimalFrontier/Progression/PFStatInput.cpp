#include "Survival/PFSurvivalPlayerController.h"
#include "Progression/PFProgressionComponent.h"
#include "Progression/PFPlayerStatsMenu.h"
#include "Inventory/PFInventoryPlayerState.h"
#include "Building/PFBuildingComponent.h"
#include "GameFramework/PlayerInput.h"
#include "Survival/PFPlayerSurvivalComponent.h"

void APFSurvivalPlayerController::ServerUpgradeStat_Implementation(EPFPlayerStat Stat)
{auto* PS=GetPlayerState<APFInventoryPlayerState>();FString Error;if(PS && PS->Progression){PS->Progression->RequestStat(Stat,GetPawn(),Error);}}
void APFSurvivalPlayerController::ToggleStatMenu(){if(!bPauseMenuOpen){SetStatMenuOpen(!bStatMenuOpen);}}
void APFSurvivalPlayerController::SetStatMenuOpen(bool bOpen)
{
    if(!IsLocalController() || bOpen==bStatMenuOpen || (bOpen && bPauseMenuOpen)){return;}
    if(bOpen)
    {
        ClearTraversalInput();SetCraftingMenuOpen(false);SetBuildingMenuOpen(false);SetStorageMenuOpen(false);
        if(Building->OpenStorage){Building->ServerCloseStorage();}Building->bBuildMode=false;bInventoryOpen=false;
        if(GetLocalPlayer() && !StatMenu){StatMenu=CreateWidget<UPFPlayerStatsMenu>(this);if(!StatMenu || !StatMenu->AddToPlayerScreen(60)){return;}}
    }
    bStatMenuOpen=bOpen;
    if(!StatMenu || !GetLocalPlayer() || !StatMenu->IsInViewport()){return;} // Headless state without input freeze.
    StatMenu->SetVisibility(bOpen?ESlateVisibility::Visible:ESlateVisibility::Collapsed);
    SetIgnoreMoveInput(bOpen);SetIgnoreLookInput(bOpen);bShowMouseCursor=bOpen;if(PlayerInput){PlayerInput->FlushPressedKeys();}
    if(bOpen)
    {StatMenu->RefreshMenu();FInputModeUIOnly Mode;Mode.SetWidgetToFocus(StatMenu->TakeWidget());Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);SetInputMode(Mode);StatMenu->SetKeyboardFocus();}
    else{FInputModeGameOnly Mode;Mode.SetConsumeCaptureMouseDown(false);SetInputMode(Mode);}
}
