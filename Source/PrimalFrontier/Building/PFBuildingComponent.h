#pragma once
#include "Components/ActorComponent.h"
#include "Building/PFBuildingCatalog.h"
#include "PFBuildingComponent.generated.h"
class APFBuildPiece;
class APFSurvivalPlayerController;
UCLASS(ClassGroup=(PrimalFrontier),meta=(BlueprintSpawnableComponent))
class PRIMALFRONTIER_API UPFBuildingComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UPFBuildingComponent();
    UPROPERTY(EditAnywhere,BlueprintReadOnly) TObjectPtr<UPFBuildingCatalog> Catalog;
    UPROPERTY(Replicated,BlueprintReadOnly) TObjectPtr<APFBuildPiece> OpenStorage;
    bool bBuildMode=false;
    int32 Selection=0;
    int32 Rotation=0;
    FString PreviewMessage;
    FString Feedback;
    FName SelectedId() const;
    bool Candidate(FName Id,int32 QuarterTurns,FTransform& Out,APFBuildPiece*& Parent,FString& Reason) const;
    APFBuildPiece* Place(FName Id,int32 QuarterTurns);
    bool Demolish(APFBuildPiece* Piece);
    bool Interact(APFBuildPiece* Piece);
    bool Transfer(bool bDeposit,FGuid StackId,int32 Quantity);
    APFBuildPiece* TracedPiece() const;
    APFSurvivalPlayerController* Controller() const;
    UFUNCTION(Server,Reliable) void ServerPlace(FName Id,int32 QuarterTurns);
    UFUNCTION(Server,Reliable) void ServerTargetAction(uint8 Action);
    UFUNCTION(Server,Reliable) void ServerTransfer(bool bDeposit,FGuid StackId,int32 Quantity);
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float Delta,ELevelTick Type,FActorComponentTickFunction* Tick) override;
private:
    double NextRequest=0;
    bool Living() const;
    bool InReach(APFBuildPiece* Piece) const;
    bool RateLimit();
    UFUNCTION(Client,Reliable) void ClientResult(const FString& Message);
};
