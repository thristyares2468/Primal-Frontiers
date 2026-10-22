#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Persistence/PFWorldPersistence.h"
#include "Persistence/PFWorldSaveFormat.h"
#include "Persistence/PFSaveFileStore.h"
#include "Inventory/PFItemCatalog.h"
#include "Inventory/PFInventoryComponent.h"
#include "Inventory/PFInventoryPlayerState.h"
#include "Inventory/PFItemPickup.h"
#include "Building/PFBuildingCatalog.h"
#include "Building/PFBuildPiece.h"
#include "Crafting/PFCraftingCatalog.h"
#include "Crafting/PFResourceNode.h"
#include "Creatures/PFCreatureCatalog.h"
#include "Creatures/PFCreatureSpawner.h"
#include "Creatures/PFCreature.h"
#include "Progression/PFProgressionComponent.h"
#include "Survival/PFSurvivalGameMode.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "World/PFWorldClock.h"
#include "Tests/AutomationCommon.h"
#include "Components/BoxComponent.h"
#include "GameFramework/PlayerStart.h"
#include "EngineUtils.h"
#include "Misc/AutomationTest.h"
#include "Misc/Base64.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/SecureHash.h"
#include "Misc/ScopeExit.h"
#include "Misc/CommandLine.h"
#include "Misc/DateTime.h"
#include "HAL/FileManager.h"
#include "Serialization/JsonSerializer.h"

