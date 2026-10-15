#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Crafting/PFCraftingCatalog.h"
#include "Crafting/PFEryndorProgression.h"
#include "Inventory/PFItemCatalog.h"
#include "Progression/PFProgressionCatalog.h"
#include "Progression/PFProgressionRecord.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFEryndorCatalogTest,"PF.Eryndor.Catalog",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FPFEryndorCatalogTest::RunTest(const FString&)
{
    auto* Items=NewObject<UPFItemCatalog>();auto* Craft=NewObject<UPFCraftingCatalog>();auto* Knowledge=NewObject<UPFProgressionCatalog>();FString Error;
    TestEqual(TEXT("Append-only items"),Items->Items.Num(),36);TestEqual(TEXT("Append-only recipes"),Craft->Recipes.Num(),21);TestEqual(TEXT("Append-only resource definitions"),Craft->Resources.Num(),15);
    for(const auto& D:Items->Items){TestNotNull(*D.Id.ToString(),Items->Find(D.Id));}
    for(const auto& D:Craft->Recipes){TestNotNull(*D.Id.ToString(),Craft->Recipe(D.Id,Items));}
    for(const auto& D:Craft->Resources){TestNotNull(*D.Id.ToString(),Craft->Resource(D.Id,Items));}
    TestTrue(TEXT("Entire knowledge graph valid/affordable/acyclic"),Knowledge->Validate(*Craft,*Items,Error));
    TestEqual(TEXT("Four appended technology branches"),Knowledge->Knowledge.Num(),6);
    TestTrue(TEXT("Diamond really requires Iron pick"),Craft->Resource(TEXT("Node_Diamond"),Items)->HarvestRequirement.MinimumGrade==EPFHarvestGrade::Iron);
    TestTrue(TEXT("Titanium really requires Diamond pick"),Craft->Resource(TEXT("Node_Titanium"),Items)->HarvestRequirement.MinimumGrade==EPFHarvestGrade::Diamond);
    // Every raw material must be consumed somewhere, not a decorative item with no use.
    for(const auto& R:Craft->Resources)
    {
        if(!PFEryndor::IsMineral(R.YieldItem)){continue;}
        TestTrue(TEXT("All new minerals have a real recipe use"),Craft->Recipes.ContainsByPredicate([&](const auto& D){return D.Ingredients.ContainsByPredicate([&](const auto& I){return I.ItemId==R.YieldItem;});}));
        TestTrue(TEXT("New ores share existing Stone XP category"),FPFProgressionTransactions::GatherCategoryForItem(R.YieldItem)==FPFProgressionTransactions::GatherCategoryForItem(TEXT("Item_Stone")));
    }
    FPFProgressionRecord Fresh;const auto Snapshot=Fresh;
    TestFalse(TEXT("No free late knowledge"),FPFProgressionTransactions::Purchase(Fresh,TEXT("Tech_Titanium"),*Knowledge,*Craft,*Items,Error));
    TestTrue(TEXT("Refusal conserves points/knowledge/XP"),Fresh.Experience==Snapshot.Experience && Fresh.Knowledge==Snapshot.Knowledge);
    FPFProgressionRecord Boundary;Boundary.Experience=FPFProgressionTransactions::ExperienceForLevel(5);
    const int32 Stats=FPFProgressionTransactions::AvailableStatPoints(Boundary);
    for(FName Id:{FName(TEXT("Tech_FieldTools")),FName(TEXT("Tech_Metalwork")),FName(TEXT("Tech_Gemwork")),FName(TEXT("Tech_Titanium")),FName(TEXT("Tech_Conductors"))})
    {if(!TestTrue(*Id.ToString(),FPFProgressionTransactions::Purchase(Boundary,Id,*Knowledge,*Craft,*Items,Error))){AddError(Error);return false;}}
    TestEqual(TEXT("All home-world utility branches cost eleven, not stat points"),FPFProgressionTransactions::AvailablePoints(Boundary,*Knowledge),1);
    TestEqual(TEXT("Stat budget remains separate"),FPFProgressionTransactions::AvailableStatPoints(Boundary),Stats);
    // Prove the main path's XP and point economy without XP grants or timer writes.
    FPFProgressionRecord Earned;
    for(auto Category:FPFProgressionTransactions::GatherCategories())
    {for(int32 N=0;N<5;++N){TestTrue(TEXT("Finite ordinary gather credit"),FPFProgressionTransactions::CreditGather(Earned,Category,*Knowledge,*Craft,*Items,Error));}}
    for(FName Id:{FName(TEXT("Recipe_WoodTool")),FName(TEXT("Recipe_Tool")),FName(TEXT("Recipe_Cook")),FName(TEXT("Recipe_Dry")),FName(TEXT("Recipe_Cord")),FName(TEXT("Recipe_Club")),FName(TEXT("Recipe_WovenGuard")),FName(TEXT("Recipe_CopperRefine"))})
    {TestTrue(TEXT("Unique ordinary baseline recipe credit"),FPFProgressionTransactions::CreditFirstCraft(Earned,Id,*Knowledge,*Craft,*Items,Error));}
    TestTrue(TEXT("Earned FieldTools"),FPFProgressionTransactions::Purchase(Earned,TEXT("Tech_FieldTools"),*Knowledge,*Craft,*Items,Error));
    for(FName Id:{FName(TEXT("Recipe_BoundTool")),FName(TEXT("Recipe_CopperPick"))}){TestTrue(TEXT("Owned FieldTools craft credit"),FPFProgressionTransactions::CreditFirstCraft(Earned,Id,*Knowledge,*Craft,*Items,Error));}
    TestTrue(TEXT("Earned Metalwork"),FPFProgressionTransactions::Purchase(Earned,TEXT("Tech_Metalwork"),*Knowledge,*Craft,*Items,Error));
    for(FName Id:{FName(TEXT("Recipe_IronRefine")),FName(TEXT("Recipe_IronPick")),FName(TEXT("Recipe_GoldRefine")),FName(TEXT("Recipe_SilverRefine"))}){TestTrue(TEXT("Unique metalwork craft credit"),FPFProgressionTransactions::CreditFirstCraft(Earned,Id,*Knowledge,*Craft,*Items,Error));}
    TestTrue(TEXT("Earned Gemwork without waiting for a gather reset"),FPFProgressionTransactions::Purchase(Earned,TEXT("Tech_Gemwork"),*Knowledge,*Craft,*Items,Error));
    for(FName Id:{FName(TEXT("Recipe_DiamondPick")),FName(TEXT("Recipe_QuartzLens")),FName(TEXT("Recipe_ObsidianBlade"))}){TestTrue(TEXT("Unique gemwork craft credit"),FPFProgressionTransactions::CreditFirstCraft(Earned,Id,*Knowledge,*Craft,*Items,Error));}
    TestEqual(TEXT("Deterministic earned progression contract"),Earned.Experience,465);
    TestTrue(TEXT("Earned Titanium knowledge affordable at actual level4"),FPFProgressionTransactions::Purchase(Earned,TEXT("Tech_Titanium"),*Knowledge,*Craft,*Items,Error));
    TestFalse(TEXT("Duplicate unlock cannot debit"),FPFProgressionTransactions::Purchase(Boundary,TEXT("Tech_Titanium"),*Knowledge,*Craft,*Items,Error));
    AddInfo(TEXT("[PrimalAgentTools] Eryndor.Catalog: definitions/uses, acyclic knowledge and separate affordable budgets, shared mineral XP, no free unlocks; level5 is an explicit boundary seed, not earned pacing."));return true;
}
#endif
