#pragma once
#include "PrimalFrontierCharacter.h"
#include "PFSurvivorCharacter.generated.h"
class UPFPlayerSurvivalComponent;
class UStaticMeshComponent;

UCLASS()
class PRIMALFRONTIER_API APFSurvivorCharacter : public APrimalFrontierCharacter
{
    GENERATED_BODY()
public:
    APFSurvivorCharacter();
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Survival") TObjectPtr<UPFPlayerSurvivalComponent> Survival;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Survival", meta=(ClampMin="1")) float JumpStaminaCost = 20.f;
    virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
    virtual bool CanJumpInternal_Implementation() const override;
    virtual void OnJumped_Implementation() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
protected:
    virtual void BeginPlay() override;
    virtual void DoMove(float Right, float Forward) override;
    UFUNCTION() void HandleDeath();
private:
    UPROPERTY(Replicated) bool bHasGatheringTool=false;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> ToolHandle;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> ToolHead;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> RemoteTool;
};
