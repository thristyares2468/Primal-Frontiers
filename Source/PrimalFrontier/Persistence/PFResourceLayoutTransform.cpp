#include "Persistence/PFResourceLayoutTransform.h"
#include "Persistence/PFWorldSaveFormat.h"
#include "Persistence/PFSaveFileStore.h"
#include "Crafting/PFCraftingCatalog.h"
#include "Inventory/PFItemCatalog.h"

bool FPFResourceLayoutTransform::Prepare(const FString& ExpectedMap,const TArray<FPFResourceLayoutEntry>& Previous,
    const TArray<FPFResourceLayoutEntry>& Added,const FPFWorldSaveData& Source,
    const UPFItemCatalog& Items,const UPFBuildingCatalog& Buildings,const UPFCraftingCatalog& Crafting,
    const UPFCreatureCatalog& Creatures,const UPFProgressionCatalog& Progression,FPFWorldSaveData& Out,FString& Error)
{
    auto Refuse=[&](const TCHAR* Why){Error=Why;return false;};
    if(&Source==&Out){return Refuse(TEXT("Separate source and destination required"));}
    if(Source.Map!=ExpectedMap || Previous.IsEmpty() || Added.IsEmpty() || Added.Num()>16 ||
       Previous.Num()+Added.Num()>128 || Source.Resources.Num()!=Previous.Num())
    {return Refuse(TEXT("Wrong map, unexpected layout or unbounded addition manifest"));}
    if(!FPFWorldSaveFormat::Validate(Source,Items,Buildings,Crafting,Creatures,Error,&Progression)){return false;}
    auto Definition=[&](const FPFResourceLayoutEntry& Entry)->const FPFResourceDefinition*
    {
        // Compare strings to known catalog IDs before any FName construction.
        const auto* D=Crafting.Resources.FindByPredicate([&](const auto& R){return R.Id.ToString()==Entry.Definition;});
        return D && Crafting.Resource(D->Id,&Items)?D:nullptr;
    };
    TSet<FString> Names;
    for(const auto& Entry:Previous)
    {
        if(!FPFSaveFileStore::ValidSlot(Entry.Name) || !Definition(Entry) || Names.Contains(Entry.Name))
        {return Refuse(TEXT("Invalid or duplicate previous resource manifest"));}
        Names.Add(Entry.Name);
        const auto* Existing=Source.Resources.FindByPredicate([&](const auto& R){return R.Name==Entry.Name;});
        if(!Existing || Existing->Definition!=Entry.Definition)
        {return Refuse(TEXT("Previous resource names/definitions differ; no guessing or deletion"));}
    }
    auto Candidate=Source;
    for(const auto& Entry:Added)
    {
        const auto* D=Definition(Entry);
        if(!FPFSaveFileStore::ValidSlot(Entry.Name) || !D || Names.Contains(Entry.Name))
        {return Refuse(TEXT("Invalid, duplicate or overlapping added resource manifest"));}
        Names.Add(Entry.Name);
        FPFResourceSaveRecord New;New.Name=Entry.Name;New.Definition=Entry.Definition;
        New.Hits=D->Hits;New.Respawn=0;Candidate.Resources.Add(MoveTemp(New));
    }
    if(!FPFWorldSaveFormat::Validate(Candidate,Items,Buildings,Crafting,Creatures,Error,&Progression)){return false;}
    Out=MoveTemp(Candidate);Error.Reset();return true;
}
