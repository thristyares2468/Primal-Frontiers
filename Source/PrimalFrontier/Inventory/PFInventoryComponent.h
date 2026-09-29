#pragma once
#include "Components/ActorComponent.h"
#include "PFInventoryComponent.generated.h"
class UPFItemCatalog;
class APawn;
class APFItemPickup;
struct FPFItemDefinition;

USTRUCT(BlueprintType)
struct FPFItemStack
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadOnly) FGuid StackId;
    UPROPERTY(BlueprintReadOnly) FName ItemId;
    UPROPERTY(BlueprintReadOnly) int32 Quantity=0;
    // Zero means nonperishable; otherwise an absolute server simulation-time deadline.
    UPROPERTY(BlueprintReadOnly) double ExpiresAt=0;
};

UCLASS(ClassGroup=(PrimalFrontier),meta=(BlueprintSpawnableComponent))
class PRIMALFRONTIER_API UPFInventoryComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UPFInventoryComponent();
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Inventory") TObjectPtr<UPFItemCatalog> Catalog;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Inventory") int32 SlotLimit=8;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Inventory") float WeightLimit=30;
    const TArray<FPFItemStack>& GetStacks() const { return Stacks; }
    const FPFItemDefinition* Definition(FName Id) const;
    static double ServerTime(const UWorld* World);
    float GetWeight() const;
    int32 Count(FName Id) const;
    bool Grant(FName Id,int32 Quantity);
    bool AddExisting(FName Id,int32 Quantity,double Deadline);
    bool Remove(FGuid Id,int32 Quantity);
    bool RemoveItem(FName Id,int32 Quantity);
    bool Split(FGuid Id,int32 Quantity);
    bool TransferTo(UPFInventoryComponent* Destination,FGuid Id,int32 Quantity);
    // Server-only atomic conversion of exact still-fresh input batches into recipe output.
    bool Transform(const TArray<FPFItemStack>& Inputs,FName Output,int32 Quantity);
    bool Consume(FGuid Id,APawn* Pawn);
    APFItemPickup* Drop(FGuid Id,int32 Quantity,APawn* Pawn);
    void PruneExpired();
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float Delta,ELevelTick Type,FActorComponentTickFunction* Tick) override;
private:
    UPROPERTY(Replicated) TArray<FPFItemStack> Stacks;
    bool Authority() const;
    bool OwnsLivingPawn(APawn* Pawn) const;
    void Changed(const TCHAR* Action);
};
