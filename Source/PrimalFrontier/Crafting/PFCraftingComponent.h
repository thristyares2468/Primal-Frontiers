// PFCraftingComponent.h
//
// One timed crafting job per player (lives on APFInventoryPlayerState).
//
// How a craft works:
//  1. Start: the server picks the exact ingredient batches (oldest food first)
//     and remembers them, but leaves them IN the bag.
//  2. Wait Duration seconds (server time).
//  3. Complete: UPFInventoryComponent::Transform re-checks those exact batches and
//     converts them to the output in one atomic step. If any input was eaten,
//     dropped, moved or spoiled, or the output won't fit, nothing converts.
// Cancel consumes nothing; death or pawn replacement cancels. Clients send only
// a recipe ID (or cancel) and never choose duration, inputs or output.
//
// History: M4 (e7ffd71). Docs: Docs/GATHERING_CRAFTING_M4.md

#pragma once
#include "Components/ActorComponent.h"
#include "Crafting/PFCraftingCatalog.h"
#include "Inventory/PFInventoryComponent.h"
#include "PFCraftingComponent.generated.h"
class APawn;
UCLASS(ClassGroup=(PrimalFrontier),meta=(BlueprintSpawnableComponent))
class PRIMALFRONTIER_API UPFCraftingComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPFCraftingComponent();

    /** Recipe definitions; loaded from DA_CraftingCatalog at BeginPlay if unset. */
    UPROPERTY(EditAnywhere,BlueprintReadOnly) TObjectPtr<UPFCraftingCatalog> Catalog;

    // Owner-only replicated job state for the crafting overlay.
    /** Recipe being crafted, or None when idle. */
    UPROPERTY(Replicated,BlueprintReadOnly) FName ActiveRecipe;
    /** Server time at which the active job completes (0 when idle). */
    UPROPERTY(Replicated,BlueprintReadOnly) double FinishAt=0;
    /** Last status message ("Completed", "Cancelled ...", "Failed ..."). */
    UPROPERTY(Replicated,BlueprintReadOnly) FString Feedback;

    /** Server: begin RecipeId for Pawn (must be this player's living survivor). False if busy or unaffordable. */
    bool Start(FName RecipeId,APawn* Pawn);
    /** Server: cancel the active job without consuming anything. */
    bool Cancel();
    /** The bag on the same PlayerState. */
    UPFInventoryComponent* Inventory() const;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
    virtual void BeginPlay() override;
    /** Server, 10 Hz: cancel on death, complete when FinishAt is reached. */
    virtual void TickComponent(float Delta,ELevelTick Type,FActorComponentTickFunction* Tick) override;

private:
    /** Copy of the recipe taken at Start (so later catalog edits can't change a running job). */
    FPFRecipeDefinition PendingRecipe;
    /** Exact stack snapshots (ID, item, quantity, deadline) reserved at Start. */
    TArray<FPFItemStack> Inputs;
    /** The pawn that started the job; if it dies or is replaced the job cancels. */
    TWeakObjectPtr<APawn> CraftPawn;
    bool ValidPawn(APawn* Pawn) const;
    /** Clear the job and publish Message. */
    void Finish(const FString& Message);
};
