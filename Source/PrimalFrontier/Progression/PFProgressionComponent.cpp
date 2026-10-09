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
{if(!CanRestore(Record,Error)){return false;}Out=Record;return true;}
bool UPFProgressionComponent::Restore(const FPFProgressionRecord& Candidate,FString& Error)
{if(!CanRestore(Candidate,Error)){return false;}Record=Candidate;GetOwner()->ForceNetUpdate();return true;}
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
    auto Candidate=Record;
    if(!FPFProgressionTransactions::Purchase(Candidate,Id,*GetDefault<UPFProgressionCatalog>(),*Crafting,*Items,Error))
    {
        KnowledgeFeedback=FString(TEXT("Refused: "))+Error;GetOwner()->ForceNetUpdate();
        UE_LOG(LogPFSurvival,Display,TEXT("[PrimalProgression] Knowledge request refused; record unchanged"));return false;
    }
    Record=MoveTemp(Candidate);KnowledgeFeedback=FString::Printf(TEXT("Learned %s; recipe access integration pending"),*GetDefault<UPFProgressionCatalog>()->Find(Id)->DisplayName.ToString());
    GetOwner()->ForceNetUpdate();
    UE_LOG(LogPFSurvival,Display,TEXT("[PrimalProgression] Knowledge learned; experience=%d level=%d points=%d"),GetExperience(),GetLevel(),GetAvailablePoints());
    Error.Reset();return true;
}
bool UPFProgressionComponent::PrepareCompletedCraft(FName Recipe,FPFProgressionRecord& Candidate,FString& Error) const
{
    const UPFCraftingCatalog* Crafting=nullptr;const UPFItemCatalog* Items=nullptr;
    if(!Authority(Crafting,Items,Error) || !FPFProgressionTransactions::Validate(Record,*GetDefault<UPFProgressionCatalog>(),*Crafting,*Items,Error)){return false;}
    Candidate=Record;
    if(Record.CreditedCrafts.Contains(Recipe)){return true;} // Valid repeat crafts still produce items, never repeated XP.
    return FPFProgressionTransactions::CreditFirstCraft(Candidate,Recipe,*GetDefault<UPFProgressionCatalog>(),*Crafting,*Items,Error);
}
void UPFProgressionComponent::CommitCompletedCraft(const FPFProgressionRecord& Candidate)
{
    if(Record.Experience!=Candidate.Experience || Record.CreditedCrafts!=Candidate.CreditedCrafts)
    {Record=Candidate;GetOwner()->ForceNetUpdate();UE_LOG(LogPFSurvival,Display,TEXT("[PrimalProgression] Completed first craft; experience=%d level=%d points=%d"),GetExperience(),GetLevel(),GetAvailablePoints());}
}
void UPFProgressionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{Super::GetLifetimeReplicatedProps(OutLifetimeProps);DOREPLIFETIME_CONDITION(UPFProgressionComponent,Record,COND_OwnerOnly);DOREPLIFETIME_CONDITION(UPFProgressionComponent,KnowledgeFeedback,COND_OwnerOnly);}
