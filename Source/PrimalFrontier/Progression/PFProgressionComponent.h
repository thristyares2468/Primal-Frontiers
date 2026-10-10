#pragma once
#include "Components/ActorComponent.h"
#include "Progression/PFProgressionRecord.h"
#include "PFProgressionComponent.generated.h"
class UPFCraftingComponent;
class UPFCraftingCatalog;
class UPFItemCatalog;
class APawn;

/** PlayerState lifetime; trusted server persistence and actual successful craft/gather hooks. No grant RPC. */
UCLASS(ClassGroup=(PrimalFrontier))
class PRIMALFRONTIER_API UPFProgressionComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UPFProgressionComponent();
    const FPFProgressionRecord& GetRecord() const { return Record; }
    UFUNCTION(BlueprintPure,Category="Progression") int32 GetExperience() const { return Record.Experience; }
    UFUNCTION(BlueprintPure,Category="Progression") int32 GetLevel() const;
    UFUNCTION(BlueprintPure,Category="Progression") int32 GetAvailablePoints() const;
    bool Capture(FPFProgressionRecord& Out,FString& Error) const;
    bool CanRestore(const FPFProgressionRecord& Candidate,FString& Error) const;
    bool Restore(const FPFProgressionRecord& Candidate,FString& Error);
    /** Read-only owner view; Start/completion independently enforce this on the server. */
    bool CanCraftRecipe(FName Recipe,FString& Error) const;
    /** Owning live pawn only. Server catalog sets price/level; client never supplies XP or target. */
    bool RequestKnowledge(FName Knowledge,APawn* Pawn,FString& Error);
    UFUNCTION(BlueprintPure,Category="Progression") FString GetKnowledgeFeedback() const {return KnowledgeFeedback;}
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
private:
    UPROPERTY(Replicated) FPFProgressionRecord Record;
    UPROPERTY(Replicated) FString KnowledgeFeedback;
    double NextKnowledgeRequestTime=0;
    // Epoch for stored remaining durations. Capture is read-only; commits/restore rebase it.
    // Unreal game time pauses with the world and never includes offline wall-clock time.
    double RecordTime=0;
    bool Authority(const UPFCraftingCatalog*& Crafting,const UPFItemCatalog*& Items,FString& Error) const;
    // Only the actual timed crafting transaction can prepare/commit a first-craft award.
    friend class UPFCraftingComponent;
    bool PrepareCompletedCraft(FName Recipe,FPFProgressionRecord& Candidate,FString& Error) const;
    void CommitCompletedCraft(const FPFProgressionRecord& Candidate);
    // Category/yield comes from the validated server node catalog, never a client reward claim.
    friend class APFResourceNode;
    bool PrepareGather(FName YieldItem,APawn* Pawn,FPFProgressionRecord& Candidate,FString& Error) const;
    void CommitGather(const FPFProgressionRecord& Candidate);
};
