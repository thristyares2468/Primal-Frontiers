#include "Progression/PFProgressionCatalog.h"
#include "Crafting/PFCraftingCatalog.h"
#include "Inventory/PFItemCatalog.h"
#include "NativeGameplayTags.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PF_Knowledge,"Progression.Knowledge");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PF_KnowledgeTool,"Progression.Knowledge.Tool");
namespace
{
bool ProgressionRefuse(FString& Error,const TCHAR* Reason){Error=Reason;return false;}
bool ValidKnowledgeId(FName Id)
{
    const FString Text=Id.ToString();
    if(!Text.StartsWith(TEXT("Tech_")) || Text.Len()<6 || Text.Len()>64){return false;}
    for(TCHAR C:Text){if(!((C>=TEXT('A') && C<=TEXT('Z')) || (C>=TEXT('a') && C<=TEXT('z')) || (C>=TEXT('0') && C<=TEXT('9')) || C==TEXT('_'))){return false;}}
    return true;
}
}
UPFProgressionCatalog::UPFProgressionCatalog()
{
    FPFKnowledgeDefinition D;D.Id=TEXT("Tech_FieldTools");D.DisplayName=FText::FromString(TEXT("Field tool knowledge"));D.Category=TAG_PF_KnowledgeTool;
    D.Recipes.Add(TEXT("Recipe_BoundTool"));Knowledge.Add(D);
}
const FPFKnowledgeDefinition* UPFProgressionCatalog::Find(FName Id) const
{
    return Knowledge.FindByPredicate([Id](const auto& D){return D.Id==Id;});
}
bool UPFProgressionCatalog::Validate(const UPFCraftingCatalog& Crafting,const UPFItemCatalog& Items,FString& Error) const
{
    if(Knowledge.IsEmpty() || Knowledge.Num()>MaximumKnowledge || Crafting.Recipes.Num()>MaximumCraftRecords){return ProgressionRefuse(Error,TEXT("Progression catalog exceeds bounded limits"));}
    TSet<FName> Ids,AssignedRecipes;
    for(const auto& D:Knowledge)
    {
        if(!ValidKnowledgeId(D.Id) || Ids.Contains(D.Id) || D.DisplayName.IsEmpty() || !D.Category.MatchesTag(TAG_PF_Knowledge) ||
            D.MinimumLevel<2 || D.MinimumLevel>10 || D.PointCost<1 || D.PointCost>27 || D.Prerequisites.Num()>MaximumKnowledge || D.Recipes.IsEmpty() || D.Recipes.Num()>MaximumCraftRecords)
        {return ProgressionRefuse(Error,TEXT("Invalid knowledge definition"));}
        Ids.Add(D.Id);TSet<FName> Prerequisites;
        for(FName P:D.Prerequisites){const auto* Parent=Find(P);if(P==D.Id || Prerequisites.Contains(P) || !Parent || Parent->MinimumLevel>D.MinimumLevel){return ProgressionRefuse(Error,TEXT("Invalid knowledge prerequisite"));}Prerequisites.Add(P);}
        for(FName Recipe:D.Recipes)
        {
            // Ordinary survival access is invariant even if a designer attempts to assign it.
            if(Recipe==TEXT("Recipe_Tool") || Recipe==TEXT("Recipe_Cook") || Recipe==TEXT("Recipe_Dry") || !Crafting.Recipe(Recipe,&Items) || AssignedRecipes.Contains(Recipe))
            {return ProgressionRefuse(Error,TEXT("Invalid, duplicate or baseline knowledge recipe"));}
            AssignedRecipes.Add(Recipe);
        }
    }
    // Kahn traversal is bounded and avoids recursion on malicious/corrupt designer data.
    TSet<FName> Resolved;
    for(int32 Pass=0;Pass<Knowledge.Num();++Pass)
    {
        bool Advanced=false;
        for(const auto& D:Knowledge){if(Resolved.Contains(D.Id)){continue;}bool Ready=true;for(FName P:D.Prerequisites){Ready&=Resolved.Contains(P);}if(Ready){Resolved.Add(D.Id);Advanced=true;}}
        if(!Advanced){break;}
    }
    if(Resolved.Num()!=Knowledge.Num()){return ProgressionRefuse(Error,TEXT("Cyclic knowledge prerequisites"));}
    for(const auto& D:Knowledge)
    {
        TArray<FName> Pending={D.Id};TSet<FName> Closure;int32 Cost=0;
        while(!Pending.IsEmpty())
        {
            const FName Id=Pending.Pop();if(Closure.Contains(Id)){continue;}Closure.Add(Id);
            const auto* Entry=Find(Id);Cost+=Entry->PointCost;Pending.Append(Entry->Prerequisites);
        }
        if(Cost>3*(D.MinimumLevel-1)){return ProgressionRefuse(Error,TEXT("Knowledge prerequisites exceed points earned at minimum level"));}
    }
    Error.Reset();return true;
}
