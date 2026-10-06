// PFItemPickup.h
//
// A stack of items lying in the world (placed in a map, dropped by a player, or
// left as creature loot). Pressing E picks it up through the server.
//
// Two ways to create one:
//  - Placed in a level: set ItemId/Quantity in the Details panel; the server
//    builds the contents (with a fresh deadline) at BeginPlay.
//  - Spawned by code: SpawnActorDeferred + Initialize(...) + FinishSpawning, which
//    keeps the exact freshness deadline of the items being dropped.
// Invalid contents (unknown item, too many, already expired) destroy the actor.
//
// History: M3 (340c538). Docs: Docs/INVENTORY_M3.md

#pragma once
#include "GameFramework/Actor.h"
#include "Inventory/PFInventoryComponent.h"
#include "PFItemPickup.generated.h"
class UTextRenderComponent;
class UPFItemCatalog;
UCLASS(Blueprintable)
class PRIMALFRONTIER_API APFItemPickup : public AActor
{
    GENERATED_BODY()

public:
    APFItemPickup();

    /** Level-placed contents (ignored when spawned through Initialize). */
    UPROPERTY(EditAnywhere,BlueprintReadOnly) FName ItemId=TEXT("Item_Wood");
    UPROPERTY(EditAnywhere,BlueprintReadOnly) int32 Quantity=5;

    /** Server, before FinishSpawning only: set contents with an existing deadline. */
    void Initialize(FName Id,int32 Count,double Deadline);

    /** Server: move the whole stack into Pawn's bag if alive, within 2.5 m, in clear view
     *  and the bag has room. On failure the pickup stays in the world. */
    bool TryPickup(APawn* Pawn);

    const FPFItemStack& GetContents() const {return Contents;}
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
    /** Server: build/validate contents; destroy invalid pickups. */
    virtual void BeginPlay() override;
    /** 2 Hz: server removes spoiled food; clients refresh the floating label. */
    virtual void Tick(float Delta) override;

private:
    /** Replicated to everyone (world items are public, unlike inventories). */
    UPROPERTY(Replicated) FPFItemStack Contents;
    UPROPERTY() TObjectPtr<UTextRenderComponent> Label;
    /** Loaded on server and clients: validation on the server, label display name everywhere. */
    UPROPERTY(Transient) TObjectPtr<UPFItemCatalog> Catalog;
    bool bInitialized=false;   // true when spawned via Initialize()
    bool bTaken=false;         // guards against double pickup before Destroy completes
};
