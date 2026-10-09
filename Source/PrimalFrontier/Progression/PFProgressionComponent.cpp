#include "Progression/PFProgressionComponent.h"
#include "Progression/PFProgressionCatalog.h"
#include "Inventory/PFInventoryPlayerState.h"
#include "Inventory/PFInventoryComponent.h"
#include "Inventory/PFItemCatalog.h"
#include "Crafting/PFCraftingComponent.h"
#include "Crafting/PFCraftingCatalog.h"
#include "Survival/PFPlayerSurvivalComponent.h"
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
    if(!IsInGameThread() || !PS || !PS->HasAuthority() || !GetWorld() || !GetWorld()->IsGameWorld() || !PS->Crafting || !PS->Inventory || !PS->Crafting->Catalog || !PS->Inventory->Catalog)
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
{Super::GetLifetimeReplicatedProps(OutLifetimeProps);DOREPLIFETIME_CONDITION(UPFProgressionComponent,Record,COND_OwnerOnly);}
