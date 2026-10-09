// PFInventoryPlayerState.cpp — see PFInventoryPlayerState.h.

#include "Inventory/PFInventoryPlayerState.h"
#include "Inventory/PFInventoryComponent.h"
#include "Crafting/PFCraftingComponent.h"
#include "Progression/PFProgressionComponent.h"
#include "Net/UnrealNetwork.h"

APFInventoryPlayerState::APFInventoryPlayerState()
{
    Inventory=CreateDefaultSubobject<UPFInventoryComponent>(TEXT("Inventory"));
    Crafting=CreateDefaultSubobject<UPFCraftingComponent>(TEXT("Crafting"));
    Progression=CreateDefaultSubobject<UPFProgressionComponent>(TEXT("Progression"));
}

void APFInventoryPlayerState::BeginPlay()
{
    Super::BeginPlay();
    if (HasAuthority() && !PersistentPlayerId.IsValid()) { PersistentPlayerId = FGuid::NewGuid(); }
}

void APFInventoryPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(APFInventoryPlayerState, PersistentPlayerId);
}
