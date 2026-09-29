#pragma once
#include "GameFramework/Actor.h"
#include "Building/PFBuildingCatalog.h"
#include "PFBuildPiece.generated.h"
class UStaticMeshComponent;
class UPFInventoryComponent;
class APlayerState;
UCLASS()
class PRIMALFRONTIER_API APFBuildPiece : public AActor
{
    GENERATED_BODY()
public:
    APFBuildPiece();
    UPROPERTY(ReplicatedUsing=OnRepShape,BlueprintReadOnly) EPFBuildKind Kind=EPFBuildKind::Foundation;
    UPROPERTY(Replicated,BlueprintReadOnly) FName DefinitionId;
    UPROPERTY(Replicated,BlueprintReadOnly) TObjectPtr<APlayerState> Builder;
    UPROPERTY(Replicated,BlueprintReadOnly) TObjectPtr<APFBuildPiece> Support;
    UPROPERTY(Replicated,BlueprintReadOnly) float Health=100;
    UPROPERTY(ReplicatedUsing=OnRepShape,BlueprintReadOnly) bool bDoorOpen=false;
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly) TObjectPtr<UPFInventoryComponent> Storage;
    void Initialize(const FPFBuildingDefinition& D,APlayerState* OwnerState,APFBuildPiece* Parent);
    bool IsPlatform() const;
    bool HasDependents() const;
    bool CanRemove() const;
    bool ToggleDoor(APlayerState* Requester);
    virtual float TakeDamage(float Amount,const FDamageEvent& Event,AController* Instigator,AActor* Causer) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
protected:
    virtual void BeginPlay() override;
private:
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Shapes;
    UFUNCTION() void OnRepShape();
};
