#include "Inventory/PFInventoryPlayerState.h"
#include "Inventory/PFInventoryComponent.h"
#include "Crafting/PFCraftingComponent.h"
APFInventoryPlayerState::APFInventoryPlayerState()
{ Inventory=CreateDefaultSubobject<UPFInventoryComponent>(TEXT("Inventory"));Crafting=CreateDefaultSubobject<UPFCraftingComponent>(TEXT("Crafting")); }
