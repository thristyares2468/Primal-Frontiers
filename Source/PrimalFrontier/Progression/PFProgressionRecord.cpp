#include "Progression/PFProgressionRecord.h"
#include "Progression/PFProgressionCatalog.h"
#include "Crafting/PFCraftingCatalog.h"
#include "Inventory/PFItemCatalog.h"

namespace {bool RecordRefuse(FString& Error,const TCHAR* Reason){Error=Reason;return false;}}
int32 FPFProgressionTransactions::LevelForExperience(int32 Experience)
{
    int32 Level=1,Threshold=0;
    for(int32 L=1;L<10;++L){Threshold+=100+50*(L-1);if(Experience<Threshold){break;}++Level;}
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
    if(R.Experience<0 || R.Experience>MaximumExperience || R.Knowledge.Num()>Catalog.Knowledge.Num() || R.CreditedCrafts.Num()>UPFProgressionCatalog::MaximumCraftRecords)
    {return RecordRefuse(Error,TEXT("Progression record exceeds bounded limits"));}
    TSet<FName> Known,Crafts;
    for(FName Id:R.Knowledge){const auto* D=Catalog.Find(Id);if(!D || Known.Contains(Id) || D->MinimumLevel>LevelForExperience(R.Experience)){return RecordRefuse(Error,TEXT("Invalid, duplicate or premature player knowledge"));}Known.Add(Id);}
    for(FName Id:R.Knowledge){for(FName P:Catalog.Find(Id)->Prerequisites){if(!Known.Contains(P)){return RecordRefuse(Error,TEXT("Player knowledge missing prerequisite"));}}}
    for(FName Id:R.CreditedCrafts){if(Crafts.Contains(Id) || !Crafting.Recipe(Id,&Items)){return RecordRefuse(Error,TEXT("Invalid or duplicate craft reward ledger"));}Crafts.Add(Id);}
    if(R.Experience<FMath::Min(MaximumExperience,R.CreditedCrafts.Num()*FirstCraftExperience) || AvailablePoints(R,Catalog)<0)
    {return RecordRefuse(Error,TEXT("Progression experience or point accounting is inconsistent"));}
    Error.Reset();return true;
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
