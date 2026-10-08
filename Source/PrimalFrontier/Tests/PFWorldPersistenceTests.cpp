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
#include <limits>
#include "HAL/FileManager.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Components/BoxComponent.h"
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
        for (TActorIterator<APFCreature> It(W); It; ++It) { ++CreatureCount; TestEqual(TEXT("Creature ID preserved"), It->PersistentId, CreatureId); }
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
    Fixture.ForwardErrorMessages(this);
    AddInfo(TEXT("[PrimalPersistence] Real file/world restore twice: player, storage, ownership, depletion, clock, pickup and creature IDs; no append duplication"));
    return true;
}
#endif
