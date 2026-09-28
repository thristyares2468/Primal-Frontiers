#pragma once
#include "Components/ActorComponent.h"
#include "Crafting/PFCraftingCatalog.h"
#include "Inventory/PFInventoryComponent.h"
#include "PFCraftingComponent.generated.h"
class APawn;
UCLASS(ClassGroup=(PrimalFrontier),meta=(BlueprintSpawnableComponent))
class PRIMALFRONTIER_API UPFCraftingComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UPFCraftingComponent();
    UPROPERTY(EditAnywhere,BlueprintReadOnly) TObjectPtr<UPFCraftingCatalog> Catalog;
    UPROPERTY(Replicated,BlueprintReadOnly) FName ActiveRecipe;
    UPROPERTY(Replicated,BlueprintReadOnly) double FinishAt=0;
    UPROPERTY(Replicated,BlueprintReadOnly) FString Feedback;
    bool Start(FName RecipeId,APawn* Pawn);
    bool Cancel();
    UPFInventoryComponent* Inventory() const;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float Delta,ELevelTick Type,FActorComponentTickFunction* Tick) override;
private:
    FPFRecipeDefinition PendingRecipe;
    TArray<FPFItemStack> Inputs;
    TWeakObjectPtr<APawn> CraftPawn;
    bool ValidPawn(APawn* Pawn) const;
    void Finish(const FString& Message);
};
