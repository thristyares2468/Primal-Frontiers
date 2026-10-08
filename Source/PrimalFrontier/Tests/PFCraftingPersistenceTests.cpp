#include "Persistence/PFWorldPersistence.h"
#include "Persistence/PFSaveFileStore.h"
#include "Inventory/PFInventoryComponent.h"
#include "Inventory/PFInventoryPlayerState.h"
#include "Crafting/PFCraftingComponent.h"
#include "Survival/PFSurvivalGameMode.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFSurvivorCharacter.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "World/PFWorldClock.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "HAL/FileManager.h"
#include "Engine/World.h"
#include "Components/BoxComponent.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Tests/AutomationCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFCraftingPersistenceTest, "PF.Persistence.ActiveCraftCancellation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFCraftingPersistenceTest::RunTest(const FString&)
{
    AddExpectedMessage(TEXT("GetSocketInfoByName.*No SkeletalMesh for Component"), EAutomationExpectedMessageFlags::Contains, 0);
    FTestWorldWrapper Fixture; if (!Fixture.CreateTestWorld(EWorldType::Game)) { return false; }
    UWorld* W = Fixture.GetTestWorld();
    W->GetOutermost()->Rename(*(TEXT("/Temp/PFCraftSave/") + FGuid::NewGuid().ToString(EGuidFormats::Digits) + TEXT("/L_Automation")),
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
    if (!TestNotNull(TEXT("Bag"), PC->GetInventory()) || !TestNotNull(TEXT("Crafting"), PC->GetCrafting())) { return false; }
    Persistence->Login(PC, TEXT(""));
    bool bRestored = true;
    TestFalse(TEXT("Refused null setup is not ready"), Persistence->RestorePlayer(nullptr, &bRestored));
    TestFalse(TEXT("Refused setup clears stale restored outcome"), bRestored);
    TestTrue(TEXT("Fresh registered survivor is ready"), Persistence->RestorePlayer(PC, &bRestored));
    TestFalse(TEXT("Fresh survivor was not restored"), bRestored);
    auto* Pawn = CastChecked<APFSurvivorCharacter>(PC->GetPawn()); Pawn->GetCharacterMovement()->DisableMovement();
    // Keep the fixture alive while time advances; use the existing recipe/catalog.
    Pawn->Survival->HungerDrainPerSecond = 0; Pawn->Survival->ThirstDrainPerSecond = 0;
    auto* Bag = PC->GetInventory(); auto* Craft = PC->GetCrafting(); FString Error;
    if (!TestTrue(TEXT("Exactly one recipe's wood"), Bag->Grant(TEXT("Item_Wood"), 3)) ||
        !TestTrue(TEXT("Exactly one recipe's stone"), Bag->Grant(TEXT("Item_Stone"), 2))) { return false; }
    const auto Original = Bag->GetStacks();
    const FString Slot = TEXT("AutomationCraft_") + FGuid::NewGuid().ToString(EGuidFormats::Digits);
    ON_SCOPE_EXIT
    {
        Persistence->ActiveSlot.Reset();
        for (bool B : {false, true}) { IFileManager::Get().Delete(*FPFSaveFileStore::Path(Slot, B)); }
    };
    auto Advance = [&]() { for (int32 Tick = 0; Tick < 70; ++Tick) { Fixture.TickTestWorld(0.1f); } };
    auto CheckIngredients = [&]()
    {
        TestEqual(TEXT("Wood quantity conserved"), Bag->Count(TEXT("Item_Wood")), 3);
        TestEqual(TEXT("Stone quantity conserved"), Bag->Count(TEXT("Item_Stone")), 2);
        TestEqual(TEXT("No output or duplicate ingredient batch"), Bag->GetStacks().Num(), Original.Num());
        TestEqual(TEXT("Cancelled job creates no tool"), Bag->Count(TEXT("Item_Tool")), 0);
        for (const auto& Before : Original)
        {
            TestTrue(TEXT("Ingredient identity/quantity/deadline unchanged"), Bag->GetStacks().ContainsByPredicate([&](const auto& S)
            { return S.StackId == Before.StackId && S.ItemId == Before.ItemId && S.Quantity == Before.Quantity && S.ExpiresAt == Before.ExpiresAt; }));
        }
    };
    for (int32 Repeat = 0; Repeat < 2; ++Repeat)
    {
        if (!TestTrue(TEXT("Start real timed craft before save"), Craft->Start(TEXT("Recipe_Tool"), Pawn)) ||
            !TestTrue(TEXT("Save while craft is active"), Persistence->Save(Slot, Error))) { AddError(Error); return false; }
        TestEqual(TEXT("Save alone does not cancel live job"), Craft->ActiveRecipe, FName(TEXT("Recipe_Tool")));
        if (Repeat == 0)
        {
            TestTrue(TEXT("Saving a fresh survivor leaves setup ready"), Persistence->RestorePlayer(PC, &bRestored));
            TestFalse(TEXT("Saving alone is not restoration"), bRestored);
        }
        if (!TestTrue(TEXT("Restore active-craft file"), Persistence->Load(Slot, Error))) { AddError(Error); return false; }
        TestTrue(TEXT("Loaded survivor setup remains ready"), Persistence->RestorePlayer(PC, &bRestored));
        TestTrue(TEXT("Successful real-file load is restoration"), bRestored);
        TestTrue(TEXT("Restore clears active recipe"), Craft->ActiveRecipe.IsNone());
        TestEqual(TEXT("Restore clears completion deadline"), Craft->FinishAt, 0.);
        CheckIngredients(); Advance(); CheckIngredients();
        if (!TestTrue(TEXT("New post-load craft allowed"), Craft->Start(TEXT("Recipe_Tool"), Pawn))) { return false; }
        Advance();
        TestEqual(TEXT("Exactly one fresh tool"), Bag->Count(TEXT("Item_Tool")), 1);
        TestEqual(TEXT("Fresh craft consumes wood once"), Bag->Count(TEXT("Item_Wood")), 0);
        TestEqual(TEXT("Fresh craft consumes stone once"), Bag->Count(TEXT("Item_Stone")), 0);
        Advance(); TestEqual(TEXT("No repeated completion"), Bag->Count(TEXT("Item_Tool")), 1);
        if (!TestTrue(TEXT("Reload replaces completed output with saved ingredients"), Persistence->Load(Slot, Error))) { AddError(Error); return false; }
        CheckIngredients();
    }
    // A departing player must not carry a stale craft job into its replacement PlayerState.
    FPFWorldSaveData Snapshot;
    if (!TestTrue(TEXT("Capture private reconnect record"), Persistence->Capture(Snapshot, Error)) ||
        !TestTrue(TEXT("Start job before actual controller teardown"), Craft->Start(TEXT("Recipe_Tool"), Pawn))) { return false; }
    const FString Options = TEXT("?PFReconnect=") + Snapshot.Players[0].ReconnectCredential.ToString(EGuidFormats::Digits);
    FPFSavedFile BeforeDeparture, AfterDeparture;
    if (!TestTrue(TEXT("Read generation before departure"), FPFSaveFileStore::Read(Slot, BeforeDeparture, Error))) { AddError(Error); return false; }
    PC->Destroy(); Pawn->Destroy(); // Real teardown captures inventory before possession disappears.
    if (!TestTrue(TEXT("Departure publishes real checkpoint"), FPFSaveFileStore::Read(Slot, AfterDeparture, Error))) { AddError(Error); return false; }
    TestTrue(TEXT("Departure advances checkpoint generation"), AfterDeparture.Generation > BeforeDeparture.Generation);
    if (!TestTrue(TEXT("Reload departure checkpoint without a controller"), Persistence->Load(Slot, Error))) { AddError(Error); return false; }
    if (!TestTrue(TEXT("Departed identity can reconnect"), Persistence->CheckLogin(Options, Error))) { AddError(Error); return false; }
    PC = W->SpawnActor<APFSurvivalPlayerController>(); Mode->RestartPlayer(PC); Persistence->Login(PC, Options);
    if (!TestTrue(TEXT("Reconnect restores ingredients"), Persistence->RestorePlayer(PC, &bRestored))) { return false; }
    TestTrue(TEXT("New login reports actual saved survivor restoration"), bRestored);
    Pawn = CastChecked<APFSurvivorCharacter>(PC->GetPawn()); Pawn->GetCharacterMovement()->DisableMovement();
    Bag = PC->GetInventory(); Craft = PC->GetCrafting();
    TestTrue(TEXT("Reconnect job idle"), Craft->ActiveRecipe.IsNone());
    CheckIngredients(); Advance(); CheckIngredients();
    Fixture.ForwardErrorMessages(this);
    AddInfo(TEXT("[PrimalPersistence] Active-craft real-file save/load, cancellation, ingredient identity, fresh single completion and controller departure/reconnect conservation checked"));
    return true;
}
#endif
