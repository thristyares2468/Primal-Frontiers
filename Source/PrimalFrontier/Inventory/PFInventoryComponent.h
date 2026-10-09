// PFInventoryComponent.h
//
// Server-authoritative bag of item stacks with slot and weight limits. Used for:
//  - each player's bag (on APFInventoryPlayerState, so it survives respawn), and
//  - storage boxes (on APFBuildPiece of kind Storage).
//
// Core guarantees:
//  - Atomic: every operation either fully succeeds or changes nothing (it works on
//    a proposed copy, or checks everything before committing).
//  - Conserving: items are never duplicated or lost by split/drop/pickup/transfer.
//  - Freshness-safe: perishable stacks carry an absolute server-time deadline; only
//    stacks with the SAME deadline merge, so moving food can never make it fresher.
//  - Private: stack contents replicate only to the owning client.
// Every mutator returns false without authority.
//
// History: M3 (340c538) core; M4 (e7ffd71) Transform for crafting;
//          M5 (5702d4b) TransferTo for storage.
// Docs:    Docs/INVENTORY_M3.md, Docs/FOOD_AND_PRESERVATION.md

#pragma once
#include "Components/ActorComponent.h"
#include "PFInventoryComponent.generated.h"
class UPFItemCatalog;
class APawn;
class APFItemPickup;
struct FPFItemDefinition;
struct FPFSavedItemStack;

/** One inventory slot: a quantity of one item from one acquisition batch. */
USTRUCT(BlueprintType)
struct FPFItemStack
{
    GENERATED_BODY()

    /** Stable per-stack ID. Clients refer to stacks only by this GUID. */
    UPROPERTY(BlueprintReadOnly) FGuid StackId;
    UPROPERTY(BlueprintReadOnly) FName ItemId;
    UPROPERTY(BlueprintReadOnly) int32 Quantity=0;
    // Zero means nonperishable; otherwise an absolute server simulation-time deadline.
    UPROPERTY(BlueprintReadOnly) double ExpiresAt=0;
};

UCLASS(ClassGroup=(PrimalFrontier),meta=(BlueprintSpawnableComponent))
class PRIMALFRONTIER_API UPFInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPFInventoryComponent();

    /** Trusted server persistence only; validate all batches before replacing the bag. */
    bool PreparePersistence(const TArray<FPFSavedItemStack>& Saved, double AgeSeconds,
        TArray<FPFItemStack>& OutStacks, FString& Error) const;
    bool RestorePersistence(const TArray<FPFSavedItemStack>& Saved, double AgeSeconds, FString& Error);

    /** Item definitions. Loaded from DA_ItemCatalog at BeginPlay if not assigned (tests assign their own). */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Inventory") TObjectPtr<UPFItemCatalog> Catalog;
    /** Maximum number of stacks (valid range 1..64). Bags default to 8, storage sets 8. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Inventory") int32 SlotLimit=8;
    /** Maximum total weight in kg. Bags 30, storage 60. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Inventory") float WeightLimit=30;

    // ---- Queries ----
    const TArray<FPFItemStack>& GetStacks() const { return Stacks; }
    /** Validated definition for Id from Catalog, or nullptr. */
    const FPFItemDefinition* Definition(FName Id) const;
    /** Synchronized server world time used for all freshness deadlines. */
    static double ServerTime(const UWorld* World);
    float GetWeight() const;
    /** Total quantity of Id across all stacks. */
    int32 Count(FName Id) const;
    /** Best valid still-fresh owned tool benefit; defaults to bare hands (1 hit/20 damage). */
    int32 GatheringHits() const;
    float MeleeDamage() const;
    /** Stable ID of the best valid carried melee benefit; None means bare hands. */
    FName MeleeItem() const;
    /** Nonstacking best valid fresh carried Protection benefit; queried by server damage. */
    float CreatureHitReduction() const;

    // ---- Server-only mutations (all atomic) ----

    /** Create Quantity new items of Id with a fresh deadline from the catalog. */
    bool Grant(FName Id,int32 Quantity);
    /** Insert items that already exist elsewhere (pickup, transfer) keeping their exact Deadline.
     *  Fills matching same-deadline stacks first, then new slots; all-or-nothing. */
    bool AddExisting(FName Id,int32 Quantity,double Deadline);
    /** Remove Quantity from one specific stack. */
    bool Remove(FGuid Id,int32 Quantity);
    /** Remove Quantity of an item type across stacks (newest slots first). Used for building costs. */
    bool RemoveItem(FName Id,int32 Quantity);
    /** Move Quantity from a stack into a new slot (same item and deadline). */
    bool Split(FGuid Id,int32 Quantity);
    /** Move Quantity of a stack into Destination. Destination must accept first; only then is the source reduced. */
    bool TransferTo(UPFInventoryComponent* Destination,FGuid Id,int32 Quantity);
    // Server-only atomic conversion of exact still-fresh input batches into recipe output.
    /** Inputs must match current stacks exactly (ID, item, deadline, enough quantity). */
    bool Transform(const TArray<FPFItemStack>& Inputs,FName Output,int32 Quantity);
    /** Eat one item from a stack; Pawn must be this inventory's own living survivor. */
    bool Consume(FGuid Id,APawn* Pawn);
    /** Spawn a world pickup 1.2 m in front of Pawn's eyes, then remove the items.
     *  Refused if the drop point is obstructed. Returns the pickup or nullptr. */
    APFItemPickup* Drop(FGuid Id,int32 Quantity,APawn* Pawn);
    /** Delete every stack whose deadline has passed. Called before each mutation and once per second. */
    void PruneExpired();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
    /** Load the default catalog if needed; only the server ticks (expiry pruning). */
    virtual void BeginPlay() override;
    virtual void TickComponent(float Delta,ELevelTick Type,FActorComponentTickFunction* Tick) override;

private:
    /** Owner-only replication: other players never see this inventory's contents. */
    UPROPERTY(Replicated) TArray<FPFItemStack> Stacks;
    bool Authority() const;
    /** Pawn is alive, server-owned, possessed and belongs to this inventory's PlayerState. */
    bool OwnsLivingPawn(APawn* Pawn) const;
    /** Force a net update and log the new slot/weight totals. */
    void Changed(const TCHAR* Action);
};
