#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Inventory/PFInventoryComponent.h"
#include "Inventory/PFItemCatalog.h"
#include "Building/PFBuildingComponent.h"
#include "PFRequestCodes.h"

bool APFSurvivalPlayerController::CanUseHotbar() const
{
    const auto* S=GetPawn()?GetPawn()->FindComponentByClass<UPFPlayerSurvivalComponent>():nullptr;
    return IsLocalController() && S && !S->IsDead() && !bPauseMenuOpen && !bInventoryOpen && !bCraftingOpen && !bStatMenuOpen && !bStorageMenuOpen && !bBuildingMenuOpen && (!Building || !Building->bBuildMode);
}
bool APFSurvivalPlayerController::PinInventoryToHotbar(int32 Slot)
{
    if(!IsLocalController() || !bInventoryOpen || bStatMenuOpen || bPauseMenuOpen){return false;}
    const auto* I=GetInventory();const int32 Index=GetSelectedInventoryIndex();
    if(!I || !I->GetStacks().IsValidIndex(Index) || !Hotbar.Assign(Slot,I->GetStacks()[Index].StackId,I,UPFInventoryComponent::ServerTime(GetWorld()))){return false;}
    ClientInventoryFeedback_Implementation(FString::Printf(TEXT("Pinned this batch to hotbar %d (session shortcut)."),Slot+1));return true;
}
void APFSurvivalPlayerController::ClearSelectedHotbar()
{
    if(IsLocalController() && bInventoryOpen && !bStatMenuOpen && !bPauseMenuOpen && Hotbar.Clear(Hotbar.Selected()))
    {ClientInventoryFeedback_Implementation(TEXT("Cleared hotbar shortcut; inventory unchanged."));}
}
bool APFSurvivalPlayerController::SelectHotbar(int32 Slot)
{
    if(!CanUseHotbar() || !Hotbar.Select(Slot)){return false;}
    CancelSurveyView();
    const auto* I=GetInventory();const auto* Stack=Hotbar.Resolve(Slot,I,UPFInventoryComponent::ServerTime(GetWorld()));
    if(Stack)
    {
        const auto* D=I->Definition(Stack->ItemId);const bool Edible=D->FoodRecovery>0 || D->WaterRecovery>0;
        ClientInventoryFeedback_Implementation(FString::Printf(TEXT("Hotbar %d: %s x%d. %s"),Slot+1,*D->DisplayName.ToString(),Stack->Quantity,
            D->OpticalMagnification>0?TEXT("Q / D-down: toggle survey scope."):Edible?TEXT("Q / D-down: use one."):TEXT("Carried tool/protection benefits remain automatic; E gathers, mouse attacks.")));
    }
    else{ClientInventoryFeedback_Implementation(TEXT("Empty or missing hotbar batch. Tab, select a stack, then 1–8 to assign."));}
    return true;
}
void APFSurvivalPlayerController::UseHotbar()
{
    if(!CanUseHotbar()){return;}
    const auto* I=GetInventory();const auto* Stack=Hotbar.Resolve(Hotbar.Selected(),I,UPFInventoryComponent::ServerTime(GetWorld()));
    if(!Stack){ClientInventoryFeedback_Implementation(TEXT("Hotbar batch expired, removed or empty; no replacement used."));return;}
    const auto* D=I->Definition(Stack->ItemId);
    if(D->OpticalMagnification>0){ToggleSurveyView();return;}
    if(D->FoodRecovery<=0 && D->WaterRecovery<=0){ClientInventoryFeedback_Implementation(TEXT("Not consumable. Carried tool/protection benefits are automatic."));return;}
    ServerInventoryAction(Stack->StackId,PFInventoryAction::Consume,1);
}
void APFSurvivalPlayerController::RouteHotbar(int32 Slot)
{
    if(bCraftingOpen && !bPauseMenuOpen){if(Slot==0){CraftTool();}else if(Slot==1){CookFood();}else if(Slot==2){DryFood();}return;}
    if(bInventoryOpen){PinInventoryToHotbar(Slot);return;}
    SelectHotbar(Slot);
}
bool APFSurvivalPlayerController::HandleHotbarKey(FKey Key)
{
    const FKey Keys[]={EKeys::One,EKeys::Two,EKeys::Three,EKeys::Four,EKeys::Five,EKeys::Six,EKeys::Seven,EKeys::Eight};
    for(int32 N=0;N<FPFHotbar::SlotCount;++N){if(Key==Keys[N]){RouteHotbar(N);return true;}}
    if(Key==EKeys::Zero){ClearSelectedHotbar();return true;}return false;
}
void APFSurvivalPlayerController::CycleHotbar(int32 Direction)
{if(CanUseHotbar()){const int32 Next=Hotbar.Selected()==INDEX_NONE?(Direction>0?0:FPFHotbar::SlotCount-1):(Hotbar.Selected()+Direction+FPFHotbar::SlotCount)%FPFHotbar::SlotCount;SelectHotbar(Next);}}
