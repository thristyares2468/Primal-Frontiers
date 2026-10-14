#include "Survival/PFTraversalMovement.h"
#include "Survival/PFSurvivorCharacter.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "NativeGameplayTags.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PF_Climbing,"State.Traversal.Climbing");
FGameplayTag UPFTraversalMovement::ClimbingState() const {return IsWallClimbing()?TAG_PF_Climbing:FGameplayTag();}
bool UPFTraversalMovement::ValidWallClimbTuning() const
{return FMath::IsFinite(WallClimbSpeed) && WallClimbSpeed>=30 && WallClimbSpeed<=400 && FMath::IsFinite(WallClimbStaminaPerSecond) && WallClimbStaminaPerSecond>=1 && WallClimbStaminaPerSecond<=100 && FMath::IsFinite(WallHoldStaminaPerSecond) && WallHoldStaminaPerSecond>=1 && WallHoldStaminaPerSecond<=100;}

bool UPFTraversalMovement::BeginWallClimb(const FVector& Normal)
{
    auto* P=Cast<APFSurvivorCharacter>(CharacterOwner);
    if(!P || !P->HasAuthority() || P->Survival->IsDead() || Normal.ContainsNaN() || !Normal.IsNormalized() || FMath::Abs(Normal.Z)>.3 ||
       !ValidWallClimbTuning()){return false;}
    ClearTraversal();WallNormal=Normal.GetSafeNormal2D();FHitResult Wall;
    if(!FindClimbWall(Wall)){WallNormal=FVector::ZeroVector;return false;}
    const float Radius=P->GetCapsuleComponent()->GetScaledCapsuleRadius();const FVector At=P->GetActorLocation();
    FVector Attach=Wall.ImpactPoint+WallNormal*(Radius+3);Attach.Z=At.Z;FHitResult Hit;
    SafeMoveUpdatedComponent(Attach-At,UpdatedComponent->GetComponentQuat(),true,Hit);
    if(Hit.bStartPenetrating){WallNormal=FVector::ZeroVector;return false;}
    bWallApproved=true;Velocity=FVector::ZeroVector;SetMovementMode(MOVE_Custom,WallClimbMode);P->PublishWallClimbState(true,false);return true;
}
void UPFTraversalMovement::ConfirmWallClimb(const FVector& Normal)
{
    auto* P=Cast<APFSurvivorCharacter>(CharacterOwner);
    if(!P || P->HasAuthority() || !P->IsLocallyControlled() || Normal.ContainsNaN() || !Normal.IsNormalized() || FMath::Abs(Normal.Z)>.3){return;}
    ClearTraversal();WallNormal=Normal.GetSafeNormal2D();bWallApproved=true;Velocity=FVector::ZeroVector;SetMovementMode(MOVE_Custom,WallClimbMode);
}
void UPFTraversalMovement::EndWallClimb(bool Landed,const TCHAR* Reason)
{
    if(!IsWallClimbing()){return;}auto* P=Cast<APFSurvivorCharacter>(CharacterOwner);const FVector Away=WallNormal;
    bWallApproved=false;WallNormal=FVector::ZeroVector;Velocity=Landed?FVector::ZeroVector:Away*70-FVector(0,0,50);
    SetMovementMode(Landed?MOVE_Walking:MOVE_Falling);bWantsGlide=false;bWantsSprint=false;
    if(P && P->HasAuthority())
    {
        P->PublishWallClimbState(false,Landed);
        UE_LOG(LogPFSurvival,Display,TEXT("[PrimalAgentTools] Wall climb ended: %s; stamina=%.2f"),Reason,P->Survival->GetVitals().Stamina);
        if(auto* PC=Cast<APFSurvivalPlayerController>(P->GetController())){PC->NotifyTraversalFeedback(Landed,Reason,true);}
    }
}
bool UPFTraversalMovement::FindClimbWall(FHitResult& Hit) const
{
    if(!CharacterOwner || WallNormal.IsNearlyZero()){return false;}
    const float Half=CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight(),Radius=CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleRadius();
    const FVector Start=UpdatedComponent->GetComponentLocation()-FVector(0,0,Half-40);FCollisionQueryParams Params(SCENE_QUERY_STAT(PFWallContact),false,CharacterOwner);
    return GetWorld()->LineTraceSingleByChannel(Hit,Start,Start-WallNormal*(Radius+65),ECC_Visibility,Params) && !Hit.bStartPenetrating &&
        FMath::Abs(Hit.ImpactNormal.Z)<=.3 && FVector::DotProduct(Hit.ImpactNormal,WallNormal)>.7 && Hit.GetComponent() &&
        Hit.GetComponent()->GetCollisionResponseToChannel(ECC_Pawn)==ECR_Block && !Hit.GetComponent()->IsSimulatingPhysics();
}
bool UPFTraversalMovement::FinishWallTop()
{
    const float Half=CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight(),Radius=CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleRadius();
    const FVector At=UpdatedComponent->GetComponentLocation(),Feet=At-FVector(0,0,Half),Probe=Feet-WallNormal*(2*Radius+16);
    FCollisionQueryParams Params(SCENE_QUERY_STAT(PFWallTop),false,CharacterOwner);FHitResult Top,Hit;
    if(!GetWorld()->LineTraceSingleByChannel(Top,Probe+FVector(0,0,65),Probe-FVector(0,0,5),ECC_Pawn,Params) || Top.bStartPenetrating || Top.ImpactNormal.Z<GetWalkableFloorZ()){return false;}
    const FVector Target=Top.ImpactPoint+FVector(0,0,Half+2),Lift(At.X,At.Y,Target.Z+6);const auto Shape=FCollisionShape::MakeCapsule(Radius,Half-1);
    if(GetWorld()->OverlapBlockingTestByChannel(Target,FQuat::Identity,ECC_Pawn,Shape,Params) ||
       GetWorld()->SweepSingleByChannel(Hit,At,Lift,FQuat::Identity,ECC_Pawn,Shape,Params) ||
       GetWorld()->SweepSingleByChannel(Hit,Lift,Target,FQuat::Identity,ECC_Pawn,Shape,Params)){return false;}
    SafeMoveUpdatedComponent(Lift-At,UpdatedComponent->GetComponentQuat(),true,Hit);if(Hit.bBlockingHit){return false;}
    SafeMoveUpdatedComponent(Target-UpdatedComponent->GetComponentLocation(),UpdatedComponent->GetComponentQuat(),true,Hit);if(Hit.bBlockingHit){return false;}
    EndWallClimb(true,TEXT("Reached clear wall top"));return true;
}
void UPFTraversalMovement::PhysCustom(float Delta,int32 Iterations)
{
    if(!IsWallClimbing()){Super::PhysCustom(Delta,Iterations);return;}
    if(!HasValidData() || !FMath::IsFinite(Delta) || Delta<=0){return;}auto* P=Cast<APFSurvivorCharacter>(CharacterOwner);if(!P){return;}
    if(P->GetLocalRole()==ROLE_SimulatedProxy){FHitResult Hit;SafeMoveUpdatedComponent(Velocity*Delta,UpdatedComponent->GetComponentQuat(),true,Hit);return;}
    if(!bWallApproved || !ValidWallClimbTuning() || P->Survival->IsDead() || P->IsTraversalInputBlocked() || !CanCarryJump() || P->Survival->GetVitals().Stamina<=0)
    {EndWallClimb(false,TEXT("Stamina empty or climb cancelled"));StartNewPhysics(Delta,Iterations+1);return;}
    FVector Input=FVector::VectorPlaneProject(Acceleration,WallNormal)/FMath::Max(1.f,GetMaxAcceleration());Input=Input.GetClampedToMaxSize(1);
    const float Rate=Input.IsNearlyZero()?WallHoldStaminaPerSecond:WallClimbStaminaPerSecond;float TravelTime=Delta;bool Exhausted=false;
    if(P->HasAuthority())
    {
        const float Current=P->Survival->GetVitals().Stamina,Cost=FMath::Min(Current,Rate*Delta);TravelTime=FMath::Min(Delta,Current/Rate);
        P->Survival->ChangeStamina(-Cost);Exhausted=P->Survival->GetVitals().Stamina<=KINDA_SMALL_NUMBER;
        if(Exhausted && P->Survival->GetVitals().Stamina>0){P->Survival->ChangeStamina(-P->Survival->GetVitals().Stamina);}
    }
    if(!Exhausted && Input.Z>0 && FinishWallTop()){return;}
    FHitResult Wall;if(!FindClimbWall(Wall)){EndWallClimb(false,TEXT("Lost wall contact"));StartNewPhysics(Delta,Iterations+1);return;}
    Velocity=Input*GetMaxSpeed();FHitResult Hit;SafeMoveUpdatedComponent(Velocity*TravelTime,UpdatedComponent->GetComponentQuat(),true,Hit);
    if(Hit.bBlockingHit)
    {
        if(Input.Z<0 && Hit.ImpactNormal.Z>=GetWalkableFloorZ()){EndWallClimb(true,TEXT("Reached ground"));return;}
        SlideAlongSurface(Velocity*TravelTime,1-Hit.Time,Hit.Normal,Hit,true);
    }
    if(Exhausted){EndWallClimb(false,TEXT("Stamina empty; fell off wall"));if(Delta-TravelTime>MIN_TICK_TIME){StartNewPhysics(Delta-TravelTime,Iterations+1);}}
}