namespace
{
FGuid FakeId(uint32 Value){return FGuid(Value,Value,Value,Value);}
bool ReadFrozen(FAutomationTestBase& Test,TArray<uint8>& Payload,TArray<uint8>& Envelope,TSharedPtr<FJsonObject>& Manifest)
{
    const FString Root=FPaths::Combine(FPaths::ProjectDir(),TEXT("Tests/Fixtures/M8_c3d221f"));FString Text;TSharedPtr<FJsonObject> Fixture;
    if(!Test.TestTrue(TEXT("Read frozen fixture"),FFileHelper::LoadFileToString(Text,*(Root/TEXT("fixture.json"))) && FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Text),Fixture))){return false;}
    if(!Test.TestTrue(TEXT("Decode original writer bytes"),FBase64::Decode(Fixture->GetStringField(TEXT("payloadBase64")),Payload) && FBase64::Decode(Fixture->GetStringField(TEXT("envelopeBase64")),Envelope))){return false;}
    if(!Test.TestTrue(TEXT("Read provenance manifest"),FFileHelper::LoadFileToString(Text,*(Root/TEXT("manifest.json"))) && FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Text),Manifest))){return false;}
    Test.TestTrue(TEXT("Historical full writer revision recorded"),Manifest->GetStringField(TEXT("WriterCommit")).StartsWith(TEXT("c3d221f")) && Manifest->GetStringField(TEXT("WriterCommit")).Len()==40);
    Test.TestEqual(TEXT("Frozen payload regression hash"),FMD5::HashBytes(Payload.GetData(),Payload.Num()).ToLower(),Manifest->GetStringField(TEXT("PayloadMd5")));
    Test.TestEqual(TEXT("Frozen envelope regression hash"),FMD5::HashBytes(Envelope.GetData(),Envelope.Num()).ToLower(),Manifest->GetStringField(TEXT("EnvelopeMd5")));
    Test.TestEqual(TEXT("Frozen payload size"),Payload.Num(),static_cast<int32>(Manifest->GetNumberField(TEXT("PayloadBytes"))));
    return true;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFFrozenM8Records,"PF.Persistence.FrozenM8Records",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FPFFrozenM8Records::RunTest(const FString&)
{
    TArray<uint8> Payload,Envelope;TSharedPtr<FJsonObject> Manifest;if(!ReadFrozen(*this,Payload,Envelope,Manifest)){return false;}
    auto* Items=NewObject<UPFItemCatalog>();auto* Buildings=NewObject<UPFBuildingCatalog>();auto* Crafting=NewObject<UPFCraftingCatalog>();auto* Creatures=NewObject<UPFCreatureCatalog>();
    FString Error;FPFWorldSaveData Data;
    if(!TestTrue(TEXT("Current validated reader accepts actual old writer bytes"),FPFWorldSaveFormat::DecodeValidated(Payload,*Items,*Buildings,*Crafting,*Creatures,Data,Error))){AddError(Error);return false;}
    TestTrue(TEXT("Original schema and counts retained"),Data.Version==1 && Data.Players.Num()==1 && Data.Structures.Num()==2 && Data.Resources.Num()==1 && Data.Creatures.Num()==1 && Data.Spawners.Num()==1 && Data.Pickups.Num()==1);
    if(Data.Players.Num()!=1 || Data.Structures.Num()!=2 || Data.Pickups.Num()!=1){return false;}
    FUTF8ToTCHAR Utf8(reinterpret_cast<const ANSICHAR*>(Payload.GetData()),Payload.Num());const FString Original(Utf8.Length(),Utf8.Get());
    TestFalse(TEXT("Old writer has no progression field"),Original.Contains(TEXT("progression")));TestFalse(TEXT("Old writer has no chest filter field"),Original.Contains(TEXT("storageRestricted")));
    const auto& Owner=Data.Players[0];TestTrue(TEXT("Only explicit fake identities"),Owner.PlayerId==FakeId(0x11111111) && Owner.ReconnectCredential==FakeId(0x22222222));
    TestTrue(TEXT("Legacy progression defaults empty, not earned"),Owner.Progression.IsEmpty());
    FPFPlayerSaveData Player,Storage,Pickup;
    if(!TestTrue(TEXT("Current reader accepts old inner player/container bytes"),FPFWorldSaveFormat::UnpackPlayer(Owner.Data,*Items,30,Player,Error) && FPFWorldSaveFormat::UnpackPlayer(Data.Structures[1].Storage,*Items,60,Storage,Error) && FPFWorldSaveFormat::UnpackPlayer(Data.Pickups[0].Data,*Items,100000,Pickup,Error))){AddError(Error);return false;}
    TestTrue(TEXT("Exact legacy attributes and position"),Player.Health==33 && Player.Stamina==61 && Player.Hunger==47 && Player.Thirst==27 && Player.Location==FVector(-500,0,120));
    TestTrue(TEXT("Original bag batches and remaining freshness"),Player.Inventory.Num()==2 && Player.Inventory[0].StackId==FakeId(0x55555555) && Player.Inventory[0].Quantity==3 && Player.Inventory[1].StackId==FakeId(0x66666666) && Player.Inventory[1].Quantity==2 && Player.Inventory[1].RemainingFreshnessSeconds==180);
    TestTrue(TEXT("Storage defaults/support/ownership and exact contents"),!Data.Structures[1].StorageRestricted && Data.Structures[1].AllowedStorageItems.IsEmpty() && Data.Structures[1].Owner==Owner.PlayerId && Data.Structures[1].Support==Data.Structures[0].Id && Storage.Inventory.Num()==2 && Storage.Inventory[0].Quantity==7 && Storage.Inventory[1].Quantity==3 && Storage.Inventory[1].RemainingFreshnessSeconds==90);
    TestTrue(TEXT("Original pickup batch preserved"),Pickup.Inventory.Num()==1 && Pickup.Inventory[0].StackId==FakeId(0x99999999) && Pickup.Inventory[0].Quantity==2);
    const FString Slot=TEXT("AutomationFrozenRecord")+FGuid::NewGuid().ToString(EGuidFormats::Digits);
    ON_SCOPE_EXIT{for(bool B:{false,true}){IFileManager::Get().Delete(*FPFSaveFileStore::Path(Slot,B));}};
    IFileManager::Get().MakeDirectory(*FPaths::GetPath(FPFSaveFileStore::Path(Slot,false)),true);
    if(!TestTrue(TEXT("Install exact old envelope, never re-encode as old"),FFileHelper::SaveArrayToFile(Envelope,*FPFSaveFileStore::Path(Slot,false)))){return false;}
    FPFSavedFile File;if(!TestTrue(TEXT("Current file reader accepts old envelope"),FPFSaveFileStore::Read(Slot,File,Error))){AddError(Error);return false;}
    TestTrue(TEXT("Original payload generation UTC exact"),File.Payload==Payload && File.Generation==1 && File.SavedUtc==static_cast<int64>(Manifest->GetNumberField(TEXT("SavedUtc"))));
    const auto Refuse=[&](const TArray<uint8>& Bad)
    {FPFWorldSaveData Out;Out.Map=TEXT("Sentinel");TestFalse(TEXT("Changed frozen candidate refuses"),FPFWorldSaveFormat::DecodeValidated(Bad,*Items,*Buildings,*Crafting,*Creatures,Out,Error));TestTrue(TEXT("Refusal atomic with reason"),Out.Map==TEXT("Sentinel") && !Error.IsEmpty());};
    auto Truncated=Payload;Truncated.SetNum(Truncated.Num()-1);Refuse(Truncated);
    TSharedPtr<FJsonObject> Json;FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Original),Json);Json->SetNumberField(TEXT("version"),99);
    FString Changed;FJsonSerializer::Serialize(Json.ToSharedRef(),TJsonWriterFactory<>::Create(&Changed));FTCHARToUTF8 Future(*Changed);TArray<uint8> FutureBytes;FutureBytes.Append(reinterpret_cast<const uint8*>(Future.Get()),Future.Length());Refuse(FutureBytes);
    TArray<uint8> After;TestTrue(TEXT("Original installed old envelope byte exact after refusal"),FFileHelper::LoadFileToArray(After,*FPFSaveFileStore::Path(Slot,false)) && After==Envelope);
    AddInfo(TEXT("[PrimalAgentTools] FrozenM8Records: actual c3d221f writer payload/envelope, fake identities/attributes/batches/freshness/ownership, missing-field defaults and atomic future/truncation refusal verified. Historical source rebuilt with current engine, not historical executable."));return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFFrozenM8Restore,"PF.Persistence.FrozenM8Restore",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FPFFrozenM8Restore::RunTest(const FString&)
{
    AddExpectedMessage(TEXT("GetSocketInfoByName.*No SkeletalMesh for Component"),EAutomationExpectedMessageFlags::Contains,0);
    TArray<uint8> Payload,Envelope;TSharedPtr<FJsonObject> Manifest;if(!ReadFrozen(*this,Payload,Envelope,Manifest)){return false;}
    FTestWorldWrapper Fixture;if(!Fixture.CreateTestWorld(EWorldType::Game)){return false;}auto* W=Fixture.GetTestWorld();
    W->GetOutermost()->Rename(*(TEXT("/Temp/PFFrozenM8/")+FGuid::NewGuid().ToString(EGuidFormats::Digits)+TEXT("/L_Automation")),nullptr,REN_DontCreateRedirectors|REN_NonTransactional);
    W->SetGameMode(FURL(nullptr,TEXT("/Engine/Maps/Entry?game=/Script/PrimalFrontier.PFSurvivalGameMode"),TRAVEL_Absolute));
    auto* Floor=W->SpawnActor<AActor>();auto* Box=NewObject<UBoxComponent>(Floor);Floor->SetRootComponent(Box);Box->SetBoxExtent(FVector(4000,4000,25));Box->SetCollisionProfileName(TEXT("BlockAll"));Box->RegisterComponent();Floor->SetActorLocation(FVector(0,0,-25));
    W->SpawnActor<APlayerStart>(FVector(-500,0,120),FRotator::ZeroRotator);auto* Clock=W->SpawnActor<APFWorldClock>();Clock->SetActorTickEnabled(false);
    FActorSpawnParameters Params;Params.Name=TEXT("LegacyWood");auto* Node=W->SpawnActor<APFResourceNode>(FVector(0,1000,0),FRotator::ZeroRotator,Params);
    Params.Name=TEXT("LegacySpawn");auto* Point=W->SpawnActorDeferred<APFCreatureSpawner>(APFCreatureSpawner::StaticClass(),FTransform(FVector(1400,0,100)));
    Point->Rename(TEXT("LegacySpawn"),nullptr,REN_DontCreateRedirectors|REN_NonTransactional);Point->bAutoSpawn=false;Point->FinishSpawning(FTransform(FVector(1400,0,100)));
    if(!Fixture.BeginPlayInTestWorld()){return false;}auto* P=W->GetSubsystem<UPFWorldPersistence>();
    const FString Slot=TEXT("AutomationFrozenRestore")+FGuid::NewGuid().ToString(EGuidFormats::Digits),Updated=Slot+TEXT("New");
    ON_SCOPE_EXIT{P->ActiveSlot.Reset();for(const auto& Owned:{Slot,Updated}){for(bool B:{false,true}){IFileManager::Get().Delete(*FPFSaveFileStore::Path(Owned,B));}const FString Original=FPFSaveFileStore::OriginalPath(Owned,TEXT("WorldProgressionV1"));IFileManager::Get().Delete(*Original);IFileManager::Get().DeleteDirectory(*FPaths::GetPath(Original),false,false);}};
    IFileManager::Get().MakeDirectory(*FPaths::GetPath(FPFSaveFileStore::Path(Slot,false)),true);
    if(!TestTrue(TEXT("Install original historical envelope"),FFileHelper::SaveArrayToFile(Envelope,*FPFSaveFileStore::Path(Slot,false)))){return false;}
    const FString Original=FPFSaveFileStore::OriginalPath(Slot,TEXT("WorldProgressionV1"));FString InspectionError;FPFSaveInspection Inspection;
    TestTrue(TEXT("Read-only old save inspection"),P->InspectSave(Slot,Inspection,InspectionError) && Inspection.WorldVersion==1);
    TestFalse(TEXT("Inspection never creates a V1 original"),IFileManager::Get().FileExists(*Original));
    {const FString Previous(FCommandLine::Get());ON_SCOPE_EXIT{FCommandLine::Set(*Previous);};FCommandLine::Set(*(Previous+TEXT(" -PFSaveSlot=")+Slot+TEXT(" -PFLoadSave")));P->ConfigureStartup();P->ApplyStartup();}
    TArray<uint8> OriginalBytes;TestTrue(TEXT("Actual V1 startup archives exact historical envelope before apply"),FFileHelper::LoadFileToArray(OriginalBytes,*Original) && OriginalBytes==Envelope);
    FString Error;if(!TestTrue(TEXT("Old save startup permits valid login"),P->CheckLogin(TEXT(""),Error))){AddError(Error);return false;}
    auto* PC=W->SpawnActor<APFSurvivalPlayerController>();W->GetAuthGameMode<APFSurvivalGameMode>()->RestartPlayer(PC);P->Login(PC,TEXT(""));bool Restored=false;
    if(!TestTrue(TEXT("Old saved owner restores through actual player adapter"),P->RestorePlayer(PC,&Restored) && Restored)){return false;}
    auto* PS=PC->GetPlayerState<APFInventoryPlayerState>();auto* Bag=PC->GetInventory();auto* V=PC->GetPawn()->FindComponentByClass<UPFPlayerSurvivalComponent>();
    const auto CheckRuntime=[&]()
    {
        TestTrue(TEXT("Owner and original vitals survive old save"),PS->PersistentPlayerId==FakeId(0x11111111) && V->GetVitals().Health==33 && V->GetVitals().Stamina==61 && V->GetVitals().Hunger==47 && V->GetVitals().Thirst==27 && Clock->Hour==21);
        TestEqual(TEXT("Old wood quantity restored without duplication"),Bag->Count(TEXT("Item_Wood")),3);
        const auto& Progress=PS->Progression->GetRecord();TestTrue(TEXT("Legacy load grants no XP knowledge craft credit windows or stats"),Progress.Experience==0 && Progress.Knowledge.IsEmpty() && Progress.CreditedCrafts.IsEmpty() && Progress.GatherWindows.IsEmpty() && Progress.Stats==FPFPlayerStatRanks() && PS->Progression->GetAvailablePoints()==0 && PS->Progression->GetAvailableStatPoints()==0);
        int32 Pieces=0;APFBuildPiece* Chest=nullptr;for(TActorIterator<APFBuildPiece> It(W);It;++It){++Pieces;if(It->PersistentId==FakeId(0x44444444)){Chest=*It;}}
        TestTrue(TEXT("Legacy structure defaults ownership support and stored items"),Pieces==2 && Chest && Chest->PersistentOwnerId==PS->PersistentPlayerId && Chest->Support && Chest->Support->PersistentId==FakeId(0x33333333) && !Chest->bRestrictStorageItems && Chest->AllowedStorageItems.IsEmpty() && Chest->Storage->Count(TEXT("Item_Wood"))==7);
        TestTrue(TEXT("Historical creature identity and spawner link"),APFCreatureSpawner::Count(W)==1 && Point->Resident && Point->Resident->PersistentId==FakeId(0xAAAAAAAA) && !Point->bAutoSpawn);
        int32 Pickups=0;for(TActorIterator<APFItemPickup> It(W);It;++It){++Pickups;TestTrue(TEXT("Historical pickup identity quantity"),It->GetContents().StackId==FakeId(0x99999999) && It->GetContents().Quantity==2);}TestEqual(TEXT("No duplicate pickup"),Pickups,1);
        const int64 Now=FDateTime::UtcNow().ToUnixTimestamp();
        if(Now-static_cast<int64>(Manifest->GetNumberField(TEXT("CapturedUtc")))>=181){TestEqual(TEXT("Offline expired player food not renewed"),Bag->Count(TEXT("Item_Food")),0);}
        if(Chest && Now-static_cast<int64>(Manifest->GetNumberField(TEXT("SavedUtc")))>=91){TestEqual(TEXT("Offline expired stored food not renewed"),Chest->Storage->Count(TEXT("Item_Food")),0);}
        TestEqual(TEXT("Old resource identity preserved"),Node->GetName(),FString(TEXT("LegacyWood")));
    };
    CheckRuntime();
    for(int32 Repeat=0;Repeat<2;++Repeat){TestTrue(TEXT("Unsaved extra item before repeated old load"),Bag->Grant(TEXT("Item_Wood"),1));if(!TestTrue(TEXT("Actual repeated frozen old-world Load"),P->Load(Slot,Error))){AddError(Error);return false;}CheckRuntime();}
    FPFWorldSaveData Current;if(!TestTrue(TEXT("Capture whole upgraded runtime"),P->Capture(Current,Error))){AddError(Error);return false;}
    TestTrue(TEXT("Normal current capture upgrades to explicit progression without duplicate owners"),Current.Version==2 && Current.Players.Num()==1 && !Current.Players[0].Progression.IsEmpty());
    if(!TestTrue(TEXT("Save upgraded state into separate disposable current slot"),P->Save(Updated,Error) && P->Load(Updated,Error))){AddError(Error);return false;}CheckRuntime();
    TArray<uint8> After;TestTrue(TEXT("Historical source envelope never rewritten"),FFileHelper::LoadFileToArray(After,*FPFSaveFileStore::Path(Slot,false)) && After==Envelope);TestFalse(TEXT("Historical source no new generation"),IFileManager::Get().FileExists(*FPFSaveFileStore::Path(Slot,true)));
    TestTrue(TEXT("V2 restoration does not create a new V1 archive"),!IFileManager::Get().FileExists(*FPFSaveFileStore::OriginalPath(Updated,TEXT("WorldProgressionV1"))));
    for(int32 Repeat=0;Repeat<3;++Repeat){if(!TestTrue(TEXT("Normal V2 save rotates both old active generations"),P->Save(Slot,Error))){AddError(Error);return false;}}
    FPFSavedFile Archived;TestTrue(TEXT("First original survives V2 A/B rotation with exact historical generation UTC payload"),FPFSaveFileStore::ReadOriginal(Slot,TEXT("WorldProgressionV1"),Archived,Error) && Archived.Payload==Payload && Archived.Generation==1 && Archived.SavedUtc==static_cast<int64>(Manifest->GetNumberField(TEXT("SavedUtc"))));
    TestTrue(TEXT("Archive stays byte exact after repeated restoration and rotation"),FFileHelper::LoadFileToArray(After,*Original) && After==Envelope);
    TestTrue(TEXT("Rotated current slot still restores"),P->Load(Slot,Error));CheckRuntime();
    Fixture.ForwardErrorMessages(this);AddInfo(TEXT("[PrimalAgentTools] FrozenM8Restore: actual old writer envelope startup/login/repeated Load/current V2 save-load, ownership/storage/creature/pickup/neutral progression and source preservation verified. Native synthetic layout; no historical executable, network or personal-world acceptance."));return true;
}
#endif
