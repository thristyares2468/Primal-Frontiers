// PFCraftingComponent.cpp — see PFCraftingComponent.h for the craft lifecycle.

#include "Crafting/PFCraftingComponent.h"
#include "PFAssetPaths.h"
#include "Progression/PFProgressionComponent.h"
#include "Progression/PFProgressionCatalog.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

UPFCraftingComponent::UPFCraftingComponent()
{
    SetIsReplicatedByDefault(true);
    PrimaryComponentTick.bCanEverTick=true;
    PrimaryComponentTick.TickInterval=0.1f;
}

void UPFCraftingComponent::BeginPlay()
{
    Super::BeginPlay();
    if(!Catalog){Catalog=LoadObject<UPFCraftingCatalog>(nullptr,PFAssetPaths::CraftingCatalog);}
    SetComponentTickEnabled(GetOwner()->HasAuthority());
}

void UPFCraftingComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME_CONDITION(UPFCraftingComponent,ActiveRecipe,COND_OwnerOnly);
    DOREPLIFETIME_CONDITION(UPFCraftingComponent,FinishAt,COND_OwnerOnly);
    DOREPLIFETIME_CONDITION(UPFCraftingComponent,Feedback,COND_OwnerOnly);
}

UPFInventoryComponent* UPFCraftingComponent::Inventory() const {return GetOwner()->FindComponentByClass<UPFInventoryComponent>();}

bool UPFCraftingComponent::ValidPawn(APawn* Pawn) const
{
    const auto* V=IsValid(Pawn)?Pawn->FindComponentByClass<UPFPlayerSurvivalComponent>():nullptr;
    return GetOwner()->HasAuthority() && V && !V->IsDead() && Pawn->HasAuthority() && Pawn->GetController() && Pawn->GetPlayerState()==GetOwner();
}

bool UPFCraftingComponent::Start(FName Id,APawn* Pawn)
{
    if(!ValidPawn(Pawn) || !ActiveRecipe.IsNone()){return false;}  // one job at a time
    auto* I=Inventory();
    const auto* Recipe=I && Catalog ? Catalog->Recipe(Id,I->Catalog):nullptr;
    if(!Recipe){Finish(TEXT("Refused: unknown or invalid recipe"));return false;}
    auto* Progression=GetOwner()->FindComponentByClass<UPFProgressionComponent>();FString AccessError;
    const bool Optional=GetDefault<UPFProgressionCatalog>()->Knowledge.ContainsByPredicate([Id](const auto& D){return D.Recipes.Contains(Id);});
    if((Progression && !Progression->CanCraftRecipe(Id,AccessError)) || (!Progression && Optional))
    {Finish(AccessError.IsEmpty()?TEXT("Locked: progression unavailable"):AccessError);return false;}
    I->PruneExpired();
    TArray<FPFItemStack> Selected;

    // Reserve inputs from the soonest-expiring stacks first, so older food gets used up.
    auto Sorted=I->GetStacks();
    Sorted.Sort([](const auto& A,const auto& B){return A.ExpiresAt<B.ExpiresAt;});
    for(const auto& Need:Recipe->Ingredients)
    {
        int32 Left=Need.Quantity;
        for(const auto& S:Sorted)
        {
            if(S.ItemId==Need.ItemId && Left>0)
            {
                auto Copy=S;
                Copy.Quantity=FMath::Min(Left,S.Quantity);
                Selected.Add(Copy);
                Left-=Copy.Quantity;
            }
        }
        if(Left>0){Finish(TEXT("Refused: insufficient fresh ingredients"));return false;}
    }

    PendingRecipe=*Recipe;
    Inputs=MoveTemp(Selected);
    CraftPawn=Pawn;
    ActiveRecipe=Id;
    FinishAt=UPFInventoryComponent::ServerTime(GetWorld())+Recipe->Duration;
    Feedback=TEXT("Crafting: keep ingredients in your bag until complete");
    GetOwner()->ForceNetUpdate();
    UE_LOG(LogPFSurvival,Display,TEXT("[PrimalCrafting] Started %s owner=%s finish=%.2f"),*Id.ToString(),*GetOwner()->GetName(),FinishAt);
    return true;
}

bool UPFCraftingComponent::Cancel()
{
    if(!GetOwner()->HasAuthority() || ActiveRecipe.IsNone()){return false;}
    Finish(TEXT("Cancelled; ingredients were not consumed"));
    return true;
}

void UPFCraftingComponent::Finish(const FString& Message)
{
    UE_LOG(LogPFSurvival,Display,TEXT("[PrimalCrafting] %s recipe=%s owner=%s"),*Message,*ActiveRecipe.ToString(),*GetOwner()->GetName());
    ActiveRecipe=NAME_None;
    FinishAt=0;
    Inputs.Reset();
    CraftPawn.Reset();
    Feedback=Message;
    GetOwner()->ForceNetUpdate();
}

void UPFCraftingComponent::TickComponent(float Delta,ELevelTick Type,FActorComponentTickFunction* Tick)
{
    Super::TickComponent(Delta,Type,Tick);
    if(!GetOwner()->HasAuthority() || ActiveRecipe.IsNone()){return;}
    // Death or respawn (new pawn) cancels the job.
    if(!ValidPawn(CraftPawn.Get())){Finish(TEXT("Cancelled: survivor died or changed"));return;}
    if(UPFInventoryComponent::ServerTime(GetWorld())<FinishAt){return;}
    // Atomic conversion of the exact reserved batches; fails cleanly if anything changed.
    auto* I=Inventory();
    auto* Progression=GetOwner()->FindComponentByClass<UPFProgressionComponent>();FPFProgressionRecord Reward;FString Error;
    // Validate the prospective reward before consuming inputs. Commit it only after actual item conversion.
    const bool RewardReady=!Progression || Progression->PrepareCompletedCraft(ActiveRecipe,Reward,Error);
    if(!RewardReady){Finish(TEXT("Failed: recipe access or progression validation refused; no conversion"));return;}
    const bool Done=I && I->Transform(Inputs,PendingRecipe.Output,PendingRecipe.OutputQuantity);
    if(Done && Progression){Progression->CommitCompletedCraft(Reward);}
    Finish(Done?TEXT("Completed"):TEXT("Failed: ingredients changed/expired or output will not fit; no conversion"));
}
