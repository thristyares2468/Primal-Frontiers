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
    UPROPERTY(EditAnywhere,BlueprintReadOnly) FName CreatureId=TEXT("Creature_Forager");
    UPROPERTY(EditAnywhere,BlueprintReadOnly) bool bAutoSpawn=true;
    UPROPERTY(EditAnywhere,BlueprintReadOnly,meta=(ClampMin="5",ClampMax="300")) float RespawnSeconds=30;
    UPROPERTY(EditAnywhere,BlueprintReadOnly) TObjectPtr<UPFCreatureCatalog> Catalog;
    UPROPERTY(Transient) TObjectPtr<APFCreature> Resident;
    static APFCreature* Spawn(UWorld* World, FName Id, FVector Location, UPFCreatureCatalog* DefinitionCatalog=nullptr);
    static int32 Count(UWorld* World);
protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
private:
    double NextSpawn=0;
};
