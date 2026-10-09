#include "UI/PFProgressionDetails.h"
#include "Progression/PFProgressionRecord.h"

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
    View.Summary=FText::FromString(FString::Printf(TEXT("Level %d | %s | %d knowledge points (spending not available yet)"),Level,*Experience,Points));
    FString Reward;
    if(Recipe.IsNone()){Reward=TEXT("Choose a recipe to see its first-craft reward.");}
    else if(Record->CreditedCrafts.Contains(Recipe)){Reward=TEXT("First-craft reward already earned. Repeats give items, no extra XP.");}
    else if(Level==10){Reward=TEXT("Level cap reached. Crafting still gives items, no extra XP.");}
    else{Reward=FString::Printf(TEXT("First completion: +%d XP once. Cancelled or failed crafts give no XP."),
        FMath::Min(FPFProgressionTransactions::FirstCraftExperience,FPFProgressionTransactions::MaximumExperience-Record->Experience));}
    View.RecipeReward=FText::FromString(Reward);return View;
}
