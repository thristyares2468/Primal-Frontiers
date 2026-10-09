#if WITH_DEV_AUTOMATION_TESTS
#include "UI/PFInventoryDetails.h"
#include "Inventory/PFInventoryComponent.h"
#include "Inventory/PFItemCatalog.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFInventoryDetailsTest,"PF.UI.InventoryDetails",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFInventoryDetailsTest::RunTest(const FString&)
{
    auto* Catalog=NewObject<UPFItemCatalog>();
    FPFItemStack Stack;Stack.StackId=FGuid::NewGuid();Stack.ItemId=TEXT("Item_Food");Stack.Quantity=3;Stack.ExpiresAt=107;
    const auto* Food=Catalog->Find(Stack.ItemId);if(!Food){AddError(TEXT("Missing validated food fixture"));return false;}
    const auto Fresh=PFInventoryDetails::Describe(&Stack,Food,100).Body.ToString();
    TestTrue(TEXT("Actual batch deadline, not catalog shelf life"),Fresh.Contains(TEXT("Fresh for 7s")));
    TestTrue(TEXT("Actual batch weight"),Fresh.Contains(TEXT("0.60 kg this batch")));
    TestTrue(TEXT("Per-portion nutrition, not stack multiplier"),Fresh.Contains(TEXT("+35 food / +10 water")));
    const auto Expired=PFInventoryDetails::Describe(&Stack,Food,107).Body.ToString();
    TestTrue(TEXT("Deadline equality is expired before prune"),Expired.Contains(TEXT("Cannot consume")));
    TestFalse(TEXT("Expired batch never advertises recovery"),Expired.Contains(TEXT("One portion")));
    Stack.ItemId=TEXT("Item_Water");Stack.ExpiresAt=0;
    const auto Water=PFInventoryDetails::Describe(&Stack,Catalog->Find(Stack.ItemId),100).Body.ToString();
    TestTrue(TEXT("Resource-category water remains drinkable"),Water.Contains(TEXT("+0 food / +35 water")));
    TestTrue(TEXT("Zero deadline is nonperishable"),Water.Contains(TEXT("Does not expire")));
    TestTrue(TEXT("Wrong replicated/catalog ID fails closed"),PFInventoryDetails::Describe(&Stack,Food,100).Title.ToString()==TEXT("DETAILS UNAVAILABLE"));
    TestTrue(TEXT("Missing catalog fails closed"),PFInventoryDetails::Describe(&Stack,nullptr,100).Title.ToString()==TEXT("DETAILS UNAVAILABLE"));
    TestTrue(TEXT("No stack never exposes old details"),PFInventoryDetails::Describe(nullptr,Food,100).Title.ToString()==TEXT("SELECT AN ITEM"));
    Stack.ItemId=TEXT("Item_Wood");
    const auto* Wood=Catalog->Find(Stack.ItemId);if(!Wood){return false;}
    FPFItemDefinition Tuned=*Wood;Tuned.Weight=1.25f;Tuned.StackLimit=12;Tuned.DisplayName=FText::FromString(TEXT("Designer tuned resource"));
    const auto Customized=PFInventoryDetails::Describe(&Stack,&Tuned,100);
    TestEqual(TEXT("Designer name retained"),Customized.Title.ToString(),Tuned.DisplayName.ToString());
    TestTrue(TEXT("Tuned limits and weight drive presentation"),Customized.Body.ToString().Contains(TEXT("3 / 12")) && Customized.Body.ToString().Contains(TEXT("3.75 kg this batch")));
    TestTrue(TEXT("Inedible resource has explicit reason"),Customized.Body.ToString().Contains(TEXT("Not consumable")));
    TestEqual(TEXT("Queries never alter stack quantity"),Stack.Quantity,3);
    AddInfo(TEXT("[PrimalUI] Batch expiry, truthful per-portion recovery, drinkable resource, unavailable data and designer tuning checked."));
    return true;
}
#endif
