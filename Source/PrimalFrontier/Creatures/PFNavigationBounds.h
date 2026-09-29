#pragma once
#include "NavMesh/NavMeshBoundsVolume.h"
#include "PFNavigationBounds.generated.h"
UCLASS()
class PRIMALFRONTIER_API APFNavigationBounds : public ANavMeshBoundsVolume
{
    GENERATED_BODY()
public:
    APFNavigationBounds(const FObjectInitializer& ObjectInitializer);
    // Setup scripts call this only in an isolated unsaved editor world.
    UFUNCTION(BlueprintCallable,CallInEditor,Category="PrimalFrontier") bool BuildNavigation();
};
