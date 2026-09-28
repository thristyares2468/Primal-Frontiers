#include "Crafting/PFCraftingComponent.h"
#include "Crafting/PFResourceNode.h"
#include "Inventory/PFItemCatalog.h"
#include "Survival/PFSurvivalGameMode.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFSurvivorCharacter.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Engine/World.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFCraftingTest,"PF.Crafting.Transactions",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFCraftingTest::RunTest(const FString&)
{
    AddExpectedMessage(TEXT("GetSocketInfoByName.*No SkeletalMesh for Component"),EAutomationExpectedMessageFlags::Contains,0);
    FTestWorldWrapper F;if(!F.CreateTestWorld(EWorldType::Game)){return false;}auto* W=F.GetTestWorld();
    W->SetGameMode(FURL(nullptr,TEXT("/Engine/Maps/Entry?game=/Script/PrimalFrontier.PFSurvivalGameMode"),TRAVEL_Absolute));
    W->SpawnActor<APlayerStart>(FVector(0,0,150),FRotator::ZeroRotator);if(!F.BeginPlayInTestWorld()){return false;}
    auto* PC=W->SpawnActor<APFSurvivalPlayerController>();W->GetAuthGameMode<APFSurvivalGameMode>()->RestartPlayer(PC);
    auto* P=Cast<APFSurvivorCharacter>(PC->GetPawn());auto* I=PC->GetInventory();
    auto* C=PC->PlayerState->FindComponentByClass<UPFCraftingComponent>();
    if(!P || !I || !TestNotNull(TEXT("Crafting component"),C)){return false;}
    P->GetCharacterMovement()->DisableMovement();I->Catalog=NewObject<UPFItemCatalog>(I);C->Catalog=NewObject<UPFCraftingCatalog>(C);
    for(auto& R:C->Catalog->Recipes){R.Duration=0.5f;}
    auto Advance=[&](int32 Ticks=8){for(int32 N=0;N<Ticks;++N){F.TickTestWorld(0.1f);}};
    TestFalse(TEXT("Insufficient ingredients"),C->Start(TEXT("Recipe_Tool"),P));
    I->Grant(TEXT("Item_Wood"),6);I->Grant(TEXT("Item_Stone"),4);
    TestTrue(TEXT("Start recipe"),C->Start(TEXT("Recipe_Tool"),P));
    TestFalse(TEXT("Cannot queue duplicate"),C->Start(TEXT("Recipe_Tool"),P));
    TestTrue(TEXT("Cancel before completion"),C->Cancel());Advance();
    TestEqual(TEXT("Cancel no output"),I->Count(TEXT("Item_Tool")),0);TestEqual(TEXT("Cancel consumes nothing"),I->Count(TEXT("Item_Wood")),6);
    TestTrue(TEXT("Restart"),C->Start(TEXT("Recipe_Tool"),P));Advance();
    TestEqual(TEXT("One tool produced"),I->Count(TEXT("Item_Tool")),1);TestEqual(TEXT("Wood consumed once"),I->Count(TEXT("Item_Wood")),3);
    Advance();TestEqual(TEXT("Completion not repeated"),I->Count(TEXT("Item_Tool")),1);
    TestTrue(TEXT("Start output-capacity test"),C->Start(TEXT("Recipe_Tool"),P));I->WeightLimit=0.1f;Advance();I->WeightLimit=30;
    TestEqual(TEXT("Output failure preserves ingredients"),I->Count(TEXT("Item_Wood")),3);TestEqual(TEXT("No overflow output"),I->Count(TEXT("Item_Tool")),1);
    TestTrue(TEXT("Start moved ingredient test"),C->Start(TEXT("Recipe_Tool"),P));I->RemoveItem(TEXT("Item_Stone"),2);Advance();
    TestEqual(TEXT("Missing ingredient no output"),I->Count(TEXT("Item_Tool")),1);TestEqual(TEXT("Other input not partially consumed"),I->Count(TEXT("Item_Wood")),3);
    I->AddExisting(TEXT("Item_Food"),1,UPFInventoryComponent::ServerTime(W)+0.25);
    TestTrue(TEXT("Start with soon expiring input"),C->Start(TEXT("Recipe_Cook"),P));Advance();
    TestEqual(TEXT("Spoiled input cannot cook"),I->Count(TEXT("Item_CookedFood")),0);TestEqual(TEXT("Spoilage failure retains fuel"),I->Count(TEXT("Item_Wood")),3);
    I->Grant(TEXT("Item_Food"),4);TestTrue(TEXT("Cook usable input"),C->Start(TEXT("Recipe_Cook"),P));Advance();
    TestEqual(TEXT("Cooked food"),I->Count(TEXT("Item_CookedFood")),1);
    TestTrue(TEXT("Dry usable input"),C->Start(TEXT("Recipe_Dry"),P));Advance();TestEqual(TEXT("Dried food"),I->Count(TEXT("Item_DriedFood")),1);
    auto Forged=I->GetStacks();Forged[0].ExpiresAt+=1;TestFalse(TEXT("Forged input deadline rejected"),I->Transform(Forged,TEXT("Item_Wood"),1));
    PC->PlayerState->SetRole(ROLE_AutonomousProxy);
    TestFalse(TEXT("Client direct craft rejected"),C->Start(TEXT("Recipe_Cook"),P));TestFalse(TEXT("Client transform rejected"),I->Transform(I->GetStacks(),TEXT("Item_Wood"),1));
    PC->PlayerState->SetRole(ROLE_Authority);
    TestFalse(TEXT("Unknown recipe rejected"),C->Start(TEXT("Recipe_Unknown"),P));
    I->Grant(TEXT("Item_Wood"),1);TestTrue(TEXT("Start before death"),C->Start(TEXT("Recipe_Cook"),P));P->Survival->ApplyDamage(1000);Advance();
    TestTrue(TEXT("Death cancels work"),C->ActiveRecipe.IsNone());TestEqual(TEXT("Death does not grant cooked item"),I->Count(TEXT("Item_CookedFood")),1);
    AddInfo(TEXT("[PrimalCrafting] Timed conversion, cancellation, capacity, stale input, fuel, death, authority and duplication checked."));F.ForwardErrorMessages(this);return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFGatheringTest,"PF.Crafting.Gathering",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFGatheringTest::RunTest(const FString&)
{
    AddExpectedMessage(TEXT("GetSocketInfoByName.*No SkeletalMesh for Component"),EAutomationExpectedMessageFlags::Contains,0);
    FTestWorldWrapper F;if(!F.CreateTestWorld(EWorldType::Game)){return false;}auto* W=F.GetTestWorld();
    W->SetGameMode(FURL(nullptr,TEXT("/Engine/Maps/Entry?game=/Script/PrimalFrontier.PFSurvivalGameMode"),TRAVEL_Absolute));
    W->SpawnActor<APlayerStart>(FVector(0,0,150),FRotator::ZeroRotator);if(!F.BeginPlayInTestWorld()){return false;}
    auto* PC=W->SpawnActor<APFSurvivalPlayerController>();W->GetAuthGameMode<APFSurvivalGameMode>()->RestartPlayer(PC);
    auto* P=Cast<APFSurvivorCharacter>(PC->GetPawn());auto* I=PC->GetInventory();if(!P || !I){return false;}
    P->GetCharacterMovement()->DisableMovement();PC->SetControlRotation(FRotator::ZeroRotator);
    FVector Eye;FRotator Look;P->GetActorEyesViewPoint(Eye,Look);
    const FTransform T(FRotator::ZeroRotator,Eye+Look.Vector()*150);
    auto* N=W->SpawnActorDeferred<APFResourceNode>(APFResourceNode::StaticClass(),T);
    N->Catalog=NewObject<UPFCraftingCatalog>(N);N->Items=NewObject<UPFItemCatalog>(N);N->Catalog->Resources[0].RespawnSeconds=1;N->FinishSpawning(T);
    auto Advance=[&](int32 Ticks){for(int32 K=0;K<Ticks;++K){F.TickTestWorld(0.1f);}};
    TestTrue(TEXT("Gather actual traced node"),N->Gather(P));TestEqual(TEXT("Yield received"),I->Count(TEXT("Item_Wood")),2);
    TestFalse(TEXT("Spam rejected"),N->Gather(P));Advance(6);I->WeightLimit=1;
    TestFalse(TEXT("Capacity failure"),N->Gather(P));TestEqual(TEXT("Capacity failure leaves resource"),N->HitsRemaining,2);I->WeightLimit=30;
    I->Grant(TEXT("Item_Tool"),1);TestTrue(TEXT("Tool gathers twice per action"),N->Gather(P));
    TestEqual(TEXT("Depleted"),N->HitsRemaining,0);TestEqual(TEXT("Conserved finite yield"),I->Count(TEXT("Item_Wood")),6);
    TestFalse(TEXT("Cannot gather depleted node"),N->Gather(P));Advance(15);TestEqual(TEXT("Resource respawn"),N->HitsRemaining,3);
    N->SetActorLocation(Eye+Look.Vector()*1000);TestFalse(TEXT("Out of reach rejected"),N->Gather(P));N->SetActorLocation(T.GetLocation());
    N->SetRole(ROLE_SimulatedProxy);TestFalse(TEXT("Client direct gather rejected"),N->Gather(P));N->SetRole(ROLE_Authority);
    PC->SetControlRotation(FRotator(0,180,0));TestFalse(TEXT("Looking away rejected"),N->Gather(P));
    AddInfo(TEXT("[PrimalGathering] Reach, aim, cooldown, finite yield, capacity, tool, depletion, respawn and authority checked."));F.ForwardErrorMessages(this);return true;
}
#endif
