#pragma once
#include "Blueprint/UserWidget.h"
#include "PFSurvivalHUD.generated.h"
class UTextBlock;
class UProgressBar;

UCLASS()
class PRIMALFRONTIER_API UPFSurvivalHUD : public UUserWidget
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintImplementableEvent, Category="Survival|UI") void PresentVitals(float Health, float MaxHealth, float Stamina, float MaxStamina, bool bDead);
    UFUNCTION(BlueprintImplementableEvent, Category="Survival|UI") void PresentNeeds(float Hunger, float Thirst, float Exposure);
protected:
    virtual void NativeOnInitialized() override;
    virtual void NativeTick(const FGeometry& Geometry, float DeltaTime) override;
private:
    UPROPERTY(Transient) TObjectPtr<UTextBlock> HealthLabel;
    UPROPERTY(Transient) TObjectPtr<UTextBlock> StaminaLabel;
    UPROPERTY(Transient) TObjectPtr<UTextBlock> StateLabel;
    UPROPERTY(Transient) TObjectPtr<UProgressBar> HealthBar;
    UPROPERTY(Transient) TObjectPtr<UProgressBar> StaminaBar;
    UPROPERTY(Transient) TObjectPtr<UTextBlock> NeedsLabel;
    UPROPERTY(Transient) TObjectPtr<UProgressBar> HungerBar;
    UPROPERTY(Transient) TObjectPtr<UProgressBar> ThirstBar;
};
