// PFSurvivalHUD.h
//
// Always-on placeholder HUD: centred aim marker, interaction prompt/feedback,
// health/stamina/food/water bars and a one-line status hint. Built in C++ from
// UMG widgets so no widget asset is needed during greybox. It only *reads* state.
//
// A Blueprint subclass can supply its own layout (then this class skips building
// the placeholder) and receive values through PresentVitals / PresentNeeds.
//
// History: M1 (f7ed11d) vitals; M2 needs; M4 aim marker; M7 prompts/feedback.

#pragma once
#include "Blueprint/UserWidget.h"
#include "PFSurvivalHUD.generated.h"
class UTextBlock;
class UProgressBar;
class APawn;

UCLASS()
class PRIMALFRONTIER_API UPFSurvivalHUD : public UUserWidget
{
    GENERATED_BODY()

public:
    /** Refreshed at 10 Hz with the possessed survivor's health/stamina. */
    UFUNCTION(BlueprintImplementableEvent, Category="Survival|UI") void PresentVitals(float Health, float MaxHealth, float Stamina, float MaxStamina, bool bDead);
    /** Refreshed at 10 Hz with food/water (0..100) and exposure (0..1). */
    UFUNCTION(BlueprintImplementableEvent, Category="Survival|UI") void PresentNeeds(float Hunger, float Thirst, float Exposure);
    /** Availability is false during missing possession; damage is an observed health drop,
     *  not a prediction or a damage-source/direction claim. Empty damage means no cue. */
    UFUNCTION(BlueprintImplementableEvent, Category="Survival|UI") void PresentStatus(bool bAvailable, const FText& Status, const FText& Damage);

protected:
    /** Build the placeholder layout unless a Blueprint child already has one. */
    virtual void NativeOnInitialized() override;
    /** Refresh text/bars from the current pawn; the prompt refreshes at 10 Hz. */
    virtual void NativeTick(const FGeometry& Geometry, float DeltaTime) override;

private:
    UPROPERTY(Transient) TObjectPtr<UTextBlock> AimMarker;
    UPROPERTY(Transient) TObjectPtr<UTextBlock> InteractionLabel;
    float InteractionRefresh=0;   // seconds since the prompt text was last rebuilt
    UPROPERTY(Transient) TObjectPtr<UTextBlock> HealthLabel;
    UPROPERTY(Transient) TObjectPtr<UTextBlock> StaminaLabel;
    UPROPERTY(Transient) TObjectPtr<UTextBlock> StateLabel;
    UPROPERTY(Transient) TObjectPtr<UTextBlock> DamageLabel;
    TWeakObjectPtr<APawn> ObservedPawn;
    float PreviousHealth=0;
    double DamageUntil=0;
    FText DamageText;
    UPROPERTY(Transient) TObjectPtr<UProgressBar> HealthBar;
    UPROPERTY(Transient) TObjectPtr<UProgressBar> StaminaBar;
    UPROPERTY(Transient) TObjectPtr<UTextBlock> NeedsLabel;
    UPROPERTY(Transient) TObjectPtr<UProgressBar> HungerBar;
    UPROPERTY(Transient) TObjectPtr<UProgressBar> ThirstBar;
};
