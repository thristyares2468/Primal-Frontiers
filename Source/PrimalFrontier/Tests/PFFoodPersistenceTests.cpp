#include "Persistence/PFWorldPersistence.h"
#include "Persistence/PFWorldSaveFormat.h"
#include "Persistence/PFSaveFileStore.h"
#include "Inventory/PFItemCatalog.h"
#include "Inventory/PFInventoryComponent.h"
#include "Inventory/PFInventoryPlayerState.h"
#include "Inventory/PFItemPickup.h"
#include "Building/PFBuildingCatalog.h"
#include "Building/PFBuildPiece.h"
#include "Survival/PFSurvivalGameMode.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "World/PFWorldClock.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Misc/DateTime.h"
#include "Misc/ScopeExit.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformProcess.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Components/BoxComponent.h"
#include "GameFramework/PlayerStart.h"
#include "Tests/AutomationCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFFoodPersistenceTest, "PF.Persistence.OfflineFoodAging",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFFoodPersistenceTest::RunTest(const FString&)
{
    AddExpectedMessage(TEXT("GetSocketInfoByName.*No SkeletalMesh for Component"), EAutomationExpectedMessageFlags::Contains, 0);
    FTestWorldWrapper Fixture;
    if (!Fixture.CreateTestWorld(EWorldType::Game)) { return false; }
    UWorld* W = Fixture.GetTestWorld();
    W->GetOutermost()->Rename(*(TEXT("/Temp/PFFoodAging/") + FGuid::NewGuid().ToString(EGuidFormats::Digits) + TEXT("/L_Automation")),
        nullptr, REN_DontCreateRedirectors | REN_NonTransactional);
    W->SetGameMode(FURL(nullptr, TEXT("/Engine/Maps/Entry?game=/Script/PrimalFrontier.PFSurvivalGameMode"), TRAVEL_Absolute));
    auto* Floor = W->SpawnActor<AActor>(); auto* Box = NewObject<UBoxComponent>(Floor); Floor->SetRootComponent(Box);
    Box->SetBoxExtent(FVector(2000, 2000, 25)); Box->SetCollisionProfileName(TEXT("BlockAll")); Box->RegisterComponent();
    Floor->SetActorLocation(FVector(0, 0, -25));
    W->SpawnActor<APlayerStart>(FVector(-500, 0, 120), FRotator::ZeroRotator);
    W->SpawnActor<APFWorldClock>();
    if (!Fixture.BeginPlayInTestWorld()) { return false; }
    auto* Mode = W->GetAuthGameMode<APFSurvivalGameMode>();
    auto* PC = W->SpawnActor<APFSurvivalPlayerController>(); Mode->RestartPlayer(PC);
    auto* Persistence = W->GetSubsystem<UPFWorldPersistence>();
    if (!TestNotNull(TEXT("Player bag"), PC->GetInventory()) || !TestNotNull(TEXT("World persistence"), Persistence)) { return false; }
    Persistence->Login(PC, TEXT(""));
    FString Error;
    const FString Slot = TEXT("AutomationFood_") + FGuid::NewGuid().ToString(EGuidFormats::Digits);
    ON_SCOPE_EXIT
    {
        Persistence->ActiveSlot.Reset(); // Teardown must not recreate the unique fixture's files.
        for (bool B : {false, true}) { IFileManager::Get().Delete(*FPFSaveFileStore::Path(Slot, B)); }
    };
    auto* Buildings = NewObject<UPFBuildingCatalog>();
    auto SpawnPiece = [&](FName Id, FVector At, APFBuildPiece* Support)
    {
        const FTransform T(At); auto* Piece = W->SpawnActorDeferred<APFBuildPiece>(APFBuildPiece::StaticClass(), T);
        Piece->Initialize(*Buildings->Find(Id), PC->PlayerState, Support); Piece->FinishSpawning(T); return Piece;
    };
    auto* Base = SpawnPiece(TEXT("Build_Foundation"), FVector(600, 600, 10), nullptr);
    auto* Chest = SpawnPiece(TEXT("Build_Storage"), FVector(600, 600, 60), Base);
    const FGuid ChestId = Chest->PersistentId;
    const double Now = UPFInventoryComponent::ServerTime(W);
    auto Populate = [&](UPFInventoryComponent* Bag)
    {
        return Bag->AddExisting(TEXT("Item_Food"), 2, Now + 0.25) &&
            Bag->AddExisting(TEXT("Item_CookedFood"), 3, Now + 60) && Bag->Grant(TEXT("Item_Wood"), 4);
    };
    if (!TestTrue(TEXT("Player food fixtures"), Populate(PC->GetInventory())) ||
        !TestTrue(TEXT("Storage food fixtures"), Populate(Chest->Storage))) { return false; }
    auto SpawnPickup = [&](FName Id, double Deadline, FVector At)
    {
        const FTransform T(At); auto* Pickup = W->SpawnActorDeferred<APFItemPickup>(APFItemPickup::StaticClass(), T);
        Pickup->Initialize(Id, 2, Deadline); Pickup->FinishSpawning(T); return Pickup;
    };
    auto* ShortPickup = SpawnPickup(TEXT("Item_Food"), Now + 0.25, FVector(300, 0, 30));
    auto* LongPickup = SpawnPickup(TEXT("Item_CookedFood"), Now + 60, FVector(300, 100, 30));
    auto* PermanentPickup = SpawnPickup(TEXT("Item_Wood"), 0, FVector(300, 200, 30));
    const FGuid ShortId = ShortPickup->GetContents().StackId, LongId = LongPickup->GetContents().StackId;
    const FGuid PermanentId = PermanentPickup->GetContents().StackId;
    const FGuid PlayerFoodId = PC->GetInventory()->GetStacks()[1].StackId, StorageFoodId = Chest->Storage->GetStacks()[1].StackId;
    if (!TestTrue(TEXT("Real food world save"), Persistence->Save(Slot, Error))) { AddError(Error); return false; }
    // No world ticks: only actual wall-clock UTC advances, as while the game is closed.
    // Two seconds safely exceeds the file format's one-second UTC precision.
    FPlatformProcess::Sleep(2.1f);
    double PreviousPlayer = 60, PreviousStorage = 60, PreviousPickup = 60;
    for (int32 Repeat = 0; Repeat < 2; ++Repeat)
    {
        if (Repeat > 0) { FPlatformProcess::Sleep(1.1f); }
        if (!TestTrue(TEXT("Load real aged food file"), Persistence->Load(Slot, Error))) { AddError(Error); return false; }
        auto CheckBag = [&](UPFInventoryComponent* Bag, FGuid Id, double& Previous)
        {
            TestEqual(TEXT("Offline expired food removed"), Bag->Count(TEXT("Item_Food")), 0);
            TestEqual(TEXT("Longer-lived food quantity conserved"), Bag->Count(TEXT("Item_CookedFood")), 3);
            TestEqual(TEXT("Nonperishable quantity conserved"), Bag->Count(TEXT("Item_Wood")), 4);
            TestEqual(TEXT("No appended inventory batches"), Bag->GetStacks().Num(), 2);
            const auto* Food = Bag->GetStacks().FindByPredicate([&](const auto& S) { return S.StackId == Id; });
            if (!TestNotNull(TEXT("Stable surviving food batch"), Food)) { return; }
            const double Left = Food->ExpiresAt - UPFInventoryComponent::ServerTime(W);
            TestTrue(TEXT("UTC age reduces lifetime on every load"), Left > 0 && Left <= Previous - 1);
            Previous = Left;
        };
        CheckBag(PC->GetInventory(), PlayerFoodId, PreviousPlayer);
        int32 PieceCount = 0, ChestCount = 0, PickupCount = 0, LongCount = 0, PermanentCount = 0;
        for (TActorIterator<APFBuildPiece> It(W); It; ++It)
        {
            ++PieceCount;
            if (It->PersistentId == ChestId) { ++ChestCount; CheckBag(It->Storage, StorageFoodId, PreviousStorage); }
        }
        for (TActorIterator<APFItemPickup> It(W); It; ++It)
        {
            ++PickupCount; const auto& S = It->GetContents();
            TestTrue(TEXT("Expired dropped batch is never restored"), S.StackId != ShortId);
            if (S.StackId == LongId)
            {
                ++LongCount; TestEqual(TEXT("Dropped food quantity conserved"), S.Quantity, 2);
                const double Left = S.ExpiresAt - UPFInventoryComponent::ServerTime(W);
                TestTrue(TEXT("Dropped food lifetime never renews"), Left > 0 && Left <= PreviousPickup - 1); PreviousPickup = Left;
            }
            if (S.StackId == PermanentId)
            {
                ++PermanentCount; TestEqual(TEXT("Nonperishable pickup has no expiry"), S.ExpiresAt, 0.);
                TestEqual(TEXT("Nonperishable pickup quantity conserved"), S.Quantity, 2);
            }
        }
        TestEqual(TEXT("Exactly original foundation and storage"), PieceCount, 2);
        TestEqual(TEXT("Exactly one storage with original identity"), ChestCount, 1);
        TestEqual(TEXT("Only two unexpired pickups"), PickupCount, 2);
        TestEqual(TEXT("Exactly one surviving food pickup"), LongCount, 1);
        TestEqual(TEXT("Exactly one permanent pickup"), PermanentCount, 1);
    }
    // A disconnected player's record has its own older capture time, independent
    // of the newer world-file timestamp. Exercise the public login/restore path.
    FPFWorldSaveData Snapshot;
    if (!TestTrue(TEXT("Capture current surviving batches"), Persistence->Capture(Snapshot, Error))) { AddError(Error); return false; }
    FPFPlayerSaveData Departed; auto* Items = NewObject<UPFItemCatalog>();
    if (!TestTrue(TEXT("Decode departure fixture"), FPFWorldSaveFormat::UnpackPlayer(Snapshot.Players[0].Data, *Items, 30, Departed, Error))) { return false; }
    const FGuid ExpiredOnReconnect = FGuid::NewGuid();
    Departed.Inventory.Add({ExpiredOnReconnect, TEXT("Item_Food"), 2, 10});
    Snapshot.Players[0].CapturedUtc = FDateTime::UtcNow().ToUnixTimestamp() - 20;
    if (!TestTrue(TEXT("Encode older departure record"), FPFWorldSaveFormat::PackPlayer(Departed, *Items, 30, Snapshot.Players[0].Data, Error))) { return false; }
    Persistence->ActiveSlot.Reset();
    PC->GetPawn()->Destroy(); PC->Destroy(); // No departure checkpoint may replace the synthetic historical record.
    TArray<uint8> Bytes;
    if (!TestTrue(TEXT("Encode separate capture times"), Persistence->Encode(Snapshot, Bytes, Error)) ||
        !TestTrue(TEXT("Write newer world file"), FPFSaveFileStore::Write(Slot, Bytes, Error)) ||
        !TestTrue(TEXT("Load without connected player"), Persistence->Load(Slot, Error))) { AddError(Error); return false; }
    const FString Options = TEXT("?PFReconnect=") + Snapshot.Players[0].ReconnectCredential.ToString(EGuidFormats::Digits);
    if (!TestTrue(TEXT("Saved identity can reconnect"), Persistence->CheckLogin(Options, Error))) { AddError(Error); return false; }
    PC = W->SpawnActor<APFSurvivalPlayerController>(); Mode->RestartPlayer(PC); Persistence->Login(PC, Options);
    if (!TestTrue(TEXT("Public reconnect restore succeeds"), Persistence->RestorePlayer(PC))) { return false; }
    TestEqual(TEXT("Reconnect keeps player identity"), PC->GetPlayerState<APFInventoryPlayerState>()->PersistentPlayerId, Departed.PlayerId);
    TestEqual(TEXT("Older departure expires food despite fresh world file"), PC->GetInventory()->Count(TEXT("Item_Food")), 0);
    TestEqual(TEXT("Reconnect conserves cooked quantity"), PC->GetInventory()->Count(TEXT("Item_CookedFood")), 3);
    TestEqual(TEXT("Reconnect conserves nonperishable quantity"), PC->GetInventory()->Count(TEXT("Item_Wood")), 4);
    TestEqual(TEXT("Reconnect does not append batches"), PC->GetInventory()->GetStacks().Num(), 2);
    const auto* Reconnected = PC->GetInventory()->GetStacks().FindByPredicate([&](const auto& S) { return S.StackId == PlayerFoodId; });
    if (TestNotNull(TEXT("Reconnect preserves cooked batch"), Reconnected))
    { TestTrue(TEXT("Reconnect uses player capture age"), Reconnected->ExpiresAt - UPFInventoryComponent::ServerTime(W) <= PreviousPlayer - 20); }
    Fixture.ForwardErrorMessages(this);
    AddInfo(TEXT("[PrimalPersistence] Real-file UTC aging checked across player/storage/pickups, repeated loads and older departure reconnect; no world ticks or private credentials logged"));
    return true;
}
#endif
