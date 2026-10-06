// PFNavigationBounds.h
//
// NavMesh bounds volume (70 x 70 x 10 m) used by the creature maps, plus an
// editor-only BuildNavigation() that the Python setup scripts call to bake the
// navmesh in a batch (commandlet) editor session, where the normal editor tick
// that would release navigation locks never runs.
//
// History: M6 (0c2d935) — written after the first setup run failed because the
//          commandlet kept the async-loading navigation lock. Docs: Docs/CREATURES_M6.md

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
    /** Build navigation synchronously and return true if the volume's centre is on the navmesh.
     *  Refuses in game worlds; does nothing outside the editor. */
    UFUNCTION(BlueprintCallable,CallInEditor,Category="PrimalFrontier") bool BuildNavigation();
};
