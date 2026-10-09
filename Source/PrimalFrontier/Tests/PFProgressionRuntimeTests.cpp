#if WITH_DEV_AUTOMATION_TESTS
#include "Progression/PFProgressionComponent.h"
#include "Persistence/PFWorldPersistence.h"
#include "Persistence/PFWorldSaveFormat.h"
#include "Persistence/PFSaveFileStore.h"
#include "Inventory/PFInventoryPlayerState.h"
#include "Inventory/PFInventoryComponent.h"
#include "Crafting/PFCraftingComponent.h"
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
    if(!TestTrue(TEXT("Explicit save publishes migrated V2"),Persistence->Save(Slot,Error) && Persistence->Load(Slot,Error))){AddError(Error);return false;}TestEqual(TEXT("Repeated migrated restore stays zero"),G->GetExperience(),0);
    Fixture.ForwardErrorMessages(this);AddInfo(TEXT("[PrimalAgentTools] Real craft success/dedupe/failure/grant refusal, private component authority, V2 repeated files, death/respawn and V1 zero defaults checked."));return true;
}
#endif
