#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Persistence/PFWorldPersistence.h"
#include "Persistence/PFSaveFileStore.h"
#include "Persistence/PFWorldSaveFormat.h"
#include "Inventory/PFInventoryComponent.h"
#include "Inventory/PFInventoryPlayerState.h"
#include "Survival/PFSurvivalGameMode.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFSurvivorCharacter.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Crafting/PFResourceNode.h"
#include "Creatures/PFCreatureSpawner.h"
#include "World/PFWorldClock.h"
#include "Components/BoxComponent.h"
#include "GameFramework/PlayerStart.h"
#include "EngineUtils.h"
#include "Tests/AutomationCommon.h"
#include "Misc/FileHelper.h"
#include "Misc/ScopeExit.h"
#include "HAL/FileManager.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFSaveInspectionTest,"PF.Persistence.SaveInspection",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFSaveInspectionTest::RunTest(const FString&)
{
    AddExpectedMessage(TEXT("GetSocketInfoByName.*No SkeletalMesh for Component"),EAutomationExpectedMessageFlags::Contains,0);
    FTestWorldWrapper Fixture;if(!Fixture.CreateTestWorld(EWorldType::Game)){return false;}auto* W=Fixture.GetTestWorld();
    W->GetOutermost()->Rename(*(TEXT("/Temp/PFInspect/")+FGuid::NewGuid().ToString(EGuidFormats::Digits)+TEXT("/L_Automation")),nullptr,REN_DontCreateRedirectors | REN_NonTransactional);
    W->SetGameMode(FURL(nullptr,TEXT("/Engine/Maps/Entry?game=/Script/PrimalFrontier.PFSurvivalGameMode"),TRAVEL_Absolute));
    auto* Floor=W->SpawnActor<AActor>();auto* Box=NewObject<UBoxComponent>(Floor);Floor->SetRootComponent(Box);Box->SetBoxExtent(FVector(2000,2000,25));Box->SetCollisionProfileName(TEXT("BlockAll"));Box->RegisterComponent();Floor->SetActorLocation(FVector(0,0,-25));
    W->SpawnActor<APlayerStart>(FVector(-500,0,120),FRotator::ZeroRotator);auto* Clock=W->SpawnActor<APFWorldClock>();
    auto* Node=W->SpawnActor<APFResourceNode>(FVector(0,1000,0),FRotator::ZeroRotator);
    auto* Point=W->SpawnActorDeferred<APFCreatureSpawner>(APFCreatureSpawner::StaticClass(),FTransform(FVector(1200,0,50)));Point->bAutoSpawn=false;Point->FinishSpawning(FTransform(FVector(1200,0,50)));
    if(!Fixture.BeginPlayInTestWorld()){return false;}
    auto* PC=W->SpawnActor<APFSurvivalPlayerController>();W->GetAuthGameMode<APFSurvivalGameMode>()->RestartPlayer(PC);
    auto* Pawn=Cast<APFSurvivorCharacter>(PC->GetPawn());auto* P=W->GetSubsystem<UPFWorldPersistence>();
    if(!TestNotNull(TEXT("Actual survivor"),Pawn) || !TestNotNull(TEXT("Actual bag"),PC->GetInventory())){return false;}
    P->Login(PC,TEXT(""));FString Error;const FString Base=TEXT("AutomationInspect_")+FGuid::NewGuid().ToString(EGuidFormats::Digits);
    TArray<FString> Slots={Base,Base+TEXT("B"),Base+TEXT("C"),Base+TEXT("D"),Base+TEXT("E"),Base+TEXT("F"),Base+TEXT("G")};
    ON_SCOPE_EXIT {P->ActiveSlot.Reset();for(const auto& Slot:Slots){for(bool Second:{false,true}){IFileManager::Get().Delete(*FPFSaveFileStore::Path(Slot,Second));}}};
    if(!TestTrue(TEXT("Seed original bag"),PC->GetInventory()->Grant(TEXT("Item_Wood"),4)) || !TestTrue(TEXT("Seed actual world save"),P->Save(Base,Error))){AddError(Error);return false;}
    FPFWorldSaveData Saved;if(!TestTrue(TEXT("Capture baseline source"),P->Capture(Saved,Error))){return false;}
    auto Install=[&](const FString& Slot,const FPFWorldSaveData& Data)
    {TArray<uint8> Bytes;return TestTrue(TEXT("Install disposable value fixture"),FPFWorldSaveFormat::Encode(Data,Bytes,Error) && FPFSaveFileStore::Write(Slot,Bytes,Error));};
    if(!Install(Slots[1],Saved)){return false;}
    TArray<uint8> OriginalBytes;FFileHelper::LoadFileToArray(OriginalBytes,*FPFSaveFileStore::Path(Slots[1],false));
    FPFSavedFile Original;FPFSaveFileStore::Read(Slots[1],Original,Error);
    PC->GetInventory()->Grant(TEXT("Item_Stone"),1);Pawn->Survival->SetHealth(33);Clock->SetHour(17);Node->RestorePersistence(1,0);
    const FGuid Batch=PC->GetInventory()->GetStacks()[0].StackId;TSet<AActor*> Actors;for(TActorIterator<AActor> It(W);It;++It){Actors.Add(*It);}
    const auto Unchanged=[&]()
    {
        TestEqual(TEXT("Inspection preserves active slot"),P->ActiveSlot,Base);TestEqual(TEXT("Inspection preserves live stone"),PC->GetInventory()->Count(TEXT("Item_Stone")),1);
        TestEqual(TEXT("Inspection preserves live health"),Pawn->Survival->GetVitals().Health,33.f);TestEqual(TEXT("Inspection preserves live clock"),Clock->Hour,17.f);TestEqual(TEXT("Inspection preserves depleted node"),Node->HitsRemaining,1);
        TestEqual(TEXT("Inspection preserves batch identity"),PC->GetInventory()->GetStacks()[0].StackId,Batch);
        TSet<AActor*> Current;for(TActorIterator<AActor> It(W);It;++It){Current.Add(*It);}TestEqual(TEXT("No spawned or destroyed actor"),Current.Num(),Actors.Num());for(auto* Actor:Actors){TestTrue(TEXT("Same actual actor"),Current.Contains(Actor));}
        TArray<uint8> Bytes;FFileHelper::LoadFileToArray(Bytes,*FPFSaveFileStore::Path(Slots[1],false));TestTrue(TEXT("Source envelope bytes unchanged"),Bytes==OriginalBytes);
        TestFalse(TEXT("No second generation published"),IFileManager::Get().FileExists(*FPFSaveFileStore::Path(Slots[1],true)));
        TestFalse(TEXT("No migration original written"),IFileManager::Get().FileExists(*FPFSaveFileStore::OriginalPath(Slots[1],TEXT("M12CopperLayout1"))));
    };
    FPFSaveInspection Info;
    for(int32 Repeat=0;Repeat<2;++Repeat)
    {
        if(!TestTrue(TEXT("Read-only actual file preflight"),P->InspectSave(Slots[1],Info,Error))){AddError(Error);return false;}
        TestEqual(TEXT("World schema reported"),Info.WorldVersion,2);TestEqual(TEXT("Original generation"),Info.Generation,Original.Generation);TestEqual(TEXT("Original UTC never reset"),Info.SavedUtc,Original.SavedUtc);
        TestEqual(TEXT("Bounded player count"),Info.Players,1);TestEqual(TEXT("Bounded resource count"),Info.Resources,1);TestEqual(TEXT("Bounded spawner count"),Info.Spawners,1);TestEqual(TEXT("Map reported"),Info.Map,FString(TEXT("L_Automation")));
        TestFalse(TEXT("No invented migration"),Info.bRequiresCopperMigration);TestFalse(TEXT("No recovery when one valid generation"),Info.bRecoveredBackup);Unchanged();
    }
    const auto Refuse=[&](const FString& Slot)
    {Info.Map=TEXT("Sentinel");Info.Generation=987;TestFalse(TEXT("Invalid/incompatible preflight refuses"),P->InspectSave(Slot,Info,Error));TestTrue(TEXT("Refusal leaves output and reason"),Info.Map==TEXT("Sentinel") && Info.Generation==987 && !Error.IsEmpty());Unchanged();};
    Refuse(TEXT("../escape"));Refuse(TEXT("Identity_Reserved"));Refuse(TEXT("Metadata_Reserved"));Refuse(Slots[6]);
    auto Bad=Saved;Bad.Map=TEXT("L_M7SurvivalArena");if(!Install(Slots[2],Bad)){return false;}Refuse(Slots[2]);
    Bad=Saved;Bad.Resources[0].Name=TEXT("MissingNode");if(!Install(Slots[3],Bad)){return false;}Refuse(Slots[3]);
    Bad=Saved;Bad.Players.Reset();if(!Install(Slots[4],Bad)){return false;}Refuse(Slots[4]);
    Bad=Saved;Bad.Resources[0].Definition=TEXT("Node_NotInCatalog");if(!Install(Slots[5],Bad)){return false;}Refuse(Slots[5]);
    // Corrupt sibling explicitly reports recovery without repairing either file.
    const TArray<uint8> Corrupt={1,2,3};FFileHelper::SaveArrayToFile(Corrupt,*FPFSaveFileStore::Path(Slots[1],true));
    TestTrue(TEXT("Verified sibling inspection allowed"),P->InspectSave(Slots[1],Info,Error));TestTrue(TEXT("Backup recovery explicit"),Info.bRecoveredBackup);
    TArray<uint8> StillCorrupt;FFileHelper::LoadFileToArray(StillCorrupt,*FPFSaveFileStore::Path(Slots[1],true));TestTrue(TEXT("Corrupt evidence not repaired"),StillCorrupt==Corrupt);
    FFileHelper::SaveArrayToFile(Corrupt,*FPFSaveFileStore::Path(Slots[6],false));Info.Map=TEXT("Sentinel");Info.Generation=987;
    TestFalse(TEXT("No healthy generation refuses"),P->InspectSave(Slots[6],Info,Error));TestTrue(TEXT("Corrupt refusal preserves output"),Info.Map==TEXT("Sentinel") && Info.Generation==987 && !Error.IsEmpty());
    FFileHelper::LoadFileToArray(StillCorrupt,*FPFSaveFileStore::Path(Slots[6],false));TestTrue(TEXT("Only corrupt original preserved"),StillCorrupt==Corrupt);
    TArray<uint8> StillOriginal;FFileHelper::LoadFileToArray(StillOriginal,*FPFSaveFileStore::Path(Slots[1],false));TestTrue(TEXT("Recovered source remains byte exact"),StillOriginal==OriginalBytes);
    AddInfo(TEXT("[PrimalPersistence] Read-only real-file inspection: repeat/UTC/generation/layout/owner/catalog/refusal/recovery preserve live actors, health, bag, node, slot and source evidence; no load guarantee."));return true;
}
#endif
