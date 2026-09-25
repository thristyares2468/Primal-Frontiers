#pragma once
#include "GameFramework/Actor.h"
#include "PFRecoveryPickup.generated.h"
class APawn;
class UTextRenderComponent;

// M2 consumable fixture. No inventory or client-supplied recovery quantities.
UCLASS(Blueprintable)
class PRIMALFRONTIER_API APFRecoveryPickup : public AActor
{
    GENERATED_BODY()
public:
    APFRecoveryPickup();
    bool TryConsume(APawn* Consumer);
    UFUNCTION(BlueprintPure, Category="Survival") float GetRemainingFreshSeconds() const;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Survival", meta=(ClampMin="1", ClampMax="86400")) float ShelfLifeSeconds = 300.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Survival", meta=(ClampMin="0", ClampMax="100")) float FoodRecovery = 35.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Survival", meta=(ClampMin="0", ClampMax="100")) float WaterRecovery = 35.f;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
private:
    bool bConsumed = false;
    UPROPERTY(Replicated) double ExpiresAtServerTime = 0;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UTextRenderComponent> Label;
    double GetServerTime() const;
};
