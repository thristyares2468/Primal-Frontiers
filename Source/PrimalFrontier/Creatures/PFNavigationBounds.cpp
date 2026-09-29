#include "Creatures/PFNavigationBounds.h"
#include "Components/BoxComponent.h"
#include "NavigationSystem.h"
#include "NavigationData.h"
#include "Engine/World.h"
#if WITH_EDITOR
#include "AssetCompilingManager.h"
#endif
APFNavigationBounds::APFNavigationBounds(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer)
{
    auto* Bounds=CreateDefaultSubobject<UBoxComponent>(TEXT("NavigationExtent"));Bounds->SetupAttachment(GetRootComponent());
    Bounds->SetBoxExtent(FVector(3500,3500,500));Bounds->SetCollisionEnabled(ECollisionEnabled::NoCollision);Bounds->SetCanEverAffectNavigation(false);
}
bool APFNavigationBounds::BuildNavigation()
{
#if WITH_EDITOR
    if(!GetWorld() || GetWorld()->IsGameWorld()){return false;}
    auto* Nav=FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());if(!Nav){return false;}
    // Batch setup has no editor tick loop to release the async-loading lock.
    // Finish all asset work before releasing only that lock; retain all other locks.
    FlushAsyncLoading();FAssetCompilingManager::Get().FinishAllCompilation();
    Nav->RemoveNavigationBuildLock(ENavigationBuildLock::AsyncLoadLock,UNavigationSystemV1::ELockRemovalRebuildAction::NoRebuild);
    Nav->OnNavigationBoundsUpdated(this);
    // Process queued bounds and octree updates before the synchronous batch build.
    Nav->Tick(0.f);Nav->Build();
    for(auto& Data:Nav->NavDataSet){if(Data){Data->EnsureBuildCompletion();}}
    FNavLocation Projected;return Nav->ProjectPointToNavigation(GetActorLocation(),Projected,FVector(500,500,600));
#else
    return false;
#endif
}
