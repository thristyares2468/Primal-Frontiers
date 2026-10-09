#if WITH_DEV_AUTOMATION_TESTS
#include "UI/PFRecipeDetails.h"
#include "Crafting/PFCraftingCatalog.h"
#include "Inventory/PFItemCatalog.h"
#include "Inventory/PFInventoryComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFRecipeDetailsTest,"PF.UI.RecipeDetails",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFRecipeDetailsTest::RunTest(const FString&)
{
    FTestWorldWrapper Fixture;if(!Fixture.CreateTestWorld(EWorldType::Game)){return false;}
    auto* W=Fixture.GetTestWorld();auto* Owner=W->SpawnActor<AActor>();
    auto* I=NewObject<UPFInventoryComponent>(Owner);Owner->AddInstanceComponent(I);I->RegisterComponent();I->Catalog=NewObject<UPFItemCatalog>(I);
    auto* Catalog=NewObject<UPFCraftingCatalog>();
    const auto* Cook=Catalog->Recipe(TEXT("Recipe_Cook"),I->Catalog);if(!Cook){return false;}
    const double Now=UPFInventoryComponent::ServerTime(W);
    TestTrue(TEXT("Owned ingredient fixture"),I->Grant(TEXT("Item_Wood"),3));
    TestTrue(TEXT("Short-lived real batch fixture"),I->AddExisting(TEXT("Item_Food"),1,Now+1));
    const auto Present=PFRecipeDetails::Describe(Cook,I,Now,false);
    TestTrue(TEXT("Catalog output displayed"),Present.Body.ToString().Contains(TEXT("Makes 1 Cooked food")));
    TestTrue(TEXT("Present ingredients do not claim success"),Present.Body.ToString().Contains(TEXT("Ingredients present; server validates")));
    TestTrue(TEXT("Busy overrides apparent affordability"),PFRecipeDetails::Describe(Cook,I,Now,true).Body.ToString().Contains(TEXT("Job running")));
    const auto Expired=PFRecipeDetails::Describe(Cook,I,Now+1,false).Body.ToString();
    TestTrue(TEXT("At deadline before prune batch is unavailable"),Expired.Contains(TEXT("Found food 0/1")) && Expired.Contains(TEXT("Missing fresh ingredients")));
    TestEqual(TEXT("Read-only view does not prune replicated inventory"),I->Count(TEXT("Item_Food")),1);
    FPFRecipeDefinition Tuned=*Cook;Tuned.Duration=1.25f;Tuned.OutputQuantity=2;Tuned.Ingredients[0].Quantity=2;
    const auto Updated=PFRecipeDetails::Describe(&Tuned,I,Now,false);
    TestTrue(TEXT("Designer time/output/cost override reflected"),Updated.Title.ToString().Contains(TEXT("1.2s")) && Updated.Body.ToString().Contains(TEXT("Makes 2 Cooked food")) && Updated.Body.ToString().Contains(TEXT("Found food 1/2")));
    TestTrue(TEXT("Missing recipe is explicit"),PFRecipeDetails::Describe(nullptr,I,Now,false).Title.ToString()==TEXT("RECIPE UNAVAILABLE"));
    TestTrue(TEXT("Missing inventory is explicit"),PFRecipeDetails::Describe(Cook,nullptr,Now,false).Title.ToString()==TEXT("RECIPE UNAVAILABLE"));
    Tuned.Output=TEXT("Item_Unknown");
    TestTrue(TEXT("Missing output definition never advertised"),PFRecipeDetails::Describe(&Tuned,I,Now,false).Title.ToString()==TEXT("RECIPE UNAVAILABLE"));
    TestEqual(TEXT("UI never spends ingredients"),I->Count(TEXT("Item_Wood")),3);
    TestEqual(TEXT("UI never creates output"),I->Count(TEXT("Item_CookedFood")),0);
    AddInfo(TEXT("[PrimalUI] Still-fresh batches, busy/unavailable state, actual output/cost/tuning and read-only conservation checked."));
    Fixture.ForwardErrorMessages(this);return true;
}
#endif
