#if WITH_DEV_AUTOMATION_TESTS
#include "Progression/PFProgressionComponent.h"
#include "Inventory/PFInventoryPlayerState.h"
#include "Inventory/PFInventoryComponent.h"
#include "Inventory/PFItemPickup.h"
#include "Crafting/PFResourceNode.h"
#include "Survival/PFSurvivalGameMode.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFSurvivorCharacter.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Survival/PFInteraction.h"
#include "Persistence/PFWorldPersistence.h"
#include "World/PFWorldClock.h"
#include "Engine/World.h"
#include "GameFramework/PlayerStart.h"
#include "Components/BoxComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "Tests/AutomationCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFGatherEventsTest,"PF.Progression.GatherEvents",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFGatherEventsTest::RunTest(const FString&)
{
    AddExpectedMessage(TEXT("GetSocketInfoByName.*No SkeletalMesh for Component"),EAutomationExpectedMessageFlags::Contains,0);
    FTestWorldWrapper Fixture;if(!Fixture.CreateTestWorld(EWorldType::Game)){return false;}auto* W=Fixture.GetTestWorld();
    W->SetGameMode(FURL(nullptr,TEXT("/Engine/Maps/Entry?game=/Script/PrimalFrontier.PFSurvivalGameMode"),TRAVEL_Absolute));
    auto* Floor=W->SpawnActor<AActor>();auto* Box=NewObject<UBoxComponent>(Floor);Floor->SetRootComponent(Box);Box->SetBoxExtent(FVector(4000,4000,25));Box->SetCollisionProfileName(TEXT("BlockAll"));Box->RegisterComponent();Floor->SetActorLocation(FVector(0,0,-25));
    W->SpawnActor<APlayerStart>(FVector(-500,0,120),FRotator::ZeroRotator);auto* Clock=W->SpawnActor<APFWorldClock>();if(!Fixture.BeginPlayInTestWorld()){return false;}
    auto* PC=W->SpawnActor<APFSurvivalPlayerController>();W->GetAuthGameMode<APFSurvivalGameMode>()->RestartPlayer(PC);
    auto* PS=PC->GetPlayerState<APFInventoryPlayerState>();auto* P=Cast<APFSurvivorCharacter>(PC->GetPawn());if(!PS || !P || !PS->Progression){return false;}
    auto* G=PS->Progression.Get();auto* I=PS->Inventory.Get();FString Error;
    P->GetCharacterMovement()->DisableMovement();P->Survival->HungerDrainPerSecond=0;P->Survival->ThirstDrainPerSecond=0;PC->SetControlRotation(FRotator::ZeroRotator);
    I->SlotLimit=64;I->WeightLimit=100;W->GetSubsystem<UPFWorldPersistence>()->ActiveSlot.Reset();
    auto Advance=[&](){for(int32 N=0;N<6;++N){Fixture.TickTestWorld(0.1f);}};
    auto Spawn=[&](FName Id){FVector Eye;FRotator Look;P->GetActorEyesViewPoint(Eye,Look);const FTransform At(FRotator::ZeroRotator,Eye+Look.Vector()*150);auto* Node=W->SpawnActorDeferred<APFResourceNode>(APFResourceNode::StaticClass(),At,P,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);if(Node){Node->ResourceId=Id;Node->FinishSpawning(At);}return Node;};
    auto Once=[&](FName Id){auto* Node=Spawn(Id);if(!Node){return false;}ON_SCOPE_EXIT{Node->Destroy();};Advance();return TestTrue(TEXT("Owned real target trace"),PFInteraction::FindTarget(P)==Node) && TestTrue(TEXT("Actual finite gather accepts"),Node->Gather(P));};
    auto* Node=Spawn(TEXT("Node_Wood"));if(!Node){return false;}ON_SCOPE_EXIT{if(IsValid(Node)){Node->Destroy();}};Advance();
    I->WeightLimit=0.01f;TestFalse(TEXT("Full bag no gather"),Node->Gather(P));TestTrue(TEXT("Full bag preserves hits/items/XP/windows"),Node->HitsRemaining==3 && I->GetStacks().IsEmpty() && G->GetExperience()==0 && G->GetRecord().GatherWindows.IsEmpty());I->WeightLimit=100;
    PC->SetControlRotation(FRotator(0,180,0));TestFalse(TEXT("Wrong aim no gather or reward"),Node->Gather(P));PC->SetControlRotation(FRotator::ZeroRotator);
    Node->SetRole(ROLE_SimulatedProxy);TestFalse(TEXT("Client node cannot gather"),Node->Gather(P));Node->SetRole(ROLE_Authority);
    P->SetRole(ROLE_AutonomousProxy);TestFalse(TEXT("Client pawn cannot gather"),Node->Gather(P));P->SetRole(ROLE_Authority);
    PS->SetRole(ROLE_AutonomousProxy);TestFalse(TEXT("Client PlayerState cannot gain XP"),Node->Gather(P));PS->SetRole(ROLE_Authority);
    PC->SetRole(ROLE_AutonomousProxy);TestFalse(TEXT("Client controller cannot gain XP"),Node->Gather(P));PC->SetRole(ROLE_Authority);
    auto* Other=W->SpawnActor<APFSurvivalPlayerController>();W->GetAuthGameMode<APFSurvivalGameMode>()->RestartPlayer(Other);Other->GetPawn()->SetActorLocation(FVector(-1000,600,100));
    PC->PlayerState=Other->PlayerState;TestFalse(TEXT("Foreign controller state cannot award to pawn state"),Node->Gather(P));PC->PlayerState=PS;
    Node->ResourceId=TEXT("Node_Unknown");TestFalse(TEXT("Unknown resource cannot yield or reward"),Node->Gather(P));Node->ResourceId=TEXT("Node_Wood");
    TestTrue(TEXT("All invalid requests leave record and node unchanged"),G->GetExperience()==0 && G->GetRecord().GatherWindows.IsEmpty() && Node->HitsRemaining==3 && I->GetStacks().IsEmpty());
    const double FirstAt=W->GetTimeSeconds();TestTrue(TEXT("Actual first gather"),Node->Gather(P));TestTrue(TEXT("One successful barehand event earns five"),G->GetExperience()==5 && Node->HitsRemaining==2 && I->Count(TEXT("Item_Wood"))==2 && G->GetRecord().GatherWindows.Num()==1 && G->GetRecord().GatherWindows[0].Rewards==1);
    TestFalse(TEXT("Immediate cooldown gather refuses"),Node->Gather(P));TestEqual(TEXT("Cooldown earns nothing"),G->GetExperience(),5);Node->Destroy();Node=nullptr;
    for(int32 N=1;N<5;++N){if(!Once(TEXT("Node_Wood"))){return false;}}
    TestTrue(TEXT("Five real events reach category budget"),G->GetExperience()==25 && G->GetRecord().GatherWindows[0].Rewards==5 && I->Count(TEXT("Item_Wood"))==10);
    if(!Once(TEXT("Node_Wood"))){return false;}TestTrue(TEXT("Exhausted budget still permits ordinary gather"),G->GetExperience()==25 && I->Count(TEXT("Item_Wood"))==12 && G->GetRecord().GatherWindows[0].Rewards==5);
    for(FName Id:{FName(TEXT("Node_Stone")),FName(TEXT("Node_Fibre")),FName(TEXT("Node_Food")),FName(TEXT("Node_Water"))}){for(int32 N=0;N<5;++N){if(!Once(Id)){return false;}}}
    FPFProgressionRecord Captured;if(!TestTrue(TEXT("Five independent categories snapshot"),G->Capture(Captured,Error))){return false;}
    TestTrue(TEXT("Only25 successful category events earn125XP/level2"),G->GetExperience()==125 && G->GetLevel()==2 && G->GetAvailablePoints()==3 && Captured.GatherWindows.Num()==5 && Captured.CreditedCrafts.IsEmpty());
    for(const auto& Window:Captured.GatherWindows){TestEqual(TEXT("Each category exhausted independently"),Window.Rewards,5);}
    const auto Wood=FPFProgressionTransactions::GatherCategoryForItem(TEXT("Item_Wood"));const auto* WoodWindow=Captured.GatherWindows.FindByPredicate([&](const auto& X){return X.Category==Wood;});
    TestTrue(TEXT("Cross-category activity never renews first window"),WoodWindow && FMath::IsNearlyEqual(WoodWindow->RemainingSeconds,1800-(W->GetTimeSeconds()-FirstAt),0.000001));
    const double WoodRemaining=WoodWindow?WoodWindow->RemainingSeconds:0;
    Clock->SetHour(23);if(!TestTrue(TEXT("Time of day snapshot remains valid"),G->Capture(Captured,Error))){return false;}WoodWindow=Captured.GatherWindows.FindByPredicate([&](const auto& X){return X.Category==Wood;});
    TestTrue(TEXT("Time of day never renews window"),WoodWindow && WoodWindow->RemainingSeconds==WoodRemaining);
    // Grants/drop/recovery are not gathering events. They must conserve this full budget.
    TestTrue(TEXT("Trusted item grant"),I->Grant(TEXT("Item_BoundTool"),1));TestEqual(TEXT("Tool grant no reward"),G->GetExperience(),125);
    const auto* Stack=I->GetStacks().FindByPredicate([](const auto& S){return S.ItemId==TEXT("Item_Wood");});if(!Stack){return false;}auto* Pickup=I->Drop(Stack->StackId,1,P);
    if(!TestNotNull(TEXT("Actual owned drop"),Pickup)){return false;}TestTrue(TEXT("Normal trace targets dropped item"),PFInteraction::FindTarget(P)==Pickup);TestTrue(TEXT("Actual recovery succeeds"),Pickup->TryPickup(P));TestTrue(TEXT("Drop/recovery no credits or craft ledger"),G->GetExperience()==125 && G->GetRecord().CreditedCrafts.IsEmpty());
    W->TimeSeconds=FirstAt+1800; // Deterministic boundary fixture; live pacing is a separate playtest.
    if(!Once(TEXT("Node_Wood"))){return false;}TestTrue(TEXT("Expired real category earns once again, one action for threehits"),G->GetExperience()==130 && I->GatheringHits()==3 && G->GetRecord().GatherWindows.FindByPredicate([&](const auto& X){return X.Category==Wood;})->Rewards==1);
    W->TimeSeconds=FirstAt+1810;
    Node=Spawn(TEXT("Node_Stone"));if(!Node){return false;}Advance();const int32 Before=I->Count(TEXT("Item_Stone"));TestTrue(TEXT("Improved tool gathers real finite node"),Node->Gather(P));
    TestTrue(TEXT("Threehit yield gives one reward, not three"),Node->HitsRemaining==0 && I->Count(TEXT("Item_Stone"))==Before+6 && G->GetExperience()==135);
    TestFalse(TEXT("Depleted node earns no repeat"),Node->Gather(P));TestEqual(TEXT("Depletion XP unchanged"),G->GetExperience(),135);Node->Destroy();Node=nullptr;
    FPFProgressionRecord Cap;if(!G->Capture(Cap,Error)){return false;}Cap.Experience=2700;TestTrue(TEXT("Trusted cap boundary seed"),G->Restore(Cap,Error));
    if(!Once(TEXT("Node_Fibre"))){return false;}TestEqual(TEXT("Cap does not block ordinary gather or inflate XP"),G->GetExperience(),2700);
    Node=Spawn(TEXT("Node_Wood"));if(!Node){return false;}Advance();P->Survival->SetHealth(0);const auto AtDeath=G->GetRecord().GatherWindows;const auto ItemsAtDeath=I->Count(TEXT("Item_Wood"));
    TestFalse(TEXT("Dead survivor no gathering or reward"),Node->Gather(P));TestTrue(TEXT("Dead refusal preserves node/XP/window/items"),Node->HitsRemaining==3 && G->GetExperience()==2700 && G->GetRecord().GatherWindows==AtDeath && I->Count(TEXT("Item_Wood"))==ItemsAtDeath);
    TestTrue(TEXT("Foreign player progression never credited"),Other->GetPlayerState<APFInventoryPlayerState>()->Progression->GetExperience()==0 && Other->GetPlayerState<APFInventoryPlayerState>()->Progression->GetRecord().GatherWindows.IsEmpty());
    Fixture.ForwardErrorMessages(this);AddInfo(TEXT("[PrimalAgentTools] GatherEvents: real owned trace/node/inventory yields, five independent budgets125XP, active expiry, one credit per threehit action, cooldown/full/dead/client/foreign/depletion refusal, grants/drop/recovery zero XP. Explicit tool/cap and clock boundary fixtures; no network/manual pacing certificate."));return true;
}
#endif
