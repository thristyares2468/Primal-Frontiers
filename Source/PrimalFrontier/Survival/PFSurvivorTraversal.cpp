#include "Survival/PFSurvivorCharacter.h"
#include "Survival/PFTraversalMovement.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"

UPFTraversalMovement* APFSurvivorCharacter::Traversal() const {return CastChecked<UPFTraversalMovement>(GetCharacterMovement());}
bool APFSurvivorCharacter::IsTraversalInputBlocked() const
{
    const auto* PC=Cast<APFSurvivalPlayerController>(GetController());
    return PC && (PC->IsPauseMenuOpen() || PC->IsInventoryOpen() || PC->IsCraftingOpen() || PC->IsStatMenuOpen() || PC->IsBuildingMenuOpen() || PC->IsStorageMenuOpen() || PC->IsMoveInputIgnored());
}
void APFSurvivorCharacter::SetSprintHeld(bool Held){Traversal()->bWantsSprint=Held && !IsTraversalInputBlocked();}
void APFSurvivorCharacter::ToggleGlide(){Traversal()->bWantsGlide=!Traversal()->bWantsGlide && Traversal()->CanGlide();}
void APFSurvivorCharacter::CancelTraversal(){Traversal()->ClearTraversal();if(!HasAuthority()){ServerCancelTraversal();}}
void APFSurvivorCharacter::ServerCancelTraversal_Implementation(){Traversal()->ClearTraversal();}
void APFSurvivorCharacter::ServerClimb_Implementation()
{
    if(Traversal()->IsWallClimbing()){Traversal()->EndWallClimb(false,TEXT("Released wall"));return;}
    FString Reason;const bool Accepted=TryClimb(Reason);
    UE_LOG(LogPFSurvival,Display,TEXT("[PrimalAgentTools] Climb %s: %s"),Accepted?TEXT("accepted"):TEXT("refused"),*Reason);
    if(auto* PC=Cast<APFSurvivalPlayerController>(GetController())){PC->NotifyTraversalFeedback(Accepted,Reason);}
}
void APFSurvivorCharacter::PublishWallClimbState(bool Active,bool Landed)
{
    if(!HasAuthority()){return;}ReplicatedWallNormal=Active?Traversal()->WallNormal:FVector::ZeroVector;
    ++WallClimbSequence;ClientWallClimbState(ReplicatedWallNormal,WallClimbSequence,Active,Landed);ForceNetUpdate();
    if(!Active){NextClimbTime=GetWorld()->GetTimeSeconds()+.5;CancelAutomaticClimb();}
}
void APFSurvivorCharacter::ClientWallClimbState_Implementation(FVector_NetQuantizeNormal Normal,uint32 Sequence,bool Active,bool Landed)
{
    if(HasAuthority() || int32(Sequence-LastWallClimbSequence)<0){return;}LastWallClimbSequence=Sequence;
    if(Survival->IsDead()){return;}auto* M=Traversal();
    if(Active){if(!IsTraversalInputBlocked()){M->ConfirmWallClimb(Normal);}}
    else if(M->IsWallClimbing()){M->EndWallClimb(Landed,TEXT("Server ended wall climb"));}
}
// Reflected legacy RPC retained for compatibility; ballistic climbing is disabled.
void APFSurvivorCharacter::ClientClimbImpulse_Implementation(FVector_NetQuantize100,bool,float,FVector_NetQuantize100){}
bool APFSurvivorCharacter::TryClimb(FString& Reason){return TryClimbFromFeet(Reason,GetActorLocation(),false);}
bool APFSurvivorCharacter::TryAutomaticClimb(FString& Reason)
{
    if(!HasAuthority() || GetNetMode()==NM_Client || AutomaticClimbUntil<=GetWorld()->GetTimeSeconds() || !Traversal()->IsFalling() || Traversal()->Velocity.Z<=0)
    {Reason=TEXT("No recent rising authoritative jump");return false;}
    AutomaticClimbUntil=0;const bool Accepted=TryClimbFromFeet(Reason,AutomaticClimbGroundFeet,true);
    UE_LOG(LogPFSurvival,Display,TEXT("[PrimalAgentTools] Automatic wall climb %s: %s"),Accepted?TEXT("accepted"):TEXT("refused"),*Reason);
    if(auto* PC=Cast<APFSurvivalPlayerController>(GetController())){PC->NotifyTraversalFeedback(Accepted,Reason);}return Accepted;
}
bool APFSurvivorCharacter::TryClimbFromFeet(FString& Reason,const FVector&,bool Automatic)
{
    auto Refuse=[&](const TCHAR* R){Reason=R;return false;};auto* M=Traversal();
    if(!HasAuthority() || GetNetMode()==NM_Client || !GetController()){return Refuse(TEXT("Server-owned survivor required"));}
    if(Survival->IsDead() || IsTraversalInputBlocked()){return Refuse(TEXT("Close menus and be alive to climb"));}
    if(M->IsWallClimbing()){return Refuse(TEXT("Already attached; F releases the wall"));}
    if(Automatic?!M->IsFalling():!(M->IsMovingOnGround() || M->IsFalling())){return Refuse(TEXT("Cannot attach in this movement mode"));}
    if(!M->CanCarryJump()){return Refuse(TEXT("Too heavy to climb; drop or store items"));}
    if(!Survival->CanSpendStamina(1)){return Refuse(TEXT("Not enough stamina to attach"));}
    const double Now=GetWorld()->GetTimeSeconds();if(Now<NextClimbTime){return Refuse(TEXT("Climb reattach cooldown"));}
    const float Half=GetCapsuleComponent()->GetScaledCapsuleHalfHeight();const FVector Start=GetActorLocation()-FVector(0,0,Half-40);
    const FVector Forward=FRotator(0,GetControlRotation().Yaw,0).Vector();FCollisionQueryParams Params(SCENE_QUERY_STAT(PFWallAttach),false,this);FHitResult Hit;
    if(!GetWorld()->LineTraceSingleByChannel(Hit,Start,Start+Forward*100,ECC_Visibility,Params) || Hit.bStartPenetrating || FMath::Abs(Hit.ImpactNormal.Z)>.3 ||
       !Hit.GetComponent() || Hit.GetComponent()->GetCollisionResponseToChannel(ECC_Pawn)!=ECR_Block || Hit.GetComponent()->IsSimulatingPhysics())
    {return Refuse(TEXT("No solid climbable wall within one metre"));}
    if(!M->BeginWallClimb(Hit.ImpactNormal)){return Refuse(TEXT("Invalid climb tuning or attachment clearance"));}
    NextClimbTime=Now+.5;CancelAutomaticClimb();Reason=TEXT("Wall attached; move to climb, F drop; rest holds stamina");return true;
}
