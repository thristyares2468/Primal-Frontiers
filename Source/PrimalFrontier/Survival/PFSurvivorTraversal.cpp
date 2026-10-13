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
    return PC && (PC->IsPauseMenuOpen() || PC->IsInventoryOpen() || PC->IsCraftingOpen() || PC->IsBuildingMenuOpen() || PC->IsStorageMenuOpen() || PC->IsMoveInputIgnored());
}
void APFSurvivorCharacter::SetSprintHeld(bool Held){Traversal()->bWantsSprint=Held && !IsTraversalInputBlocked();}
void APFSurvivorCharacter::ToggleGlide(){Traversal()->bWantsGlide=!Traversal()->bWantsGlide && Traversal()->CanGlide();}
void APFSurvivorCharacter::CancelTraversal(){Traversal()->ClearTraversal();if(!HasAuthority()){ServerCancelTraversal();}}
void APFSurvivorCharacter::ServerCancelTraversal_Implementation(){Traversal()->ClearTraversal();}
void APFSurvivorCharacter::ServerClimb_Implementation()
{
    FString Reason;const bool Accepted=TryClimb(Reason);
    UE_LOG(LogPFSurvival,Display,TEXT("[PrimalTraversal] Climb %s: %s at %s yaw=%.1f"),Accepted?TEXT("accepted"):TEXT("refused"),*Reason,*GetActorLocation().ToString(),GetControlRotation().Yaw);
    if(auto* PC=Cast<APFSurvivalPlayerController>(GetController())){PC->NotifyTraversalFeedback(Accepted,Reason);}
}
void APFSurvivorCharacter::ClientClimbImpulse_Implementation(FVector_NetQuantize100 Impulse,bool /*bAutomatic*/)
{
    if(HasAuthority() || Survival->IsDead()){return;}Traversal()->ClearTraversal();Traversal()->ClimbUntil=GetWorld()->GetTimeSeconds()+1.5;
    Traversal()->AutomaticClimbDrive=FVector(Impulse.X,Impulse.Y,0);
    LaunchCharacter(Impulse,true,true);
}
bool APFSurvivorCharacter::TryClimb(FString& Reason)
{
    const FVector Feet=GetActorLocation()-FVector(0,0,GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
    return TryClimbFromFeet(Reason,Feet,false);
}
bool APFSurvivorCharacter::TryAutomaticClimb(FString& Reason)
{
    if(!HasAuthority() || GetNetMode()==NM_Client || AutomaticClimbUntil<=GetWorld()->GetTimeSeconds() || !Traversal()->IsFalling() || Traversal()->Velocity.Z<=0)
    {Reason=TEXT("No recent rising authoritative jump");return false;}
    // One attempt per jump/contact even when clearance fails; no wall bounce spam.
    AutomaticClimbUntil=0;
    const bool Accepted=TryClimbFromFeet(Reason,AutomaticClimbGroundFeet,true);
    UE_LOG(LogPFSurvival,Display,TEXT("[PrimalTraversal] Automatic climb %s: %s"),Accepted?TEXT("accepted"):TEXT("refused"),*Reason);
    if(auto* PC=Cast<APFSurvivalPlayerController>(GetController())){PC->NotifyTraversalFeedback(Accepted,Reason);}
    return Accepted;
}
bool APFSurvivorCharacter::TryClimbFromFeet(FString& Reason,const FVector& Feet,bool bAutomatic)
{
    auto Refuse=[&](const TCHAR* R){Reason=R;return false;};auto* M=Traversal();
    if(!HasAuthority() || GetNetMode()==NM_Client || !GetController()){return Refuse(TEXT("Server-owned survivor required"));}
    if(Survival->IsDead()){return Refuse(TEXT("Cannot climb while dead"));}
    if(IsTraversalInputBlocked()){return Refuse(TEXT("Close the current menu before climbing"));}
    if(bAutomatic?!M->IsFalling():!M->IsMovingOnGround())
    {return Refuse(TEXT("Press F from the ground; W + Space climbs during a rising jump"));}
    if(!M->CanCarryJump()){return Refuse(TEXT("Too heavy to climb; drop or store items"));}
    if(!Survival->CanSpendStamina(ClimbStaminaCost)){return Refuse(TEXT("Not enough stamina to climb"));}
    const double Now=GetWorld()->GetTimeSeconds();if(Now<NextClimbTime){return Refuse(TEXT("Climb cooldown"));}NextClimbTime=Now+0.4;
    const float Radius=GetCapsuleComponent()->GetScaledCapsuleRadius(),Half=GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
    const FVector Start=GetActorLocation();
    const FVector Forward=FRotator(0,GetControlRotation().Yaw,0).Vector();
    FCollisionQueryParams Params(SCENE_QUERY_STAT(PFLedge),false,this);FHitResult Wall,Top,Clear;
    if(!GetWorld()->LineTraceSingleByChannel(Wall,Feet+FVector(0,0,40),Feet+FVector(0,0,40)+Forward*100,ECC_Visibility,Params) || Wall.ImpactNormal.Z>0.3)
    {return Refuse(TEXT("No ledge within one metre"));}
    const FVector Probe=Wall.ImpactPoint+Forward*(Radius+12);
    if(!GetWorld()->LineTraceSingleByChannel(Top,FVector(Probe.X,Probe.Y,Feet.Z+MaxClimbHeight+5),FVector(Probe.X,Probe.Y,Feet.Z+45),ECC_Visibility,Params) || Top.ImpactNormal.Z<M->GetWalkableFloorZ())
    {return Refuse(TEXT("No walkable low ledge"));}
    const double Height=Top.ImpactPoint.Z-Feet.Z;
    if(Top.bStartPenetrating || Height>MaxClimbHeight)
    {Reason=FString::Printf(TEXT("Wall too high; ledge climbing supports 45-%.0f cm"),MaxClimbHeight);return false;}
    if(Height<=45){return Refuse(TEXT("Ledge too low; walk or jump over it"));}
    const double RemainingHeight=Top.ImpactPoint.Z-(Start.Z-Half);
    if(RemainingHeight<=0){return Refuse(TEXT("Already above ledge"));}
    const FVector Target=Top.ImpactPoint+FVector(0,0,Half+2),Lift=Start+FVector(0,0,RemainingHeight+65),Across(Target.X,Target.Y,Lift.Z);
    const auto Shape=FCollisionShape::MakeCapsule(Radius,Half-1);
    if(GetWorld()->OverlapBlockingTestByChannel(Target,FQuat::Identity,ECC_Pawn,Shape,Params) ||
        GetWorld()->SweepSingleByChannel(Clear,Start,Lift,FQuat::Identity,ECC_Pawn,Shape,Params) ||
        GetWorld()->SweepSingleByChannel(Clear,Lift,Across,FQuat::Identity,ECC_Pawn,Shape,Params))
    {return Refuse(TEXT("Ledge or overhead capsule clearance obstructed"));}
    const double Gravity=FMath::Abs(M->GetGravityZ());if(Gravity<1 || !FMath::IsFinite(Gravity)){return Refuse(TEXT("Unsupported gravity"));}
    const double Up=FMath::Sqrt(2*Gravity*(RemainingHeight+60));
    const double Seconds=(Up+FMath::Sqrt(Up*Up-2*Gravity*RemainingHeight))/Gravity;
    FVector Impulse=(Target-Start)/Seconds;Impulse.Z=Up;
    if(Impulse.ContainsNaN() || Impulse.Size()>1500 || !Survival->SpendStamina(ClimbStaminaCost)){return Refuse(TEXT("Invalid impulse or unavailable stamina"));}
    M->ClearTraversal();M->ClimbUntil=Now+1.5;NextClimbTime=Now+1.5;LaunchCharacter(Impulse,true,true);
    // Manual contact climbs need the same bounded drive as jump-contact climbs.
    // The falling solver otherwise strips XY on the first touching-wall sweep.
    M->AutomaticClimbDrive=FVector(Impulse.X,Impulse.Y,0);
    ClientClimbImpulse(Impulse,bAutomatic);ForceNetUpdate();Reason=TEXT("Server-derived short ledge climb");return true;
}
