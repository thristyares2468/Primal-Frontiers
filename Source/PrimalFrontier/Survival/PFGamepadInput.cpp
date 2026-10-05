#include "Survival/PFSurvivalPlayerController.h"
#include "Building/PFBuildingComponent.h"
#include "Components/InputComponent.h"
#include "InputCoreTypes.h"

// Movement, look and A/jump use the existing Enhanced Input template context.
// These buttons share the keyboard paths and their server-validated RPCs.
void APFSurvivalPlayerController::BindGamepadControls()
{
    InputComponent->BindKey(EKeys::Gamepad_Special_Right,IE_Pressed,this,&APFSurvivalPlayerController::TogglePauseMenu).bExecuteWhenPaused=true;
    InputComponent->BindKey(EKeys::Gamepad_Special_Left,IE_Pressed,this,&APFSurvivalPlayerController::ToggleInventory);
    InputComponent->BindKey(EKeys::Gamepad_FaceButton_Top,IE_Pressed,this,&APFSurvivalPlayerController::ToggleCrafting);
    InputComponent->BindKey(EKeys::Gamepad_RightShoulder,IE_Pressed,this,&APFSurvivalPlayerController::ToggleBuilding);
    InputComponent->BindKey(EKeys::Gamepad_FaceButton_Left,IE_Pressed,this,&APFSurvivalPlayerController::GamepadPrimary);
    InputComponent->BindKey(EKeys::Gamepad_FaceButton_Right,IE_Pressed,this,&APFSurvivalPlayerController::GamepadBack);
    InputComponent->BindKey(EKeys::Gamepad_DPad_Up,IE_Pressed,this,&APFSurvivalPlayerController::GamepadUp);
    InputComponent->BindKey(EKeys::Gamepad_DPad_Down,IE_Pressed,this,&APFSurvivalPlayerController::GamepadDown);
    InputComponent->BindKey(EKeys::Gamepad_DPad_Left,IE_Pressed,this,&APFSurvivalPlayerController::GamepadLeft);
    InputComponent->BindKey(EKeys::Gamepad_DPad_Right,IE_Pressed,this,&APFSurvivalPlayerController::GamepadRight);
    InputComponent->BindKey(EKeys::Gamepad_RightTrigger,IE_Pressed,this,&APFSurvivalPlayerController::PlaceBuilding);
    InputComponent->BindKey(EKeys::Gamepad_LeftShoulder,IE_Pressed,this,&APFSurvivalPlayerController::DemolishBuilding);
}
void APFSurvivalPlayerController::GamepadPrimary()
{
    if(bPauseMenuOpen){return;}
    if(bInventoryOpen){InventoryConsume();}
    else if(bCraftingOpen){CraftTool();}
    else{Interact();}
}
void APFSurvivalPlayerController::GamepadBack()
{
    if(bPauseMenuOpen){SetPauseMenuOpen(false);return;}
    bInventoryOpen=false;bCraftingOpen=false;Building->bBuildMode=false;
}
void APFSurvivalPlayerController::GamepadUp()
{if(bPauseMenuOpen){return;}if(bInventoryOpen){InventoryPrevious();}else if(bCraftingOpen){CookFood();}else if(Building->bBuildMode){NextBuilding();}}
void APFSurvivalPlayerController::GamepadDown()
{if(bPauseMenuOpen){return;}if(bInventoryOpen){InventoryNext();}else if(bCraftingOpen){DryFood();}else if(Building->bBuildMode){RotateBuilding();}}
void APFSurvivalPlayerController::GamepadLeft()
{if(bPauseMenuOpen){return;}if(bInventoryOpen){InventorySplit();}else if(bCraftingOpen){CancelCraft();}else if(Building->bBuildMode){StoreItem();}}
void APFSurvivalPlayerController::GamepadRight()
{if(bPauseMenuOpen){return;}if(bInventoryOpen){InventoryDrop();}else if(Building->bBuildMode){TakeStoredItem();}}
