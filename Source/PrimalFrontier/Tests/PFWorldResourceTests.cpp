#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Crafting/PFCraftingCatalog.h"
#include "Crafting/PFResourceNode.h"
#include "Inventory/PFItemCatalog.h"
#include "Inventory/PFInventoryComponent.h"
#include "Survival/PFSurvivalGameMode.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFSurvivorCharacter.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFWorldResourcesTest,"PF.World.Resources",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFWorldResourcesTest::RunTest(const FString&)
{
    AddExpectedMessage(TEXT("GetSocketInfoByName.*No SkeletalMesh for Component"),EAutomationExpectedMessageFlags::Contains,0);
    FTestWorldWrapper Fixture;
    if(!Fixture.CreateTestWorld(EWorldType::Game)){return false;}
    UWorld* World=Fixture.GetTestWorld();
    World->SetGameMode(FURL(nullptr,TEXT("/Engine/Maps/Entry?game=/Script/PrimalFrontier.PFSurvivalGameMode"),TRAVEL_Absolute));
    World->SpawnActor<APlayerStart>(FVector(0,0,150),FRotator::ZeroRotator);
    if(!Fixture.BeginPlayInTestWorld()){return false;}
    auto* Controller=World->SpawnActor<APFSurvivalPlayerController>();
    World->GetAuthGameMode<APFSurvivalGameMode>()->RestartPlayer(Controller);
    auto* Pawn=Cast<APFSurvivorCharacter>(Controller->GetPawn());
    auto* Inventory=Controller->GetInventory();
    if(!TestNotNull(TEXT("Survivor"),Pawn) || !TestNotNull(TEXT("Inventory"),Inventory)){return false;}
    Pawn->GetCharacterMovement()->DisableMovement();
    Controller->SetControlRotation(FRotator::ZeroRotator);
    Inventory->Catalog=NewObject<UPFItemCatalog>(Inventory);
    auto* Catalog=NewObject<UPFCraftingCatalog>();
    FVector Eye;FRotator Look;Pawn->GetActorEyesViewPoint(Eye,Look);
    auto* Node=World->SpawnActor<APFResourceNode>(Eye+FVector(150,0,0),FRotator::ZeroRotator);
    Node->Items=Inventory->Catalog;Node->Catalog=Catalog;Node->ResourceId=TEXT("Node_Fibre");Node->HitsRemaining=3;
    TestTrue(TEXT("Gather fibre through authoritative aim validation"),Node->Gather(Pawn));
    TestEqual(TEXT("Fibre yield"),Inventory->Count(TEXT("Item_Fibre")),2);
    Node->SetRole(ROLE_SimulatedProxy);
    TestFalse(TEXT("Client cannot gather directly"),Node->Gather(Pawn));
    Node->SetRole(ROLE_Authority);
    // WorldSettings clamps a single long tick; advance real simulation frames
    // so the node's 0.5-second authority cooldown has actually elapsed.
    const double Before=UPFInventoryComponent::ServerTime(World);
    for(int32 Frame=0;Frame<6;++Frame){Fixture.TickTestWorld(0.1f);}
    TestTrue(TEXT("Fixture advances beyond gathering cooldown"),UPFInventoryComponent::ServerTime(World)-Before>0.5);
    Node->ResourceId=TEXT("Node_Water");Node->HitsRemaining=1;
    Pawn->Survival->SetHunger(60);Pawn->Survival->SetThirst(10);
    TestTrue(TEXT("Collect finite water portions"),Node->Gather(Pawn));
    TestEqual(TEXT("Water node depletes"),Node->HitsRemaining,0);
    TestFalse(TEXT("Depleted water cannot be harvested"),Node->Gather(Pawn));
    TestEqual(TEXT("Collected two portions"),Inventory->Count(TEXT("Item_Water")),2);
    const auto* Stack=Inventory->GetStacks().FindByPredicate([](const FPFItemStack& S){return S.ItemId==TEXT("Item_Water");});
    if(!TestNotNull(TEXT("Water stack"),Stack)){return false;}
    const FGuid WaterId=Stack->StackId;
    Controller->PlayerState->SetRole(ROLE_AutonomousProxy);
    TestFalse(TEXT("Client cannot consume directly"),Inventory->Consume(WaterId,Pawn));
    Controller->PlayerState->SetRole(ROLE_Authority);
    TestTrue(TEXT("Server drinks one portion"),Inventory->Consume(WaterId,Pawn));
    TestEqual(TEXT("One portion spent"),Inventory->Count(TEXT("Item_Water")),1);
    TestEqual(TEXT("Water restores thirst"),Pawn->Survival->GetVitals().Thirst,45.f);
    TestEqual(TEXT("Water does not feed player"),Pawn->Survival->GetVitals().Hunger,60.f);
    Pawn->Survival->SetThirst(100);
    TestFalse(TEXT("Full thirst does not waste water"),Inventory->Consume(WaterId,Pawn));
    TestEqual(TEXT("Refusal preserves portion"),Inventory->Count(TEXT("Item_Water")),1);
    AddInfo(TEXT("[PrimalWorld] Fibre and finite water collection, consumption and authority verified."));
    Fixture.ForwardErrorMessages(this);return true;
}
#endif
