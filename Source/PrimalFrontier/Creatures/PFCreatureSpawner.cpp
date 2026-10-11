// PFCreatureSpawner.cpp — see PFCreatureSpawner.h.

#include "Creatures/PFCreatureSpawner.h"
#include "PFAssetPaths.h"
#include "Creatures/PFCreature.h"
#include "NavigationSystem.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "World/PFWorldClock.h"

DEFINE_LOG_CATEGORY_STATIC(LogPFEcology,Log,All);

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
    if(!HasAuthority() || GetWorld()->GetNetMode()==NM_Client){SetActorTickEnabled(false);}
    NextSpawn=GetWorld()->GetTimeSeconds()+2;
}

const FPFCreatureSpawnPolicy* APFCreatureSpawner::EcologyContext(UPFCreatureCatalog*& OutCreatures,
    FGameplayTag& OutPhase, int32& OutTickets, FString& Error) const
{
    const auto Refuse=[&Error](const TCHAR* Reason)->const FPFCreatureSpawnPolicy*{Error=Reason;return nullptr;};
    auto* W=GetWorld();
    if(!HasAuthority() || !W || !W->IsGameWorld() || W->GetNetMode()==NM_Client || !W->GetAuthGameMode())
    {return Refuse(TEXT("Ecology requires an authoritative game world"));}
    if(!bUseSpawnPolicy || !SpawnPolicies){return Refuse(TEXT("Ecology is not configured on this point"));}
    auto* Definitions=Catalog.Get();
    if(!Definitions){Definitions=LoadObject<UPFCreatureCatalog>(nullptr,PFAssetPaths::CreatureCatalog);}
    if(!Definitions){return Refuse(TEXT("Creature catalog is unavailable"));}
    APFWorldClock* Clock=nullptr;
    for(TActorIterator<APFWorldClock> It(W);It;++It)
    {
        if(Clock){return Refuse(TEXT("Ecology requires exactly one world clock"));}
        Clock=*It;
    }
    if(!Clock){return Refuse(TEXT("Ecology requires exactly one world clock"));}
    int32 Tickets=0;
    const auto* Policy=SpawnPolicies->ResolvePhase(*Definitions,Biome,Clock->Phase,Tickets,Error);
    if(!Policy){return nullptr;}
    OutCreatures=Definitions; OutPhase=Clock->Phase; OutTickets=Tickets;
    return Policy;
}

bool APFCreatureSpawner::ResolveEcologyRequest(int32 Roll,FName& OutId,float& OutDelay,FString& Error) const
{
    UPFCreatureCatalog* Definitions=nullptr; FGameplayTag Phase; int32 Tickets=0;
    const auto* Policy=EcologyContext(Definitions,Phase,Tickets,Error);
    if(!Policy){return false;}
    const auto Refuse=[&Error](const TCHAR* Reason){Error=Reason;return false;};
    if(!bAutoSpawn){return Refuse(TEXT("Ecology point is disabled"));}
    if(IsValid(Resident)){return Refuse(TEXT("Ecology resident still exists (living or corpse)"));}
    if(Count(GetWorld())>=8){return Refuse(TEXT("Global creature limit reached (including corpses)"));}
    int32 Residents=0; TSet<APFCreature*> Owned;
    for(TActorIterator<APFCreatureSpawner> It(GetWorld());It;++It)
    {
        if(!It->bUseSpawnPolicy){continue;}
        // One policy/catalog domain prevents overlapping assets from evading a biome budget.
        auto* OtherDefinitions=It->Catalog.Get();
        if(!OtherDefinitions){OtherDefinitions=LoadObject<UPFCreatureCatalog>(nullptr,PFAssetPaths::CreatureCatalog);}
        if(It->SpawnPolicies!=SpawnPolicies || OtherDefinitions!=Definitions)
        {return Refuse(TEXT("Ecology points must share one policy and creature catalog"));}
        if(!SpawnPolicies->Policies.ContainsByPredicate([&](const auto& P){return P.Biome==It->Biome;}))
        {return Refuse(TEXT("An ecology point has an unknown biome"));}
        if(IsValid(It->Resident))
        {
            if(It->Resident->GetWorld()!=GetWorld() || Owned.Contains(It->Resident))
            {return Refuse(TEXT("Ecology resident ownership is invalid or duplicated"));}
            Owned.Add(It->Resident);
            if(It->Biome==Biome){++Residents;}
        }
    }
    if(Residents>=Policy->MaximumResidents){return Refuse(TEXT("Biome resident limit reached (including disabled points and corpses)"));}
    FName Candidate;
    if(!SpawnPolicies->Choose(*Definitions,Biome,Phase,Roll,Candidate,Error)){return false;}
    OutId=Candidate; OutDelay=Policy->RespawnSeconds; Error.Reset(); return true;
}

