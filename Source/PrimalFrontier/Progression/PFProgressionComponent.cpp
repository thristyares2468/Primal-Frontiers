#include "Progression/PFProgressionComponent.h"
#include "Progression/PFProgressionCatalog.h"
#include "Inventory/PFInventoryPlayerState.h"
#include "Inventory/PFInventoryComponent.h"
#include "Inventory/PFItemCatalog.h"
#include "Crafting/PFCraftingComponent.h"
#include "Crafting/PFCraftingCatalog.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

UPFProgressionComponent::UPFProgressionComponent()
{
    SetIsReplicatedByDefault(true);PrimaryComponentTick.bCanEverTick=false;
}
int32 UPFProgressionComponent::GetLevel() const { return FPFProgressionTransactions::LevelForExperience(Record.Experience); }
int32 UPFProgressionComponent::GetAvailablePoints() const { return FPFProgressionTransactions::AvailablePoints(Record,*GetDefault<UPFProgressionCatalog>()); }
bool UPFProgressionComponent::Authority(const UPFCraftingCatalog*& Crafting,const UPFItemCatalog*& Items,FString& Error) const
{
    const auto* PS=Cast<APFInventoryPlayerState>(GetOwner());
    if(!IsInGameThread() || !PS || !PS->HasAuthority() || !GetWorld() || !GetWorld()->IsGameWorld() || GetWorld()->GetNetMode()==NM_Client || !PS->Crafting || !PS->Inventory || !PS->Crafting->Catalog || !PS->Inventory->Catalog)
    {Error=TEXT("Requires authoritative PlayerState and loaded crafting/item catalogs");return false;}
    Crafting=PS->Crafting->Catalog;Items=PS->Inventory->Catalog;return true;
}
bool UPFProgressionComponent::CanRestore(const FPFProgressionRecord& Candidate,FString& Error) const
{
    const UPFCraftingCatalog* Crafting=nullptr;const UPFItemCatalog* Items=nullptr;
    return Authority(Crafting,Items,Error) && FPFProgressionTransactions::Validate(Candidate,*GetDefault<UPFProgressionCatalog>(),*Crafting,*Items,Error);
}
bool UPFProgressionComponent::Capture(FPFProgressionRecord& Out,FString& Error) const
{
    const UPFCraftingCatalog* Crafting=nullptr;const UPFItemCatalog* Items=nullptr;
    if(!Authority(Crafting,Items,Error)){return false;}
    const double Now=GetWorld()->GetTimeSeconds();
    if(!FMath::IsFinite(Now) || !FMath::IsFinite(RecordTime) || Now<RecordTime || Now<0)
    {Error=TEXT("Invalid or backward progression clock");return false;}
    auto Candidate=Record;
    if(!FPFProgressionTransactions::AdvanceGatherWindows(Candidate,Now-RecordTime,*GetDefault<UPFProgressionCatalog>(),*Crafting,*Items,Error)){return false;}
    Out=MoveTemp(Candidate);return true;
}
bool UPFProgressionComponent::Restore(const FPFProgressionRecord& Candidate,FString& Error)
{
    if(!CanRestore(Candidate,Error)){return false;}
    const double Now=GetWorld()->GetTimeSeconds();
    if(!FMath::IsFinite(Now) || Now<0){Error=TEXT("Invalid progression restore clock");return false;}
    Record=Candidate;RecordTime=Now;GetOwner()->ForceNetUpdate();return true;
}
bool UPFProgressionComponent::CanCraftRecipe(FName Recipe,FString& Error) const
{
    const auto* PS=Cast<APFInventoryPlayerState>(GetOwner());
    const auto* Catalog=GetDefault<UPFProgressionCatalog>();
    if(!IsInGameThread() || !PS || !PS->Crafting || !PS->Inventory || !PS->Crafting->Catalog || !PS->Inventory->Catalog)
    {Error=TEXT("Progression catalogs unavailable");return false;}
    if(!FPFProgressionTransactions::Validate(Record,*Catalog,*PS->Crafting->Catalog,*PS->Inventory->Catalog,Error)){return false;}
    if(!PS->Crafting->Catalog->Recipe(Recipe,PS->Inventory->Catalog)){Error=TEXT("Unknown or invalid recipe");return false;}
    for(const auto& D:Catalog->Knowledge)
    {
        if(D.Recipes.Contains(Recipe) && !Record.Knowledge.Contains(D.Id))
        {Error=FString::Printf(TEXT("Locked: learn %s (level %d, %d points)"),*D.DisplayName.ToString(),D.MinimumLevel,D.PointCost);return false;}
    }
    Error.Reset();return true;
}
bool UPFProgressionComponent::RequestKnowledge(FName Id,APawn* Pawn,FString& Error)
{
    const UPFCraftingCatalog* Crafting=nullptr;const UPFItemCatalog* Items=nullptr;
    if(!Authority(Crafting,Items,Error)){return false;} // Client calls never alter even feedback.
    const auto* V=IsValid(Pawn)?Pawn->FindComponentByClass<UPFPlayerSurvivalComponent>():nullptr;
    const auto* Controller=V?Pawn->GetController():nullptr;
    auto Refuse=[&](const TCHAR* Why){Error=Why;KnowledgeFeedback=FString(TEXT("Refused: "))+Why;GetOwner()->ForceNetUpdate();return false;};
    if(!V || V->IsDead() || !Pawn->HasAuthority() || Pawn->GetWorld()!=GetWorld() || !Controller ||
        !Controller->HasAuthority() || Controller->GetPawn()!=Pawn || Pawn->GetPlayerState()!=GetOwner() || Controller->PlayerState!=GetOwner())
    {return Refuse(TEXT("requires your living authoritative survivor"));}
    const double Now=GetWorld()->GetTimeSeconds();
    if(Now<NextKnowledgeRequestTime){return Refuse(TEXT("knowledge request cooldown"));}
    NextKnowledgeRequestTime=Now+0.25; // Active server-world time, unrelated to time-of-day commands.
    FPFProgressionRecord Candidate;
    if(!Capture(Candidate,Error) || !FPFProgressionTransactions::Purchase(Candidate,Id,*GetDefault<UPFProgressionCatalog>(),*Crafting,*Items,Error))
    {
        KnowledgeFeedback=FString(TEXT("Refused: "))+Error;GetOwner()->ForceNetUpdate();
        UE_LOG(LogPFSurvival,Display,TEXT("[PrimalProgression] Knowledge request refused; record unchanged"));return false;
    }
    Record=MoveTemp(Candidate);RecordTime=Now;KnowledgeFeedback=FString::Printf(TEXT("Learned %s; recipe access available"),*GetDefault<UPFProgressionCatalog>()->Find(Id)->DisplayName.ToString());
    GetOwner()->ForceNetUpdate();
    UE_LOG(LogPFSurvival,Display,TEXT("[PrimalProgression] Knowledge learned; experience=%d level=%d points=%d"),GetExperience(),GetLevel(),GetAvailablePoints());
    Error.Reset();return true;
}
bool UPFProgressionComponent::PrepareCompletedCraft(FName Recipe,FPFProgressionRecord& Candidate,FString& Error) const
{
    const UPFCraftingCatalog* Crafting=nullptr;const UPFItemCatalog* Items=nullptr;
    if(!Authority(Crafting,Items,Error) || !CanCraftRecipe(Recipe,Error)){return false;}
    if(!Capture(Candidate,Error)){return false;}
    if(Candidate.CreditedCrafts.Contains(Recipe)){return true;} // Valid repeat crafts still produce items, never repeated XP.
    return FPFProgressionTransactions::CreditFirstCraft(Candidate,Recipe,*GetDefault<UPFProgressionCatalog>(),*Crafting,*Items,Error);
}
void UPFProgressionComponent::CommitCompletedCraft(const FPFProgressionRecord& Candidate)
{
    const bool FirstCraft=Record.CreditedCrafts!=Candidate.CreditedCrafts;
    if(Record.Experience!=Candidate.Experience || FirstCraft || Record.GatherWindows!=Candidate.GatherWindows)
    {
        Record=Candidate;RecordTime=GetWorld()->GetTimeSeconds();GetOwner()->ForceNetUpdate();
        if(FirstCraft){UE_LOG(LogPFSurvival,Display,TEXT("[PrimalProgression] Completed first craft; experience=%d level=%d points=%d"),GetExperience(),GetLevel(),GetAvailablePoints());}
    }
}
void UPFProgressionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{Super::GetLifetimeReplicatedProps(OutLifetimeProps);DOREPLIFETIME_CONDITION(UPFProgressionComponent,Record,COND_OwnerOnly);DOREPLIFETIME_CONDITION(UPFProgressionComponent,KnowledgeFeedback,COND_OwnerOnly);}
