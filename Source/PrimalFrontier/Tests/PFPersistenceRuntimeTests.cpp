#include "Persistence/PFPlayerSaveAdapter.h"
#include "Persistence/PFSaveFileStore.h"
#include "Inventory/PFInventoryComponent.h"
#include "Inventory/PFInventoryPlayerState.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFSurvivalGameMode.h"
#include "Survival/PFSurvivorCharacter.h"
#include "Survival/PFPlayerSurvivalComponent.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerStart.h"
#include "Tests/AutomationCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFPersistenceAdapterTest, "PF.Persistence.ServerPlayerAdapter",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFPersistenceAdapterTest::RunTest(const FString&)
{
    // Native fixture deliberately has no Blueprint presentation meshes (as in Lifecycle).
    AddExpectedMessage(TEXT("GetSocketInfoByName.*No SkeletalMesh for Component"),
        EAutomationExpectedMessageFlags::Contains, 0);
    FTestWorldWrapper Fixture;
    if (!Fixture.CreateTestWorld(EWorldType::Game)) { return false; }
    UWorld* World = Fixture.GetTestWorld();
    World->SetGameMode(FURL(nullptr, TEXT("/Engine/Maps/Entry?game=/Script/PrimalFrontier.PFSurvivalGameMode"), TRAVEL_Absolute));
    auto* Floor = World->SpawnActor<AActor>();
    auto* Box = NewObject<UBoxComponent>(Floor); Floor->SetRootComponent(Box);
    Box->SetBoxExtent(FVector(2000, 2000, 25)); Box->SetCollisionProfileName(TEXT("BlockAll")); Box->RegisterComponent();
    Floor->SetActorLocation(FVector(0, 0, -25));
    World->SpawnActor<APlayerStart>(FVector(100, 200, 120), FRotator::ZeroRotator);
    if (!Fixture.BeginPlayInTestWorld()) { return false; }
    auto* PC = World->SpawnActor<APFSurvivalPlayerController>();
    World->GetAuthGameMode<APFSurvivalGameMode>()->RestartPlayer(PC);
    auto* Pawn = Cast<APFSurvivorCharacter>(PC->GetPawn());
    if (!TestNotNull(TEXT("Survivor"), Pawn) || !TestNotNull(TEXT("Inventory"), PC->GetInventory())) { return false; }
    auto* Bag = PC->GetInventory();
    TestTrue(TEXT("Grant wood"), Bag->Grant(TEXT("Item_Wood"), 5));
    TestTrue(TEXT("Finite food batch"), Bag->AddExisting(TEXT("Item_Food"), 2, UPFInventoryComponent::ServerTime(World) + 10));
    Pawn->Survival->SetHealth(65); Pawn->Survival->ChangeStamina(-40);
    Pawn->Survival->SetHunger(27); Pawn->Survival->SetThirst(33);
    FPFPlayerSaveData Saved; FString Error;
    if (!TestTrue(TEXT("Real server capture"), FPFPlayerSaveAdapter::Capture(PC, Saved, Error))) { AddError(Error); return false; }
    TestTrue(TEXT("Server-issued ID"), Saved.PlayerId.IsValid());
    const FGuid FoodId = Saved.Inventory.Last().StackId;
    Bag->RemoveItem(TEXT("Item_Wood"), 5); Bag->Grant(TEXT("Item_Stone"), 3); Pawn->Survival->SetHealth(90);
    if (!TestTrue(TEXT("Atomic server restore"), FPFPlayerSaveAdapter::Restore(PC, Saved, 5, Error))) { AddError(Error); return false; }
    TestEqual(TEXT("Wood restored"), Bag->Count(TEXT("Item_Wood")), 5);
    TestEqual(TEXT("Replacement removes unsaved stone"), Bag->Count(TEXT("Item_Stone")), 0);
    TestEqual(TEXT("Health restored"), Pawn->Survival->GetVitals().Health, 65.f);
    TestEqual(TEXT("Stamina restored"), Pawn->Survival->GetVitals().Stamina, 60.f);
    TestEqual(TEXT("Hunger restored"), Pawn->Survival->GetVitals().Hunger, 27.f);
    TestEqual(TEXT("Thirst restored"), Pawn->Survival->GetVitals().Thirst, 33.f);
    TestEqual(TEXT("Food keeps stack ID"), Bag->GetStacks().Last().StackId, FoodId);
    TestTrue(TEXT("Offline age reduces lifetime without renewal"),
        FMath::IsNearlyEqual(Bag->GetStacks().Last().ExpiresAt - UPFInventoryComponent::ServerTime(World), 5., 0.001));
    TestTrue(TEXT("Repeated load replaces rather than appends"), FPFPlayerSaveAdapter::Restore(PC, Saved, 5, Error));
    TestEqual(TEXT("No repeated-load duplication"), Bag->Count(TEXT("Item_Wood")), 5);
    auto Invalid = Saved; Invalid.Inventory[0].Quantity = -1;
    TestFalse(TEXT("Invalid restore refused"), FPFPlayerSaveAdapter::Restore(PC, Invalid, 0, Error));
    TestEqual(TEXT("Refusal preserves bag"), Bag->Count(TEXT("Item_Wood")), 5);
    Invalid = Saved; Invalid.PlayerId = FGuid::NewGuid();
    TestFalse(TEXT("Other identity refused"), FPFPlayerSaveAdapter::Restore(PC, Invalid, 0, Error));
    Invalid = Saved; Invalid.Location.Z = 0;
    TestFalse(TEXT("Inside ground refused"), FPFPlayerSaveAdapter::Restore(PC, Invalid, 0, Error));
    PC->SetRole(ROLE_AutonomousProxy);
    TestFalse(TEXT("Client capture refused"), FPFPlayerSaveAdapter::Capture(PC, Invalid, Error));
    TestFalse(TEXT("Client restore refused"), FPFPlayerSaveAdapter::Restore(PC, Saved, 0, Error));
    PC->SetRole(ROLE_Authority);
    PC->PlayerState->SetRole(ROLE_AutonomousProxy);
    TestFalse(TEXT("Client inventory injection refused"), Bag->RestorePersistence(Saved.Inventory, 0, Error));
    PC->PlayerState->SetRole(ROLE_Authority);
    TestTrue(TEXT("Restore after food expires offline"), FPFPlayerSaveAdapter::Restore(PC, Saved, 11, Error));
    TestEqual(TEXT("Expired food stays gone"), Bag->Count(TEXT("Item_Food")), 0);
    Fixture.ForwardErrorMessages(this);
    AddInfo(TEXT("[PrimalPersistence] Server capture/replacement, stable batches, offline spoilage, collision and authority checked"));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFPersistenceFileTest, "PF.Persistence.FileGenerations",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)
