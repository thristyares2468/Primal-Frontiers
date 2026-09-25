#pragma once
#include "GameFramework/Actor.h"
#include "Inventory/PFInventoryComponent.h"
#include "PFItemPickup.generated.h"
class UTextRenderComponent;
UCLASS(Blueprintable)
class PRIMALFRONTIER_API APFItemPickup : public AActor
{
    GENERATED_BODY()
public:
    APFItemPickup();
    UPROPERTY(EditAnywhere,BlueprintReadOnly) FName ItemId=TEXT("Item_Wood");
    UPROPERTY(EditAnywhere,BlueprintReadOnly) int32 Quantity=5;
    void Initialize(FName Id,int32 Count,double Deadline);
    bool TryPickup(APawn* Pawn);
    const FPFItemStack& GetContents() const {return Contents;}
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
protected:
    virtual void BeginPlay() override;
    virtual void Tick(float Delta) override;
private:
    UPROPERTY(Replicated) FPFItemStack Contents;
    UPROPERTY() TObjectPtr<UTextRenderComponent> Label;
    bool bInitialized=false;
    bool bTaken=false;
};
