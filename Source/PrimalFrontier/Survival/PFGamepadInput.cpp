// PFGamepadInput.cpp
//
// Gamepad half of APFSurvivalPlayerController (kept in its own file for clarity).
// Buttons call the SAME handlers as the keyboard, so every action still goes
// through the server-validated RPCs. Face buttons and the D-pad are contextual:
// they act on whichever overlay (inventory, crafting, building) is open; the
// overlays are mutually exclusive so each press has exactly one meaning.
//
// Layout (Xbox names):  Menu = pause  | View = bag   | Y = crafting | RB = build
//                       X = primary   | B = back     | D-pad = contextual
//                       RT = place/attack | LB = demolish
//
// History: M7 (b5a3165). Docs: Docs/PLAYTEST.md, Docs/DECISIONS.md (2026-10-04).

#include "Survival/PFSurvivalPlayerController.h"
#include "Building/PFBuildingComponent.h"
#include "Crafting/PFCraftingHUD.h"
#include "Components/InputComponent.h"
#include "InputCoreTypes.h"

// Movement, look and A/jump outside modal menus use the existing template context.
// These buttons share the keyboard paths and their server-validated RPCs.
void APFSurvivalPlayerController::BindGamepadControls()
{
    BindControl(EKeys::Gamepad_Special_Right,&APFSurvivalPlayerController::TogglePauseMenu,TEXT("General"),TEXT("Menu: pause / resume"),true);
    BindControl(EKeys::Gamepad_Special_Left,&APFSurvivalPlayerController::ToggleInventory,TEXT("General"),TEXT("View: open / close inventory"));
    BindControl(EKeys::Gamepad_FaceButton_Top,&APFSurvivalPlayerController::ToggleCrafting,TEXT("General"),TEXT("Y: open / close crafting"));
    BindControl(EKeys::Gamepad_RightShoulder,&APFSurvivalPlayerController::ToggleBuilding,TEXT("General"),TEXT("RB: open / close building"));
    BindControl(EKeys::Gamepad_FaceButton_Left,&APFSurvivalPlayerController::GamepadPrimary,TEXT("Contextual"),TEXT("X: inventory consume; craft selected recipe; otherwise interact"));
    BindControl(EKeys::Gamepad_FaceButton_Right,&APFSurvivalPlayerController::GamepadBack,TEXT("General"),TEXT("B: close overlay / resume"));
    BindControl(EKeys::Gamepad_DPad_Up,&APFSurvivalPlayerController::GamepadUp,TEXT("Contextual"),TEXT("Inventory previous; crafting previous recipe; building next piece"));
    BindControl(EKeys::Gamepad_DPad_Down,&APFSurvivalPlayerController::GamepadDown,TEXT("Contextual"),TEXT("Inventory next; crafting next recipe; building rotate"));
    BindControl(EKeys::Gamepad_DPad_Left,&APFSurvivalPlayerController::GamepadLeft,TEXT("Contextual"),TEXT("Inventory split; crafting cancel; building store one"));
    BindControl(EKeys::Gamepad_DPad_Right,&APFSurvivalPlayerController::GamepadRight,TEXT("Contextual"),TEXT("Inventory drop one; building take one"));
    BindControl(EKeys::Gamepad_RightTrigger,&APFSurvivalPlayerController::PlaceBuilding,TEXT("Building / world"),TEXT("RT: place preview; attack when overlays are closed"));
    BindControl(EKeys::Gamepad_LeftShoulder,&APFSurvivalPlayerController::DemolishBuilding,TEXT("Building"),TEXT("LB: demolish owned targeted piece"));
}

/** X: eat (bag open), craft tool (crafting open), otherwise interact. */
void APFSurvivalPlayerController::GamepadPrimary()
{
    if(bPauseMenuOpen){return;}
    if(bInventoryOpen){InventoryConsume();}
    else if(bCraftingOpen && CraftingHUD){CraftingHUD->CraftSelected();}
    else{Interact();}
}

/** B: close the pause menu, or close every overlay. */
void APFSurvivalPlayerController::GamepadBack()
{
    if(bPauseMenuOpen)
    {
        SetPauseMenuOpen(false);
        return;
    }
    bInventoryOpen=false;
    SetCraftingMenuOpen(false);
    Building->bBuildMode=false;
}

/** D-up: previous stack | cook | next building piece. */
void APFSurvivalPlayerController::GamepadUp()
{
    if(bPauseMenuOpen){return;}
    if(bInventoryOpen){InventoryPrevious();}
    else if(bCraftingOpen && CraftingHUD){CraftingHUD->MoveSelection(-1);}
    else if(Building->bBuildMode){NextBuilding();}
}

/** D-down: next stack | dry | rotate piece. */
void APFSurvivalPlayerController::GamepadDown()
{
    if(bPauseMenuOpen){return;}
    if(bInventoryOpen){InventoryNext();}
    else if(bCraftingOpen && CraftingHUD){CraftingHUD->MoveSelection(1);}
    else if(Building->bBuildMode){RotateBuilding();}
}

/** D-left: split stack | cancel craft | store one item. */
void APFSurvivalPlayerController::GamepadLeft()
{
    if(bPauseMenuOpen){return;}
    if(bInventoryOpen){InventorySplit();}
    else if(bCraftingOpen){CancelCraft();}
    else if(Building->bBuildMode){StoreItem();}
}

/** D-right: drop one | (nothing in crafting) | take one stored item. */
void APFSurvivalPlayerController::GamepadRight()
{
    if(bPauseMenuOpen){return;}
    if(bInventoryOpen){InventoryDrop();}
    else if(Building->bBuildMode){TakeStoredItem();}
}
