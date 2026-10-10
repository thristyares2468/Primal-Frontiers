#if WITH_DEV_AUTOMATION_TESTS
#include "Progression/PFProgressionRecord.h"
#include "Progression/PFProgressionCatalog.h"
#include "Crafting/PFCraftingCatalog.h"
#include "Inventory/PFItemCatalog.h"
#include "Misc/AutomationTest.h"
#include <limits>

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFProgressionRecordsTest,"PF.Progression.Records",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFProgressionRecordsTest::RunTest(const FString&)
{
    auto* Catalog=NewObject<UPFProgressionCatalog>();auto* Crafting=NewObject<UPFCraftingCatalog>();auto* Items=NewObject<UPFItemCatalog>();FString Error;
    FPFProgressionRecord R;TestTrue(TEXT("Valid fresh record and real catalogs"),FPFProgressionTransactions::Validate(R,*Catalog,*Crafting,*Items,Error));
    TestEqual(TEXT("Fresh level one"),FPFProgressionTransactions::LevelForExperience(R.Experience),1);TestEqual(TEXT("No free points"),FPFProgressionTransactions::AvailablePoints(R,*Catalog),0);
    auto Purchase=[&](FName Id){return FPFProgressionTransactions::Purchase(R,Id,*Catalog,*Crafting,*Items,Error);};
    auto Credit=[&](FName Id){return FPFProgressionTransactions::CreditFirstCraft(R,Id,*Catalog,*Crafting,*Items,Error);};
    TestFalse(TEXT("Level gate"),Purchase(TEXT("Tech_FieldTools")));TestFalse(TEXT("Unknown knowledge"),Purchase(TEXT("Tech_Unknown")));TestFalse(TEXT("Unknown recipe"),Credit(TEXT("Recipe_Unknown")));
    TestEqual(TEXT("Refusals preserve fresh state"),R.Experience,0);TestTrue(TEXT("First real tool craft reward"),Credit(TEXT("Recipe_Tool")));TestFalse(TEXT("Repeat craft grants no XP"),Credit(TEXT("Recipe_Tool")));TestEqual(TEXT("Reward conserved"),R.Experience,20);
    for(const TCHAR* Id:{TEXT("Recipe_Cook"),TEXT("Recipe_Dry"),TEXT("Recipe_Cord"),TEXT("Recipe_BoundTool")}){TestTrue(TEXT("Distinct real recipes reward once"),Credit(Id));}
    TestEqual(TEXT("Five crafts level two"),FPFProgressionTransactions::LevelForExperience(R.Experience),2);TestEqual(TEXT("Exactly three earned points"),FPFProgressionTransactions::AvailablePoints(R,*Catalog),3);
    TestTrue(TEXT("Optional knowledge purchase"),Purchase(TEXT("Tech_FieldTools")));TestEqual(TEXT("Exact point spending"),FPFProgressionTransactions::AvailablePoints(R,*Catalog),1);TestFalse(TEXT("Duplicate purchase"),Purchase(TEXT("Tech_FieldTools")));
    TestEqual(TEXT("Purchase creates no extra knowledge"),R.Knowledge.Num(),1);TestEqual(TEXT("Purchase awards no XP"),R.Experience,100);
    const int32 Thresholds[]={0,100,250,450,700,1000,1350,1750,2200,2700};
    for(int32 I=0;I<10;++I){TestEqual(TEXT("Exact level boundary"),FPFProgressionTransactions::LevelForExperience(Thresholds[I]),I+1);if(I>0){TestEqual(TEXT("Just below boundary"),FPFProgressionTransactions::LevelForExperience(Thresholds[I]-1),I);}}
    auto Award=[&](int32 N){return FPFProgressionTransactions::AwardExperience(R,N,*Catalog,*Crafting,*Items,Error);};
    TestFalse(TEXT("Negative award"),Award(-1));TestFalse(TEXT("Zero award"),Award(0));TestFalse(TEXT("Int overflow input"),Award(std::numeric_limits<int32>::max()));TestEqual(TEXT("Invalid awards leave XP unchanged"),R.Experience,100);
    TestTrue(TEXT("Multi-level award bounded at cap"),Award(2700));TestEqual(TEXT("Cap exact"),R.Experience,2700);TestEqual(TEXT("Nine level awards minus purchase"),FPFProgressionTransactions::AvailablePoints(R,*Catalog),25);
    TestFalse(TEXT("No repeated cap awards"),Award(1));TestTrue(TEXT("Craft at cap still records one-shot identity"),Credit(TEXT("Recipe_Club")));TestFalse(TEXT("No replay of capped reward"),Credit(TEXT("Recipe_Club")));TestEqual(TEXT("No capped XP inflation"),R.Experience,2700);
    FPFProgressionRecord Corrupt=R;Corrupt.Experience=2701;TestFalse(TEXT("Out of range saved XP"),FPFProgressionTransactions::Validate(Corrupt,*Catalog,*Crafting,*Items,Error));Corrupt=R;const FName FirstCredited=Corrupt.CreditedCrafts[0];Corrupt.CreditedCrafts.Add(FirstCredited);TestFalse(TEXT("Duplicate reward ledger"),FPFProgressionTransactions::Validate(Corrupt,*Catalog,*Crafting,*Items,Error));
    Corrupt=R;Corrupt.Knowledge.Add(TEXT("Tech_Unknown"));TestFalse(TEXT("Unknown owned ID"),FPFProgressionTransactions::Validate(Corrupt,*Catalog,*Crafting,*Items,Error));Corrupt=R;Corrupt.Experience=20;TestFalse(TEXT("Missing credited XP"),FPFProgressionTransactions::Validate(Corrupt,*Catalog,*Crafting,*Items,Error));
    Corrupt=R;Corrupt.CreditedCrafts.SetNum(129);TestFalse(TEXT("Ledger bounded"),FPFProgressionTransactions::Validate(Corrupt,*Catalog,*Crafting,*Items,Error));
    // Test designer prerequisite accounting with a second existing optional recipe.
    auto Child=Catalog->Knowledge[0];Child.Id=TEXT("Tech_ProtectiveWeaving");Child.Recipes={FName(TEXT("Recipe_WovenGuard"))};Child.MinimumLevel=3;Child.PointCost=3;Child.Prerequisites={FName(TEXT("Tech_FieldTools"))};const int32 ChildIndex=Catalog->Knowledge.Add(Child);
    FPFProgressionRecord Other;Other.Experience=250;TestFalse(TEXT("Prerequisite missing refuses complete purchase"),FPFProgressionTransactions::Purchase(Other,Child.Id,*Catalog,*Crafting,*Items,Error));TestEqual(TEXT("No partial purchase"),Other.Knowledge.Num(),0);
    TestTrue(TEXT("Learn parent"),FPFProgressionTransactions::Purchase(Other,TEXT("Tech_FieldTools"),*Catalog,*Crafting,*Items,Error));TestTrue(TEXT("Learn child"),FPFProgressionTransactions::Purchase(Other,Child.Id,*Catalog,*Crafting,*Items,Error));TestEqual(TEXT("Whole graph exact costs"),FPFProgressionTransactions::AvailablePoints(Other,*Catalog),1);
    auto Bad=Other;Bad.Knowledge.Remove(TEXT("Tech_FieldTools"));TestFalse(TEXT("Invalid missing-parent record"),FPFProgressionTransactions::Validate(Bad,*Catalog,*Crafting,*Items,Error));
    Catalog->Knowledge[0].MinimumLevel=3;Catalog->Knowledge[0].Prerequisites={Child.Id};TestFalse(TEXT("Cycle rejected without recursion"),Catalog->Validate(*Crafting,*Items,Error));Catalog->Knowledge[0].Prerequisites.Empty();Catalog->Knowledge[0].MinimumLevel=2;
    Catalog->Knowledge[ChildIndex].Id=Catalog->Knowledge[0].Id;TestFalse(TEXT("Duplicate knowledge ID"),Catalog->Validate(*Crafting,*Items,Error));Catalog->Knowledge.Pop();
    Catalog->Knowledge[0].PointCost=4;TestFalse(TEXT("Unreachable minimum-level price refused"),Catalog->Validate(*Crafting,*Items,Error));Catalog->Knowledge[0].PointCost=2;
    Catalog->Knowledge[0].Recipes={FName(TEXT("Recipe_Tool"))};TestFalse(TEXT("Baseline tool cannot be gated"),Catalog->Validate(*Crafting,*Items,Error));Catalog->Knowledge[0].Recipes={FName(TEXT("Recipe_BoundTool"))};
    Catalog->Knowledge[0].PointCost=28;TestFalse(TEXT("Cost upper bound"),Catalog->Validate(*Crafting,*Items,Error));Catalog->Knowledge[0].PointCost=2;
    Catalog->Knowledge[0].PointCost=std::numeric_limits<int32>::max();TestEqual(TEXT("Read-only points getter refuses malformed cost without overflow"),FPFProgressionTransactions::AvailablePoints(R,*Catalog),INDEX_NONE);Catalog->Knowledge[0].PointCost=2;
    auto Duplicate=R;Duplicate.Knowledge.Add(R.Knowledge[0]);TestEqual(TEXT("Read-only points query refuses duplicate knowledge"),FPFProgressionTransactions::AvailablePoints(Duplicate,*Catalog),INDEX_NONE);
    TestTrue(TEXT("Baseline real recipes remain usable without any player integration"),Crafting->Recipe(TEXT("Recipe_Tool"),Items) && Crafting->Recipe(TEXT("Recipe_Cook"),Items) && Crafting->Recipe(TEXT("Recipe_Dry"),Items));
    AddInfo(TEXT("[PrimalAgentTools] Pure XP/knowledge records, cap/accounting/dedupe/cycle/refusal checked; no live reward or recipe gate is implemented."));return true;
}
#endif
