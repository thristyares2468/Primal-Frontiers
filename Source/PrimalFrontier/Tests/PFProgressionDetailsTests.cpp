#if WITH_DEV_AUTOMATION_TESTS
#include "UI/PFProgressionDetails.h"
#include "Progression/PFProgressionRecord.h"
#include "Progression/PFProgressionCatalog.h"
#include "Crafting/PFCraftingCatalog.h"
#include "Inventory/PFItemCatalog.h"
#include "Misc/AutomationTest.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFProgressionDetailsTest,"PF.UI.ProgressionDetails",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFProgressionDetailsTest::RunTest(const FString&)
{
    FPFProgressionRecord Record;
    auto Describe=[&](int32 Points,FName Recipe){return PFProgressionDetails::Describe(&Record,Points,Recipe);};
    const auto Fresh=Describe(0,TEXT("Recipe_Tool"));
    TestTrue(TEXT("Actual zero record has first threshold"),Fresh.Summary.ToString().Contains(TEXT("Level 1 | 0 / 100 XP | 100 to next level | 0 knowledge points")));
    TestFalse(TEXT("No obsolete spending claim"),Fresh.Summary.ToString().Contains(TEXT("spending not available yet")));
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
    auto* Catalog=NewObject<UPFProgressionCatalog>();const auto* Crafting=GetDefault<UPFCraftingCatalog>();const auto* Items=GetDefault<UPFItemCatalog>();
    auto Knowledge=[&](){return PFProgressionDetails::DescribeKnowledge(&Record,&Catalog->Knowledge[0],Catalog,Crafting,Items);};
    Record={};auto Locked=Knowledge();
    TestFalse(TEXT("Level one cannot learn"),Locked.bCanLearn);TestTrue(TEXT("Exact lock requirements"),Locked.Requirement.ToString().Contains(TEXT("Level 2 | Cost 2 points | Available 0")));
    TestTrue(TEXT("Both bounded gather and first-craft earning are explained"),Locked.Requirement.ToString().Contains(TEXT("Gather resources for limited XP")) && Locked.Requirement.ToString().Contains(TEXT("different recipes for first-craft XP")));
    Record.Experience=100;TestTrue(TEXT("Available optional knowledge"),Knowledge().bCanLearn);
    TestEqual(TEXT("Read-only purchase evaluation does not spend"),FPFProgressionTransactions::AvailablePoints(Record,*Catalog),3);TestTrue(TEXT("No optimistic grant"),Record.Knowledge.IsEmpty());
    Record.Knowledge.Add(TEXT("Tech_FieldTools"));TestTrue(TEXT("Learned is explicit"),Knowledge().bLearned);TestFalse(TEXT("Learned not purchasable twice"),Knowledge().bCanLearn);
    TestTrue(TEXT("Learned useful access described"),Knowledge().Requirement.ToString().Contains(TEXT("Recipe access available")));
    TestFalse(TEXT("Learned view does not keep obsolete leveling instruction"),Knowledge().Requirement.ToString().Contains(TEXT("Gather resources")));
    Record={};Record.Experience=100;
    FPFKnowledgeDefinition Other=Catalog->Knowledge[0];Other.Id=TEXT("Tech_Other");Other.DisplayName=FText::FromString(TEXT("Other test knowledge"));Other.Recipes={TEXT("Recipe_Cord")};Other.PointCost=2;Catalog->Knowledge.Add(Other);
    Record.Knowledge.Add(Other.Id);TestFalse(TEXT("Insufficient remaining points"),Knowledge().bCanLearn);TestTrue(TEXT("Unaffordable explains remaining point"),Knowledge().Requirement.ToString().Contains(TEXT("Available 1")) && Knowledge().Requirement.ToString().Contains(TEXT("Not enough")));
    Record.Knowledge.Reset();Catalog->Knowledge[1].PointCost=1;Catalog->Knowledge[0].Prerequisites={Other.Id};
    TestFalse(TEXT("Missing prerequisite stays locked"),Knowledge().bCanLearn);TestTrue(TEXT("Prerequisite display name readable"),Knowledge().Requirement.ToString().Contains(TEXT("Learn first: Other test knowledge")));
    Record.Knowledge.Add(Other.Id);TestTrue(TEXT("Satisfied prerequisite and exact cost allow view"),Knowledge().bCanLearn);
    Record.Experience=-1;TestTrue(TEXT("Corrupt record explicit unavailable"),Knowledge().Requirement.ToString().Contains(TEXT("Knowledge unavailable")));TestFalse(TEXT("Corrupt record disables purchase"),Knowledge().bCanLearn);
    TestTrue(TEXT("Missing owner explicit unavailable"),PFProgressionDetails::DescribeKnowledge(nullptr,&Catalog->Knowledge[0],Catalog,Crafting,Items).Requirement.ToString().Contains(TEXT("unavailable")));
    TestTrue(TEXT("Baseline no optional label"),PFProgressionDetails::DescribeKnowledge(&Record,nullptr,Catalog,Crafting,Items).Requirement.IsEmpty());
    AddInfo(TEXT("[PrimalUI] Private progression, level boundaries, conditional/deduplicated reward, cap and unavailable presentation checked."));return true;
}
#endif
