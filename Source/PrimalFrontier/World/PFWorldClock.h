#pragma once
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "PFWorldClock.generated.h"
class UDirectionalLightComponent;

UCLASS(Blueprintable)
class PRIMALFRONTIER_API APFWorldClock : public AActor
{
    GENERATED_BODY()
public:
    APFWorldClock();
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="World", meta=(ClampMin="60",ClampMax="86400")) float DayLengthSeconds=900;
    UPROPERTY(ReplicatedUsing=OnRep_Hour, BlueprintReadOnly) float Hour=9;
    UPROPERTY(Replicated, BlueprintReadOnly) FGameplayTag Phase;
    bool SetHour(float NewHour);
    void Advance(float Seconds);
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
private:
    UPROPERTY() TObjectPtr<UDirectionalLightComponent> Sun;
    UPROPERTY() TObjectPtr<UDirectionalLightComponent> NightFill;
    UFUNCTION() void OnRep_Hour();
    void UpdatePhase();
};
