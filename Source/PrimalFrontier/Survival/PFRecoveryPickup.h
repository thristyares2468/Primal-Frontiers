// PFRecoveryPickup.h
//
// M2 single-use "ration" placed in maps: pressing E near it restores a fixed
// amount of food and water, then it disappears. It predates the inventory (M3):
// it is eaten directly from the world rather than picked up.
//
// Rations are perishable: the server sets an absolute expiry deadline at spawn
// and replicates it so every client shows the same countdown. Expired rations
// are refused and removed. Clients never supply recovery amounts or deadlines.
//
// History: M2 (8be2a14). Docs: Docs/SURVIVAL_M2.md, Docs/FOOD_AND_PRESERVATION.md

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

    /** Server: eat the ration if Consumer is alive, within 2.5 m, has a clear line of
     *  sight, isn't already full, and the ration is fresh. Destroys it on success. */
    bool TryConsume(APawn* Consumer);

    /** Seconds until expiry (0 when spoiled), using synchronized server time. */
    UFUNCTION(BlueprintPure, Category="Survival") float GetRemainingFreshSeconds() const;

    /** Lifetime from spawn. 300 s is short test tuning so expiry is observable. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Survival", meta=(ClampMin="1", ClampMax="86400")) float ShelfLifeSeconds = 300.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Survival", meta=(ClampMin="0", ClampMax="100")) float FoodRecovery = 35.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Survival", meta=(ClampMin="0", ClampMax="100")) float WaterRecovery = 35.f;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
    /** Server: fix the expiry deadline. */
    virtual void BeginPlay() override;
    /** 1 Hz: server removes spoiled rations; clients update the countdown label. */
    virtual void Tick(float DeltaSeconds) override;

private:
    bool bConsumed = false;   // guards against double consumption before Destroy completes
    /** Absolute server world time of expiry (GameState::GetServerWorldTimeSeconds). */
    UPROPERTY(Replicated) double ExpiresAtServerTime = 0;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UTextRenderComponent> Label;
    /** Server-synchronized world time (falls back to local time without a GameState). */
    double GetServerTime() const;
};
