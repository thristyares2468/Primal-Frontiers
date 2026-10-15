#pragma once
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "PFCraftingStation.generated.h"
class APawn;
class UStaticMeshComponent;
class UTextRenderComponent;
struct FPFRecipeDefinition;

/** Public, stateless greybox camp station. Player-built/owned stations are separate future work. */
UCLASS()
class PRIMALFRONTIER_API APFCraftingStation : public AActor
{
    GENERATED_BODY()
public:
    APFCraftingStation();
    UPROPERTY(EditAnywhere,Replicated,BlueprintReadOnly) FGameplayTag StationType;
    UPROPERTY(EditAnywhere,Replicated,BlueprintReadOnly) float Radius=250;
    UPROPERTY(EditAnywhere,Replicated,BlueprintReadOnly) float SpeedMultiplier=1.25f;
    UFUNCTION(BlueprintPure) static FGameplayTag WorkbenchType();
    UFUNCTION(BlueprintPure) static FGameplayTag RefineryType();
    FString DisplayName() const;
    bool Supports(const FPFRecipeDefinition& Recipe) const;
    bool CanUse(const FPFRecipeDefinition& Recipe,const APawn* Pawn,bool RequireAuthority) const;
    static APFCraftingStation* FindBest(const FPFRecipeDefinition& Recipe,const APawn* Pawn,bool RequireAuthority);
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
protected:
    virtual void BeginPlay() override;
private:
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Mesh;
    UPROPERTY() TObjectPtr<UTextRenderComponent> Label;
};
