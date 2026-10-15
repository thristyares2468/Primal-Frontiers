#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Crafting/PFHarvestCapabilities.h"
#include "Crafting/PFCraftingCatalog.h"
#include "Crafting/PFResourceNode.h"
#include "Inventory/PFItemCatalog.h"
#include "Inventory/PFInventoryComponent.h"
#include "Inventory/PFInventoryPlayerState.h"
#include "Progression/PFProgressionComponent.h"
#include "Survival/PFSurvivalGameMode.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFSurvivorCharacter.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Survival/PFInteraction.h"
#include "Persistence/PFWorldPersistence.h"
#include "Engine/World.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/BoxComponent.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"

namespace PFHarvestFixture
{
    void Tool(UPFItemCatalog* Items,FName Id,FGameplayTag Family,EPFHarvestGrade Grade,int32 Hits)
    {
        FPFItemDefinition D=*Items->Find(TEXT("Item_Tool"));D.Id=Id;D.HarvestFamilies.Reset();
        D.HarvestFamilies.AddTag(Family);D.HarvestGrade=Grade;D.GatheringHits=Hits;Items->Items.Add(D);
    }
    FPFHarvestRequirement Required(FGameplayTag Family,EPFHarvestGrade Grade)
    {FPFHarvestRequirement R;R.Family=Family;R.MinimumGrade=Grade;return R;}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFHarvestDataTest,"PF.Harvest.Definitions",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFHarvestDataTest::RunTest(const FString&)
{
    auto* Items=NewObject<UPFItemCatalog>();auto* Craft=NewObject<UPFCraftingCatalog>();
    for(const auto& D:Items->Items){TestNotNull(TEXT("All legacy item definitions remain valid"),Items->Find(D.Id));}
    const TArray<FName> Baseline={TEXT("Node_Wood"),TEXT("Node_Stone"),TEXT("Node_Food"),TEXT("Node_Fibre"),TEXT("Node_Water")};
    for(const auto& D:Craft->Resources){TestNotNull(TEXT("All nodes have valid requirements"),Craft->Resource(D.Id,Items));if(Baseline.Contains(D.Id)){TestFalse(TEXT("No implicit legacy access gate"),D.HarvestRequirement.Family.IsValid());}}
    using namespace PFHarvestFixture;
    for(auto Grade:{EPFHarvestGrade::Wood,EPFHarvestGrade::Stone,EPFHarvestGrade::Copper,EPFHarvestGrade::Iron,EPFHarvestGrade::Diamond,EPFHarvestGrade::Titanium,EPFHarvestGrade::Deepforge})
    {TestTrue(TEXT("Mechanical grades supported as data, not shipped items"),PFHarvest::ValidRequirement(Required(PFHarvest::Pick(),Grade)));}
    auto R=Required(PFHarvest::Pick(),EPFHarvestGrade::Iron);
    TestEqual(TEXT("Exact readable requirement"),PFHarvest::RequirementText(R),FString(TEXT("Requires Iron pick")));
    R.MinimumGrade=EPFHarvestGrade::Tech;TestFalse(TEXT("Tech cannot bypass unimplemented charge"),PFHarvest::ValidRequirement(R));
    TestTrue(TEXT("Tech explicitly unimplemented"),PFHarvest::RequirementText(R).Contains(TEXT("NOT IMPLEMENTED")));
    R.MinimumGrade=EPFHarvestGrade(255);TestFalse(TEXT("Unknown enum refuses"),PFHarvest::ValidRequirement(R));
    R=Required(FGameplayTag::RequestGameplayTag(TEXT("Item.Category.Tool")),EPFHarvestGrade::Iron);
    TestFalse(TEXT("Unrelated registered tag refuses"),PFHarvest::ValidRequirement(R));
    Craft->Resources[0].HarvestRequirement=R;TestNull(TEXT("Resource lookup fails closed"),Craft->Resource(Craft->Resources[0].Id,Items));
    Tool(Items,TEXT("Test_IronPick"),PFHarvest::Pick(),EPFHarvestGrade::Iron,2);TestNotNull(TEXT("Valid family and grade"),Items->Find(TEXT("Test_IronPick")));
    auto& D=Items->Items.Last();D.HarvestGrade=EPFHarvestGrade::None;TestNull(TEXT("Family with no grade invalid"),Items->Find(D.Id));
    D.HarvestGrade=EPFHarvestGrade::Iron;D.GatheringHits=0;TestNull(TEXT("Typed tool must gather"),Items->Find(D.Id));
    D.GatheringHits=2;D.HarvestFamilies.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Item.Category.Food")));TestNull(TEXT("Mixed unknown family invalid"),Items->Find(D.Id));
    D.HarvestFamilies.Reset();TestNull(TEXT("Grade without family invalid"),Items->Find(D.Id));
    D.HarvestFamilies.AddTag(PFHarvest::Pick());D.Category=FGameplayTag::RequestGameplayTag(TEXT("Item.Category.Weapon"));TestNull(TEXT("Weapon cannot grant harvesting capability"),Items->Find(D.Id));
    AddInfo(TEXT("[PrimalAgentTools] Harvest.Definitions: explicit legacy defaults, mechanical grades, native family allowlist, invalid data/Tech fail closed; no new live content."));return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFHarvestInventoryTest,"PF.Harvest.Selection",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFHarvestInventoryTest::RunTest(const FString&)
{
    FTestWorldWrapper Fixture;if(!Fixture.CreateTestWorld(EWorldType::Game)){return false;}auto* W=Fixture.GetTestWorld();
    auto* Owner=W->SpawnActor<APFInventoryPlayerState>();auto* I=Owner->Inventory.Get();I->Catalog=NewObject<UPFItemCatalog>(I);I->SlotLimit=64;I->WeightLimit=100;
    using namespace PFHarvestFixture;
    Tool(I->Catalog,TEXT("Test_StonePick"),PFHarvest::Pick(),EPFHarvestGrade::Stone,4);
    Tool(I->Catalog,TEXT("Test_CopperPick"),PFHarvest::Pick(),EPFHarvestGrade::Copper,5);
    Tool(I->Catalog,TEXT("Test_IronPick"),PFHarvest::Pick(),EPFHarvestGrade::Iron,2);
    Tool(I->Catalog,TEXT("Test_DiamondPick"),PFHarvest::Pick(),EPFHarvestGrade::Diamond,3);
    Tool(I->Catalog,TEXT("Test_IronAxe"),PFHarvest::Axe(),EPFHarvestGrade::Iron,8);
    Tool(I->Catalog,TEXT("Test_SpoilingPick"),PFHarvest::Pick(),EPFHarvestGrade::Iron,7);I->Catalog->Items.Last().ShelfLifeSeconds=30;
    const auto R=Required(PFHarvest::Pick(),EPFHarvestGrade::Iron);int32 Hits=77;FString Error;
    TestFalse(TEXT("Bare hands cannot satisfy Iron pick"),I->ResolveHarvest(R,Hits,Error));TestEqual(TEXT("Failure preserves output"),Hits,77);
    TestEqual(TEXT("Readable missing capability"),Error,FString(TEXT("Requires Iron pick")));
    TestTrue(TEXT("Legacy tool actual grant"),I->Grant(TEXT("Item_BoundTool"),1));
    TestFalse(TEXT("Legacy speed is not access"),I->ResolveHarvest(R,Hits,Error));
    TestTrue(TEXT("Stone actual grant"),I->Grant(TEXT("Test_StonePick"),1));TestFalse(TEXT("Faster under-tier denied"),I->ResolveHarvest(R,Hits,Error));
    TestTrue(TEXT("Copper actual grant"),I->Grant(TEXT("Test_CopperPick"),1));TestFalse(TEXT("Copper not Iron shortcut"),I->ResolveHarvest(R,Hits,Error));
    TestTrue(TEXT("Axe actual grant"),I->Grant(TEXT("Test_IronAxe"),1));TestFalse(TEXT("Faster wrong family denied"),I->ResolveHarvest(R,Hits,Error));
    TestTrue(TEXT("Exact Iron pick grant"),I->Grant(TEXT("Test_IronPick"),1));TestTrue(TEXT("Exact grade resolves"),I->ResolveHarvest(R,Hits,Error));TestEqual(TEXT("Compatible speed only"),Hits,2);
    TestTrue(TEXT("Higher Diamond pick grant"),I->Grant(TEXT("Test_DiamondPick"),1));TestTrue(TEXT("Higher compatible resolves"),I->ResolveHarvest(R,Hits,Error));TestEqual(TEXT("Best compatible speed"),Hits,3);
    TestTrue(TEXT("Remove higher tool"),I->RemoveItem(TEXT("Test_DiamondPick"),1));TestTrue(TEXT("Exact tool fallback"),I->ResolveHarvest(R,Hits,Error));TestEqual(TEXT("Recomputed each action"),Hits,2);
    TestTrue(TEXT("Remove exact tool"),I->RemoveItem(TEXT("Test_IronPick"),1));TestFalse(TEXT("No cached access after removal"),I->ResolveHarvest(R,Hits,Error));
    TestTrue(TEXT("Fresh test tool"),I->Grant(TEXT("Test_SpoilingPick"),1));W->TimeSeconds=31;
    const int32 Slots=I->GetStacks().Num();TestFalse(TEXT("Expired tool cannot confer access"),I->ResolveHarvest(R,Hits,Error));TestEqual(TEXT("Read-only query does not prune inventory"),I->GetStacks().Num(),Slots);
    FPFHarvestRequirement Legacy;TestTrue(TEXT("Legacy nodes retain fastest gathering"),I->ResolveHarvest(Legacy,Hits,Error));TestEqual(TEXT("Unrestricted legacy speed"),Hits,8);
    auto* Foreign=W->SpawnActor<APFInventoryPlayerState>();Foreign->Inventory->Catalog=I->Catalog;
    TestFalse(TEXT("Other owner's tool not consulted"),Foreign->Inventory->ResolveHarvest(R,Hits,Error));
    AddInfo(TEXT("[PrimalAgentTools] Harvest.Selection: real owned grants/removals, bare/legacy/under/wrong/exact/higher/freshness/private selection. No state mutation from query."));return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFHarvestAtomicTest,"PF.Harvest.AtomicGather",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFHarvestAtomicTest::RunTest(const FString&)
{
    AddExpectedMessage(TEXT("GetSocketInfoByName.*No SkeletalMesh for Component"),EAutomationExpectedMessageFlags::Contains,0);
    FTestWorldWrapper Fixture;if(!Fixture.CreateTestWorld(EWorldType::Game)){return false;}auto* W=Fixture.GetTestWorld();
    W->SetGameMode(FURL(nullptr,TEXT("/Engine/Maps/Entry?game=/Script/PrimalFrontier.PFSurvivalGameMode"),TRAVEL_Absolute));
    auto* Floor=W->SpawnActor<AActor>();auto* Box=NewObject<UBoxComponent>(Floor);Floor->SetRootComponent(Box);Box->SetBoxExtent(FVector(2000,2000,25));Box->SetCollisionProfileName(TEXT("BlockAll"));Box->RegisterComponent();Floor->SetActorLocation(FVector(0,0,-25));
    W->SpawnActor<APlayerStart>(FVector(-500,0,120),FRotator::ZeroRotator);if(!Fixture.BeginPlayInTestWorld()){return false;}
    auto* PC=W->SpawnActor<APFSurvivalPlayerController>();W->GetAuthGameMode<APFSurvivalGameMode>()->RestartPlayer(PC);
    auto* P=Cast<APFSurvivorCharacter>(PC->GetPawn());auto* PS=PC->GetPlayerState<APFInventoryPlayerState>();if(!P || !PS){return false;}
    W->GetSubsystem<UPFWorldPersistence>()->ActiveSlot.Reset();P->GetCharacterMovement()->DisableMovement();PC->SetControlRotation(FRotator::ZeroRotator);
    P->Survival->HungerDrainPerSecond=0;P->Survival->ThirstDrainPerSecond=0;
    auto* I=PS->Inventory.Get();auto* G=PS->Progression.Get();I->Catalog=NewObject<UPFItemCatalog>(I);I->SlotLimit=64;I->WeightLimit=100;
    using namespace PFHarvestFixture;
    Tool(I->Catalog,TEXT("Test_StonePick"),PFHarvest::Pick(),EPFHarvestGrade::Stone,4);
    Tool(I->Catalog,TEXT("Test_IronAxe"),PFHarvest::Axe(),EPFHarvestGrade::Iron,8);
    Tool(I->Catalog,TEXT("Test_IronPick"),PFHarvest::Pick(),EPFHarvestGrade::Iron,2);
    auto* Craft=NewObject<UPFCraftingCatalog>();auto* D=Craft->Resources.FindByPredicate([](const auto& X){return X.Id==TEXT("Node_Stone");});
    D->HarvestRequirement=Required(PFHarvest::Pick(),EPFHarvestGrade::Iron);
    FVector Eye;FRotator Look;P->GetActorEyesViewPoint(Eye,Look);const FTransform At(FRotator::ZeroRotator,Eye+Look.Vector()*150);
    auto* Node=W->SpawnActorDeferred<APFResourceNode>(APFResourceNode::StaticClass(),At,P,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
    Node->Catalog=Craft;Node->Items=I->Catalog;Node->ResourceId=TEXT("Node_Stone");Node->FinishSpawning(At);
    TestTrue(TEXT("Actual owned trace"),PFInteraction::FindTarget(P)==Node);const float Stamina=P->Survival->GetVitals().Stamina;
    auto Refused=[&](const TCHAR* Name){const int32 Slots=I->GetStacks().Num();const int32 XP=G->GetExperience();const auto Windows=G->GetRecord().GatherWindows;
        TestFalse(Name,Node->Gather(P));TestTrue(TEXT("Refusal preserves hits/items/reward/stamina/respawn"),Node->HitsRemaining==3 && Node->RespawnAt==0 && I->Count(TEXT("Item_Stone"))==0 && I->GetStacks().Num()==Slots && G->GetExperience()==XP && G->GetRecord().GatherWindows==Windows && P->Survival->GetVitals().Stamina==Stamina);};
    Refused(TEXT("Bare denied"));I->Grant(TEXT("Test_StonePick"),1);Refused(TEXT("Under-tier denied"));I->Grant(TEXT("Test_IronAxe"),1);Refused(TEXT("Wrong family denied"));
    I->Grant(TEXT("Test_IronPick"),1);Node->SetRole(ROLE_SimulatedProxy);Refused(TEXT("Client node denied"));Node->SetRole(ROLE_Authority);
    PC->SetRole(ROLE_AutonomousProxy);Refused(TEXT("Client controller denied"));PC->SetRole(ROLE_Authority);
    auto* Other=W->SpawnActor<APFSurvivalPlayerController>();W->GetAuthGameMode<APFSurvivalGameMode>()->RestartPlayer(Other);Other->GetPawn()->SetActorLocation(FVector(-1200,600,100));
    PC->PlayerState=Other->PlayerState;Refused(TEXT("Foreign controller state denied"));PC->PlayerState=PS;
    PC->SetControlRotation(FRotator(0,180,0));Refused(TEXT("Wrong aim denied"));PC->SetControlRotation(FRotator::ZeroRotator);
    I->bEnforceWeightLimit=true;I->WeightLimit=I->GetWeight();Refused(TEXT("Full bag does not spend node"));I->WeightLimit=100;
    TestTrue(TEXT("Same-clock successful action proves no refusal cooldown"),Node->Gather(P));TestTrue(TEXT("Exact compatible yield and one reward"),Node->HitsRemaining==1 && I->Count(TEXT("Item_Stone"))==4 && G->GetExperience()==5);
    TestFalse(TEXT("Success cooldown rejects repeat"),Node->Gather(P));TestEqual(TEXT("No duplicate output"),I->Count(TEXT("Item_Stone")),4);
    for(int32 N=0;N<6;++N){Fixture.TickTestWorld(.1f);}TestTrue(TEXT("Final finite hit"),Node->Gather(P));
    TestTrue(TEXT("Total yield never inflated by faster wrong tool"),Node->HitsRemaining==0 && I->Count(TEXT("Item_Stone"))==6 && G->GetExperience()==10 && Node->RespawnAt>0);
    TestFalse(TEXT("Depleted repeat denied"),Node->Gather(P));Fixture.ForwardErrorMessages(this);
    AddInfo(TEXT("[PrimalAgentTools] Harvest.AtomicGather: real possessed aim, state-preserving access/authority/foreign/full/aim refusals, immediate recovery, exact compatible finite yield, cooldown/depletion and bounded XP. Synthetic Iron gate on legacy stone in disposable world only."));return true;
}
#endif
