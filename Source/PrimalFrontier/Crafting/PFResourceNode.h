#pragma once
#include "GameFramework/Actor.h"
#include "PFResourceNode.generated.h"
class UPFCraftingCatalog;
class UPFItemCatalog;
class UStaticMeshComponent;
class UTextRenderComponent;
class APawn;
UCLASS()
class PRIMALFRONTIER_API APFResourceNode : public AActor
{
    GENERATED_BODY()
public:
    APFResourceNode();
    UPROPERTY(EditAnywhere,Replicated,BlueprintReadOnly) FName ResourceId=TEXT("Node_Wood");
    UPROPERTY(EditAnywhere,BlueprintReadOnly) TObjectPtr<UPFCraftingCatalog> Catalog;
    UPROPERTY(EditAnywhere,BlueprintReadOnly) TObjectPtr<UPFItemCatalog> Items;
    UPROPERTY(Replicated,BlueprintReadOnly) int32 HitsRemaining=0;
    UPROPERTY(Replicated,BlueprintReadOnly) double RespawnAt=0;
    bool Gather(APawn* Pawn);
    bool IsConfigurationValid() const;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
protected:
    virtual void BeginPlay() override;
    virtual void Tick(float Delta) override;
private:
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Mesh;
    UPROPERTY() TObjectPtr<UTextRenderComponent> Label;
    double NextHitAt=0;
};
