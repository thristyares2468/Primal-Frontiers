#pragma once
#include "CoreMinimal.h"
class UPFInventoryComponent;
struct FPFItemStack;

/** Local shortcuts, not additional inventory, equipment or replicated item authority.
 *  Exact batch identities never silently bind a replacement after expiry/removal. */
class PRIMALFRONTIER_API FPFHotbar
{
public:
    static constexpr int32 SlotCount=8;
    bool Assign(int32 Slot,FGuid Id,const UPFInventoryComponent* Inventory,double Now);
    bool Select(int32 Slot);
    bool Clear(int32 Slot);
    FGuid Identity(int32 Slot) const {return Slot>=0 && Slot<SlotCount?Ids[Slot]:FGuid();}
    const FPFItemStack* Resolve(int32 Slot,const UPFInventoryComponent* Inventory,double Now) const;
    int32 Selected() const {return SelectedSlot;}
private:
    FGuid Ids[SlotCount];
    int32 SelectedSlot=INDEX_NONE;
};
