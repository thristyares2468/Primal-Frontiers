// PFInventoryPlayerState.h
//
// PlayerState that carries the player's bag (UPFInventoryComponent) and crafting
// queue (UPFCraftingComponent). PlayerState outlives the pawn, so items and the
// player identity survive death/respawn, while vitals reset with the new pawn.
// Reconnect currently starts empty; persistence is planned for M8.
//
// History: M3 (340c538) inventory; M4 (e7ffd71) crafting. Docs: Docs/INVENTORY_M3.md

#pragma once
#include "GameFramework/PlayerState.h"
#include "PFInventoryPlayerState.generated.h"
class UPFInventoryComponent;
class UPFCraftingComponent;
UCLASS()
class PRIMALFRONTIER_API APFInventoryPlayerState : public APlayerState
{
    GENERATED_BODY()

public:
    APFInventoryPlayerState();

    /** Server-issued public ownership key. A reconnect credential is a separate secret. */
    UPROPERTY(Replicated, BlueprintReadOnly) FGuid PersistentPlayerId;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
protected:
    virtual void BeginPlay() override;
public:

    /** The player's bag (8 slots / 30 kg by default). Contents replicate to the owner only. */
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly) TObjectPtr<UPFInventoryComponent> Inventory;

    /** One timed crafting job at a time, converting items in Inventory on completion. */
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly) TObjectPtr<UPFCraftingComponent> Crafting;
};
