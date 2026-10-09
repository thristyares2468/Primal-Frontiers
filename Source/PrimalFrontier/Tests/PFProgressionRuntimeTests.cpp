#if WITH_DEV_AUTOMATION_TESTS
#include "Progression/PFProgressionComponent.h"
#include "Persistence/PFWorldPersistence.h"
#include "Persistence/PFWorldSaveFormat.h"
#include "Persistence/PFSaveFileStore.h"
#include "Inventory/PFInventoryPlayerState.h"
#include "Inventory/PFInventoryComponent.h"
#include "Crafting/PFCraftingComponent.h"
#include "Crafting/PFResourceNode.h"
#include "Survival/PFInteraction.h"
#include "Survival/PFSurvivalGameMode.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFSurvivorCharacter.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "World/PFWorldClock.h"
#include "Engine/World.h"
#include "Components/BoxComponent.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "HAL/FileManager.h"
#include "Tests/AutomationCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFProgressionComponentTest,"PF.Progression.Component",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFProgressionComponentTest::RunTest(const FString&)
{
    AddExpectedMessage(TEXT("GetSocketInfoByName.*No SkeletalMesh for Component"),EAutomationExpectedMessageFlags::Contains,0);
    FTestWorldWrapper Fixture;if(!Fixture.CreateTestWorld(EWorldType::Game)){return false;}auto* W=Fixture.GetTestWorld();
    W->GetOutermost()->Rename(*(TEXT("/Temp/PFProgression/")+FGuid::NewGuid().ToString(EGuidFormats::Digits)+TEXT("/L_Automation")),nullptr,REN_DontCreateRedirectors | REN_NonTransactional);
    W->SetGameMode(FURL(nullptr,TEXT("/Engine/Maps/Entry?game=/Script/PrimalFrontier.PFSurvivalGameMode"),TRAVEL_Absolute));
    auto* Floor=W->SpawnActor<AActor>();auto* Box=NewObject<UBoxComponent>(Floor);Floor->SetRootComponent(Box);Box->SetBoxExtent(FVector(4000,4000,25));Box->SetCollisionProfileName(TEXT("BlockAll"));Box->RegisterComponent();Floor->SetActorLocation(FVector(0,0,-25));
    W->SpawnActor<APlayerStart>(FVector(-500,0,120),FRotator::ZeroRotator);W->SpawnActor<APFWorldClock>();if(!Fixture.BeginPlayInTestWorld()){return false;}
    auto* PC=W->SpawnActor<APFSurvivalPlayerController>();W->GetAuthGameMode<APFSurvivalGameMode>()->RestartPlayer(PC);
    auto* PS=PC->GetPlayerState<APFInventoryPlayerState>();auto* P=Cast<APFSurvivorCharacter>(PC->GetPawn());auto* Persistence=W->GetSubsystem<UPFWorldPersistence>();
    if(!PS || !P || !TestNotNull(TEXT("Player progression"),PS->Progression.Get()) || !Persistence){return false;}Persistence->Login(PC,TEXT(""));
    auto* G=PS->Progression.Get();auto* I=PS->Inventory.Get();auto* C=PS->Crafting.Get();FString Error;
    P->GetCharacterMovement()->DisableMovement();P->Survival->HungerDrainPerSecond=0;P->Survival->ThirstDrainPerSecond=0;P->Survival->ExposureDamagePerSecond=0;
    C->Catalog=NewObject<UPFCraftingCatalog>(C);for(auto& R:C->Catalog->Recipes){R.Duration=0.5f;}
    auto Advance=[&](int32 Ticks=8){for(int32 N=0;N<Ticks;++N){Fixture.TickTestWorld(0.1f);}};
    TestTrue(TEXT("Fresh zero state"),G->GetExperience()==0 && G->GetLevel()==1 && G->GetAvailablePoints()==0 && G->GetRecord().CreditedCrafts.IsEmpty());
    TestFalse(TEXT("Missing inputs cannot craft/reward"),C->Start(TEXT("Recipe_Tool"),P));
    if(!TestTrue(TEXT("Grant fixtures"),I->Grant(TEXT("Item_Wood"),6) && I->Grant(TEXT("Item_Stone"),4) && I->Grant(TEXT("Item_Tool"),1))){return false;}
    TestEqual(TEXT("Developer grants never reward"),G->GetExperience(),0);
    TestTrue(TEXT("Start then cancel"),C->Start(TEXT("Recipe_Tool"),P));C->Cancel();Advance();TestEqual(TEXT("Cancel zero reward"),G->GetExperience(),0);
    TestTrue(TEXT("Start failed output capacity"),C->Start(TEXT("Recipe_Tool"),P));I->WeightLimit=0.1f;Advance();I->WeightLimit=30;TestEqual(TEXT("Failed conversion zero reward"),G->GetExperience(),0);
    TestTrue(TEXT("Start then change input"),C->Start(TEXT("Recipe_Tool"),P));I->RemoveItem(TEXT("Item_Stone"),4);Advance();TestEqual(TEXT("Changed ingredients zero reward"),G->GetExperience(),0);I->Grant(TEXT("Item_Stone"),4);
    TestTrue(TEXT("First actual successful craft"),C->Start(TEXT("Recipe_Tool"),P));Advance();TestEqual(TEXT("Successful conversion output"),I->Count(TEXT("Item_Tool")),2);
    TestTrue(TEXT("Actual first craft grants20 once"),G->GetExperience()==20 && G->GetRecord().CreditedCrafts==TArray<FName>{FName(TEXT("Recipe_Tool"))});
    TestTrue(TEXT("Repeat recipe remains available"),C->Start(TEXT("Recipe_Tool"),P));Advance();TestEqual(TEXT("Repeat craft still produces item"),I->Count(TEXT("Item_Tool")),3);TestEqual(TEXT("Repeat craft never reawards"),G->GetExperience(),20);
    // Reconnect before ANY manual save exercises the in-memory roster, not just disk migration.
    FPFWorldSaveData BeforeDisconnect;if(!TestTrue(TEXT("Capture reconnect credential fixture"),Persistence->Capture(BeforeDisconnect,Error))){AddError(Error);return false;}
    const FString Reconnect=TEXT("?PFReconnect=")+BeforeDisconnect.Players[0].ReconnectCredential.ToString(EGuidFormats::Digits);
    PC->Destroy();PC=W->SpawnActor<APFSurvivalPlayerController>();W->GetAuthGameMode<APFSurvivalGameMode>()->RestartPlayer(PC);Persistence->Login(PC,Reconnect);
    if(!TestTrue(TEXT("Reconnect before first save restores progression"),Persistence->RestorePlayer(PC))){return false;}
    PS=PC->GetPlayerState<APFInventoryPlayerState>();P=Cast<APFSurvivorCharacter>(PC->GetPawn());G=PS->Progression;I=PS->Inventory;C=PS->Crafting;
    TestTrue(TEXT("Unsaved reconnect retains XP/ledger and tools"),G->GetExperience()==20 && G->GetRecord().CreditedCrafts.Num()==1 && I->Count(TEXT("Item_Tool"))==3);
    P->GetCharacterMovement()->DisableMovement();
    FPFProgressionRecord Seed=G->GetRecord();Seed.Experience=100;Seed.Knowledge={FName(TEXT("Tech_FieldTools"))};
    if(!TestTrue(TEXT("Trusted server restores valid earned fixture"),G->Restore(Seed,Error))){AddError(Error);return false;}
    TestTrue(TEXT("Derived level/points"),G->GetLevel()==2 && G->GetAvailablePoints()==1);
    TestTrue(TEXT("Trusted existing bound-tool compatibility fixture"),I->Grant(TEXT("Item_BoundTool"),1));
    auto Bad=Seed;Bad.Experience=19;TestFalse(TEXT("Invalid accounting refused atomically"),G->Restore(Bad,Error));TestEqual(TEXT("Invalid restore preserves XP"),G->GetExperience(),100);
    PS->SetRole(ROLE_AutonomousProxy);FPFProgressionRecord ClientOut=Seed;ClientOut.Experience=250;
    TestFalse(TEXT("Client direct restore refuses"),G->Restore({},Error));TestFalse(TEXT("Client capture refuses"),G->Capture(ClientOut,Error));TestTrue(TEXT("Client outputs/state unchanged"),ClientOut.Experience==250 && G->GetExperience()==100);PS->SetRole(ROLE_Authority);
    const FString Slot=TEXT("AutomationProgression_")+FGuid::NewGuid().ToString(EGuidFormats::Digits);const FString LegacySlot=Slot+TEXT("_Legacy");
    ON_SCOPE_EXIT{Persistence->ActiveSlot.Reset();for(const auto& S:{Slot,LegacySlot}){for(bool B:{false,true}){IFileManager::Get().Delete(*FPFSaveFileStore::Path(S,B));}}};
    if(!TestTrue(TEXT("Save real earned world"),Persistence->Save(Slot,Error))){AddError(Error);return false;}FPFSavedFile File;FPFWorldSaveData Snapshot;
    if(!TestTrue(TEXT("Read actual V2 snapshot"),FPFSaveFileStore::Read(Slot,File,Error) && Persistence->Decode(File.Payload,Snapshot,Error))){AddError(Error);return false;}TestEqual(TEXT("Runtime writer now explicitV2"),Snapshot.Version,2);
    for(int32 Repeat=0;Repeat<2;++Repeat){G->Restore({},Error);if(!TestTrue(TEXT("Actual load restores earned state"),Persistence->Load(Slot,Error))){AddError(Error);return false;}TestTrue(TEXT("Repeated load never reawards or loses knowledge"),G->GetExperience()==100 && G->GetRecord().Knowledge==Seed.Knowledge && G->GetRecord().CreditedCrafts==Seed.CreditedCrafts && G->GetAvailablePoints()==1);}
    P->Survival->SetHealth(0);Advance(35);TestTrue(TEXT("Death/respawn retains same PlayerState progression"),PC->GetPlayerState<APFInventoryPlayerState>()==PS && PC->GetPawn()!=P && PC->GetPawn() && G->GetExperience()==100 && G->GetRecord().Knowledge==Seed.Knowledge);
    // Produce a real old-format world using the same validated IDs/bags; no private save is rewritten.
    Snapshot.Version=1;for(auto& Entry:Snapshot.Players){Entry.Progression.Reset();}TArray<uint8> LegacyBytes;
    if(!TestTrue(TEXT("Write legacy fixture"),FPFWorldSaveFormat::Encode(Snapshot,LegacyBytes,Error) && FPFSaveFileStore::Write(LegacySlot,LegacyBytes,Error)) || !TestTrue(TEXT("Runtime V1 default migration"),Persistence->Load(LegacySlot,Error))){AddError(Error);return false;}
    TestTrue(TEXT("Legacy existing tools do not give retrospective XP"),I->Count(TEXT("Item_Tool"))==3 && G->GetExperience()==0 && G->GetRecord().Knowledge.IsEmpty() && G->GetRecord().CreditedCrafts.IsEmpty());
    TestTrue(TEXT("Actual V1 existing bound tool remains usable but grants no learned recipe"),I->Count(TEXT("Item_BoundTool"))==1 && I->GatheringHits()==3 && !G->CanCraftRecipe(TEXT("Recipe_BoundTool"),Error));
    if(!TestTrue(TEXT("Explicit save publishes migrated V2"),Persistence->Save(Slot,Error) && Persistence->Load(Slot,Error))){AddError(Error);return false;}TestEqual(TEXT("Repeated migrated restore stays zero"),G->GetExperience(),0);
    Fixture.ForwardErrorMessages(this);AddInfo(TEXT("[PrimalAgentTools] Real craft success/dedupe/failure/grant refusal, private component authority, V2 repeated files, death/respawn and V1 zero defaults checked."));return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFEarnedUpgradeTest,"PF.Progression.EarnedUpgrade",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFEarnedUpgradeTest::RunTest(const FString&)
{
    AddExpectedMessage(TEXT("GetSocketInfoByName.*No SkeletalMesh for Component"),EAutomationExpectedMessageFlags::Contains,0);
    FTestWorldWrapper Fixture;if(!Fixture.CreateTestWorld(EWorldType::Game)){return false;}auto* W=Fixture.GetTestWorld();
    W->SetGameMode(FURL(nullptr,TEXT("/Engine/Maps/Entry?game=/Script/PrimalFrontier.PFSurvivalGameMode"),TRAVEL_Absolute));
    auto* Floor=W->SpawnActor<AActor>();auto* Box=NewObject<UBoxComponent>(Floor);Floor->SetRootComponent(Box);Box->SetBoxExtent(FVector(4000,4000,25));Box->SetCollisionProfileName(TEXT("BlockAll"));Box->RegisterComponent();Floor->SetActorLocation(FVector(0,0,-25));
    W->SpawnActor<APlayerStart>(FVector(-500,0,120),FRotator::ZeroRotator);W->SpawnActor<APFWorldClock>();if(!Fixture.BeginPlayInTestWorld()){return false;}
    auto* PC=W->SpawnActor<APFSurvivalPlayerController>();W->GetAuthGameMode<APFSurvivalGameMode>()->RestartPlayer(PC);
    auto* PS=PC->GetPlayerState<APFInventoryPlayerState>();auto* P=Cast<APFSurvivorCharacter>(PC->GetPawn());if(!PS || !P){return false;}
    auto* G=PS->Progression.Get();auto* I=PS->Inventory.Get();auto* C=PS->Crafting.Get();if(!G || !I || !C || !C->Catalog || !I->Catalog){return false;}
    P->GetCharacterMovement()->DisableMovement();P->Survival->HungerDrainPerSecond=0;P->Survival->ThirstDrainPerSecond=0;PC->SetControlRotation(FRotator::ZeroRotator);
    auto Advance=[&](int32 Ticks){for(int32 N=0;N<Ticks;++N){Fixture.TickTestWorld(0.1f);}};
    TestTrue(TEXT("Earned route starts empty and zero"),I->GetStacks().IsEmpty() && G->GetExperience()==0 && G->GetRecord().Knowledge.IsEmpty());
    FString Error;PC->ServerLearnKnowledge(TEXT("Tech_FieldTools"));TestTrue(TEXT("Premature owned purchase refuses without rewards"),G->GetKnowledgeFeedback().StartsWith(TEXT("Refused:")) && G->GetExperience()==0 && G->GetRecord().Knowledge.IsEmpty());
    TestFalse(TEXT("Optional craft initially locked"),C->Start(TEXT("Recipe_BoundTool"),P));
    // Only real target gathering populates the bag. No Grant/Restore/XP seeds or shortened recipes.
    auto Gather=[&](FName Resource,int32 Actions)
    {
        FVector Eye;FRotator Look;P->GetActorEyesViewPoint(Eye,Look);const FTransform At(FRotator::ZeroRotator,Eye+Look.Vector()*150);
        auto* Node=W->SpawnActorDeferred<APFResourceNode>(APFResourceNode::StaticClass(),At,P,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
        if(!Node){return false;}Node->ResourceId=Resource;Node->FinishSpawning(At);
        ON_SCOPE_EXIT{Node->Destroy();};
        for(int32 N=0;N<Actions;++N){Advance(6);if(!TestTrue(TEXT("Reach/aim selects real temporary node"),PFInteraction::FindTarget(P)==Node) || !TestTrue(TEXT("Normal gather accepts owned living pawn"),Node->Gather(P))){return false;}}
        return true;
    };
    if(!Gather(TEXT("Node_Wood"),3) || !Gather(TEXT("Node_Wood"),3) || !Gather(TEXT("Node_Stone"),1) || !Gather(TEXT("Node_Food"),2) || !Gather(TEXT("Node_Fibre"),3) || !Gather(TEXT("Node_Fibre"),1)){return false;}
    TestTrue(TEXT("Finite bare-hand gathering yields exact route inputs"),I->Count(TEXT("Item_Wood"))==12 && I->Count(TEXT("Item_Stone"))==2 && I->Count(TEXT("Item_Food"))==4 && I->Count(TEXT("Item_Fibre"))==8);
    TestEqual(TEXT("Gathering does not invent craft rewards"),G->GetExperience(),0);
    auto Craft=[&](FName Recipe,int32 ExpectedXP)
    {
        const auto* D=C->Catalog->Recipe(Recipe,I->Catalog);if(!D || !TestTrue(TEXT("Real recipe starts"),C->Start(Recipe,P))){return false;}
        Advance(FMath::CeilToInt(D->Duration*10)+10);TestTrue(TEXT("Real duration completes successfully"),C->ActiveRecipe.IsNone() && C->Feedback==TEXT("Completed"));
        return TestEqual(TEXT("Only actual distinct conversion rewards XP"),G->GetExperience(),ExpectedXP);
    };
    if(!Craft(TEXT("Recipe_Tool"),20) || !Craft(TEXT("Recipe_Cook"),40) || !Craft(TEXT("Recipe_Dry"),60) || !Craft(TEXT("Recipe_Cord"),80) || !Craft(TEXT("Recipe_Club"),100)){return false;}
    TestTrue(TEXT("Five distinct earned crafts reach first knowledge boundary"),G->GetLevel()==2 && G->GetAvailablePoints()==3 && G->GetRecord().CreditedCrafts.Num()==5);
    if(!Craft(TEXT("Recipe_Cord"),100)){return false;}TestEqual(TEXT("Repeat cord gives no duplicate credit"),G->GetRecord().CreditedCrafts.Num(),5);
    TestFalse(TEXT("Owned points alone don't unlock optional conversion"),C->Start(TEXT("Recipe_BoundTool"),P));
    PC->ServerLearnKnowledge(TEXT("Tech_FieldTools"));TestTrue(TEXT("Earned owned purchase costs exactly two"),G->GetAvailablePoints()==1 && G->GetRecord().Knowledge.Contains(TEXT("Tech_FieldTools")) && G->GetExperience()==100);
    Advance(4);PC->ServerLearnKnowledge(TEXT("Tech_FieldTools"));TestTrue(TEXT("Already-known request preserves points and items"),G->GetAvailablePoints()==1 && G->GetKnowledgeFeedback().StartsWith(TEXT("Refused:")) && I->Count(TEXT("Item_Tool"))==1);
    if(!Craft(TEXT("Recipe_BoundTool"),120)){return false;}
    TestTrue(TEXT("Single earned upgrade and exact full-route conservation"),I->Count(TEXT("Item_BoundTool"))==1 && I->Count(TEXT("Item_Tool"))==0 && I->Count(TEXT("Item_Cord"))==0 && I->Count(TEXT("Item_Wood"))==1 && I->Count(TEXT("Item_Stone"))==0 && I->Count(TEXT("Item_Fibre"))==0 && I->Count(TEXT("Item_Food"))==1 && I->Count(TEXT("Item_CookedFood"))==1 && I->Count(TEXT("Item_DriedFood"))==1 && I->Count(TEXT("Item_Club"))==1);
    TestTrue(TEXT("Upgrade retains earned accounting and useful performance"),G->GetRecord().CreditedCrafts.Num()==6 && G->GetAvailablePoints()==1 && I->GatheringHits()==3);
    if(!Gather(TEXT("Node_Fibre"),1)){return false;}TestTrue(TEXT("Earned upgraded tool takes finite three-hit yield without extra XP"),I->Count(TEXT("Item_Fibre"))==6 && G->GetExperience()==120);
    Fixture.ForwardErrorMessages(this);AddInfo(TEXT("[PrimalAgentTools] Earned first upgrade: real bare-hand trace gathering, seven timed jobs/six unique credits,100XP purchase boundary,120XP final,exact2point cost and three-hit benefit; no item/XP grants."));return true;
}
#endif
