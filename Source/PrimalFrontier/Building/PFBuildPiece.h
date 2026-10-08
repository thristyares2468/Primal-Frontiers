// PFBuildPiece.h
//
// One placed structure (foundation, wall, floor, ceiling, door or storage box).
// Spawned only by UPFBuildingComponent::Place on the server and replicated to all.
//
// Rules held here:
//  - Ownership: PersistentOwnerId is the server-assigned ownership key. Builder
//    is the currently bound PlayerState and can be rebound after reconnect.
//    Only the owner may open, damage or demolish (cooperative rule, not PvP).
//  - Support: Support points at the piece this one rests on. A piece that still
//    supports others (or a storage box that isn't empty) can't be removed or
//    destroyed, so nothing is left floating and no items are silently lost.
//  - Shape: built from up to four cube components so a door can be a frame plus
//    a panel that hides when open.
//  - Storage: every piece has an inventory component, but only Storage pieces use it.
//
// History: M5 (5702d4b). Docs: Docs/BUILDING_M5.md

#pragma once
#include "GameFramework/Actor.h"
#include "Building/PFBuildingCatalog.h"
#include "PFBuildPiece.generated.h"
class UStaticMeshComponent;
class UPFInventoryComponent;
class APlayerState;
UCLASS()
class PRIMALFRONTIER_API APFBuildPiece : public AActor
{
    GENERATED_BODY()

public:
    APFBuildPiece();
    UPROPERTY() FGuid PersistentId;
    UPROPERTY(Replicated, BlueprintReadOnly) FGuid PersistentOwnerId;
    bool IsOwnedBy(const APlayerState* Requester) const;
    void BindPersistentOwner(APlayerState* OwnerState);
    void RefreshPersistenceShape() { OnRepShape(); }

    /** Which shape to show; replicated so clients rebuild the shape on change. */
    UPROPERTY(ReplicatedUsing=OnRepShape,BlueprintReadOnly) EPFBuildKind Kind=EPFBuildKind::Foundation;
    /** Catalog ID it was built from (shown on the build HUD). */
    UPROPERTY(Replicated,BlueprintReadOnly) FName DefinitionId;
    /** Current owner's PlayerState; rebind by stable ownership key after reconnect. */
    UPROPERTY(Replicated,BlueprintReadOnly) TObjectPtr<APlayerState> Builder;
    /** Piece this one rests on; null for foundations. */
    UPROPERTY(Replicated,BlueprintReadOnly) TObjectPtr<APFBuildPiece> Support;
    UPROPERTY(Replicated,BlueprintReadOnly) float Health=100;
    /** Door state; replicated so every client hides/shows the panel. */
    UPROPERTY(ReplicatedUsing=OnRepShape,BlueprintReadOnly) bool bDoorOpen=false;
    /** Private storage (8 slots / 60 kg); contents replicate only to the owner. */
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly) TObjectPtr<UPFInventoryComponent> Storage;

    /** Server, before FinishSpawning: copy definition, owner and support. */
    void Initialize(const FPFBuildingDefinition& D,APlayerState* OwnerState,APFBuildPiece* Parent);
    /** Foundations, floors and ceilings can hold walls, doors and storage. */
    bool IsPlatform() const;
    /** True if any other piece names this one as its Support. */
    bool HasDependents() const;
    /** No dependents and (if storage) empty. */
    bool CanRemove() const;
    /** Server: owner-only door toggle. */
    bool ToggleDoor(APlayerState* Requester);
    /** Server: owner-only damage; lethal damage is refused while CanRemove() is false. */
    virtual float TakeDamage(float Amount,const FDamageEvent& Event,AController* Instigator,AActor* Causer) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
    /** Build the shape; non-storage pieces disable their unused inventory tick. */
    virtual void BeginPlay() override;

private:
    /** Four reusable cube parts; OnRepShape decides which are visible/solid. */
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Shapes;
    /** Rebuild the visible/collidable shape from Kind and bDoorOpen. */
    UFUNCTION() void OnRepShape();
};
