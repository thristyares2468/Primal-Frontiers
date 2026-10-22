#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Crafting/PFCraftingComponent.h"
#include "Crafting/PFCraftingStation.h"
#include "Inventory/PFItemCatalog.h"
#include "Inventory/PFInventoryPlayerState.h"
#include "Progression/PFProgressionComponent.h"
#include "Progression/PFProgressionCatalog.h"
#include "Survival/PFSurvivalGameMode.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFSurvivorCharacter.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UI/PFRecipeDetails.h"
#include "Tests/AutomationCommon.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFBatchRefiningTest,"PF.Crafting.BatchRefining",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FPFBatchRefiningTest::RunTest(const FString&)
{
    AddExpectedMessage(TEXT("GetSocketInfoByName.*No SkeletalMesh for Component"),EAutomationExpectedMessageFlags::Contains,0);
    FTestWorldWrapper F;if(!F.CreateTestWorld(EWorldType::Game)){return false;}auto* W=F.GetTestWorld();
    W->SetGameMode(FURL(nullptr,TEXT("/Engine/Maps/Entry?game=/Script/PrimalFrontier.PFSurvivalGameMode"),TRAVEL_Absolute));
    W->SpawnActor<APlayerStart>(FVector(0,0,150),FRotator::ZeroRotator);if(!F.BeginPlayInTestWorld()){return false;}
    auto* PC=W->SpawnActor<APFSurvivalPlayerController>();W->GetAuthGameMode<APFSurvivalGameMode>()->RestartPlayer(PC);
    auto* P=Cast<APFSurvivorCharacter>(PC->GetPawn());auto* C=PC->GetCrafting();auto* I=PC->GetInventory();
    if(!P || !C || !I || !C->Catalog || !I->Catalog){return false;}
    P->GetCharacterMovement()->DisableMovement();P->SetActorLocation(FVector(0,0,150));P->Survival->HungerDrainPerSecond=0;P->Survival->ThirstDrainPerSecond=0;
    auto Tick=[&](int32 N){for(int32 K=0;K<N;++K){F.TickTestWorld(.1f);}};
    const FName Batch=TEXT("Recipe_CopperBatch");const auto* R=C->Catalog->Recipe(Batch,I->Catalog);
    const auto* Iron=C->Catalog->Recipe(TEXT("Recipe_IronBatch"),I->Catalog);
    if(!TestNotNull(TEXT("Actual loaded copper batch"),R) || !TestNotNull(TEXT("Actual loaded iron batch"),Iron)){return false;}
    FString Error;TestTrue(TEXT("Loaded catalogs and all knowledge remain valid"),GetDefault<UPFProgressionCatalog>()->Validate(*C->Catalog,*I->Catalog,Error));
    TestTrue(TEXT("Two outputs, required refinery, unchanged portable default"),R->OutputQuantity==2 && R->RequiredStation==APFCraftingStation::RefineryType() && !C->Catalog->Recipe(TEXT("Recipe_CopperRefine"),I->Catalog)->RequiredStation.IsValid());
    auto* G=PC->GetPlayerState<APFInventoryPlayerState>()->Progression.Get();
    TestFalse(TEXT("Iron batch needs metalworking knowledge"),G->CanCraftRecipe(Iron->Id,Error));
    auto* Invalid=NewObject<UPFCraftingCatalog>();auto* Bad=Invalid->Recipes.FindByPredicate([&](const auto& D){return D.Id==Batch;});
    Bad->RequiredStation=FGameplayTag::RequestGameplayTag(TEXT("Item.Category.Resource"));
    TestTrue(TEXT("Known unrelated tag refused as station requirement"),Invalid->Recipe(Batch,I->Catalog)==nullptr);
    TestTrue(TEXT("Fixture ore and wood"),I->Grant(TEXT("Item_CopperOre"),12) && I->Grant(TEXT("Item_Wood"),4));
    TestFalse(TEXT("No station refuses without any inputs consumed"),C->Start(Batch,P));TestTrue(TEXT("Explicit station refusal"),C->Feedback.Contains(TEXT("Requires refinery")));
    auto* Station=W->SpawnActor<APFCraftingStation>(FVector(120,0,150),FRotator::ZeroRotator);
    TestFalse(TEXT("Workbench cannot satisfy refinery requirement"),C->Start(Batch,P));
    Station->StationType=APFCraftingStation::RefineryType();Station->SpeedMultiplier=1.5f;
    Station->SetRole(ROLE_SimulatedProxy);TestFalse(TEXT("Client role provider refused"),C->Start(Batch,P));Station->SetRole(ROLE_Authority);
    PC->PlayerState->SetRole(ROLE_AutonomousProxy);TestFalse(TEXT("Client direct craft refused"),C->Start(Batch,P));PC->PlayerState->SetRole(ROLE_Authority);
    const auto Detail=PFRecipeDetails::Describe(R,I,UPFInventoryComponent::ServerTime(W),false);
    TestTrue(TEXT("Details show output, fuel cost and required station"),Detail.Body.ToString().Contains(TEXT("Makes 2")) && Detail.Body.ToString().Contains(TEXT("Wood 4/1")) && Detail.Body.ToString().Contains(TEXT("Requires refinery")));
    TestTrue(TEXT("Actual required-station job"),C->Start(Batch,P));TestEqual(TEXT("Server chooses eight second copper batch"),C->FinishAt-UPFInventoryComponent::ServerTime(W),8.0);
    TestFalse(TEXT("Duplicate craft refused"),C->Start(Batch,P));TestEqual(TEXT("Fuel not consumed at start"),I->Count(TEXT("Item_Wood")),4);
    P->SetActorLocation(FVector(-1000,0,150));Tick(2);TestTrue(TEXT("Leaving cancels and keeps fuel/ore"),C->ActiveRecipe.IsNone() && I->Count(TEXT("Item_Wood"))==4 && I->Count(TEXT("Item_CopperOre"))==12);
    P->SetActorLocation(FVector(0,0,150));TestTrue(TEXT("Start before obstruction"),C->Start(Batch,P));
    auto* Wall=W->SpawnActor<AActor>();auto* Box=NewObject<UBoxComponent>(Wall);Wall->SetRootComponent(Box);Box->RegisterComponent();Box->SetBoxExtent(FVector(10,80,150));Box->SetCollisionProfileName(TEXT("BlockAll"));Wall->SetActorLocation(FVector(60,0,200));Tick(2);
    TestTrue(TEXT("Obstruction cancels required job"),C->ActiveRecipe.IsNone());Wall->Destroy();Tick(1);
    TestTrue(TEXT("Start before removed fuel"),C->Start(Batch,P));TestTrue(TEXT("Remove reserved fuel"),I->RemoveItem(TEXT("Item_Wood"),4));Tick(90);
    TestTrue(TEXT("Missing exact fuel leaves ore and no output"),C->ActiveRecipe.IsNone() && I->Count(TEXT("Item_CopperOre"))==12 && I->Count(TEXT("Item_CopperIngot"))==0);
    TestFalse(TEXT("Missing fuel refuses new job"),C->Start(Batch,P));TestTrue(TEXT("Replenish fuel"),I->Grant(TEXT("Item_Wood"),4));
    TestTrue(TEXT("Start before capacity changes"),C->Start(Batch,P));const int32 Slots=I->SlotLimit;I->SlotLimit=1;Tick(90);I->SlotLimit=Slots;
    TestTrue(TEXT("Output capacity failure keeps fuel and ore"),C->ActiveRecipe.IsNone() && I->Count(TEXT("Item_Wood"))==4 && I->Count(TEXT("Item_CopperOre"))==12 && I->Count(TEXT("Item_CopperIngot"))==0);
    TestTrue(TEXT("Copper batch completes"),C->Start(Batch,P));Tick(90);
    TestTrue(TEXT("Two ingots for four ore and one wood exactly"),I->Count(TEXT("Item_CopperIngot"))==2 && I->Count(TEXT("Item_CopperOre"))==8 && I->Count(TEXT("Item_Wood"))==3);
    Tick(90);TestEqual(TEXT("Completion cannot duplicate output"),I->Count(TEXT("Item_CopperIngot")),2);
    TestTrue(TEXT("Start before destruction"),C->Start(Batch,P));Station->Destroy();Tick(2);TestTrue(TEXT("Destroyed refinery cancels"),C->ActiveRecipe.IsNone() && I->Count(TEXT("Item_Wood"))==3);
    // A learned player uses existing coal and flint as exact ordinary ingredients.
    FPFProgressionRecord Seed;Seed.Experience=250;Seed.Knowledge={TEXT("Tech_FieldTools"),TEXT("Tech_Metalwork")};
    TestTrue(TEXT("Known metalworker fixture"),G->Restore(Seed,Error));
    const auto Remaining=I->GetStacks();for(const auto& S:Remaining){I->Remove(S.StackId,S.Quantity);}
    TestTrue(TEXT("Iron ingredients"),I->Grant(TEXT("Item_IronOre"),4) && I->Grant(TEXT("Item_Coal"),1) && I->Grant(TEXT("Item_Flint"),2));
    Station=W->SpawnActor<APFCraftingStation>(FVector(120,0,150),FRotator::ZeroRotator);Station->StationType=APFCraftingStation::RefineryType();Station->SpeedMultiplier=1.5f;
    TestTrue(TEXT("Iron batch starts"),C->Start(Iron->Id,P));Tick(110);
    TestTrue(TEXT("Iron batch consumes one coal, two flint, four ore for two ingots"),I->Count(TEXT("Item_IronIngot"))==2 && I->Count(TEXT("Item_Coal"))==0 && I->Count(TEXT("Item_Flint"))==0 && I->Count(TEXT("Item_IronOre"))==0);
    AddInfo(TEXT("[PrimalCrafting] BatchRefining: actual catalogs, required station, fuel/ore conservation, authority, range/occlusion/destruction cancellation, capacity and knowledge verified."));
    F.ForwardErrorMessages(this);return true;
}
#endif
