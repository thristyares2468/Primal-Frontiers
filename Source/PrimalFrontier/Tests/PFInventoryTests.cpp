// PFInventoryTests.cpp
//
// Automation test: PF.Inventory.Transactions   (M3, 340c538)
// Pure component test (no pawn): stacking and spill-over, split conservation,
// atomic weight/slot failures, invalid IDs/quantities, stale stack IDs,
// per-batch food freshness (split keeps the deadline, different batches never
// merge, only the old batch expires), client-role refusal and invalid designer data.
// Run: Session Frontend > Automation, filter "PF.Inventory".

#include "Inventory/PFInventoryComponent.h"
#include "Inventory/PFItemCatalog.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include <limits>
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFInventoryTest,"PF.Inventory.Transactions",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFInventoryTest::RunTest(const FString&)
{
    FTestWorldWrapper Fixture; if(!Fixture.CreateTestWorld(EWorldType::Game)){return false;}
    auto* Owner=Fixture.GetTestWorld()->SpawnActor<AActor>(); Owner->SetReplicates(true);
    auto* I=NewObject<UPFInventoryComponent>(Owner); Owner->AddInstanceComponent(I);
    I->Catalog=NewObject<UPFItemCatalog>(I); I->RegisterComponent();
    if(!Fixture.BeginPlayInTestWorld()){return false;}
    TestEqual(TEXT("Starts empty"),I->GetStacks().Num(),0);
    TestTrue(TEXT("Grant wood"),I->Grant(TEXT("Item_Wood"),15));
    TestTrue(TEXT("Stacks fill and spill"),I->Grant(TEXT("Item_Wood"),10));
    TestEqual(TEXT("Two stacks"),I->GetStacks().Num(),2);
    TestEqual(TEXT("First stack full"),I->GetStacks()[0].Quantity,20);
    TestTrue(TEXT("Split part"),I->Split(I->GetStacks()[0].StackId,7));
    TestEqual(TEXT("Split conserves count"),I->Count(TEXT("Item_Wood")),25);
    TestTrue(TEXT("Remove by stable stack id"),I->Remove(I->GetStacks().Last().StackId,3));
    TestEqual(TEXT("Partial remove"),I->Count(TEXT("Item_Wood")),22);
    TestFalse(TEXT("Weight overflow rejected"),I->Grant(TEXT("Item_Stone"),20));
    TestEqual(TEXT("Failed insertion atomic"),I->Count(TEXT("Item_Stone")),0);
    I->SlotLimit=I->GetStacks().Num();
    TestFalse(TEXT("Slot overflow rejected"),I->Grant(TEXT("Item_Food"),1));
    TestFalse(TEXT("Full slots cannot split"),I->Split(I->GetStacks()[0].StackId,1));
    I->SlotLimit=8;
    TestFalse(TEXT("Unknown item"),I->Grant(TEXT("Item_Fake"),1));
    for(const int32 N:{0,-1,MAX_int32}){TestFalse(TEXT("Invalid quantity"),I->Grant(TEXT("Item_Wood"),N));}
    TestFalse(TEXT("Stale stack request"),I->Remove(FGuid::NewGuid(),1));
    TestFalse(TEXT("Cannot remove more than owned"),I->RemoveItem(TEXT("Item_Wood"),23));
    TestTrue(TEXT("Remove all wood"),I->RemoveItem(TEXT("Item_Wood"),22));
    const double Deadline=UPFInventoryComponent::ServerTime(Fixture.GetTestWorld())+2;
    TestTrue(TEXT("Food batch"),I->AddExisting(TEXT("Item_Food"),4,Deadline));
    TestTrue(TEXT("Different batch stays separate"),I->AddExisting(TEXT("Item_Food"),2,Deadline+10));
    TestEqual(TEXT("Two batch slots"),I->GetStacks().Num(),2);
    TestTrue(TEXT("Split fresh batch"),I->Split(I->GetStacks()[0].StackId,2));
    TestEqual(TEXT("Split retains original deadline"),I->GetStacks().Last().ExpiresAt,Deadline);
    TestFalse(TEXT("NaN deadline rejected"),I->AddExisting(TEXT("Item_Food"),1,std::numeric_limits<double>::quiet_NaN()));
    TestFalse(TEXT("Food needs deadline"),I->AddExisting(TEXT("Item_Food"),1,0));
    Owner->SetRole(ROLE_AutonomousProxy);
    TestFalse(TEXT("Client grant rejected"),I->Grant(TEXT("Item_Wood"),1));
    TestFalse(TEXT("Client insert rejected"),I->AddExisting(TEXT("Item_Food"),1,Deadline));
    TestFalse(TEXT("Client remove rejected"),I->Remove(I->GetStacks()[0].StackId,1));
    TestFalse(TEXT("Client split rejected"),I->Split(I->GetStacks()[0].StackId,1));
    Owner->SetRole(ROLE_Authority);
    for(int32 Tick=0;Tick<30;++Tick){Fixture.TickTestWorld(0.1f);} I->PruneExpired();
    TestTrue(TEXT("Fixture actually crossed expiry deadline"),UPFInventoryComponent::ServerTime(Fixture.GetTestWorld())>Deadline);
    TestEqual(TEXT("Only old batch expires, including split"),I->Count(TEXT("Item_Food")),2);
    TestEqual(TEXT("New batch retains deadline"),I->GetStacks()[0].ExpiresAt,Deadline+10);
    TestFalse(TEXT("Expired batch cannot return"),I->AddExisting(TEXT("Item_Food"),2,Deadline));
    auto& D=I->Catalog->Items[0]; D.Weight=-1;
    TestFalse(TEXT("Invalid designer data rejected"),I->Grant(TEXT("Item_Wood"),1));
    AddInfo(TEXT("[PrimalInventory] Atomic capacity, stacking/splitting, invalid requests, authority and batch expiry checked."));
    Fixture.ForwardErrorMessages(this); return true;
}
#endif
