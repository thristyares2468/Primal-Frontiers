#include "UI/PFProgressionDetails.h"
#include "Progression/PFProgressionRecord.h"
#include "Progression/PFProgressionCatalog.h"
#include "Crafting/PFCraftingCatalog.h"
#include "Inventory/PFItemCatalog.h"

PFProgressionDetails::FView PFProgressionDetails::Describe(const FPFProgressionRecord* Record,int32 Points,FName Recipe)
{
    FView View;
    if(!Record || Record->Experience<0 || Record->Experience>FPFProgressionTransactions::MaximumExperience || Points<0 || Points>27)
    {
        View.Summary=FText::FromString(TEXT("Progression unavailable - waiting for your player state."));
        View.RecipeReward=FText::FromString(TEXT("First-craft reward unavailable."));return View;
    }
    const int32 Level=FPFProgressionTransactions::LevelForExperience(Record->Experience);
    const FString Experience=Level==10?FString::Printf(TEXT("%d XP | Level cap"),Record->Experience):
        FString::Printf(TEXT("%d / %d XP | %d to next level"),Record->Experience,
            FPFProgressionTransactions::ExperienceForLevel(Level+1),FPFProgressionTransactions::ExperienceForLevel(Level+1)-Record->Experience);
    View.Summary=FText::FromString(FString::Printf(TEXT("Level %d | %s | %d knowledge point%s"),Level,*Experience,Points,Points==1?TEXT(""):TEXT("s")));
    FString Reward;
    if(Recipe.IsNone()){Reward=TEXT("Choose a recipe to see its first-craft reward.");}
    else if(Record->CreditedCrafts.Contains(Recipe)){Reward=TEXT("First-craft reward already earned. Repeats give items, no extra XP.");}
    else if(Level==10){Reward=TEXT("Level cap reached. Crafting still gives items, no extra XP.");}
    else{Reward=FString::Printf(TEXT("First completion: +%d XP once. Cancelled or failed crafts give no XP."),
        FMath::Min(FPFProgressionTransactions::FirstCraftExperience,FPFProgressionTransactions::MaximumExperience-Record->Experience));}
    View.RecipeReward=FText::FromString(Reward);return View;
}

PFProgressionDetails::FKnowledgeView PFProgressionDetails::DescribeKnowledge(const FPFProgressionRecord* Record,
    const FPFKnowledgeDefinition* D,const UPFProgressionCatalog* Catalog,const UPFCraftingCatalog* Crafting,const UPFItemCatalog* Items)
{
    FKnowledgeView View;if(!D){return View;}
    View.Button=FText::FromString(TEXT("Learn unavailable"));FString Error;
    if(!Record || !Catalog || !Crafting || !Items || Catalog->Find(D->Id)!=D ||
        !FPFProgressionTransactions::Validate(*Record,*Catalog,*Crafting,*Items,Error))
    {View.Requirement=FText::FromString(TEXT("Knowledge unavailable - waiting for valid player data."));return View;}
    View.bLearned=Record->Knowledge.Contains(D->Id);
    const int32 Points=FPFProgressionTransactions::AvailablePoints(*Record,*Catalog);
    if(View.bLearned)
    {View.Requirement=FText::FromString(D->DisplayName.ToString()+TEXT(" learned. Recipe access available."));View.Button=FText::FromString(TEXT("Already learned"));return View;}
    FString Requirement=FString::Printf(TEXT("Locked: %s | Level %d | Cost %d points | Available %d"),*D->DisplayName.ToString(),D->MinimumLevel,D->PointCost,Points);
    TArray<FString> Missing;
    for(FName Id:D->Prerequisites){if(!Record->Knowledge.Contains(Id)){Missing.Add(Catalog->Find(Id)->DisplayName.ToString());}}
    if(!Missing.IsEmpty()){Requirement+=TEXT("\nLearn first: ")+FString::Join(Missing,TEXT(", "));}
    auto Candidate=*Record;View.bCanLearn=FPFProgressionTransactions::Purchase(Candidate,D->Id,*Catalog,*Crafting,*Items,Error);
    if(FPFProgressionTransactions::LevelForExperience(Record->Experience)<D->MinimumLevel){Requirement+=TEXT("\nGather resources for limited XP, or complete different recipes for first-craft XP.");}
    else if(Points<D->PointCost){Requirement+=TEXT("\nNot enough knowledge points.");}
    View.Requirement=FText::FromString(Requirement);
    View.Button=FText::FromString(FString::Printf(TEXT("Learn for %d points (K / D-right)"),D->PointCost));return View;
}
