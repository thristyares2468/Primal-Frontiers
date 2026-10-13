#include "Survival/PFTraversalMovement.h"
#include "Survival/PFSurvivorCharacter.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Inventory/PFInventoryComponent.h"
#include "GameFramework/PlayerState.h"
#include "Engine/World.h"

namespace
{
class FPFTraversalMove : public FSavedMove_Character
{
public:
    bool Sprint=false,Glide=false;
    virtual void Clear() override {Super::Clear();Sprint=false;Glide=false;}
    virtual uint8 GetCompressedFlags() const override
    {return Super::GetCompressedFlags() | (Sprint?FLAG_Custom_0:0) | (Glide?FLAG_Custom_1:0);}
    virtual bool CanCombineWith(const FSavedMovePtr& New,ACharacter* C,float MaxDelta) const override
    {const auto* Other=static_cast<const FPFTraversalMove*>(New.Get());return Sprint==Other->Sprint && Glide==Other->Glide && Super::CanCombineWith(New,C,MaxDelta);}
    virtual void SetMoveFor(ACharacter* C,float Delta,const FVector& Accel,FNetworkPredictionData_Client_Character& Data) override
    {Super::SetMoveFor(C,Delta,Accel,Data);const auto* M=CastChecked<UPFTraversalMovement>(C->GetCharacterMovement());Sprint=M->bWantsSprint;Glide=M->bWantsGlide;}
    virtual void PrepMoveFor(ACharacter* C) override
    {Super::PrepMoveFor(C);auto* M=CastChecked<UPFTraversalMovement>(C->GetCharacterMovement());M->bWantsSprint=Sprint;M->bWantsGlide=Glide;}
private:
    using Super=FSavedMove_Character;
};
class FPFTraversalPrediction : public FNetworkPredictionData_Client_Character
{
public:
    explicit FPFTraversalPrediction(const UCharacterMovementComponent& M):FNetworkPredictionData_Client_Character(M){}
    virtual FSavedMovePtr AllocateNewMove() override {return FSavedMovePtr(new FPFTraversalMove());}
};
}
float UPFTraversalMovement::CarryRatio() const
{
    const auto* PS=CharacterOwner?CharacterOwner->GetPlayerState():nullptr;
    const auto* I=PS?PS->FindComponentByClass<UPFInventoryComponent>():nullptr;
    if(!I){return 0;}const float Weight=I->GetWeight();
    if(!FMath::IsFinite(Weight) || !FMath::IsFinite(I->WeightLimit) || I->WeightLimit<=0){return MAX_flt;}
    return Weight/I->WeightLimit;
}
float UPFTraversalMovement::CarrySpeedFraction() const
{
    const double Ratio=CarryRatio();return float(FMath::Clamp(1./FMath::Square(FMath::Max(1.,Ratio)),double(MinimumCarrySpeedFraction),1.));
}
bool UPFTraversalMovement::CanCarryJump() const {return CarryRatio()<JumpWeightRatio;}
bool UPFTraversalMovement::CanSprint() const
{
    const auto* P=Cast<APFSurvivorCharacter>(CharacterOwner);
    return P && P->Survival->CanSpendStamina(1) && !P->IsTraversalInputBlocked() && CarryRatio()<=1 && IsMovingOnGround() && !IsCrouching();
}
bool UPFTraversalMovement::CanGlide() const
{
    const auto* P=Cast<APFSurvivorCharacter>(CharacterOwner);
    return P && P->Survival->CanSpendStamina(1) && !P->IsTraversalInputBlocked() && CarryRatio()<=1 && IsFalling() && GetWorld()->GetTimeSeconds()>=ClimbUntil;
}
bool UPFTraversalMovement::IsSprinting() const {return bWantsSprint && CanSprint();}
bool UPFTraversalMovement::IsGliding() const
{
    const auto* P=Cast<APFSurvivorCharacter>(CharacterOwner);
    if(P && P->GetLocalRole()==ROLE_SimulatedProxy){return P->IsRemoteGliding() && !P->Survival->IsDead() && IsFalling();}
    return bWantsGlide && CanGlide();
}
void UPFTraversalMovement::ClearTraversal()
{bWantsSprint=false;bWantsGlide=false;ClimbUntil=0;AutomaticClimbDrive=FVector::ZeroVector;if(auto* P=Cast<APFSurvivorCharacter>(CharacterOwner)){P->CancelAutomaticClimb();}}
float UPFTraversalMovement::GetMaxSpeed() const
{
    const float Base=Super::GetMaxSpeed();
    return (IsMovingOnGround() || IsFalling())?Base*CarrySpeedFraction()*(IsSprinting()?SprintMultiplier:1.f):Base;
}
float UPFTraversalMovement::GetGravityZ() const {return Super::GetGravityZ()*(IsGliding()?0.15f:1.f);}
bool UPFTraversalMovement::DoJump(bool Replay,float Delta){return CanCarryJump() && Super::DoJump(Replay,Delta);}
void UPFTraversalMovement::UpdateCharacterStateBeforeMovement(float Delta)
{
    Super::UpdateCharacterStateBeforeMovement(Delta);
    auto* P=Cast<APFSurvivorCharacter>(CharacterOwner);if(!P){return;}
    if(P->Survival->IsDead() || P->IsTraversalInputBlocked()){ClearTraversal();return;}
    if(P->HasAuthority() && FMath::IsFinite(Delta) && Delta>0)
    {
        const float Rate=IsGliding()?GlideStaminaPerSecond:(IsSprinting() && (!Acceleration.IsNearlyZero() || Velocity.SizeSquared2D()>100)?SprintStaminaPerSecond:0);
        if(Rate>0 && !P->Survival->SpendStamina(Rate*Delta)){bWantsSprint=false;bWantsGlide=false;}
    }
    // Clear landed gliding intent rather than silently deploying on the next jump.
    if(!IsFalling()){bWantsGlide=false;AutomaticClimbDrive=FVector::ZeroVector;P->CancelAutomaticClimb();}
}
void UPFTraversalMovement::HandleImpact(const FHitResult& Hit,float TimeSlice,const FVector& MoveDelta)
{
    Super::HandleImpact(Hit,TimeSlice,MoveDelta);
    auto* P=Cast<APFSurvivorCharacter>(CharacterOwner);
    if(P && P->HasAuthority() && IsFalling() && Hit.bBlockingHit && Hit.ImpactNormal.Z<=0.3f)
    {FString Reason;P->TryAutomaticClimb(Reason);}
}
void UPFTraversalMovement::PhysFalling(float Delta,int32 Iterations)
{
    const bool Glide=IsGliding(),Climb=GetWorld()->GetTimeSeconds()<ClimbUntil;
    TGuardValue<float> Air(AirControl,Climb?0.f:(Glide?0.75f:AirControl));
    TGuardValue<float> Brake(BrakingDecelerationFalling,Climb?0.f:BrakingDecelerationFalling);
    // A contact-triggered launch initially touches the wall. UE correctly strips
    // horizontal velocity there; reapply only the server-approved bounded drive
    // until the capsule rises clear. Swept falling collision still runs normally.
    if(Climb && !AutomaticClimbDrive.IsNearlyZero())
    {Velocity.X=AutomaticClimbDrive.X;Velocity.Y=AutomaticClimbDrive.Y;}
    if(Glide){Velocity.Z=FMath::Max(Velocity.Z,-GlideDescentSpeed);}
    Super::PhysFalling(Delta,Iterations);
    if(IsGliding()){Velocity.Z=FMath::Max(Velocity.Z,-GlideDescentSpeed);}
}
void UPFTraversalMovement::UpdateFromCompressedFlags(uint8 Flags)
{Super::UpdateFromCompressedFlags(Flags);bWantsSprint=(Flags&FSavedMove_Character::FLAG_Custom_0)!=0;bWantsGlide=(Flags&FSavedMove_Character::FLAG_Custom_1)!=0;}
FNetworkPredictionData_Client* UPFTraversalMovement::GetPredictionData_Client() const
{if(!ClientPredictionData){const_cast<UPFTraversalMovement*>(this)->ClientPredictionData=new FPFTraversalPrediction(*this);}return ClientPredictionData;}
