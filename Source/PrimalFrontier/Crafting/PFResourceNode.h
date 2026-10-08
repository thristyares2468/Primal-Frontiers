// PFResourceNode.h
//
// A gatherable placeholder node in the world (wood pile, stone outcrop, forage
// patch). Pressing E while aiming at it asks the server to gather. Each node has
// a finite number of hits; when depleted it hides and refills after its respawn
// time. Carrying the gathering tool spends two hits per action (faster), but the
// total yield per node stays the same.
//
// The server validates reach/aim (via PFInteraction::FindTarget), life state,
// a 0.5 s cooldown and inventory capacity BEFORE reducing the node, so a full bag
// never wastes a hit.
//
// History: M4 (e7ffd71); M7 (b5a3165) shared interaction query.
// Docs:    Docs/GATHERING_CRAFTING_M4.md

#pragma once
#include "GameFramework/Actor.h"
#include "PFResourceNode.generated.h"
class UPFCraftingCatalog;
class UPFItemCatalog;
class UStaticMeshComponent;
class UTextRenderComponent;
class APawn;
UCLASS()
class PRIMALFRONTIER_API APFResourceNode : public AActor
{
    GENERATED_BODY()

public:
    APFResourceNode();

    /** Which FPFResourceDefinition this node uses (Node_Wood / Node_Stone / Node_Food). */
    UPROPERTY(EditAnywhere,Replicated,BlueprintReadOnly) FName ResourceId=TEXT("Node_Wood");
    /** Optional overrides; default catalogs are loaded at BeginPlay. */
    UPROPERTY(EditAnywhere,BlueprintReadOnly) TObjectPtr<UPFCraftingCatalog> Catalog;
    UPROPERTY(EditAnywhere,BlueprintReadOnly) TObjectPtr<UPFItemCatalog> Items;

    /** Hits left before depletion (replicated for labels and prompts). */
    UPROPERTY(Replicated,BlueprintReadOnly) int32 HitsRemaining=0;
    /** Server time when a depleted node refills (0 while not depleted). */
    UPROPERTY(Replicated,BlueprintReadOnly) double RespawnAt=0;

    /** Server: give Pawn one gather action's yield if every check passes. */
    bool Gather(APawn* Pawn);
    /** True if ResourceId resolves to a valid definition. */
    bool IsConfigurationValid() const;
    bool RestorePersistence(int32 Hits, double RemainingRespawn);

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
    /** Load catalogs; server fills HitsRemaining from the definition. */
    virtual void BeginPlay() override;
    /** 5 Hz: server refills depleted nodes; clients update visibility and label. */
    virtual void Tick(float Delta) override;

private:
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Mesh;
    UPROPERTY() TObjectPtr<UTextRenderComponent> Label;
    /** Server time before which further gathering is refused (anti-spam). */
    double NextHitAt=0;
};
