#include "Persistence/PFWorldPersistence.h"
#include "Persistence/PFResourceLayoutTransform.h"
#include "Progression/PFProgressionCatalog.h"
#include "Crafting/PFResourceNode.h"
#include "Engine/World.h"
#include "EngineUtils.h"

bool UPFWorldPersistence::PrepareCopperLayout(FPFWorldSaveData& Data,bool& Changed,FString& Error)
{
    Changed=false;
    const FString Map=UWorld::RemovePIEPrefix(GetWorld()->GetMapName());
    if(Map!=TEXT("L_PrimalFrontier_OpenWorld")){return true;}
    if(Data.Map!=Map){Error=TEXT("Copper layout source/map mismatch");return false;}
    const TCHAR* Definitions[]={TEXT("Node_Wood"),TEXT("Node_Stone"),TEXT("Node_Food"),TEXT("Node_Wood"),TEXT("Node_Wood"),TEXT("Node_Stone"),TEXT("Node_Food"),TEXT("Node_Food"),TEXT("Node_Stone"),TEXT("Node_Water"),TEXT("Node_Water"),TEXT("Node_Fibre"),TEXT("Node_Fibre"),TEXT("Node_Stone"),TEXT("Node_Food")};
    TArray<FPFResourceLayoutEntry> Previous;
    for(int32 Index=0;Index<15;++Index){Previous.Add({FString::Printf(TEXT("PFResourceNode_%d"),Index),Definitions[Index]});}
    TSet<FString> Found;APFResourceNode* Copper=nullptr;int32 Count=0;
    for(TActorIterator<APFResourceNode> It(GetWorld());It;++It)
    {
        ++Count;const FString Name=It->GetName();const auto* Entry=Previous.FindByPredicate([&](const auto& R){return R.Name==Name;});
        if(Entry)
        {
            if(Found.Contains(Name) || It->ResourceId.ToString()!=Entry->Definition || !It->IsConfigurationValid())
            {Error=TEXT("Authored old resource identity/configuration differs from reviewed baseline");return false;}
            Found.Add(Name);
        }
        else if(It->ActorHasTag(TEXT("PF.M12.CopperTest")) && It->ResourceId==TEXT("Node_Copper") && !Copper && It->IsConfigurationValid())
        {Copper=*It;}
        else{Error=TEXT("Unreviewed extra/renamed resource; no automatic layout guessing");return false;}
    }
    if(Found.Num()!=15 || Count!=(Copper?16:15))
    {Error=TEXT("Incomplete authored resource baseline for Copper edge");return false;}
    if(!Copper){return true;} // Before fixture authoring, preserve the exact old runtime path.
    const auto* Existing=Data.Resources.FindByPredicate([&](const auto& R){return R.Name==Copper->GetName();});
    if(Data.Resources.Num()==16 && Existing && Existing->Definition==TEXT("Node_Copper")){return true;}
    FPFWorldSaveData Candidate;const TArray<FPFResourceLayoutEntry> Added={{Copper->GetName(),TEXT("Node_Copper")}};
    if(!FPFResourceLayoutTransform::Prepare(Map,Previous,Added,Data,*Items,*Buildings,*Crafting,*Creatures,*GetDefault<UPFProgressionCatalog>(),Candidate,Error)){return false;}
    Data=MoveTemp(Candidate);Changed=true;return true;
}
