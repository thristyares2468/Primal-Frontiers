#include "Progression/PFProgressionRecord.h"
#include "Progression/PFProgressionCatalog.h"
#include "Crafting/PFCraftingCatalog.h"
#include "Inventory/PFItemCatalog.h"
#include "NativeGameplayTags.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PF_GatherWood,"Progression.Gather.Wood");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PF_GatherStone,"Progression.Gather.Stone");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PF_GatherFibre,"Progression.Gather.Fibre");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PF_GatherFood,"Progression.Gather.Food");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PF_GatherWater,"Progression.Gather.Water");

namespace {bool RecordRefuse(FString& Error,const TCHAR* Reason){Error=Reason;return false;}}
TArray<FGameplayTag> FPFProgressionTransactions::GatherCategories()
{return {TAG_PF_GatherWood,TAG_PF_GatherStone,TAG_PF_GatherFibre,TAG_PF_GatherFood,TAG_PF_GatherWater};}
FGameplayTag FPFProgressionTransactions::GatherCategoryForItem(FName Item)
{
    if(Item==TEXT("Item_Wood")){return TAG_PF_GatherWood;}if(Item==TEXT("Item_Stone")){return TAG_PF_GatherStone;}
    if(Item==TEXT("Item_Fibre")){return TAG_PF_GatherFibre;}if(Item==TEXT("Item_Food")){return TAG_PF_GatherFood;}
    if(Item==TEXT("Item_Water")){return TAG_PF_GatherWater;}return {};
}
int32 FPFProgressionTransactions::ExperienceForLevel(int32 Level)
{
    const int32 Steps=FMath::Clamp(Level,1,10)-1;
    return 100*Steps+25*Steps*(Steps-1);
}
int32 FPFProgressionTransactions::LevelForExperience(int32 Experience)
{
    int32 Level=1;
    for(int32 L=2;L<=10;++L){if(Experience<ExperienceForLevel(L)){break;}++Level;}
    return Level;
}
int32 FPFProgressionTransactions::AvailablePoints(const FPFProgressionRecord& Record,const UPFProgressionCatalog& Catalog)
{
    if(Record.Experience<0 || Record.Experience>MaximumExperience || Record.Knowledge.Num()>UPFProgressionCatalog::MaximumKnowledge){return INDEX_NONE;}
    int32 Spent=0;TSet<FName> Seen;
    for(FName Id:Record.Knowledge){const auto* D=Catalog.Find(Id);if(!D || Seen.Contains(Id) || D->PointCost<1 || D->PointCost>27){return INDEX_NONE;}Seen.Add(Id);Spent+=D->PointCost;}
    return 3*(LevelForExperience(Record.Experience)-1)-Spent;
}
bool FPFProgressionTransactions::Validate(const FPFProgressionRecord& R,const UPFProgressionCatalog& Catalog,
    const UPFCraftingCatalog& Crafting,const UPFItemCatalog& Items,FString& Error)
{
    if(!Catalog.Validate(Crafting,Items,Error)){return false;}
    if(R.Experience<0 || R.Experience>MaximumExperience || R.Knowledge.Num()>Catalog.Knowledge.Num() || R.CreditedCrafts.Num()>UPFProgressionCatalog::MaximumCraftRecords || R.GatherWindows.Num()>MaximumGatherCategories)
    {return RecordRefuse(Error,TEXT("Progression record exceeds bounded limits"));}
    TSet<FName> Known,Crafts;
    for(FName Id:R.Knowledge){const auto* D=Catalog.Find(Id);if(!D || Known.Contains(Id) || D->MinimumLevel>LevelForExperience(R.Experience)){return RecordRefuse(Error,TEXT("Invalid, duplicate or premature player knowledge"));}Known.Add(Id);}
    for(FName Id:R.Knowledge){for(FName P:Catalog.Find(Id)->Prerequisites){if(!Known.Contains(P)){return RecordRefuse(Error,TEXT("Player knowledge missing prerequisite"));}}}
    for(FName Id:R.CreditedCrafts){if(Crafts.Contains(Id) || !Crafting.Recipe(Id,&Items)){return RecordRefuse(Error,TEXT("Invalid or duplicate craft reward ledger"));}Crafts.Add(Id);}
    const auto Categories=GatherCategories();TSet<FGameplayTag> Gathered;int32 GatherCredits=0;
    for(const auto& W:R.GatherWindows)
    {
        if(!Categories.Contains(W.Category) || Gathered.Contains(W.Category) || W.Rewards<1 || W.Rewards>MaximumGatherRewards ||
            !FMath::IsFinite(W.RemainingSeconds) || W.RemainingSeconds<=0 || W.RemainingSeconds>GatherWindowSeconds)
        {return RecordRefuse(Error,TEXT("Invalid, duplicate or unbounded gather reward window"));}
        Gathered.Add(W.Category);GatherCredits+=W.Rewards;
    }
    if(R.Experience<FMath::Min(MaximumExperience,R.CreditedCrafts.Num()*FirstCraftExperience+GatherCredits*GatherExperience) || AvailablePoints(R,Catalog)<0)
    {return RecordRefuse(Error,TEXT("Progression experience or point accounting is inconsistent"));}
    Error.Reset();return true;
}
bool FPFProgressionTransactions::CreditGather(FPFProgressionRecord& R,FGameplayTag Category,const UPFProgressionCatalog& Catalog,
    const UPFCraftingCatalog& Crafting,const UPFItemCatalog& Items,FString& Error)
{
    if(!Validate(R,Catalog,Crafting,Items,Error)){return false;}
    if(!GatherCategories().Contains(Category) || R.Experience==MaximumExperience){return RecordRefuse(Error,TEXT("Unknown gather category or experience cap reached"));}
    auto Candidate=R;auto* Window=Candidate.GatherWindows.FindByPredicate([Category](const auto& W){return W.Category==Category;});
    if(Window && Window->Rewards==MaximumGatherRewards){return RecordRefuse(Error,TEXT("Gather reward window exhausted"));}
    if(!Window){FPFGatherRewardWindow New;New.Category=Category;New.RemainingSeconds=GatherWindowSeconds;Candidate.GatherWindows.Add(New);Window=&Candidate.GatherWindows.Last();}
    ++Window->Rewards;Candidate.Experience=FMath::Min(MaximumExperience,Candidate.Experience+GatherExperience);
    if(!Validate(Candidate,Catalog,Crafting,Items,Error)){return false;}R=MoveTemp(Candidate);return true;
}
bool FPFProgressionTransactions::AdvanceGatherWindows(FPFProgressionRecord& R,double ActiveSeconds,const UPFProgressionCatalog& Catalog,
    const UPFCraftingCatalog& Crafting,const UPFItemCatalog& Items,FString& Error)
{
    if(!Validate(R,Catalog,Crafting,Items,Error)){return false;}
    if(!FMath::IsFinite(ActiveSeconds) || ActiveSeconds<0){return RecordRefuse(Error,TEXT("Invalid active gather elapsed time"));}
    auto Candidate=R;
    for(int32 I=Candidate.GatherWindows.Num()-1;I>=0;--I)
    {if(ActiveSeconds>=Candidate.GatherWindows[I].RemainingSeconds){Candidate.GatherWindows.RemoveAt(I);}else{Candidate.GatherWindows[I].RemainingSeconds-=ActiveSeconds;}}
    if(!Validate(Candidate,Catalog,Crafting,Items,Error)){return false;}R=MoveTemp(Candidate);return true;
}
bool FPFProgressionTransactions::AwardExperience(FPFProgressionRecord& R,int32 Amount,const UPFProgressionCatalog& Catalog,
    const UPFCraftingCatalog& Crafting,const UPFItemCatalog& Items,FString& Error)
{
    if(!Validate(R,Catalog,Crafting,Items,Error)){return false;}
    if(Amount<=0 || Amount>MaximumExperience || R.Experience==MaximumExperience){return RecordRefuse(Error,TEXT("Invalid XP amount or level cap reached"));}
    auto Candidate=R;Candidate.Experience=FMath::Min(MaximumExperience,Candidate.Experience+Amount);
    if(!Validate(Candidate,Catalog,Crafting,Items,Error)){return false;}R=MoveTemp(Candidate);return true;
}
bool FPFProgressionTransactions::CreditFirstCraft(FPFProgressionRecord& R,FName Recipe,const UPFProgressionCatalog& Catalog,
    const UPFCraftingCatalog& Crafting,const UPFItemCatalog& Items,FString& Error)
{
    if(!Validate(R,Catalog,Crafting,Items,Error)){return false;}
    if(!Crafting.Recipe(Recipe,&Items) || R.CreditedCrafts.Contains(Recipe) || R.CreditedCrafts.Num()==UPFProgressionCatalog::MaximumCraftRecords)
    {return RecordRefuse(Error,TEXT("Unknown or already credited recipe"));}
    auto Candidate=R;Candidate.Experience=FMath::Min(MaximumExperience,Candidate.Experience+FirstCraftExperience);Candidate.CreditedCrafts.Add(Recipe);
    if(!Validate(Candidate,Catalog,Crafting,Items,Error)){return false;}R=MoveTemp(Candidate);return true;
}
bool FPFProgressionTransactions::Purchase(FPFProgressionRecord& R,FName Id,const UPFProgressionCatalog& Catalog,
    const UPFCraftingCatalog& Crafting,const UPFItemCatalog& Items,FString& Error)
{
    if(!Validate(R,Catalog,Crafting,Items,Error)){return false;}const auto* D=Catalog.Find(Id);
    if(!D || R.Knowledge.Contains(Id) || LevelForExperience(R.Experience)<D->MinimumLevel || AvailablePoints(R,Catalog)<D->PointCost)
    {return RecordRefuse(Error,TEXT("Unknown, owned, unaffordable or premature knowledge"));}
    for(FName P:D->Prerequisites){if(!R.Knowledge.Contains(P)){return RecordRefuse(Error,TEXT("Knowledge prerequisite not learned"));}}
    auto Candidate=R;Candidate.Knowledge.Add(Id);if(!Validate(Candidate,Catalog,Crafting,Items,Error)){return false;}R=MoveTemp(Candidate);return true;
}
