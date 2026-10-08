#include "PFCommands.h"
#include "PFRequestCodes.h"
#include "Persistence/PFWorldPersistence.h"
#include "Persistence/PFSaveFileStore.h"
#include "Inventory/PFInventoryPlayerState.h"
#include "Inventory/PFInventoryComponent.h"
#include "Crafting/PFCraftingComponent.h"
#include "Crafting/PFResourceNode.h"
#include "Building/PFBuildingComponent.h"
#include "Building/PFBuildPiece.h"
#include "Creatures/PFCreature.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "World/PFWorldClock.h"

#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/GameStateBase.h"
#include "HAL/IConsoleManager.h"

/** Opt-in disposable live-world scenario. Create and Restore run in separate server processes. */
class FPFPersistenceLiveExercise : public IAutomationLatentCommand
{
public:
    explicit FPFPersistenceLiveExercise(FAutomationTestBase* InTest) : Test(InTest), Started(FPlatformTime::Seconds())
    {
        FParse::Value(FCommandLine::Get(), TEXT("PFExpectedPlayers="), Expected);
        FParse::Value(FCommandLine::Get(), TEXT("PFSaveSlot="), Slot);
        bRestore = FParse::Param(FCommandLine::Get(), TEXT("PFLoadSave"));
    }
    bool Update() override
    {
        const double Now = FPlatformTime::Seconds();
        if (Stage == 99) { return Now - Changed > (bClient ? 3 : 20); }
        if (Now - Started > 150) { Test->AddError(FString::Printf(TEXT("[PrimalPersistence] Live timeout stage=%d client=%d"), Stage, bClient)); return true; }
        UWorld* W = nullptr;
        for (const auto& Context : GEngine->GetWorldContexts())
        { if (Context.World() && Context.World()->IsGameWorld()) { W = Context.World(); break; } }
        if (!W) { return false; }
        bClient = W->GetNetMode() == NM_Client;
        TArray<APFSurvivalPlayerController*> Players;
        for (auto It = W->GetPlayerControllerIterator(); It; ++It)
        {
            auto* PC = Cast<APFSurvivalPlayerController>(It->Get());
            if (PC && PC->GetPawn() && PC->GetInventory() && PC->GetPlayerState<APFInventoryPlayerState>()) { Players.Add(PC); }
        }
        if (Players.Num() != (bClient ? 1 : Expected)) { return false; }
        Players.Sort([](const auto& A, const auto& B) { return A.PlayerState->GetPlayerId() < B.PlayerState->GetPlayerId(); });
        auto* PC = Players[0]; auto* V = PC->GetPawn()->FindComponentByClass<UPFPlayerSurvivalComponent>();
        auto* Persistence = W->GetSubsystem<UPFWorldPersistence>(); APFWorldClock* Clock = nullptr;
        TArray<APFResourceNode*> Wood, Stone, Food; int32 Buildings = 0, Creatures = 0;
        for (TActorIterator<APFWorldClock> It(W); It; ++It) { Clock = *It; }
        for (TActorIterator<APFResourceNode> It(W); It; ++It)
        {
            if (It->ResourceId == TEXT("Node_Wood")) { Wood.Add(*It); }
            if (It->ResourceId == TEXT("Node_Stone")) { Stone.Add(*It); }
            if (It->ResourceId == TEXT("Node_Food")) { Food.Add(*It); }
        }
        for (TActorIterator<APFBuildPiece> It(W); It; ++It) { ++Buildings; }
        for (TActorIterator<APFCreature> It(W); It; ++It) { ++Creatures; }
        if (!V || !Clock || Wood.Num() < 3 || Stone.Num() < Expected || Food.IsEmpty() || Creatures != 2) { return false; }
        Wood.Sort([](const auto& A, const auto& B) { return A.GetActorLocation().Y < B.GetActorLocation().Y; });
        Stone.Sort([](const auto& A, const auto& B) { return A.GetActorLocation().X < B.GetActorLocation().X; });
        auto Advance = [&](int32 Next) { Stage = Next; Changed = Now; Test->AddInfo(FString::Printf(TEXT("[PrimalPersistence] %s phase=%s stage=%d"), bClient ? TEXT("Client") : TEXT("Server"), bRestore ? TEXT("Restore") : TEXT("Create"), Stage)); };
        auto Check = [&](const TCHAR* Label, bool Value) { return Test->TestTrue(Label, Value); };
        auto Aim = [](APFSurvivalPlayerController* P, FVector At)
        { FVector Eye; FRotator Look; P->GetPawn()->GetActorEyesViewPoint(Eye, Look); P->SetControlRotation((At - Eye).Rotation()); };
        auto Approach = [&](APFSurvivalPlayerController* P, APFResourceNode* Node)
        { P->GetPawn()->SetActorLocation(Node->GetActorLocation() + FVector(-180, 0, 70)); Aim(P, Node->GetActorLocation()); };
        if (bClient)
        {
            if (Stage == 90)
            {
                if (PC->GetInventoryMessage() != TEXT("Refused: invalid stack, quantity, space or life state"))
                {
                    if (Now - Changed > 10)
                    { Test->AddError(TEXT("[PrimalAgentTools] Invalid inventory RPC received no server refusal within 10 seconds")); return true; }
                    return false;
                }
                Test->AddInfo(TEXT("[PrimalAgentTools] Invalid inventory RPC acknowledged by server refusal"));
                Advance(91); return false;
            }
            if (Stage == 91)
            {
                if (Now - Changed < 1) { return false; } // Allow subsequent property delivery before conservation checks.
                const auto& After = PC->GetInventory()->GetStacks();
                Test->TestEqual(TEXT("Rejected network request preserves slot count"), After.Num(), ClientInventoryBefore.Num());
                for (const auto& Before : ClientInventoryBefore)
                {
                    Test->TestTrue(TEXT("Rejected network request preserves batch identity, item, quantity and freshness"),
                        After.ContainsByPredicate([&](const auto& S)
                        { return S.StackId == Before.StackId && S.ItemId == Before.ItemId && S.Quantity == Before.Quantity && S.ExpiresAt == Before.ExpiresAt; }));
                }
                Test->TestEqual(TEXT("Rejected network request preserves crafted tool"), PC->GetInventory()->Count(TEXT("Item_Tool")), 1);
                Test->TestEqual(TEXT("Rejected network request preserves player health"), V->GetVitals().Health, 65.f);
                Test->TestEqual(TEXT("Rejected network request preserves structures"), Buildings, 2);
                int32 OtherPlayers = 0, StoragePieces = 0, OwnedStorage = 0;
                const auto* OwnState = PC->GetPlayerState<APFInventoryPlayerState>();
                for (APlayerState* State : W->GetGameState()->PlayerArray)
                {
                    const auto* Other = Cast<APFInventoryPlayerState>(State);
                    if (!Other || Other == OwnState) { continue; }
                    ++OtherPlayers;
                    const auto* Bag = Other->FindComponentByClass<UPFInventoryComponent>();
                    if (Test->TestTrue(TEXT("Remote player inventory component exists"), Bag != nullptr))
                    { Test->TestEqual(TEXT("Remote private inventory never reaches this client"), Bag->GetStacks().Num(), 0); }
                }
                Test->TestEqual(TEXT("Privacy check includes every other connected player"), OtherPlayers, Expected - 1);
                for (TActorIterator<APFBuildPiece> It(W); It; ++It)
                {
                    if (It->Kind != EPFBuildKind::Storage) { continue; }
                    ++StoragePieces;
                    Test->TestTrue(TEXT("Storage public ownership key is valid"), It->PersistentOwnerId.IsValid());
                    Test->TestTrue(TEXT("Storage owner is one of the connected public identities"),
                        W->GetGameState()->PlayerArray.ContainsByPredicate([&](const auto& State)
                        { const auto* S = Cast<APFInventoryPlayerState>(State); return S && S->PersistentPlayerId == It->PersistentOwnerId; }));
                    const bool bOwnsStorage = It->PersistentOwnerId == OwnState->PersistentPlayerId;
                    Test->TestEqual(TEXT("Storage ownership query matches the public identity"), It->IsOwnedBy(OwnState), bOwnsStorage);
                    if (bOwnsStorage)
                    { ++OwnedStorage; Test->TestEqual(TEXT("Owned storage contents replicate after load/reconnect"), It->Storage->Count(TEXT("Item_Wood")), 1); }
                    else
                    { Test->TestEqual(TEXT("Foreign storage contents never reach this client"), It->Storage->GetStacks().Num(), 0); }
                }
                Test->TestEqual(TEXT("Privacy check inspected restored storage"), StoragePieces, 1);
                if (Expected == 1) { Test->TestEqual(TEXT("One-client prerequisite checks the owner branch"), OwnedStorage, 1); }
                Test->AddInfo(FString::Printf(TEXT("[PrimalAgentTools] Storage privacy role owner=%d foreign=%d otherPlayers=%d"), OwnedStorage, StoragePieces - OwnedStorage, OtherPlayers));
                Test->AddInfo(TEXT("[PrimalAgentTools] Owner/foreign bag and storage privacy assertions inspected after load/reconnect"));
                Test->AddInfo(TEXT("[PrimalAgentTools] Negative-quantity Split crossed real client/server RPC; inventory conservation assertions inspected"));
                PF::AgentTools::ExecuteCommand(TEXT("PF.ExportTestReport"), {TEXT("M8PersistenceClient")}, W);
                Advance(99); return false;
            }
            if (Clock->Hour < 17 || Clock->Hour >= 18 || Buildings != 2 || PC->GetInventory()->Count(TEXT("Item_Tool")) != 1 ||
                PC->GetInventory()->Count(TEXT("Item_Fibre")) != 1 ||
                !FMath::IsNearlyEqual(V->GetVitals().Health, 65.f) || !PC->GetPlayerState<APFInventoryPlayerState>()->PersistentPlayerId.IsValid()) { return false; }
            const auto Before = PC->GetInventory()->Count(TEXT("Item_Wood"));
            FString Error;
            Test->TestFalse(TEXT("Client direct save rejected"), Persistence->Save(Slot, Error));
            Test->TestFalse(TEXT("Client direct load rejected"), Persistence->Load(Slot, Error));
            for (const TCHAR* Name : {TEXT("PF.SaveWorld"), TEXT("PF.LoadWorld"), TEXT("PF.TestPersistence")})
            { Test->TestTrue(TEXT("Client PF persistence command rejected"), PF::AgentTools::ExecuteCommand(Name, {}, W, false).HasErrors()); }
            Test->TestFalse(TEXT("Client cannot create items"), PC->GetInventory()->Grant(TEXT("Item_Wood"), 1));
            Test->TestEqual(TEXT("Client refusals conserve inventory"), PC->GetInventory()->Count(TEXT("Item_Wood")), Before);
            TSet<FGuid> Seen;
            if (!W->GetGameState()) { return false; }
            for (APlayerState* State : W->GetGameState()->PlayerArray)
            {
                const auto* S = Cast<APFInventoryPlayerState>(State);
                if (S)
                {
                    if (!S->PersistentPlayerId.IsValid()) { return false; } // Wait for the property's initial network delivery.
                    Test->TestTrue(TEXT("Independent replicated public identities"), !Seen.Contains(S->PersistentPlayerId)); Seen.Add(S->PersistentPlayerId);
                }
            }
            if (Seen.Num() < Expected) { return false; }
            Test->TestEqual(TEXT("All connected player identities replicated"), Seen.Num(), Expected);
            Test->AddInfo(TEXT("[PrimalPersistence] Owner inventory/vitals, world structures/time and independent public identities replicated; client save/load/grant refused"));
            // No server inventory feedback is generated by the server-driven fixture or direct refusals above.
            // Require an empty baseline so an old generic refusal cannot falsely acknowledge this probe.
            if (!Test->TestTrue(TEXT("RPC probe starts without stale feedback"), PC->GetInventoryMessage().IsEmpty())) { return true; }
            ClientInventoryBefore = PC->GetInventory()->GetStacks();
            const auto* Tool = ClientInventoryBefore.FindByPredicate([](const auto& S) { return S.ItemId == TEXT("Item_Tool"); });
            if (!Test->TestTrue(TEXT("RPC probe uses an existing valid stack"), Tool != nullptr)) { return true; }
            PC->ServerInventoryAction(Tool->StackId, PFInventoryAction::Split, -1);
            Test->AddInfo(TEXT("[PrimalAgentTools] Sent one negative-quantity Split through the owning client's generated ServerInventoryAction RPC"));
            Advance(90); return false;
        }
        for (auto* P : Players)
        {
            auto* Needs = P->GetPawn()->FindComponentByClass<UPFPlayerSurvivalComponent>();
            Needs->HungerDrainPerSecond = 0; Needs->ThirstDrainPerSecond = 0; Needs->StaminaRecoveryPerSecond = 0;
            Needs->ExposureDamagePerSecond = 0;
        }
        Clock->DayLengthSeconds = 86400;
        if (bRestore)
        {
            if (Buildings != 2) { return false; }
            for (auto* P : Players)
            {
                Test->TestEqual(TEXT("Tool survives server restart"), P->GetInventory()->Count(TEXT("Item_Tool")), 1);
                Test->TestEqual(TEXT("Unsaved change captured on disconnect"), P->GetInventory()->Count(TEXT("Item_Fibre")), 1);
                Test->TestEqual(TEXT("Saved health survives reconnect"), P->GetPawn()->FindComponentByClass<UPFPlayerSurvivalComponent>()->GetVitals().Health, 65.f);
            }
            for (TActorIterator<APFBuildPiece> It(W); It; ++It)
            {
                Test->TestTrue(TEXT("Ownership matches reconnecting player"), Players.ContainsByPredicate([&](const auto* P) { return It->IsOwnedBy(P->PlayerState); }));
                if (It->Kind == EPFBuildKind::Storage) { Test->TestEqual(TEXT("Storage persists across process restart"), It->Storage->Count(TEXT("Item_Wood")), 1); }
            }
            FPFWorldSaveData Data; FString Error;
            if (!Check(TEXT("Restored server snapshot validates"), Persistence->Capture(Data, Error))) { Test->AddError(Error); return true; }
            Test->TestEqual(TEXT("Only original player records after restart"), Data.Players.Num(), Expected);
            Test->TestTrue(TEXT("Server PF record validation"), !PF::AgentTools::ExecuteCommand(TEXT("PF.TestPersistence"), {}, W).HasErrors());
            PF::AgentTools::ExecuteCommand(TEXT("PF.ExportTestReport"), {TEXT("M8PersistenceRestartServer")}, W);
            Test->AddInfo(TEXT("[PrimalPersistence] Separate-process restart assertions inspected player records, tools, structures, storage and ownership; see test result. Credentials omitted."));
            Advance(99); return false;
        }
        if (Stage == 0)
        {
            Test->TestEqual(TEXT("Fresh server has no placed structures"), Buildings, 0);
            for (int32 Index = 0; Index < Expected; ++Index) { Approach(Players[Index], Wood[Index]); }
            Advance(1); return false;
        }
        if (Stage == 1 && Now - Changed > 0.7)
        {
            for (int32 Index = 0; Index < Expected; ++Index)
            {
                Aim(Players[Index], Wood[Index]->GetActorLocation());
                if (!Check(TEXT("Gather actual map wood"), Wood[Index]->Gather(Players[Index]->GetPawn()))) { return true; }
            }
            ++Hits; Changed = Now;
            if (Hits == 3) { for (int32 Index = 0; Index < Expected; ++Index) { Approach(Players[Index], Stone[Index]); } Advance(2); }
            return false;
        }
        if (Stage == 2 && Now - Changed > 0.7)
        {
            for (int32 Index = 0; Index < Expected; ++Index)
            {
                Aim(Players[Index], Stone[Index]->GetActorLocation());
                if (!Check(TEXT("Gather actual map stone"), Stone[Index]->Gather(Players[Index]->GetPawn())) ||
                    !Check(TEXT("Craft tool from gathered resources"), Players[Index]->GetCrafting()->Start(TEXT("Recipe_Tool"), Players[Index]->GetPawn()))) { return true; }
            }
            Advance(3); return false;
        }
        if (Stage == 3 && Now - Changed > 6)
        {
            for (auto* P : Players) { if (!Check(TEXT("Timed craft completed"), P->GetInventory()->Count(TEXT("Item_Tool")) == 1)) { return true; } }
            PC->GetPawn()->SetActorLocation(FVector(-1200, 0, 100)); Aim(PC, FVector(-800, 0, 0));
            if (!Check(TEXT("Build foundation using gathered wood"), PC->Building->Place(TEXT("Build_Foundation"), 0) != nullptr)) { return true; }
            Approach(PC, Wood.Last()); Advance(4); return false;
        }
        if (Stage == 4 && Now - Changed > 0.7)
        {
            Aim(PC, Wood.Last()->GetActorLocation());
            if (!Check(TEXT("Gather shelter/storage materials"), Wood.Last()->Gather(PC->GetPawn()))) { return true; }
            PC->GetPawn()->SetActorLocation(FVector(-1200, 0, 100)); Aim(PC, FVector(-800, 0, 20));
            auto* Chest = PC->Building->Place(TEXT("Build_Storage"), 0);
            if (!Check(TEXT("Place real supported storage"), Chest != nullptr)) { return true; }
            Aim(PC, Chest->GetActorLocation());
            const auto* Stack = PC->GetInventory()->GetStacks().FindByPredicate([](const auto& S) { return S.ItemId == TEXT("Item_Wood"); });
            if (!Check(TEXT("Open owned storage"), PC->Building->Interact(Chest)) || !Stack ||
                !Check(TEXT("Store one gathered item"), PC->Building->Transfer(true, Stack->StackId, 1))) { return true; }
            Approach(PC, Food[0]); Advance(5); return false;
        }
        if (Stage == 5 && Now - Changed > 0.7)
        {
            Aim(PC, Food[0]->GetActorLocation());
            if (!Check(TEXT("Gather food for saved freshness"), Food[0]->Gather(PC->GetPawn()))) { return true; }
            for (int32 Index = 0; Index < Expected; ++Index)
            {
                auto* P = Players[Index]; P->GetPawn()->SetActorLocation(FVector(-1200, Index * 400, 100)); P->SetControlRotation(FRotator::ZeroRotator);
                auto* Needs = P->GetPawn()->FindComponentByClass<UPFPlayerSurvivalComponent>(); Needs->SetHealth(65); Needs->SetHunger(60); Needs->SetThirst(70);
            }
            Advance(6); return false;
        }
        if (Stage == 6 && Now - Changed > 1)
        {
            FString Error; Clock->SetHour(17);
            if (!Check(TEXT("PF server world save"), !PF::AgentTools::ExecuteCommand(TEXT("PF.SaveWorld"), {Slot}, W).HasErrors())) { return true; }
            FPFWorldSaveData Original;
            if (!Check(TEXT("Capture saved state"), Persistence->Capture(Original, Error))) { Test->AddError(Error); return true; }
            PC->GetInventory()->Grant(TEXT("Item_Stone"), 1); V->SetHealth(90); Clock->SetHour(9);
            if (!Check(TEXT("PF server world load"), !PF::AgentTools::ExecuteCommand(TEXT("PF.LoadWorld"), {Slot}, W).HasErrors())) { return true; }
            Test->TestEqual(TEXT("Loading removes unsaved item"), PC->GetInventory()->Count(TEXT("Item_Stone")), 0);
            FPFWorldSaveData Restored;
            if (!Check(TEXT("Validate restored world"), Persistence->Capture(Restored, Error))) { Test->AddError(Error); return true; }
            Test->TestEqual(TEXT("No duplicate player records"), Restored.Players.Num(), Original.Players.Num());
            Test->TestEqual(TEXT("No duplicate structures"), Restored.Structures.Num(), Original.Structures.Num());
            for (const auto& S : Original.Structures) { Test->TestTrue(TEXT("Structure stable ID retained"), Restored.Structures.ContainsByPredicate([&](const auto& R) { return R.Id == S.Id && R.Owner == S.Owner; })); }
            Test->TestTrue(TEXT("PF nondestructive codec test"), !PF::AgentTools::ExecuteCommand(TEXT("PF.TestPersistence"), {}, W).HasErrors());
            // Test-only nonperishable marker, deliberately after the last manual save.
            // The restart phase must find it through the real disconnect checkpoint.
            for (auto* P : Players) { Test->TestTrue(TEXT("Post-save disconnect marker"), P->GetInventory()->Grant(TEXT("Item_Fibre"), 1)); }
            Test->TestTrue(TEXT("PF.Help runs"), IConsoleManager::Get().ProcessUserConsoleInput(TEXT("PF.Help"), *GLog, W));
            PF::AgentTools::ExecuteCommand(TEXT("PF.ExportTestReport"), {TEXT("M8PersistenceCreateServer")}, W);
            Advance(99); return false;
        }
        return false;
    }
private:
    FAutomationTestBase* Test;
    double Started, Changed = 0;
    int32 Expected = 1, Stage = 0, Hits = 0;
    FString Slot;
    TArray<FPFItemStack> ClientInventoryBefore;
    bool bClient = false, bRestore = false;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFPersistenceLiveTest, "PF.Persistence.Live",
    EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)
bool FPFPersistenceLiveTest::RunTest(const FString&)
{
    FString Slot; int32 Expected = 1; FParse::Value(FCommandLine::Get(), TEXT("PFExpectedPlayers="), Expected);
    if (!FParse::Param(FCommandLine::Get(), TEXT("PFRunPersistenceLiveTests")) ||
        !FParse::Value(FCommandLine::Get(), TEXT("PFSaveSlot="), Slot) || !Slot.StartsWith(TEXT("AutomationM8")) ||
        !FPFSaveFileStore::ValidSlot(Slot) || (Expected != 1 && Expected != 2))
    { AddError(TEXT("Requires isolated map -PFRunPersistenceLiveTests -PFSaveSlot=AutomationM8<unique> -PFExpectedPlayers=1|2")); return false; }
    ADD_LATENT_AUTOMATION_COMMAND(FPFPersistenceLiveExercise(this)); return true;
}
#endif
