#pragma once
#include "GameFramework/CharacterMovementComponent.h"
#include "GameplayTagContainer.h"
#include "PFTraversalMovement.generated.h"

/** Saved-move intentions only; speed, weight, life and stamina are derived on each role.
 * Server movement validation never accepts a client speed/weight/transform. */
UCLASS()
class PRIMALFRONTIER_API UPFTraversalMovement : public UCharacterMovementComponent
{
    GENERATED_BODY()
public:
    bool bWantsSprint=false;
    bool bWantsGlide=false;
    static constexpr uint8 WallClimbMode=1;
    bool IsWallClimbing() const {return MovementMode==MOVE_Custom && CustomMovementMode==WallClimbMode;}
    FGameplayTag ClimbingState() const;
    FVector WallNormal=FVector::ZeroVector; // confirmed by server, never taken from a movement flag
    FVector WallRight() const {return FVector::CrossProduct(FVector::UpVector,-WallNormal).GetSafeNormal();}
    bool BeginWallClimb(const FVector& Normal);
    void ConfirmWallClimb(const FVector& Normal); // owning-client server acknowledgement only
    void EndWallClimb(bool bLanded,const TCHAR* Reason);
    UPROPERTY(EditDefaultsOnly,Category="Traversal|Climbing",meta=(ClampMin="30",ClampMax="400")) float WallClimbSpeed=140;
    UPROPERTY(EditDefaultsOnly,Category="Traversal|Climbing",meta=(ClampMin="1",ClampMax="100")) float WallClimbStaminaPerSecond=12;
    UPROPERTY(EditDefaultsOnly,Category="Traversal|Climbing",meta=(ClampMin="1",ClampMax="100")) float WallHoldStaminaPerSecond=4;
    // Inactive legacy source compatibility fields; no ballistic drive reads them.
    double ClimbUntil=0;
    FVector AutomaticClimbDrive=FVector::ZeroVector;
    FVector ClimbLandingTarget=FVector::ZeroVector;
    UPROPERTY(EditDefaultsOnly,Category="Traversal",meta=(ClampMin="1",ClampMax="3")) float SprintMultiplier=1.5f;
    UPROPERTY(EditDefaultsOnly,Category="Traversal",meta=(ClampMin="0.01",ClampMax="1")) float MinimumCarrySpeedFraction=0.1f;
    UPROPERTY(EditDefaultsOnly,Category="Traversal",meta=(ClampMin="1",ClampMax="2")) float JumpWeightRatio=1.1f;
    UPROPERTY(EditDefaultsOnly,Category="Traversal",meta=(ClampMin="1",ClampMax="100")) float SprintStaminaPerSecond=10;
    UPROPERTY(EditDefaultsOnly,Category="Traversal",meta=(ClampMin="1",ClampMax="100")) float GlideStaminaPerSecond=8;
    UPROPERTY(EditDefaultsOnly,Category="Traversal",meta=(ClampMin="50",ClampMax="600")) float GlideDescentSpeed=220;
    float CarryRatio() const;
    float CarrySpeedFraction() const;
    bool CanSprint() const;
    bool CanGlide() const;
    bool CanCarryJump() const;
    bool IsSprinting() const;
    bool IsGliding() const;
    void ClearTraversal();
    virtual float GetMaxSpeed() const override;
    virtual float GetGravityZ() const override;
    virtual bool DoJump(bool bReplayingMoves,float DeltaTime) override;
    virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;
    virtual void PhysFalling(float DeltaSeconds,int32 Iterations) override;
    virtual void PhysWalking(float DeltaSeconds,int32 Iterations) override;
    virtual void PhysCustom(float DeltaSeconds,int32 Iterations) override;
    virtual void HandleImpact(const FHitResult& Hit,float TimeSlice=0.f,const FVector& MoveDelta=FVector::ZeroVector) override;
    virtual void UpdateFromCompressedFlags(uint8 Flags) override;
    virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;
private:
    bool bWallApproved=false;
    bool ValidWallClimbTuning() const;
    bool FindClimbWall(FHitResult& Hit) const;
    bool FinishWallTop();
};
