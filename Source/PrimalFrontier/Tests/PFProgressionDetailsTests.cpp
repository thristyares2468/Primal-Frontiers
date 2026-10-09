#if WITH_DEV_AUTOMATION_TESTS
#include "UI/PFProgressionDetails.h"
#include "Progression/PFProgressionRecord.h"
#include "Misc/AutomationTest.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFProgressionDetailsTest,"PF.UI.ProgressionDetails",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFProgressionDetailsTest::RunTest(const FString&)
{
    FPFProgressionRecord Record;
    auto Describe=[&](int32 Points,FName Recipe){return PFProgressionDetails::Describe(&Record,Points,Recipe);};
    const auto Fresh=Describe(0,TEXT("Recipe_Tool"));
    TestTrue(TEXT("Actual zero record has first threshold"),Fresh.Summary.ToString().Contains(TEXT("Level 1 | 0 / 100 XP | 100 to next level | 0 knowledge points")));
    TestTrue(TEXT("No invented purchase ability"),Fresh.Summary.ToString().Contains(TEXT("spending not available yet")));
    TestTrue(TEXT("Conditional first completion, never free XP"),Fresh.RecipeReward.ToString().Contains(TEXT("+20 XP once")) && Fresh.RecipeReward.ToString().Contains(TEXT("failed crafts give no XP")));
    Record.Experience=20;Record.CreditedCrafts.Add(TEXT("Recipe_Tool"));
    TestTrue(TEXT("Credited recipe never advertises repeat award"),Describe(0,TEXT("Recipe_Tool")).RecipeReward.ToString().Contains(TEXT("already earned")));
    TestTrue(TEXT("Uncredited cook has own reward"),Describe(0,TEXT("Recipe_Cook")).RecipeReward.ToString().Contains(TEXT("+20 XP once")));
    TestTrue(TEXT("No selected recipe has no promise"),Describe(0,NAME_None).RecipeReward.ToString().Contains(TEXT("Choose a recipe")));
    const int32 Thresholds[]={0,100,250,450,700,1000,1350,1750,2200,2700};
    for(int32 N=0;N<10;++N)
    {
        Record.Experience=Thresholds[N];TestEqual(TEXT("Shared cumulative threshold"),FPFProgressionTransactions::ExperienceForLevel(N+1),Thresholds[N]);
        TestTrue(TEXT("Level boundary shown"),Describe(N*3,NAME_None).Summary.ToString().StartsWith(FString::Printf(TEXT("Level %d |"),N+1)));
        if(N>0){TestEqual(TEXT("Before boundary stays previous level"),FPFProgressionTransactions::LevelForExperience(Thresholds[N]-1),N);}
    }
    Record.Experience=2690;TestTrue(TEXT("Near cap displays actual ten XP, not twenty"),Describe(24,TEXT("Recipe_Cook")).RecipeReward.ToString().Contains(TEXT("+10 XP once")));
    Record.Experience=2700;TestTrue(TEXT("Cap explicit, no next level"),Describe(27,TEXT("Recipe_Cook")).Summary.ToString().Contains(TEXT("2700 XP | Level cap")));
    TestTrue(TEXT("Uncredited cap recipe never advertises XP"),Describe(27,TEXT("Recipe_Cook")).RecipeReward.ToString().Contains(TEXT("no extra XP")));
    TestTrue(TEXT("Missing owner is explicit"),PFProgressionDetails::Describe(nullptr,0,NAME_None).Summary.ToString().Contains(TEXT("waiting for your player state")));
    TestTrue(TEXT("Invalid accounting is unavailable"),Describe(INDEX_NONE,NAME_None).RecipeReward.ToString().Contains(TEXT("unavailable")));
    Record.Experience=-1;TestTrue(TEXT("Malformed record never invents level"),Describe(0,NAME_None).Summary.ToString().Contains(TEXT("unavailable")));
    TestEqual(TEXT("Read-only view preserves source"),Record.CreditedCrafts.Num(),1);
    AddInfo(TEXT("[PrimalUI] Private progression, level boundaries, conditional/deduplicated reward, cap and unavailable presentation checked."));return true;
}
#endif
