#include "Persistence/PFWorldSaveFormat.h"
#include "Persistence/PFWorldPersistence.h"
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
#include "Creatures/PFCreature.h"
#include "Creatures/PFCreatureSpawner.h"
#include "Survival/PFSurvivalGameMode.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFSurvivorCharacter.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "World/PFWorldClock.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Misc/DateTime.h"
#include "Misc/ScopeExit.h"
#include "Misc/CommandLine.h"
#include <limits>
#include "HAL/FileManager.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/PlayerStart.h"
#include "Tests/AutomationCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFWorldRecordTest, "PF.Persistence.WorldRecords",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFWorldRecordTest::RunTest(const FString&)
{
    auto* Items = NewObject<UPFItemCatalog>(); auto* Buildings = NewObject<UPFBuildingCatalog>();
    auto* Crafting = NewObject<UPFCraftingCatalog>(); auto* Creatures = NewObject<UPFCreatureCatalog>();
    FString Error; FPFWorldSaveData Data; Data.Map = TEXT("L_Automation");
    FPFPlayerSaveData Player; Player.PlayerId = FGuid::NewGuid();
    Player.Inventory.Add({FGuid::NewGuid(), TEXT("Item_Wood"), 3, 0});
    FPFWorldPlayerRecord Owner; Owner.PlayerId = Player.PlayerId; Owner.ReconnectCredential = FGuid::NewGuid();
    Owner.CapturedUtc = FDateTime::UtcNow().ToUnixTimestamp();
    if (!TestTrue(TEXT("Encode player batch"), FPFWorldSaveFormat::PackPlayer(Player, *Items, 30, Owner.Data, Error))) { return false; }
    Data.Players.Add(Owner);
    auto Piece = [&](const TCHAR* Definition, FGuid Support)
    {
        FPFStructureSaveRecord S; S.Id = FGuid::NewGuid(); S.Owner = Owner.PlayerId; S.Definition = Definition;
        S.Health = 100; S.Support = Support; FPFPlayerSaveData Bag; Bag.PlayerId = S.Id;
        TestTrue(TEXT("Encode structure bag"), FPFWorldSaveFormat::PackPlayer(Bag, *Items, 60, S.Storage, Error));
        Data.Structures.Add(S); return S.Id;
    };
    const FGuid Base = Piece(TEXT("Build_Foundation"), {});
    const FGuid Wall = Piece(TEXT("Build_Wall"), Base);
    Piece(TEXT("Build_Ceiling"), Wall); Piece(TEXT("Build_Storage"), Base);
    auto Valid = [&](const FPFWorldSaveData& D) { return FPFWorldSaveFormat::Validate(D, *Items, *Buildings, *Crafting, *Creatures, Error); };
    if (!TestTrue(TEXT("Real foundation/wall/roof/storage graph"), Valid(Data))) { AddError(Error); return false; }
    TArray<uint8> Bytes; FPFWorldSaveData Decoded;
    TestTrue(TEXT("World metadata encode"), FPFWorldSaveFormat::Encode(Data, Bytes, Error));
    if (!TestTrue(TEXT("World metadata decode"), FPFWorldSaveFormat::Decode(Bytes, Decoded, Error))) { AddError(Error); return false; }
    TestTrue(TEXT("Decoded semantic validation"), Valid(Decoded));
    TestEqual(TEXT("Stable structure ID"), Decoded.Structures[0].Id, Base);
    TestEqual(TEXT("Private credential round trip"), Decoded.Players[0].ReconnectCredential, Owner.ReconnectCredential);
    auto Bad = Data; Bad.Structures[1].Support = Bad.Structures[2].Id;
    TestFalse(TEXT("Support cycle rejected"), Valid(Bad));
    Bad = Data; Bad.Structures[2].Support = Base;
    TestFalse(TEXT("Roof cannot bypass wall support"), Valid(Bad));
    Bad = Data; Bad.Structures[1].Support = FGuid::NewGuid();
    TestFalse(TEXT("Missing support rejected"), Valid(Bad));
    Bad = Data; Bad.Structures[0].Rotation.Yaw = 1.e20;
    TestFalse(TEXT("Unbounded rotation rejected"), Valid(Bad));
    Bad = Data; Bad.Structures[0].Definition = TEXT("Unknown_PersistenceDefinition");
    TestFalse(TEXT("Unknown catalog definition rejected"), Valid(Bad));
    FPFPlayerSaveData Stored; Stored.PlayerId = Data.Structures[3].Id; Stored.Inventory = Player.Inventory;
    Bad = Data; FPFWorldSaveFormat::PackPlayer(Stored, *Items, 60, Bad.Structures[3].Storage, Error);
    TestFalse(TEXT("Same stack in bag and storage rejected"), Valid(Bad));
    Bad = Data; Bad.Players.Add(Owner);
    TestFalse(TEXT("Duplicated player/credential rejected"), Valid(Bad));
    auto SecondOwner = Owner; SecondOwner.PlayerId = FGuid::NewGuid(); SecondOwner.ReconnectCredential = FGuid::NewGuid();
    FPFPlayerSaveData Second; Second.PlayerId = SecondOwner.PlayerId;
    FPFWorldSaveFormat::PackPlayer(Second, *Items, 30, SecondOwner.Data, Error);
    Bad = Data; Bad.Players.Add(SecondOwner); Bad.Structures[1].Owner = SecondOwner.PlayerId;
    TestFalse(TEXT("Cross-owner support rejected"), Valid(Bad));
    FPFPickupSaveRecord Pickup; Pickup.Location.X = std::numeric_limits<double>::infinity(); Bad = Data; Bad.Pickups.Add(Pickup);
    TestFalse(TEXT("Invalid pickup position rejected"), Valid(Bad)); TestFalse(TEXT("Explicit refusal reason"), Error.IsEmpty());
    Decoded.Map = TEXT("Preserved");
    TestFalse(TEXT("Truncated JSON refused"), FPFWorldSaveFormat::Decode({uint8('{')}, Decoded, Error));
    TestEqual(TEXT("Failed decode preserves output"), Decoded.Map, FString(TEXT("Preserved")));
    Bad = Data; Bad.Version = 2; FPFWorldSaveFormat::Encode(Bad, Bytes, Error);
    TestFalse(TEXT("Future version requires migration"), FPFWorldSaveFormat::Decode(Bytes, Decoded, Error));
    AddInfo(TEXT("[PrimalPersistence] World records, support graph, independent ownership, duplicate batches and future-version refusal checked"));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFWorldRestoreTest, "PF.Persistence.WorldRuntime",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFWorldRestoreTest::RunTest(const FString&)
{
    AddExpectedMessage(TEXT("GetSocketInfoByName.*No SkeletalMesh for Component"), EAutomationExpectedMessageFlags::Contains, 0);
    FTestWorldWrapper Fixture; if (!Fixture.CreateTestWorld(EWorldType::Game)) { return false; }
    UWorld* W = Fixture.GetTestWorld();
    // Rename this transient fixture's package only. No saved map or project asset is modified.
    W->GetOutermost()->Rename(*(TEXT("/Temp/PFPersistence/") + FGuid::NewGuid().ToString(EGuidFormats::Digits) + TEXT("/L_Automation")),
        nullptr, REN_DontCreateRedirectors | REN_NonTransactional);
    W->SetGameMode(FURL(nullptr, TEXT("/Engine/Maps/Entry?game=/Script/PrimalFrontier.PFSurvivalGameMode"), TRAVEL_Absolute));
    auto* Floor = W->SpawnActor<AActor>(); auto* Box = NewObject<UBoxComponent>(Floor); Floor->SetRootComponent(Box);
    Box->SetBoxExtent(FVector(4000, 4000, 25)); Box->SetCollisionProfileName(TEXT("BlockAll")); Box->RegisterComponent();
    Floor->SetActorLocation(FVector(0, 0, -25));
    W->SpawnActor<APlayerStart>(FVector(-500, 0, 120), FRotator::ZeroRotator);
    auto* Clock = W->SpawnActor<APFWorldClock>();
    auto* Node = W->SpawnActor<APFResourceNode>(FVector(0, 1000, 0), FRotator::ZeroRotator);
    auto* SpawnPoint = W->SpawnActorDeferred<APFCreatureSpawner>(APFCreatureSpawner::StaticClass(), FTransform(FVector(1200, 0, 50)));
    SpawnPoint->bAutoSpawn = false; SpawnPoint->FinishSpawning(FTransform(FVector(1200, 0, 50)));
    if (!Fixture.BeginPlayInTestWorld()) { return false; }
    auto* PC = W->SpawnActor<APFSurvivalPlayerController>(); W->GetAuthGameMode<APFSurvivalGameMode>()->RestartPlayer(PC);
    auto* Persistence = W->GetSubsystem<UPFWorldPersistence>(); FString Error;
    if (!TestNotNull(TEXT("World subsystem"), Persistence) || !TestNotNull(TEXT("Bag"), PC->GetInventory())) { return false; }
    Persistence->Login(PC, TEXT(""));
    auto* Pawn = CastChecked<APFSurvivorCharacter>(PC->GetPawn());
    auto* Buildings = NewObject<UPFBuildingCatalog>();
    auto SpawnPiece = [&](FName Id, FVector At, APFBuildPiece* Support)
    {
        const FTransform T(At); auto* Piece = W->SpawnActorDeferred<APFBuildPiece>(APFBuildPiece::StaticClass(), T);
        Piece->Initialize(*Buildings->Find(Id), PC->PlayerState, Support); Piece->FinishSpawning(T); return Piece;
    };
    auto* Base = SpawnPiece(TEXT("Build_Foundation"), FVector(600, 600, 10), nullptr);
    auto* Chest = SpawnPiece(TEXT("Build_Storage"), FVector(600, 600, 60), Base);
    const FGuid ChestId = Chest->PersistentId, OwnerId = Chest->PersistentOwnerId;
    TestTrue(TEXT("Player bag wood"), PC->GetInventory()->Grant(TEXT("Item_Wood"), 4));
    TestTrue(TEXT("Chest stone"), Chest->Storage->Grant(TEXT("Item_Stone"), 2));
    const FGuid StorageBatchId = Chest->Storage->GetStacks()[0].StackId;
    auto* Pickup = W->SpawnActorDeferred<APFItemPickup>(APFItemPickup::StaticClass(), FTransform(FVector(300, 0, 30)));
    Pickup->Initialize(TEXT("Item_Fibre"), 2, 0); Pickup->FinishSpawning(FTransform(FVector(300, 0, 30)));
    const FGuid PickupId = Pickup->GetContents().StackId;
    auto* Creature = W->SpawnActor<APFCreature>(FVector(1200, 0, 50), FRotator::ZeroRotator);
    const FGuid CreatureId = Creature->PersistentId; SpawnPoint->RestorePersistence(Creature, 5, false);
    TestTrue(TEXT("Depleted node fixture"), Node->RestorePersistence(0, 10));
    Clock->SetHour(22); Pawn->Survival->SetHealth(65);
    FPFWorldSaveData Snapshot;
    if (!TestTrue(TEXT("Authoritative whole-world capture"), Persistence->Capture(Snapshot, Error))) { AddError(Error); return false; }
    const FString Slot = TEXT("AutomationWorld_") + FGuid::NewGuid().ToString(EGuidFormats::Digits);
    ON_SCOPE_EXIT
    {
        // Delete only this fixture's unique generated files through Unreal's file API.
        for (bool B : {false, true}) { IFileManager::Get().Delete(*FPFSaveFileStore::Path(Slot, B)); }
    };
    if (!TestTrue(TEXT("Write real world slot"), Persistence->Save(Slot, Error))) { AddError(Error); return false; }
    // Reported M8 regression: saving appeared to return the world to a fresh state.
    // Save must capture/publish only; loading and natural respawn are separate.
    TestTrue(TEXT("Save retains the same possessed pawn"),PC->GetPawn()==Pawn && IsValid(Pawn));
    TestTrue(TEXT("Save retains live structures, pickup and creature instances"),IsValid(Base) && IsValid(Chest) && IsValid(Pickup) && IsValid(Creature));
    TestEqual(TEXT("Save preserves player health"),Pawn->Survival->GetVitals().Health,65.f);
    TestEqual(TEXT("Save preserves inventory"),PC->GetInventory()->Count(TEXT("Item_Wood")),4);
    TestEqual(TEXT("Save preserves storage"),Chest->Storage->Count(TEXT("Item_Stone")),2);
    TestEqual(TEXT("Save preserves depleted resource"),Node->HitsRemaining,0);
    TestEqual(TEXT("Save preserves world time"),Clock->Hour,22.f);
    FPFWorldSaveData AfterSave;TArray<uint8> BeforeBytes,AfterBytes;
    if(!TestTrue(TEXT("Capture live world after saving"),Persistence->Capture(AfterSave,Error))){AddError(Error);return false;}
    // Capture timestamps can cross a real-time second; they are metadata, not
    // simulation. Normalize only those before comparing all serialized state.
    for(auto& P:AfterSave.Players)
    {if(const auto* Before=Snapshot.Players.FindByPredicate([&](const auto& V){return V.PlayerId==P.PlayerId;})){P.CapturedUtc=Before->CapturedUtc;}}
    if(!TestTrue(TEXT("Encode before/after live records"),Persistence->Encode(Snapshot,BeforeBytes,Error) && Persistence->Encode(AfterSave,AfterBytes,Error))){AddError(Error);return false;}
    TestTrue(TEXT("Saving changes no serialized live gameplay state"),BeforeBytes==AfterBytes);
    PC->GetInventory()->RemoveItem(TEXT("Item_Wood"), 4); PC->GetInventory()->Grant(TEXT("Item_Fibre"), 3);
    Chest->Storage->RemoveItem(TEXT("Item_Stone"), 2); Chest->Health = 50;
    Clock->SetHour(9); Node->RestorePersistence(3, 0); Pawn->Survival->SetHealth(90);
    for (int32 Repeat = 0; Repeat < 2; ++Repeat)
    {
        if (!TestTrue(TEXT("Restore real world slot"), Persistence->Load(Slot, Error))) { AddError(Error); return false; }
        TestEqual(TEXT("Bag restored once"), PC->GetInventory()->Count(TEXT("Item_Wood")), 4);
        TestEqual(TEXT("Unsaved bag item removed"), PC->GetInventory()->Count(TEXT("Item_Fibre")), 0);
        TestEqual(TEXT("Health restored"), Pawn->Survival->GetVitals().Health, 65.f);
        TestEqual(TEXT("Night restored"), Clock->Hour, 22.f); TestEqual(TEXT("Depletion restored"), Node->HitsRemaining, 0);
        int32 PieceCount = 0, PickupCount = 0, CreatureCount = 0;
        for (TActorIterator<APFBuildPiece> It(W); It; ++It)
        {
            ++PieceCount; if (It->PersistentId != ChestId) { continue; }
            TestEqual(TEXT("Chest owner key restored"), It->PersistentOwnerId, OwnerId);
            TestTrue(TEXT("Connected owner rebound"), It->IsOwnedBy(PC->PlayerState) && It->Builder == PC->PlayerState);
            TestEqual(TEXT("Stored quantity restored once"), It->Storage->Count(TEXT("Item_Stone")), 2);
            TestEqual(TEXT("Storage stable batch ID"), It->Storage->GetStacks()[0].StackId, StorageBatchId);
        }
        for (TActorIterator<APFItemPickup> It(W); It; ++It) { ++PickupCount; TestEqual(TEXT("Dropped batch ID preserved"), It->GetContents().StackId, PickupId); }
        for (TActorIterator<APFCreature> It(W); It; ++It)
        {
            ++CreatureCount; TestEqual(TEXT("Creature ID preserved"), It->PersistentId, CreatureId);
            TestTrue(TEXT("Living creature retains collision"), !It->IsDead() && It->GetCapsuleComponent()->GetCollisionEnabled() != ECollisionEnabled::NoCollision);
        }
        TestEqual(TEXT("No duplicate structures"), PieceCount, 2); TestEqual(TEXT("No duplicate pickups"), PickupCount, 1);
        TestEqual(TEXT("No duplicate creatures"), CreatureCount, 1);
        TestTrue(TEXT("Spawner rebound to restored resident"), IsValid(SpawnPoint->Resident) && SpawnPoint->Resident->PersistentId == CreatureId);
    }
    TestFalse(TEXT("Unknown credential refused"), Persistence->CheckLogin(TEXT("?PFReconnect=") + FGuid::NewGuid().ToString(EGuidFormats::Digits), Error));
    TestFalse(TEXT("Duplicate connected identity refused"), Persistence->CheckLogin(TEXT("?PFReconnect=") + Snapshot.Players[0].ReconnectCredential.ToString(EGuidFormats::Digits), Error));
    TestFalse(TEXT("Malformed credential refused"), Persistence->CheckLogin(TEXT("?PFReconnect=invalid"), Error));
    TestFalse(TEXT("Missing load is safe"), Persistence->Load(Slot + TEXT("_missing"), Error));
    TestEqual(TEXT("Failed load preserves player"), PC->GetInventory()->Count(TEXT("Item_Wood")), 4);
    TestTrue(TEXT("Unsaved departure marker"), PC->GetInventory()->Grant(TEXT("Item_Fibre"), 1));
    PC->Destroy(); // Exercise the real engine controller teardown ordering.
    FPFSavedFile DepartureFile; FPFWorldSaveData Departure; FPFPlayerSaveData DepartedPlayer;
    if (!TestTrue(TEXT("Departure writes active slot"), FPFSaveFileStore::Read(Slot, DepartureFile, Error)) ||
        !TestTrue(TEXT("Departure world validates"), Persistence->Decode(DepartureFile.Payload, Departure, Error))) { AddError(Error); return false; }
    TestTrue(TEXT("Departure updated generation"), DepartureFile.Generation > 1);
    if (!TestTrue(TEXT("Departed player decode"), FPFWorldSaveFormat::UnpackPlayer(Departure.Players[0].Data, *NewObject<UPFItemCatalog>(), 30, DepartedPlayer, Error))) { return false; }
    TestTrue(TEXT("Disconnect preserves change since manual save"), DepartedPlayer.Inventory.ContainsByPredicate([](const auto& S) { return S.ItemId == TEXT("Item_Fibre") && S.Quantity == 1; }));
    TestTrue(TEXT("Sole standalone owner can reopen a world after another world's profile"),
        Persistence->CheckLogin(TEXT("?PFReconnect=")+FGuid::NewGuid().ToString(EGuidFormats::Digits),Error));
    Fixture.ForwardErrorMessages(this);
    AddInfo(TEXT("[PrimalPersistence] Real file/world restore twice: player, storage, ownership, depletion, clock, pickup and creature IDs; no append duplication"));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFWorldStartupFailureTest, "PF.Persistence.StartupFailurePreservesSave",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFWorldStartupFailureTest::RunTest(const FString&)
{
    AddExpectedError(TEXT("Startup load refused:"), EAutomationExpectedErrorFlags::Contains, 1);
    AddExpectedError(TEXT("Startup world restore refused:"), EAutomationExpectedErrorFlags::Contains, 1);
    for (bool bLayoutMismatch : {false, true})
    {
        FTestWorldWrapper Fixture;
        if (!Fixture.CreateTestWorld(EWorldType::Game)) { return false; }
        UWorld* W = Fixture.GetTestWorld();
        W->GetOutermost()->Rename(*(TEXT("/Temp/PFStartupFailure/") + FGuid::NewGuid().ToString(EGuidFormats::Digits) + TEXT("/L_Automation")),
            nullptr, REN_DontCreateRedirectors | REN_NonTransactional);
        W->SetGameMode(FURL(nullptr, TEXT("/Engine/Maps/Entry?game=/Script/PrimalFrontier.PFSurvivalGameMode"), TRAVEL_Absolute));
        W->SpawnActor<APFWorldClock>();
        if (!Fixture.BeginPlayInTestWorld()) { return false; }
        auto* Persistence = W->GetSubsystem<UPFWorldPersistence>();
        if (!TestNotNull(TEXT("Startup fixture subsystem"), Persistence)) { return false; }
        FPFWorldSaveData Data; Data.Map = TEXT("L_Automation");
        if (bLayoutMismatch)
        {
            FPFResourceSaveRecord Node; Node.Name = TEXT("SavedOnlyNode"); Node.Definition = TEXT("Node_Wood"); Node.Hits = 3;
            Data.Resources.Add(Node); // Valid record; this authored node is absent from the current map.
        }
        else { Data.Version = 2; } // Valid file checksum; unsupported world payload.
        FString Error; TArray<uint8> Bytes;
        const FString Slot = TEXT("AutomationStartupFailure_") + FGuid::NewGuid().ToString(EGuidFormats::Digits);
        ON_SCOPE_EXIT
        {
            for (bool B : {false, true}) { IFileManager::Get().Delete(*FPFSaveFileStore::Path(Slot, B)); }
        };
        if (!TestTrue(TEXT("Encode startup failure fixture"), FPFWorldSaveFormat::Encode(Data, Bytes, Error)) ||
            !TestTrue(TEXT("Write unique startup fixture"), FPFSaveFileStore::Write(Slot, Bytes, Error))) { AddError(Error); return false; }
        FPFSavedFile Before;
        if (!TestTrue(TEXT("Read original generation"), FPFSaveFileStore::Read(Slot, Before, Error))) { return false; }
        {
            // Configure through the real startup path; restore process arguments before other tests/world teardown.
            const FString PreviousCommandLine(FCommandLine::Get());
            ON_SCOPE_EXIT { FCommandLine::Set(*PreviousCommandLine); };
            FCommandLine::Set(*(PreviousCommandLine + TEXT(" -PFSaveSlot=") + Slot + TEXT(" -PFLoadSave")));
            Persistence->ConfigureStartup(); Persistence->ApplyStartup();
        }
        TestFalse(TEXT("Failed startup refuses login"), Persistence->CheckLogin(TEXT(""), Error));
        FPFWorldSaveData Output; Output.Map = TEXT("Preserved");
        TestFalse(TEXT("Failed startup refuses default-world capture"), Persistence->Capture(Output, Error));
        TestEqual(TEXT("Refused capture preserves output"), Output.Map, FString(TEXT("Preserved")));
        TestFalse(TEXT("Failed startup refuses default-world save"), Persistence->Save(Slot, Error));
        TestTrue(TEXT("Save refusal identifies startup failure"), Error.Contains(TEXT("Startup")));
        FPFSavedFile After;
        if (!TestTrue(TEXT("Original generation remains readable"), FPFSaveFileStore::Read(Slot, After, Error))) { return false; }
        TestEqual(TEXT("No new generation published after failure"), After.Generation, Before.Generation);
        TestTrue(TEXT("Original payload preserved"), After.Payload == Before.Payload);
        Fixture.ForwardErrorMessages(this);
    }
    AddInfo(TEXT("[PrimalPersistence] Exercised unsupported startup payload and authored-layout mismatch: login/capture/save refusal and original generation/payload preservation assertions"));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFWorldRejectedLoadTest, "PF.Persistence.RejectedLoadPreservesWorld",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFWorldRejectedLoadTest::RunTest(const FString&)
{
    AddExpectedMessage(TEXT("GetSocketInfoByName.*No SkeletalMesh for Component"), EAutomationExpectedMessageFlags::Contains, 0);
    FTestWorldWrapper Fixture;
    if (!Fixture.CreateTestWorld(EWorldType::Game)) { return false; }
    UWorld* W = Fixture.GetTestWorld();
    W->GetOutermost()->Rename(*(TEXT("/Temp/PFRejectedLoad/") + FGuid::NewGuid().ToString(EGuidFormats::Digits) + TEXT("/L_Automation")),
        nullptr, REN_DontCreateRedirectors | REN_NonTransactional);
    W->SetGameMode(FURL(nullptr, TEXT("/Engine/Maps/Entry?game=/Script/PrimalFrontier.PFSurvivalGameMode"), TRAVEL_Absolute));
    auto* Floor = W->SpawnActor<AActor>(); auto* Box = NewObject<UBoxComponent>(Floor); Floor->SetRootComponent(Box);
    Box->SetBoxExtent(FVector(4000, 4000, 25)); Box->SetCollisionProfileName(TEXT("BlockAll")); Box->RegisterComponent();
    Floor->SetActorLocation(FVector(0, 0, -25));
    W->SpawnActor<APlayerStart>(FVector(-500, 0, 120), FRotator::ZeroRotator);
    auto* Clock = W->SpawnActor<APFWorldClock>();
    auto* Node = W->SpawnActor<APFResourceNode>(FVector(0, 1000, 0), FRotator::ZeroRotator);
    if (!Fixture.BeginPlayInTestWorld()) { return false; }
    auto* PC = W->SpawnActor<APFSurvivalPlayerController>(); W->GetAuthGameMode<APFSurvivalGameMode>()->RestartPlayer(PC);
    auto* Persistence = W->GetSubsystem<UPFWorldPersistence>();
    auto* Pawn = Cast<APFSurvivorCharacter>(PC->GetPawn()); FString Error;
    if (!TestNotNull(TEXT("Save subsystem"), Persistence) || !TestNotNull(TEXT("Survivor"), Pawn) ||
        !TestNotNull(TEXT("Player inventory"), PC->GetInventory())) { return false; }
    Persistence->Login(PC, TEXT(""));
    auto* Buildings = NewObject<UPFBuildingCatalog>();
    auto SpawnPiece = [&](FName Id, FVector At, APFBuildPiece* Support)
    {
        const FTransform T(At); auto* Piece = W->SpawnActorDeferred<APFBuildPiece>(APFBuildPiece::StaticClass(), T);
        Piece->Initialize(*Buildings->Find(Id), PC->PlayerState, Support); Piece->FinishSpawning(T); return Piece;
    };
    auto* Base = SpawnPiece(TEXT("Build_Foundation"), FVector(600, 600, 10), nullptr);
    auto* Chest = SpawnPiece(TEXT("Build_Storage"), FVector(600, 600, 60), Base);
    if (!TestTrue(TEXT("Player wood fixture"), PC->GetInventory()->Grant(TEXT("Item_Wood"), 4)) ||
        !TestTrue(TEXT("Storage stone fixture"), Chest->Storage->Grant(TEXT("Item_Stone"), 2)) ||
        !TestTrue(TEXT("Partial resource fixture"), Node->RestorePersistence(1, 0))) { return false; }
    Pawn->Survival->SetHealth(65); Clock->SetHour(22);
    const FVector Location = Pawn->GetActorLocation();
    const FGuid ChestId = Chest->PersistentId, OwnerId = Chest->PersistentOwnerId;
    const FGuid BagBatch = PC->GetInventory()->GetStacks()[0].StackId, ChestBatch = Chest->Storage->GetStacks()[0].StackId;
    FPFWorldSaveData Good;
    if (!TestTrue(TEXT("Capture running baseline"), Persistence->Capture(Good, Error))) { AddError(Error); return false; }
    const FString ActiveSlot = TEXT("AutomationRejected_") + FGuid::NewGuid().ToString(EGuidFormats::Digits);
    TArray<FString> Slots{ActiveSlot};
    ON_SCOPE_EXIT
    {
        // Prevent fixture teardown from publishing a logout checkpoint after unique-file cleanup.
        Persistence->ActiveSlot.Reset();
        for (const auto& Slot : Slots) { for (bool B : {false, true}) { IFileManager::Get().Delete(*FPFSaveFileStore::Path(Slot, B)); } }
    };
    if (!TestTrue(TEXT("Save good active world"), Persistence->Save(ActiveSlot, Error))) { AddError(Error); return false; }
    FPFSavedFile Original;
    if (!TestTrue(TEXT("Read active generation"), FPFSaveFileStore::Read(ActiveSlot, Original, Error))) { return false; }
    const TCHAR* Cases[] = {TEXT("unsupported version"), TEXT("wrong map"), TEXT("resource layout"), TEXT("unsafe player ground")};
    for (int32 Case = 0; Case < UE_ARRAY_COUNT(Cases); ++Case)
    {
        auto Bad = Good; Bad.Hour = 5; Bad.Structures[0].Health = 50; Bad.Resources[0].Hits = 0;
        if (Case == 0) { Bad.Version = 2; }
        if (Case == 1) { Bad.Map = TEXT("L_M7SurvivalArena"); }
        if (Case == 2) { Bad.Resources[0].Name = TEXT("DifferentAuthoredNode"); }
        if (Case == 3)
        {
            FPFPlayerSaveData Unsafe;
            if (!TestTrue(TEXT("Decode fixture player"), FPFWorldSaveFormat::UnpackPlayer(Bad.Players[0].Data, *PC->GetInventory()->Catalog, 30, Unsafe, Error))) { return false; }
            Unsafe.Location = FVector(100000, 0, 120); // Finite/bounded, but outside this fixture's walkable floor.
            if (!TestTrue(TEXT("Encode unsafe location fixture"), FPFWorldSaveFormat::PackPlayer(Unsafe, *PC->GetInventory()->Catalog, 30, Bad.Players[0].Data, Error))) { return false; }
        }
        const FString RejectedSlot = ActiveSlot + FString::Printf(TEXT("_%d"), Case); Slots.Add(RejectedSlot);
        TArray<uint8> Bytes;
        if (!TestTrue(TEXT("Encode rejected candidate"), FPFWorldSaveFormat::Encode(Bad, Bytes, Error)) ||
            !TestTrue(TEXT("Write checksummed rejected candidate"), FPFSaveFileStore::Write(RejectedSlot, Bytes, Error))) { AddError(Error); return false; }
        TestFalse(*FString::Printf(TEXT("Refuse %s"), Cases[Case]), Persistence->Load(RejectedSlot, Error));
        TestFalse(TEXT("Explicit load refusal reason"), Error.IsEmpty());
        TestEqual(TEXT("Active slot preserved"), Persistence->ActiveSlot, ActiveSlot);
        TestTrue(TEXT("Same pawn remains possessed"), PC->GetPawn() == Pawn);
        TestEqual(TEXT("Player position preserved"), Pawn->GetActorLocation(), Location);
        TestEqual(TEXT("Health preserved"), Pawn->Survival->GetVitals().Health, 65.f);
        TestEqual(TEXT("Inventory quantity preserved"), PC->GetInventory()->Count(TEXT("Item_Wood")), 4);
        TestEqual(TEXT("Inventory batch preserved"), PC->GetInventory()->GetStacks()[0].StackId, BagBatch);
        if (!TestTrue(TEXT("Original structure actors retained"), IsValid(Base) && IsValid(Chest))) { return false; }
        TestEqual(TEXT("Storage identity preserved"), Chest->PersistentId, ChestId);
        TestTrue(TEXT("Owner and support retained"), Chest->PersistentOwnerId == OwnerId && Chest->Builder == PC->PlayerState && Chest->Support == Base);
        TestEqual(TEXT("Storage quantity preserved"), Chest->Storage->Count(TEXT("Item_Stone")), 2);
        TestEqual(TEXT("Storage batch preserved"), Chest->Storage->GetStacks()[0].StackId, ChestBatch);
        TestEqual(TEXT("Structure health preserved"), Base->Health, Good.Structures[0].Health);
        TestEqual(TEXT("Resource state preserved"), Node->HitsRemaining, 1);
        TestEqual(TEXT("Clock preserved"), Clock->Hour, 22.f);
        int32 Pieces = 0; for (TActorIterator<APFBuildPiece> It(W); It; ++It) { ++Pieces; }
        TestEqual(TEXT("No replacement/duplicate structures"), Pieces, 2);
        FPFSavedFile After;
        if (!TestTrue(TEXT("Active file still readable"), FPFSaveFileStore::Read(ActiveSlot, After, Error))) { return false; }
        TestTrue(TEXT("Active generation and bytes unchanged"), After.Generation == Original.Generation && After.Payload == Original.Payload);
    }
    TestTrue(TEXT("Valid saving remains available after refusals"), Persistence->Save(ActiveSlot, Error));
    Fixture.ForwardErrorMessages(this);
    AddInfo(TEXT("[PrimalPersistence] Rejected real-file version/map/layout/unsafe-ground loads checked against running player, inventory, structures/storage/owner, resource, clock and active generation"));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFWorldCorpseLootTest, "PF.Persistence.CorpseLootRestore",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFWorldCorpseLootTest::RunTest(const FString&)
{
    FTestWorldWrapper Fixture;
    if (!Fixture.CreateTestWorld(EWorldType::Game)) { return false; }
    UWorld* W = Fixture.GetTestWorld();
    W->GetOutermost()->Rename(*(TEXT("/Temp/PFCorpseLoot/") + FGuid::NewGuid().ToString(EGuidFormats::Digits) + TEXT("/L_Automation")),
        nullptr, REN_DontCreateRedirectors | REN_NonTransactional);
    W->SetGameMode(FURL(nullptr, TEXT("/Engine/Maps/Entry?game=/Script/PrimalFrontier.PFSurvivalGameMode"), TRAVEL_Absolute));
    W->SpawnActor<APFWorldClock>();
    if (!Fixture.BeginPlayInTestWorld()) { return false; }
    auto* Persistence = W->GetSubsystem<UPFWorldPersistence>();
    auto* Original = W->SpawnActor<APFCreature>(FVector(1000, 0, 100), FRotator::ZeroRotator);
    if (!TestNotNull(TEXT("Corpse save subsystem"), Persistence) || !TestNotNull(TEXT("Forager fixture"), Original)) { return false; }
    const FGuid CreatureId = Original->PersistentId;
    TestTrue(TEXT("Real authoritative lethal damage"), Original->TakeDamage(1000, FDamageEvent(), nullptr, nullptr) > 0);
    FPFWorldSaveData Baseline; FString Error;
    if (!TestTrue(TEXT("Capture corpse and loot"), Persistence->Capture(Baseline, Error))) { AddError(Error); return false; }
    if (!TestEqual(TEXT("One saved corpse"), Baseline.Creatures.Num(), 1) ||
        !TestEqual(TEXT("One saved loot pickup"), Baseline.Pickups.Num(), 1)) { return false; }
    FGuid LootId; int32 LootQuantity = 0; double LootDeadline = 0;
    for (TActorIterator<APFItemPickup> It(W); It; ++It)
    { LootId = It->GetContents().StackId; LootQuantity = It->GetContents().Quantity; LootDeadline = It->GetContents().ExpiresAt; }
    TestTrue(TEXT("Loot starts with a finite expiration"), LootDeadline > UPFInventoryComponent::ServerTime(W));
    const FString Slot = TEXT("AutomationCorpse_") + FGuid::NewGuid().ToString(EGuidFormats::Digits);
    ON_SCOPE_EXIT
    {
        Persistence->ActiveSlot.Reset();
        for (bool B : {false, true}) { IFileManager::Get().Delete(*FPFSaveFileStore::Path(Slot, B)); }
    };
    if (!TestTrue(TEXT("Save real corpse slot"), Persistence->Save(Slot, Error))) { AddError(Error); return false; }
    for (bool bLootRemoved : {false, true})
    {
        if (bLootRemoved)
        {
            // Model an already removed/expired world drop; do not invent a player transfer.
            for (TActorIterator<APFItemPickup> It(W); It; ++It) { It->Destroy(); }
            if (!TestTrue(TEXT("Save corpse without its previous drop"), Persistence->Save(Slot, Error))) { AddError(Error); return false; }
        }
        for (int32 Repeat = 0; Repeat < 2; ++Repeat)
        {
            if (!TestTrue(TEXT("Load corpse slot repeatedly"), Persistence->Load(Slot, Error))) { AddError(Error); return false; }
            int32 CorpseCount = 0, PickupCount = 0;
            for (TActorIterator<APFCreature> It(W); It; ++It)
            {
                ++CorpseCount;
                TestEqual(TEXT("Stable corpse identity"), It->PersistentId, CreatureId);
                TestTrue(TEXT("Restored corpse remains dead"), It->IsDead() && It->State.ToString() == TEXT("Creature.State.Dead"));
                TestEqual(TEXT("Corpse collision stays off"), It->GetCapsuleComponent()->GetCollisionEnabled(), ECollisionEnabled::NoCollision);
                TestTrue(TEXT("Corpse has a bounded removal timer"), It->GetLifeSpan() > 0 && It->GetLifeSpan() <= 12);
                TestEqual(TEXT("Further corpse damage has no effect"), It->TakeDamage(1000, FDamageEvent(), nullptr, nullptr), 0.f);
            }
            for (TActorIterator<APFItemPickup> It(W); It; ++It)
            {
                ++PickupCount;
                TestEqual(TEXT("Loot batch identity retained"), It->GetContents().StackId, LootId);
                TestEqual(TEXT("Loot quantity retained once"), It->GetContents().Quantity, LootQuantity);
                TestTrue(TEXT("Restoration never renews loot lifetime"), It->GetContents().ExpiresAt <= LootDeadline + 0.001);
            }
            TestEqual(TEXT("Exactly one corpse"), CorpseCount, 1);
            TestEqual(TEXT("No replayed or duplicated death loot"), PickupCount, bLootRemoved ? 0 : 1);
        }
    }
    Fixture.ForwardErrorMessages(this);
    AddInfo(TEXT("[PrimalPersistence] Real corpse/drop save and repeated load checked: stable IDs, no renewed food life, no loot replay after removing the drop"));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFWorldStructureCollisionTest, "PF.Persistence.StructureCollisionRestore",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFWorldStructureCollisionTest::RunTest(const FString&)
{
    AddExpectedMessage(TEXT("GetSocketInfoByName.*No SkeletalMesh for Component"), EAutomationExpectedMessageFlags::Contains, 0);
    FTestWorldWrapper Fixture;
    if (!Fixture.CreateTestWorld(EWorldType::Game)) { return false; }
    UWorld* W = Fixture.GetTestWorld();
    W->GetOutermost()->Rename(*(TEXT("/Temp/PFStructureCollision/") + FGuid::NewGuid().ToString(EGuidFormats::Digits) + TEXT("/L_Automation")),
        nullptr, REN_DontCreateRedirectors | REN_NonTransactional);
    W->SetGameMode(FURL(nullptr, TEXT("/Engine/Maps/Entry?game=/Script/PrimalFrontier.PFSurvivalGameMode"), TRAVEL_Absolute));
    auto* Floor = W->SpawnActor<AActor>(); auto* Box = NewObject<UBoxComponent>(Floor); Floor->SetRootComponent(Box);
    Box->SetBoxExtent(FVector(4000, 4000, 25)); Box->SetCollisionProfileName(TEXT("BlockAll")); Box->RegisterComponent();
    Floor->SetActorLocation(FVector(0, 0, -25));
    W->SpawnActor<APlayerStart>(FVector(-500, 0, 120), FRotator::ZeroRotator);
    W->SpawnActor<APFWorldClock>();
    if (!Fixture.BeginPlayInTestWorld()) { return false; }
    auto* PC = W->SpawnActor<APFSurvivalPlayerController>(); W->GetAuthGameMode<APFSurvivalGameMode>()->RestartPlayer(PC);
    auto* Persistence = W->GetSubsystem<UPFWorldPersistence>(); FString Error;
    if (!TestNotNull(TEXT("Structure save subsystem"), Persistence) || !TestTrue(TEXT("Fixture pawn"), PC->GetPawn() != nullptr)) { return false; }
    Persistence->Login(PC, TEXT(""));
    auto* Buildings = NewObject<UPFBuildingCatalog>();
    auto SpawnPiece = [&](FName Id, FVector At, APFBuildPiece* Support)
    {
        const FTransform T(At); auto* Piece = W->SpawnActorDeferred<APFBuildPiece>(APFBuildPiece::StaticClass(), T);
        Piece->Initialize(*Buildings->Find(Id), PC->PlayerState, Support); Piece->FinishSpawning(T); return Piece;
    };
    auto* Base = SpawnPiece(TEXT("Build_Foundation"), FVector(600, 0, 15), nullptr);
    auto* Door = SpawnPiece(TEXT("Build_Door"), FVector(600, 0, 180), Base);
    const FGuid BaseId = Base->PersistentId, DoorId = Door->PersistentId;
    if (!TestTrue(TEXT("Owner opens fixture door"), Door->ToggleDoor(PC->PlayerState))) { return false; }
    auto GapBlocked = [&](APFBuildPiece* AtDoor, float Y)
    {
        FHitResult Hit; const FVector Center = AtDoor->GetActorLocation() + FVector(0, Y, -25);
        return W->LineTraceSingleByChannel(Hit, Center - FVector(80, 0, 0), Center + FVector(80, 0, 0), ECC_Visibility);
    };
    auto AboveFoundationBlocked = [&](APFBuildPiece* AtBase, APFBuildPiece* IgnoreDoor)
    {
        FHitResult Hit; FCollisionQueryParams Params(SCENE_QUERY_STAT(PFRestoredFoundation), false, IgnoreDoor);
        const FVector Center = AtBase->GetActorLocation();
        return W->LineTraceSingleByChannel(Hit, Center + FVector(0, 0, 70), Center + FVector(0, 0, 40), ECC_Visibility, Params);
    };
    auto PlayerCapsuleBlocked = [&](APFBuildPiece* AtDoor)
    {
        FHitResult Hit; const FVector Center = AtDoor->GetActorLocation() + FVector(0, 0, -25);
        const auto* Capsule = CastChecked<APFSurvivorCharacter>(PC->GetPawn())->GetCapsuleComponent();
        FCollisionQueryParams Params(SCENE_QUERY_STAT(PFRestoredDoorPassage), false, PC->GetPawn());
        return W->SweepSingleByChannel(Hit, Center - FVector(80, 0, 0), Center + FVector(80, 0, 0), FQuat::Identity,
            ECC_Pawn, FCollisionShape::MakeCapsule(Capsule->GetScaledCapsuleRadius(), Capsule->GetScaledCapsuleHalfHeight()), Params);
    };
    if (!TestFalse(TEXT("Baseline open doorway is clear"), GapBlocked(Door, 0)) ||
        !TestFalse(TEXT("Baseline above thin foundation is clear"), AboveFoundationBlocked(Base, Door))) { return false; }
    TestTrue(TEXT("Baseline frame stays solid"), GapBlocked(Door, 130));
    if (!TestFalse(TEXT("Baseline player capsule fits doorway"), PlayerCapsuleBlocked(Door))) { return false; }
    const FString Slot = TEXT("AutomationShape_") + FGuid::NewGuid().ToString(EGuidFormats::Digits);
    ON_SCOPE_EXIT
    {
        Persistence->ActiveSlot.Reset();
        for (bool B : {false, true}) { IFileManager::Get().Delete(*FPFSaveFileStore::Path(Slot, B)); }
    };
    if (!TestTrue(TEXT("Save actual open-door world"), Persistence->Save(Slot, Error))) { AddError(Error); return false; }
    for (int32 Repeat = 0; Repeat < 2; ++Repeat)
    {
        if (!TestTrue(TEXT("Load actual structure world"), Persistence->Load(Slot, Error))) { AddError(Error); return false; }
        Base = nullptr; Door = nullptr;
        for (TActorIterator<APFBuildPiece> It(W); It; ++It)
        { if (It->PersistentId == BaseId) { Base = *It; } if (It->PersistentId == DoorId) { Door = *It; } }
        if (!TestNotNull(TEXT("Restored foundation"), Base) || !TestNotNull(TEXT("Restored door"), Door)) { return false; }
        TestTrue(TEXT("Saved open state and ownership restored"), Door->bDoorOpen && Door->IsOwnedBy(PC->PlayerState));
        TestFalse(TEXT("Restored open doorway is clear immediately"), GapBlocked(Door, 0));
        TestFalse(TEXT("Player capsule crosses restored open doorway"), PlayerCapsuleBlocked(Door));
        TestFalse(TEXT("Restored hidden cubes do not block above platform"), AboveFoundationBlocked(Base, Door));
        TestTrue(TEXT("Restored door frame stays solid"), GapBlocked(Door, 130));
        TestTrue(TEXT("Owner closes restored door"), Door->ToggleDoor(PC->PlayerState));
        TestTrue(TEXT("Closed restored door blocks doorway"), GapBlocked(Door, 0));
        TestTrue(TEXT("Closed restored door blocks player capsule"), PlayerCapsuleBlocked(Door));
        TestTrue(TEXT("Owner reopens restored door"), Door->ToggleDoor(PC->PlayerState));
        TestFalse(TEXT("Reopened restored doorway becomes clear"), GapBlocked(Door, 0));
        TestFalse(TEXT("Reopened door clears player capsule"), PlayerCapsuleBlocked(Door));
    }
    Fixture.ForwardErrorMessages(this);
    AddInfo(TEXT("[PrimalPersistence] Real structure save/load traces checked: open gap, solid frame/closed panel, no hidden platform blockers and owner toggles"));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFWorldDeadPlayerTest, "PF.Persistence.DeadPlayerRespawn",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFWorldDeadPlayerTest::RunTest(const FString&)
{
    AddExpectedMessage(TEXT("GetSocketInfoByName.*No SkeletalMesh for Component"), EAutomationExpectedMessageFlags::Contains, 0);
    FTestWorldWrapper Fixture; if (!Fixture.CreateTestWorld(EWorldType::Game)) { return false; }
    UWorld* W = Fixture.GetTestWorld();
    W->GetOutermost()->Rename(*(TEXT("/Temp/PFDeadPlayer/") + FGuid::NewGuid().ToString(EGuidFormats::Digits) + TEXT("/L_Automation")),
        nullptr, REN_DontCreateRedirectors | REN_NonTransactional);
    W->SetGameMode(FURL(nullptr, TEXT("/Engine/Maps/Entry?game=/Script/PrimalFrontier.PFSurvivalGameMode"), TRAVEL_Absolute));
    auto* Floor = W->SpawnActor<AActor>(); auto* Box = NewObject<UBoxComponent>(Floor); Floor->SetRootComponent(Box);
    Box->SetBoxExtent(FVector(2000, 2000, 25)); Box->SetCollisionProfileName(TEXT("BlockAll")); Box->RegisterComponent();
    Floor->SetActorLocation(FVector(0, 0, -25));
    auto* Start = W->SpawnActor<APlayerStart>(FVector(-500, 0, 120), FRotator::ZeroRotator);
    W->SpawnActor<APFWorldClock>();
    if (!Fixture.BeginPlayInTestWorld()) { return false; }
    auto* Mode = W->GetAuthGameMode<APFSurvivalGameMode>(); Mode->RespawnDelay = 0.1f;
    auto* PC = W->SpawnActor<APFSurvivalPlayerController>(); Mode->RestartPlayer(PC);
    auto* Pawn = Cast<APFSurvivorCharacter>(PC->GetPawn());
    auto* Persistence = W->GetSubsystem<UPFWorldPersistence>();
    if (!TestNotNull(TEXT("Native survivor"), Pawn) || !TestNotNull(TEXT("Bag"), PC->GetInventory())) { return false; }
    Persistence->Login(PC, TEXT(""));
    auto* Bag = PC->GetInventory(); FString Error;
    if (!TestTrue(TEXT("Original conserved batch"), Bag->Grant(TEXT("Item_Wood"), 5))) { return false; }
    const FGuid BatchId = Bag->GetStacks()[0].StackId;
    const FGuid PlayerId = PC->GetPlayerState<APFInventoryPlayerState>()->PersistentPlayerId;
    const FString Slot = TEXT("AutomationDead_") + FGuid::NewGuid().ToString(EGuidFormats::Digits);
    ON_SCOPE_EXIT
    {
        Persistence->ActiveSlot.Reset(); // Fixture teardown must not recreate its deleted files.
        for (bool B : {false, true}) { IFileManager::Get().Delete(*FPFSaveFileStore::Path(Slot, B)); }
    };
    Pawn->TakeDamage(1000, FDamageEvent(), PC, nullptr);
    TestTrue(TEXT("Actual death before save"), Pawn->Survival->IsDead());
    if (!TestTrue(TEXT("Save dead player to real file"), Persistence->Save(Slot, Error))) { AddError(Error); return false; }
    auto RespawnAndCheck = [&]() -> bool
    {
        const TWeakObjectPtr<APFSurvivorCharacter> Previous(Pawn);
        for (int32 Tick = 0; Tick < 5; ++Tick) { Fixture.TickTestWorld(0.1f); }
        Pawn = Cast<APFSurvivorCharacter>(PC->GetPawn());
        if (!TestNotNull(TEXT("Automatic replacement after dead state"), Pawn)) { return false; }
        TestFalse(TEXT("Dead pawn destroyed exactly once"), Previous.IsValid());
        TestFalse(TEXT("Replacement is alive"), Pawn->Survival->IsDead());
        TestEqual(TEXT("Respawn health resets"), Pawn->Survival->GetVitals().Health, 100.f);
        TestTrue(TEXT("Respawn uses PlayerStart"), FVector2D(Pawn->GetActorLocation()).Equals(FVector2D(Start->GetActorLocation()), 1));
        TestTrue(TEXT("Replacement collision enabled"), Pawn->GetActorEnableCollision());
        TestTrue(TEXT("PlayerState bag survives pawn replacement"), PC->GetInventory() == Bag);
        TestEqual(TEXT("Stable identity through respawn"), PC->GetPlayerState<APFInventoryPlayerState>()->PersistentPlayerId, PlayerId);
        TestEqual(TEXT("Conserved quantity through respawn"), Bag->Count(TEXT("Item_Wood")), 5);
        TestEqual(TEXT("One conserved batch"), Bag->GetStacks().Num(), 1);
        if (!Bag->GetStacks().IsEmpty()) { TestEqual(TEXT("Conserved stack identity"), Bag->GetStacks()[0].StackId, BatchId); }
        int32 Pawns = 0, Pickups = 0;
        for (TActorIterator<APFSurvivorCharacter> It(W); It; ++It) { ++Pawns; }
        for (TActorIterator<APFItemPickup> It(W); It; ++It) { ++Pickups; }
        TestEqual(TEXT("No duplicate survivor"), Pawns, 1);
        TestEqual(TEXT("Death/load does not generate duplicate drops"), Pickups, 0);
        return true;
    };
    if (!RespawnAndCheck()) { return false; }
    for (int32 Repeat = 0; Repeat < 2; ++Repeat)
    {
        Bag->RemoveItem(TEXT("Item_Wood"), 5); Bag->Grant(TEXT("Item_Stone"), 1);
        if (!TestTrue(TEXT("Restore saved dead state"), Persistence->Load(Slot, Error))) { AddError(Error); return false; }
        TestTrue(TEXT("Saved zero health follows death lifecycle"), Pawn->Survival->IsDead());
        TestFalse(TEXT("Load during pending respawn is refused"), Persistence->Load(Slot, Error));
        TestTrue(TEXT("Pending-respawn refusal is explicit"), Error.Contains(TEXT("awaits respawn")));
        TestEqual(TEXT("Refusal preserves restored quantity"), Bag->Count(TEXT("Item_Wood")), 5);
        TestEqual(TEXT("Unsaved item removed once"), Bag->Count(TEXT("Item_Stone")), 0);
        if (!RespawnAndCheck()) { return false; }
    }
    if (!TestTrue(TEXT("Save replacement alive state"), Persistence->Save(Slot, Error)) ||
        !TestTrue(TEXT("Restore alive state after dead generations"), Persistence->Load(Slot, Error))) { AddError(Error); return false; }
    TestFalse(TEXT("Latest generation remains alive"), Pawn->Survival->IsDead());
    TestEqual(TEXT("Latest load still conserves batch quantity"), Bag->Count(TEXT("Item_Wood")), 5);
    Fixture.ForwardErrorMessages(this);
    AddInfo(TEXT("[PrimalPersistence] Real dead-state file load, repeated automatic respawn, safe pending-respawn refusal and identity/quantity conservation checked"));
    return true;
}
#endif
