// PFCreatureSpawner.h
//
// Map-placed spawn point that keeps ONE resident creature alive. After the
// resident dies (and its corpse is gone) it waits RespawnSeconds, then spawns
// a replacement. Spawn() is also used by the PF.SpawnCreature developer command.
//
// Safety limits enforced by Spawn(): server/game world only, valid catalog ID,
// location within +/-1000 m, projected onto the navmesh, capsule clearance, and
// a world-wide cap of 8 creatures (corpses included) to bound AI and replication.
//
// History: M6 (0c2d935). Docs: Docs/CREATURES_M6.md

#pragma once
#include "GameFramework/Actor.h"
#include "Creatures/PFCreatureCatalog.h"
#include "PFCreatureSpawner.generated.h"
class APFCreature;
UCLASS(Blueprintable)
class PRIMALFRONTIER_API APFCreatureSpawner : public AActor
{
    GENERATED_BODY()

public:
    APFCreatureSpawner();

    /** Which creature this point maintains. */
    UPROPERTY(EditAnywhere,BlueprintReadOnly) FName CreatureId=TEXT("Creature_Forager");
    /** Set false to pause spawning (PF.ResetCreatures does this). */
    UPROPERTY(EditAnywhere,BlueprintReadOnly) bool bAutoSpawn=true;
    /** Delay after the resident is gone before spawning another (5..300 s). */
    UPROPERTY(EditAnywhere,BlueprintReadOnly,meta=(ClampMin="5",ClampMax="300")) float RespawnSeconds=30;
    UPROPERTY(EditAnywhere,BlueprintReadOnly) TObjectPtr<UPFCreatureCatalog> Catalog;
    /** The creature currently owned by this point (may be dead/corpse). */
    UPROPERTY(Transient) TObjectPtr<APFCreature> Resident;

    /** Validated server spawn (see file header). Returns nullptr when any check fails. */
    static APFCreature* Spawn(UWorld* World, FName Id, FVector Location, UPFCreatureCatalog* DefinitionCatalog=nullptr);
    /** Number of creature actors in World, living or dead. */
    static int32 Count(UWorld* World);

protected:
    /** Clients disable ticking; first spawn attempt happens 2 s after start. */
    virtual void BeginPlay() override;
    /** 1 Hz: maintain the resident. */
    virtual void Tick(float DeltaSeconds) override;

private:
    double NextSpawn=0;
};
