#if WITH_DEV_AUTOMATION_TESTS
#include "Inventory/PFInventoryComponent.h"
#include "Inventory/PFItemCatalog.h"
#include "Persistence/PFPlayerSaveFormat.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFLargeStackTest,"PF.Inventory.LargeStacks",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFLargeStackTest::RunTest(const FString&)
{
    FTestWorldWrapper F;if(!F.CreateTestWorld(EWorldType::Game)){return false;}
    auto* W=F.GetTestWorld();auto* Owner=W->SpawnActor<AActor>();Owner->SetReplicates(true);
    auto* Bag=NewObject<UPFInventoryComponent>(Owner);Owner->AddInstanceComponent(Bag);
    Bag->Catalog=NewObject<UPFItemCatalog>(Bag);Bag->WeightLimit=20000;Bag->RegisterComponent();
    auto& D=Bag->Catalog->Items[0];D.StackLimit=PFItemLimits::MaxStackQuantity;D.Weight=0.01f;
    auto* Other=W->SpawnActor<AActor>();Other->SetReplicates(true);
    auto* Destination=NewObject<UPFInventoryComponent>(Other);Other->AddInstanceComponent(Destination);
    Destination->Catalog=Bag->Catalog;Destination->WeightLimit=1;Destination->RegisterComponent();
    if(!F.BeginPlayInTestWorld()){return false;}
    TestEqual(TEXT("Literal 999k ceiling"),PFItemLimits::MaxStackQuantity,999000);
    if(!TestTrue(TEXT("Maximum stack inserted atomically"),Bag->Grant(D.Id,999000))){return false;}
    TestEqual(TEXT("One bounded slot, not per-unit records"),Bag->GetStacks().Num(),1);
    TestEqual(TEXT("Exact maximum quantity"),Bag->GetStacks()[0].Quantity,999000);
    TestTrue(TEXT("Real unit weight multiplied by quantity"),FMath::IsNearlyEqual(Bag->GetWeight(),9990.f,0.01f));
    const auto Id=Bag->GetStacks()[0].StackId;
    for(int32 Quantity:{0,-1,999001,MAX_int32}){TestFalse(TEXT("Out of bounded request range refused"),Bag->Grant(D.Id,Quantity));}
    TestEqual(TEXT("Rejected request does not change count"),Bag->Count(D.Id),999000);
    TestTrue(TEXT("Large split preserves quantities"),Bag->Split(Id,333000));
    const auto SplitId=Bag->GetStacks().Last().StackId;
    TestEqual(TEXT("Split conservation"),Bag->Count(D.Id),999000);
    TestFalse(TEXT("Real destination weight refusal atomic"),Bag->TransferTo(Destination,SplitId,333000));
    TestEqual(TEXT("Rejected transfer keeps source"),Bag->Count(D.Id),999000);
    TestEqual(TEXT("Rejected transfer keeps destination empty"),Destination->Count(D.Id),0);
    Destination->WeightLimit=20000;
    TestTrue(TEXT("Whole large batch transfer succeeds once"),Bag->TransferTo(Destination,SplitId,333000));
    TestEqual(TEXT("Transfer conservation"),Bag->Count(D.Id)+Destination->Count(D.Id),999000);
    TestFalse(TEXT("Transferred stale GUID cannot transfer twice"),Bag->TransferTo(Destination,SplitId,333000));
    FPFPlayerSaveData Data;Data.PlayerId=FGuid(1,2,3,4);
    Data.Inventory.Add({Destination->GetStacks()[0].StackId,D.Id,333000,0});
    FPFPlayerSaveLimits Limits;Limits.Weight=20000;TArray<uint8> Bytes;FString Error;
    if(!TestTrue(TEXT("Real V1 archive encodes large stack"),FPFPlayerSaveFormat::Encode(Data,*Bag->Catalog,Limits,Bytes,Error))){AddError(Error);return false;}
    FPFPlayerSaveData Decoded;
    if(!TestTrue(TEXT("Real V1 archive decodes large stack"),FPFPlayerSaveFormat::Decode(Bytes,*Bag->Catalog,Limits,Decoded,Error))){AddError(Error);return false;}
    TestEqual(TEXT("Saved count exact"),Decoded.Inventory[0].Quantity,333000);
    TestTrue(TEXT("Runtime restore validates large batch"),Destination->RestorePersistence(Decoded.Inventory,0,Error));
    TestEqual(TEXT("Restore twice cannot duplicate"),Destination->Count(D.Id),333000);
    TestTrue(TEXT("Repeated runtime restore"),Destination->RestorePersistence(Decoded.Inventory,0,Error));
    TestEqual(TEXT("Second restore same count"),Destination->Count(D.Id),333000);
    Data.Inventory[0].Quantity=999001;const auto PreviousBytes=Bytes;
    TestFalse(TEXT("Per-stack ceiling enforced by save validation"),FPFPlayerSaveFormat::Encode(Data,*Bag->Catalog,Limits,Bytes,Error));
    TestTrue(TEXT("Failed encode preserves output"),Bytes==PreviousBytes);
    D.StackLimit=999001;TestNull(TEXT("Over-ceiling catalog definition fails closed"),Bag->Catalog->Find(D.Id));
    TestFalse(TEXT("Invalid designer ceiling cannot create items"),Bag->Grant(D.Id,1));D.StackLimit=999000;
    Data.Inventory[0].Quantity=999000;
    TestTrue(TEXT("Maximum quantity V1 archive encode"),FPFPlayerSaveFormat::Encode(Data,*Bag->Catalog,Limits,Bytes,Error));
    if(TestTrue(TEXT("Maximum quantity V1 archive decode"),FPFPlayerSaveFormat::Decode(Bytes,*Bag->Catalog,Limits,Decoded,Error)))
    {TestEqual(TEXT("Exact 999k archive round trip"),Decoded.Inventory[0].Quantity,999000);}
    Owner->SetRole(ROLE_AutonomousProxy);
    TestFalse(TEXT("Client large grant denied"),Bag->Grant(D.Id,333000));
    TestFalse(TEXT("Client split denied"),Bag->Split(Id,1000));
    TestFalse(TEXT("Client transfer denied"),Bag->TransferTo(Destination,Id,1000));
    Owner->SetRole(ROLE_Authority);
    TestEqual(TEXT("Client refusal preserves aggregate"),Bag->Count(D.Id)+Destination->Count(D.Id),999000);
    AddInfo(TEXT("[PrimalInventory] 999k boundary, weight, split/transfer/authority and real V1/runtime restore verified. Baseline defaults and soft carrying policy remain separate."));
    F.ForwardErrorMessages(this);return true;
}
#endif
