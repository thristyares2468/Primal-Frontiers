// PFCreatureSpawner.cpp — see PFCreatureSpawner.h.

#include "Creatures/PFCreatureSpawner.h"
#include "PFAssetPaths.h"
#include "Creatures/PFCreature.h"
#include "NavigationSystem.h"
#include "EngineUtils.h"
#include "Engine/World.h"

APFCreatureSpawner::APFCreatureSpawner()
{
    PrimaryActorTick.bCanEverTick=true;
    PrimaryActorTick.TickInterval=1;
    SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
}

int32 APFCreatureSpawner::Count(UWorld* World)
{
    int32 Total=0;
    if(World){for(TActorIterator<APFCreature> It(World);It;++It){++Total;}}
    return Total;
}

APFCreature* APFCreatureSpawner::Spawn(UWorld* World,FName Id,FVector Location,UPFCreatureCatalog* DefinitionCatalog)
{
    // Authority, sane location and the global 8-creature cap.
    if(!World || !World->IsGameWorld() || World->GetNetMode()==NM_Client || !World->GetAuthGameMode() || Location.ContainsNaN() || Location.GetAbsMax()>100000 || Count(World)>=8){return nullptr;}
    if(!DefinitionCatalog){DefinitionCatalog=LoadObject<UPFCreatureCatalog>(nullptr,PFAssetPaths::CreatureCatalog);}
    if(!DefinitionCatalog || !DefinitionCatalog->Find(Id)){return nullptr;}
    // Must land on the navmesh, with room for the capsule (radius 38, half-height 50).
    auto* Nav=FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
    FNavLocation Projected;
    if(!Nav || !Nav->ProjectPointToNavigation(Location,Projected,FVector(200,200,300))){return nullptr;}
    const FVector Center=Projected.Location+FVector(0,0,52);
    if(World->OverlapBlockingTestByChannel(Center,FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(38,50))){return nullptr;}
    const FTransform Transform(Center);
    auto* Creature=World->SpawnActorDeferred<APFCreature>(APFCreature::StaticClass(),Transform,nullptr,nullptr,ESpawnActorCollisionHandlingMethod::DontSpawnIfColliding);
    if(Creature)
    {
        Creature->Catalog=DefinitionCatalog;
        Creature->CreatureId=Id;
        Creature->FinishSpawning(Transform);
    }
    // BeginPlay destroys creatures with invalid definitions, hence the IsValid check.
    return IsValid(Creature)?Creature:nullptr;
}

double APFCreatureSpawner::PersistenceRespawnRemaining() const
{
    return FMath::Clamp(NextSpawn - GetWorld()->GetTimeSeconds(), 0., 300.);
}

void APFCreatureSpawner::RestorePersistence(APFCreature* Creature, double Remaining, bool bEnabled)
{
    if (!HasAuthority() || !FMath::IsFinite(Remaining) || Remaining < 0 || Remaining > 300) { return; }
    Resident = Creature; bAutoSpawn = bEnabled; NextSpawn = GetWorld()->GetTimeSeconds() + Remaining;
}

void APFCreatureSpawner::BeginPlay()
{
    Super::BeginPlay();
    if(!HasAuthority()){SetActorTickEnabled(false);}
    NextSpawn=GetWorld()->GetTimeSeconds()+2;
}

void APFCreatureSpawner::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if(!HasAuthority() || !bAutoSpawn){return;}
    const double Now=GetWorld()->GetTimeSeconds();
    // While the resident exists, keep pushing the next spawn time forward (only counts once it's dead/gone).
    if(IsValid(Resident))
    {
        if(!Resident->IsDead()){NextSpawn=Now+FMath::Clamp(RespawnSeconds,5.f,300.f);}
        return;
    }
    // Resident gone: spawn when due. A failed attempt (cap reached, blocked) retries in 3 s.
    if(Now>=NextSpawn)
    {
        Resident=Spawn(GetWorld(),CreatureId,GetActorLocation(),Catalog);
        NextSpawn=Now+(Resident?FMath::Clamp(RespawnSeconds,5.f,300.f):3.f);
    }
}
