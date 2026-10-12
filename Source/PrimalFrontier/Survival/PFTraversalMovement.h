#pragma once
#include "GameFramework/CharacterMovementComponent.h"
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
    double ClimbUntil=0; // short server-confirmed impulse, not persistent state
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
    virtual void UpdateFromCompressedFlags(uint8 Flags) override;
    virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;
};
