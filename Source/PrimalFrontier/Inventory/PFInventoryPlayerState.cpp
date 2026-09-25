#include "Inventory/PFInventoryPlayerState.h"
#include "Inventory/PFInventoryComponent.h"
APFInventoryPlayerState::APFInventoryPlayerState()
{ Inventory=CreateDefaultSubobject<UPFInventoryComponent>(TEXT("Inventory")); }
