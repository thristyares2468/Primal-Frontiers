#if WITH_DEV_AUTOMATION_TESTS
#include "Crafting/PFCraftingStation.h"
#include "Crafting/PFCraftingComponent.h"
#include "Inventory/PFItemCatalog.h"
#include "Survival/PFSurvivalGameMode.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFSurvivorCharacter.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFCampStationTest,"PF.Crafting.Stations",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FPFCampStationTest::RunTest(const FString&)
{
    AddExpectedMessage(TEXT("GetSocketInfoByName.*No SkeletalMesh for Component"),EAutomationExpectedMessageFlags::Contains,0);
    FTestWorldWrapper F;if(!F.CreateTestWorld(EWorldType::Game)){return false;}auto* W=F.GetTestWorld();
    W->SetGameMode(FURL(nullptr,TEXT("/Engine/Maps/Entry?game=/Script/PrimalFrontier.PFSurvivalGameMode"),TRAVEL_Absolute));
    W->SpawnActor<APlayerStart>(FVector(0,0,150),FRotator::ZeroRotator);if(!F.BeginPlayInTestWorld()){return false;}
    auto* PC=W->SpawnActor<APFSurvivalPlayerController>();W->GetAuthGameMode<APFSurvivalGameMode>()->RestartPlayer(PC);
    auto* P=Cast<APFSurvivorCharacter>(PC->GetPawn());auto* C=PC->GetCrafting();auto* I=PC->GetInventory();
    if(!P || !C || !I){return false;}P->GetCharacterMovement()->DisableMovement();P->SetActorLocation(FVector(0,0,150));
    C->Catalog=NewObject<UPFCraftingCatalog>(C);I->Catalog=NewObject<UPFItemCatalog>(I);
    P->Survival->HungerDrainPerSecond=0;P->Survival->ThirstDrainPerSecond=0;
    auto Tick=[&](int32 Count){for(int32 K=0;K<Count;++K){F.TickTestWorld(.1f);}};
    TestTrue(TEXT("Inputs granted"),I->Grant(TEXT("Item_Wood"),10) && I->Grant(TEXT("Item_Stone"),4));
    TestTrue(TEXT("Existing portable craft remains available"),C->Start(TEXT("Recipe_Tool"),P));
    TestEqual(TEXT("Portable duration unchanged"),C->FinishAt-UPFInventoryComponent::ServerTime(W),5.0);
    TestTrue(TEXT("Cancel portable"),C->Cancel());
    auto* Bench=W->SpawnActor<APFCraftingStation>(FVector(120,0,150),FRotator::ZeroRotator);
    const auto* Tool=C->Catalog->Recipe(TEXT("Recipe_Tool"),I->Catalog);const auto* Metal=C->Catalog->Recipe(TEXT("Recipe_CopperRefine"),I->Catalog);
    if(!Bench || !Tool || !Metal){return false;}
    TestTrue(TEXT("Physical bench selected"),APFCraftingStation::FindBest(*Tool,P,true)==Bench);
    TestTrue(TEXT("Start near bench"),C->Start(Tool->Id,P));
    TestEqual(TEXT("Server chooses 125 percent station"),C->ActiveStationMultiplier,1.25f);
    TestEqual(TEXT("Station duration"),C->FinishAt-UPFInventoryComponent::ServerTime(W),4.0);
    TestFalse(TEXT("Duplicate start refused"),C->Start(Tool->Id,P));
    P->SetActorLocation(FVector(-1000,0,150));Tick(2);
    TestTrue(TEXT("Leaving cancels"),C->ActiveRecipe.IsNone() && C->Feedback.Contains(TEXT("ingredients kept")));
    TestEqual(TEXT("Leaving preserves wood"),I->Count(TEXT("Item_Wood")),10);TestEqual(TEXT("Leaving creates no output"),I->Count(TEXT("Item_Tool")),0);
    P->SetActorLocation(FVector(0,0,150));TestTrue(TEXT("Restart at bench"),C->Start(Tool->Id,P));
    auto* Wall=W->SpawnActor<AActor>();auto* Box=NewObject<UBoxComponent>(Wall);Wall->SetRootComponent(Box);Box->RegisterComponent();Box->SetBoxExtent(FVector(10,80,150));Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);Box->SetCollisionResponseToAllChannels(ECR_Block);Wall->SetActorLocation(FVector(60,0,200));
    Tick(2);TestTrue(TEXT("Occlusion cancels boosted job"),C->ActiveRecipe.IsNone());
    TestTrue(TEXT("Occluded bench never grants bonus"),APFCraftingStation::FindBest(*Tool,P,true)==nullptr);
    TestTrue(TEXT("Portable still works behind wall"),C->Start(Tool->Id,P));TestEqual(TEXT("No hidden bonus"),C->ActiveStationMultiplier,1.f);C->Cancel();Wall->Destroy();Tick(1);
    Bench->SetRole(ROLE_SimulatedProxy);TestTrue(TEXT("Client-role station ignored for authority"),APFCraftingStation::FindBest(*Tool,P,true)==nullptr);Bench->SetRole(ROLE_Authority);
    PC->PlayerState->SetRole(ROLE_AutonomousProxy);TestFalse(TEXT("Client direct start refused"),C->Start(Tool->Id,P));PC->PlayerState->SetRole(ROLE_Authority);
    Bench->SpeedMultiplier=100;TestFalse(TEXT("Invalid multiplier fails closed"),Bench->Supports(*Tool));Bench->SpeedMultiplier=1.25f;
    Bench->StationType=FGameplayTag();TestFalse(TEXT("Unknown station tag fails closed"),Bench->Supports(*Tool));Bench->StationType=APFCraftingStation::WorkbenchType();
    auto* Refinery=W->SpawnActor<APFCraftingStation>(FVector(120,100,150),FRotator::ZeroRotator);Refinery->StationType=APFCraftingStation::RefineryType();Refinery->SpeedMultiplier=1.5f;
    TestTrue(TEXT("Refinery wins metal bonus, no stacked stations"),APFCraftingStation::FindBest(*Metal,P,true)==Refinery);
    TestFalse(TEXT("Refinery doesn't make tools"),Refinery->Supports(*Tool));
    TestTrue(TEXT("Food has no camp station bonus"),APFCraftingStation::FindBest(*C->Catalog->Recipe(TEXT("Recipe_Cook"),I->Catalog),P,true)==nullptr);
    Refinery->Destroy();Tick(1);TestTrue(TEXT("Start final real tool craft"),C->Start(Tool->Id,P));Tick(30);
    TestEqual(TEXT("Not prematurely completed"),I->Count(TEXT("Item_Tool")),0);Tick(15);
    TestEqual(TEXT("One completed output"),I->Count(TEXT("Item_Tool")),1);TestEqual(TEXT("Wood consumed exactly once"),I->Count(TEXT("Item_Wood")),7);TestEqual(TEXT("Stone consumed exactly once"),I->Count(TEXT("Item_Stone")),2);
    TestTrue(TEXT("Another job can start"),C->Start(Tool->Id,P));Bench->Destroy();Tick(2);
    TestTrue(TEXT("Destroyed station cancels"),C->ActiveRecipe.IsNone());TestEqual(TEXT("Destroyed station keeps remaining ingredients"),I->Count(TEXT("Item_Wood")),7);
    AddInfo(TEXT("[PrimalCrafting] Station authority, range, occlusion, invalid configuration, portable compatibility and atomic output conservation verified."));
    F.ForwardErrorMessages(this);return true;
}
#endif