bool FPFPersistenceFileTest::RunTest(const FString&)
{
    const FString Slot = TEXT("Automation_") + FGuid::NewGuid().ToString(EGuidFormats::Digits);
    FString Error; FPFSavedFile Read;
    const TArray<uint8> First = {1, 2, 3}, Second = {4, 5, 6}, Third = {7, 8, 9};
    auto Cleanup = [&]
    {
        for (bool B : {false, true})
        {
            IFileManager::Get().Delete(*FPFSaveFileStore::Path(Slot, B));
            IFileManager::Get().Delete(*(FPFSaveFileStore::Path(Slot, B) + TEXT(".pending")));
        }
    };
    TestFalse(TEXT("Missing slot safe"), FPFSaveFileStore::Read(Slot, Read, Error));
    if (!TestTrue(TEXT("First safe write"), FPFSaveFileStore::Write(Slot, First, Error))) { AddError(Error); Cleanup(); return false; }
    TestTrue(TEXT("First read"), FPFSaveFileStore::Read(Slot, Read, Error));
    TestEqual(TEXT("First generation"), Read.Generation, uint64(1));
    TestTrue(TEXT("Second write"), FPFSaveFileStore::Write(Slot, Second, Error));
    TestTrue(TEXT("Second read"), FPFSaveFileStore::Read(Slot, Read, Error));
    TestTrue(TEXT("Second payload"), Read.Payload == Second);
    TestEqual(TEXT("Second generation"), Read.Generation, uint64(2));
    TestTrue(TEXT("Third write"), FPFSaveFileStore::Write(Slot, Third, Error));
    TestTrue(TEXT("Third read"), FPFSaveFileStore::Read(Slot, Read, Error));
    TestEqual(TEXT("Third generation"), Read.Generation, uint64(3));
    TestFalse(TEXT("Traversal slot rejected"), FPFSaveFileStore::Write(TEXT("../unsafe"), First, Error));
    TestFalse(TEXT("Empty replacement refused"), FPFSaveFileStore::Write(Slot, {}, Error));
    TestTrue(TEXT("Failed write leaves active save"), FPFSaveFileStore::Read(Slot, Read, Error));
    TestTrue(TEXT("Active payload unchanged"), Read.Payload == Third);
    // Corrupt only this test's newest generation through the supported file API.
    FFileHelper::SaveArrayToFile(TArray<uint8>({0, 0, 0}), *FPFSaveFileStore::Path(Slot, false));
    TestTrue(TEXT("Valid previous generation recovers"), FPFSaveFileStore::Read(Slot, Read, Error));
    TestTrue(TEXT("Backup recovery explicit"), Read.bRecoveredBackup);
    TestTrue(TEXT("Backup payload"), Read.Payload == Second);
    TestFalse(TEXT("Saving never overwrites corrupt evidence"), FPFSaveFileStore::Write(Slot, First, Error));
    FFileHelper::SaveArrayToFile(TArray<uint8>({0}), *FPFSaveFileStore::Path(Slot, true));
    TestFalse(TEXT("Both corrupt safely refused"), FPFSaveFileStore::Read(Slot, Read, Error));
    TestTrue(TEXT("Failed read preserves previous output"), Read.Payload == Second);
    Cleanup();
    AddInfo(TEXT("[PrimalPersistence] Verified A/B generation writes, invalid slot/refusal preservation and explicit backup recovery"));
    return true;
}
#endif
