// PFInteraction.cpp
//
// See PFInteraction.h. Two-stage query:
//  1) an exact line trace from the eyes (what the crosshair is on), then
//  2) a 12 cm sphere sweep for small targets, accepted only if the target's centre
//     is within reach and not hidden behind something else.

#include "Survival/PFInteraction.h"
#include "Inventory/PFItemPickup.h"
#include "Crafting/PFResourceNode.h"
#include "Survival/PFRecoveryPickup.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
namespace PFInteraction
{
/** Actor types that E can act on. Doors/storage are handled separately by the building component. */
static bool Supported(AActor* A){return IsValid(A) && (A->IsA<APFItemPickup>() || A->IsA<APFResourceNode>() || A->IsA<APFRecoveryPickup>());}

AActor* FindTarget(const APawn* Pawn,float Reach)
{
    if(!IsValid(Pawn) || !Pawn->GetWorld() || !FMath::IsFinite(Reach) || Reach<=0 || Reach>500){return nullptr;}
    // Use the pawn's eye viewpoint: identical on client and server (camera is pinned to it).
    FVector Eye;
    FRotator Look;
    Pawn->GetActorEyesViewPoint(Eye,Look);
    const FVector End=Eye+Look.Vector()*Reach;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(PFInteraction),false,Pawn);
    FHitResult Hit;
    if(Pawn->GetWorld()->LineTraceSingleByChannel(Hit,Eye,End,ECC_Visibility,Params) && Supported(Hit.GetActor())){return Hit.GetActor();}
    // Small aim tolerance for dropped cubes, with an independent occlusion check.
    if(Pawn->GetWorld()->SweepSingleByChannel(Hit,Eye,End,FQuat::Identity,ECC_Visibility,FCollisionShape::MakeSphere(12),Params) && Supported(Hit.GetActor()))
    {
        AActor* Target=Hit.GetActor();
        if(FVector::Dist(Eye,Target->GetActorLocation())>Reach){return nullptr;}
        // Clear line from the eyes to the target centre (or the first thing hit IS the target).
        FHitResult Sight;
        if(!Pawn->GetWorld()->LineTraceSingleByChannel(Sight,Eye,Target->GetActorLocation(),ECC_Visibility,Params) || Sight.GetActor()==Target){return Target;}
    }
    return nullptr;
}
}
