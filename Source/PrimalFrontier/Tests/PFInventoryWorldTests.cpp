// PFInventoryWorldTests.cpp
//
// Automation test: PF.Inventory.WorldTransfers   (M3, 340c538)
// Uses the real GameMode/PlayerState/pawn: saved catalog loads, drop/pickup
// round trip keeps quantity and freshness, full bag leaves the pickup in the world,
// distance and other-player checks, eating applies real nutrition, dead players
// can't drop, and the inventory survives respawn.
// Run: Session Frontend > Automation, filter "PF.Inventory".

#include "Inventory/PFInventoryComponent.h"
#include "Inventory/PFItemCatalog.h"
#include "Inventory/PFItemPickup.h"
#include "Survival/PFSurvivalGameMode.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFSurvivorCharacter.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFInventoryWorldTest,"PF.Inventory.WorldTransfers",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFInventoryWorldTest::RunTest(const FString&)
{
    AddExpectedMessage(TEXT("GetSocketInfoByName.*No SkeletalMesh for Component"),EAutomationExpectedMessageFlags::Contains,0);
    FTestWorldWrapper Fixture; if(!Fixture.CreateTestWorld(EWorldType::Game)){return false;}
    auto* World=Fixture.GetTestWorld();
    World->SetGameMode(FURL(nullptr,TEXT("/Engine/Maps/Entry?game=/Script/PrimalFrontier.PFSurvivalGameMode"),TRAVEL_Absolute));
    World->SpawnActor<APlayerStart>(FVector(0,0,150),FRotator::ZeroRotator);
    if(!Fixture.BeginPlayInTestWorld()){return false;}
    auto* PC=World->SpawnActor<APFSurvivalPlayerController>(); auto* Mode=World->GetAuthGameMode<APFSurvivalGameMode>(); Mode->RestartPlayer(PC);
    auto* Pawn=Cast<APFSurvivorCharacter>(PC->GetPawn()); auto* I=PC->GetInventory();
    if(!TestNotNull(TEXT("Inventory on real PlayerState"),I) || !Pawn){return false;}
    Pawn->GetCharacterMovement()->DisableMovement();
    TestNotNull(TEXT("Saved data asset loaded"),I->Definition(TEXT("Item_Food")));
    TestTrue(TEXT("World grant"),I->Grant(TEXT("Item_Food"),4));
    const auto Source=I->GetStacks()[0];
    auto* Dropped=I->Drop(Source.StackId,2,Pawn);
    if(!TestNotNull(TEXT("Dropped food actor"),Dropped)){return false;}
    TestEqual(TEXT("Drop conserves count"),I->Count(TEXT("Item_Food"))+Dropped->GetContents().Quantity,4);
    TestEqual(TEXT("Drop retains freshness"),Dropped->GetContents().ExpiresAt,Source.ExpiresAt);
    I->WeightLimit=0.5;
    TestFalse(TEXT("Full inventory refuses pickup"),Dropped->TryPickup(Pawn));
    TestTrue(TEXT("Failed pickup leaves actor"),IsValid(Dropped));
    I->WeightLimit=30;
    const FVector Near=Dropped->GetActorLocation(); Dropped->SetActorLocation(FVector(2000,0,150));
    TestFalse(TEXT("Distant pickup rejected"),Dropped->TryPickup(Pawn)); Dropped->SetActorLocation(Near);
    auto* Other=World->SpawnActor<APFSurvivalPlayerController>(); Mode->RestartPlayer(Other);
    TestNull(TEXT("Cannot drop through another player's pawn"),I->Drop(Source.StackId,1,Other->GetPawn()));
    Other->GetPawn()->SetActorLocation(FVector(1000,1000,150));
    TestTrue(TEXT("Pickup succeeds"),Dropped->TryPickup(Pawn));
    TestFalse(TEXT("Duplicate pickup refused"),Dropped->TryPickup(Pawn));
    TestEqual(TEXT("Round trip quantity"),I->Count(TEXT("Item_Food")),4);
    TestEqual(TEXT("Round trip deadline"),I->GetStacks()[0].ExpiresAt,Source.ExpiresAt);
    Pawn->Survival->SetHunger(20); Pawn->Survival->SetThirst(20);
    TestTrue(TEXT("Consume owned food"),I->Consume(Source.StackId,Pawn));
    TestEqual(TEXT("Consumption uses one item"),I->Count(TEXT("Item_Food")),3);
    TestEqual(TEXT("Food applies actual nutrition"),Pawn->Survival->GetVitals().Hunger,55.f);
    TestFalse(TEXT("Arbitrary guid cannot consume"),I->Consume(FGuid::NewGuid(),Pawn));
    Pawn->Survival->ApplyDamage(1000);
    TestNull(TEXT("Dead player cannot drop"),I->Drop(Source.StackId,1,Pawn));
    TestTrue(TEXT("Respawn succeeds"),Mode->RespawnPlayer(PC));
    TestTrue(TEXT("Inventory object survives pawn replacement"),PC->GetInventory()==I);
    TestEqual(TEXT("Respawn retains quantity"),I->Count(TEXT("Item_Food")),3);
    TestEqual(TEXT("Respawn retains expiry"),I->GetStacks()[0].ExpiresAt,Source.ExpiresAt);
    AddInfo(TEXT("[PrimalInventory] Data asset, pickup/drop atomicity, ownership, consumption and respawn verified."));
    Fixture.ForwardErrorMessages(this); return true;
}
#endif
