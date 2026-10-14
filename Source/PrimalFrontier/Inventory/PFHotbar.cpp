#include "Inventory/PFHotbar.h"
#include "Inventory/PFInventoryComponent.h"

const FPFItemStack* FPFHotbar::Resolve(int32 Slot,const UPFInventoryComponent* Inventory,double Now) const
{
    const FGuid Id=Identity(Slot);
    if(!Inventory || !Id.IsValid() || !FMath::IsFinite(Now)){return nullptr;}
    return Inventory->GetStacks().FindByPredicate([&](const auto& S)
    {return S.StackId==Id && S.Quantity>0 && (S.ExpiresAt<=0 || S.ExpiresAt>Now) && Inventory->Definition(S.ItemId);});
}
bool FPFHotbar::Assign(int32 Slot,FGuid Id,const UPFInventoryComponent* Inventory,double Now)
{
    if(Slot<0 || Slot>=SlotCount || !Id.IsValid() || !Inventory || !FMath::IsFinite(Now)){return false;}
    const auto* Stack=Inventory->GetStacks().FindByPredicate([&](const auto& S)
    {return S.StackId==Id && S.Quantity>0 && (S.ExpiresAt<=0 || S.ExpiresAt>Now) && Inventory->Definition(S.ItemId);});
    if(!Stack){return false;}
    for(auto& Existing:Ids){if(Existing==Id){Existing.Invalidate();}}
    Ids[Slot]=Id;SelectedSlot=Slot;return true;
}
bool FPFHotbar::Select(int32 Slot)
{if(Slot<0 || Slot>=SlotCount){return false;}SelectedSlot=Slot;return true;}
bool FPFHotbar::Clear(int32 Slot)
{if(Slot<0 || Slot>=SlotCount){return false;}Ids[Slot].Invalidate();return true;}
