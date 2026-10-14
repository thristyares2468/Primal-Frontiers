#include "Survival/PFSurvivalPlayerController.h"
#include "Inventory/PFInventoryHUD.h"
#include "Building/PFBuildingComponent.h"
#include "GameFramework/PlayerInput.h"
#include "Inventory/PFInventoryComponent.h"

bool APFSurvivalPlayerController::SelectInventoryStack(FGuid Id)
{
    const auto* I=GetInventory();
    if(!bInventoryOpen || bStatMenuOpen || bPauseMenuOpen || !I || !Id.IsValid()){return false;}
    const double Now=UPFInventoryComponent::ServerTime(GetWorld());
    if(!I->GetStacks().ContainsByPredicate([&](const auto& S){return S.StackId==Id && (S.ExpiresAt<=0 || S.ExpiresAt>Now);})){return false;}
    SelectedInventoryStack=Id;bInventorySelectionInitialized=true;return true;
}

void APFSurvivalPlayerController::ToggleInventory(){if(!bPauseMenuOpen){SetInventoryOpen(!bInventoryOpen);}}
void APFSurvivalPlayerController::SetInventoryOpen(bool Open)
{
    if(!IsLocalController() || Open==bInventoryOpen || (Open && bPauseMenuOpen)){return;}
    if(Open)
    {
        if(GetLocalPlayer() && (!InventoryHUD || !InventoryHUD->IsInViewport())){return;}
        ClearTraversalInput();SetCraftingMenuOpen(false);SetBuildingMenuOpen(false);SetStorageMenuOpen(false);
        if(Building->OpenStorage){Building->ServerCloseStorage();}Building->bBuildMode=false;InitializeInventorySelection();
    }
    bInventoryOpen=Open;bStatMenuOpen=false;
    if(!InventoryHUD || !GetLocalPlayer() || !InventoryHUD->IsInViewport()){return;}
    InventoryHUD->SetVisibility(Open?ESlateVisibility::Visible:ESlateVisibility::Collapsed);InventoryHUD->SetPage(false);
    if(Open)
    {
        SetIgnoreMoveInput(true);SetIgnoreLookInput(true);bInventoryInputBlocked=true;bShowMouseCursor=true;
        if(PlayerInput){PlayerInput->FlushPressedKeys();}
        FInputModeUIOnly Mode;Mode.SetWidgetToFocus(InventoryHUD->TakeWidget());Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);SetInputMode(Mode);InventoryHUD->SetKeyboardFocus();
    }
    else
    {
        if(bInventoryInputBlocked){SetIgnoreMoveInput(false);SetIgnoreLookInput(false);bInventoryInputBlocked=false;}
        bShowMouseCursor=false;if(PlayerInput){PlayerInput->FlushPressedKeys();}
        FInputModeGameOnly Mode;Mode.SetConsumeCaptureMouseDown(false);SetInputMode(Mode);
    }
    UE_LOG(LogTemp,Display,TEXT("[PrimalAgentTools] Inventory open=%d; upgrades embedded; world continues running"),Open);
}
bool APFSurvivalPlayerController::HandleInventoryKey(FKey K,bool Repeat)
{
    if(!bInventoryOpen){return false;}if(Repeat){return true;}
    if(K==EKeys::Tab || K==EKeys::Gamepad_Special_Left){SetInventoryOpen(false);return true;}
    if(K==EKeys::Escape || K==EKeys::Gamepad_FaceButton_Right){if(bStatMenuOpen){SetStatMenuOpen(false);}else{SetInventoryOpen(false);}return true;}
    if(K==EKeys::U){SetStatMenuOpen(!bStatMenuOpen);return true;}
    if(K==EKeys::Gamepad_RightShoulder){SetStatMenuOpen(true);return true;}
    if(K==EKeys::Gamepad_LeftShoulder){SetStatMenuOpen(false);return true;}
    if(K==EKeys::P || K==EKeys::Gamepad_Special_Right){SetInventoryOpen(false);SetPauseMenuOpen(true);return true;}
    if(K==EKeys::C || K==EKeys::Gamepad_FaceButton_Top){SetInventoryOpen(false);SetCraftingMenuOpen(true);return true;}
    if(K==EKeys::B){SetInventoryOpen(false);ToggleBuilding();return true;}
    if(bStatMenuOpen){return false;}
    if(HandleHotbarKey(K)){return true;}
    if(K==EKeys::Up || K==EKeys::Gamepad_DPad_Up){InventoryPrevious();return true;}
    if(K==EKeys::Down || K==EKeys::Gamepad_DPad_Down){InventoryNext();return true;}
    if(K==EKeys::X || K==EKeys::Gamepad_DPad_Left){InventorySplit();return true;}
    if(K==EKeys::G || K==EKeys::Gamepad_DPad_Right){InventoryDrop();return true;}
    if(K==EKeys::Q || K==EKeys::Gamepad_FaceButton_Left){InventoryConsume();return true;}
    return false;
}