bool APFCreatureSpawner::TrySpawnEcology(FString& Error)
{
    UPFCreatureCatalog* Definitions=nullptr; FGameplayTag Phase; int32 Tickets=0;
    if(!EcologyContext(Definitions,Phase,Tickets,Error)){return false;}
    if(GetWorld()->GetTimeSeconds()<NextSpawn){Error=TEXT("Ecology respawn delay has not elapsed");return false;}
    FName Id; float Delay=0;
    // Only validated server phase data can supply the ticket range. Dormant phases never draw/spawn.
    if(!ResolveEcologyRequest(Tickets>0?FMath::RandHelper(Tickets):0,Id,Delay,Error)){return false;}
    auto* Creature=Spawn(GetWorld(),Id,GetActorLocation(),Definitions);
    if(!Creature){Error=TEXT("Ecology navigation, clearance or global spawn check refused");return false;}
    Resident=Creature; NextSpawn=GetWorld()->GetTimeSeconds()+Delay; LastEcologyRefusal.Reset();
    UE_LOG(LogPFEcology,Log,TEXT("[PrimalAgentTools] Ecology spawned %s biome=%s phase=%s point=%s"),
        *Id.ToString(),*Biome.ToString(),*Phase.ToString(),*GetName());
    Error.Reset(); return true;
}

void APFCreatureSpawner::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if(!HasAuthority() || GetWorld()->GetNetMode()==NM_Client || !bAutoSpawn){return;}
    const double Now=GetWorld()->GetTimeSeconds();
    // While the resident exists, keep pushing the next spawn time forward (only counts once it's dead/gone).
    if(IsValid(Resident))
    {
        if(!Resident->IsDead())
        {
            float Delay=FMath::Clamp(RespawnSeconds,5.f,300.f);
            if(bUseSpawnPolicy)
            {
                UPFCreatureCatalog* Definitions=nullptr; FGameplayTag Phase; int32 Tickets=0; FString Error;
                const auto* Policy=EcologyContext(Definitions,Phase,Tickets,Error);
                if(Policy){Delay=Policy->RespawnSeconds;} // Keep residents even when configuration is invalid/dormant.
            }
            NextSpawn=Now+Delay;
        }
        return;
    }
    // Resident gone: spawn when due. A failed attempt (cap reached, blocked) retries in 3 s.
    if(Now>=NextSpawn)
    {
        if(bUseSpawnPolicy)
        {
            FString Error;
            if(!TrySpawnEcology(Error))
            {
                NextSpawn=Now+3;
                if(Error!=LastEcologyRefusal)
                {
                    UE_LOG(LogPFEcology,Log,TEXT("[PrimalAgentTools] Ecology refused point=%s: %s"),*GetName(),*Error);
                    LastEcologyRefusal=Error;
                }
            }
            return;
        }
        Resident=Spawn(GetWorld(),CreatureId,GetActorLocation(),Catalog);
        NextSpawn=Now+(Resident?FMath::Clamp(RespawnSeconds,5.f,300.f):3.f);
    }
}
