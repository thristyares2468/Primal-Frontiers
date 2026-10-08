// PFCommandTests.cpp
//
// Automation tests for the PF.* developer command layer (PFCommands.cpp).
// History: ccbad56 (PrimalAgentTools foundation), extended in M3 340c538 and
// M6 0c2d935 as inventory and creature commands were added.
//   PF.PrimalAgentTools.CommandArguments: every command in CommandSpecs() is a
//     registered console command with help text and refuses extra arguments;
//     argument validation (quantities, IDs, NaN/inf, time and teleport bounds,
//     path traversal in report names); PF.Help is recorded in the history.
//   PF.PrimalAgentTools.MissingSystemsAreBlocked: commands without a backing system
//     report NOT IMPLEMENTED; mutating commands need an authoritative world.
//   PF.PrimalAgentTools.TeleportAndRuntimeReset: teleport in a real test world,
//     collision/ground checks, PF.ResetTestWorld is honestly NOT IMPLEMENTED, and
//     an ambiguous multiplayer target is refused.
// Run: Session Frontend > Automation, filter "PF.PrimalAgentTools".

#include "PFCommands.h"
#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
using namespace PF::AgentTools;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFArgumentsTest, "PF.PrimalAgentTools.CommandArguments",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)
bool FPFArgumentsTest::RunTest(const FString&)
{
    auto Valid = [&](const TCHAR* Name, TArray<FString> Args)
    {
        const FCommandSpec* S = CommandSpecs().FindByPredicate([&](const FCommandSpec& Entry) { return Entry.Name == Name; });
        return S && ValidateArguments(*S, Args).IsEmpty();
    };
    for (const FCommandSpec& S : CommandSpecs())
    {
        TestNotNull(*S.Name, IConsoleManager::Get().FindConsoleObject(*S.Name));
        TestFalse(TEXT("Nonempty discoverable help"), S.Help.IsEmpty());
        TArray<FString> Excess; for (int32 I = 0; I <= S.MaxArgs; ++I) { Excess.Add(TEXT("bad")); }
        TestFalse(*(S.Name + TEXT(" refuses excess arguments")), Valid(*S.Name, Excess));
    }
    TestTrue(TEXT("Item grant syntax"), Valid(TEXT("PF.GiveItem"), {TEXT("Item_Wood"), TEXT("20")}));
    for (const TCHAR* Quantity : {TEXT("0"), TEXT("-1"), TEXT("1.5"), TEXT("1junk"), TEXT("2147483648"), TEXT("99999999999999999999"), TEXT("")})
    { TestFalse(TEXT("Invalid quantity"), Valid(TEXT("PF.GiveItem"), {TEXT("Item_Wood"), Quantity})); }
    TestFalse(TEXT("Invalid item ID"), Valid(TEXT("PF.GiveItem"), {TEXT("../Wood"), TEXT("1")}));
    TestTrue(TEXT("Creature registry identifier syntax"), Valid(TEXT("PF.SpawnCreature"), {TEXT("BP_TestWolf")}));
    TestFalse(TEXT("Arbitrary creature class path blocked"), Valid(TEXT("PF.SpawnCreature"), {TEXT("/Script/Engine.Actor")}));
    for (const TCHAR* Name : {TEXT("PF.SetHealth"), TEXT("PF.SetHunger"), TEXT("PF.SetThirst")})
    {
        TestTrue(TEXT("Finite value accepted for adapter clamping"), Valid(Name, {TEXT("-10")}));
        for (const TCHAR* Value : {TEXT("NaN"), TEXT("inf"), TEXT("1junk"), TEXT(""), TEXT("--1")}) { TestFalse(TEXT("Invalid attribute value"), Valid(Name, {Value})); }
    }
    TestTrue(TEXT("Time lower bound"), Valid(TEXT("PF.SetTimeOfDay"), {TEXT("0")}));
    TestTrue(TEXT("Time upper bound"), Valid(TEXT("PF.SetTimeOfDay"), {TEXT("23")}));
    for (const TCHAR* Value : {TEXT("-1"), TEXT("24"), TEXT("NaN")}) { TestFalse(TEXT("Invalid time"), Valid(TEXT("PF.SetTimeOfDay"), {Value})); }
    TestTrue(TEXT("Teleport syntax"), Valid(TEXT("PF.Teleport"), {TEXT("500"), TEXT("300"), TEXT("200")}));
    TestFalse(TEXT("Teleport bounds"), Valid(TEXT("PF.Teleport"), {TEXT("100001"), TEXT("0"), TEXT("200")}));
    TestFalse(TEXT("Teleport nonfinite"), Valid(TEXT("PF.Teleport"), {TEXT("NaN"), TEXT("0"), TEXT("200")}));
    TestFalse(TEXT("Teleport fractional player ID"), Valid(TEXT("PF.Teleport"), {TEXT("0"), TEXT("0"), TEXT("200"), TEXT("1.5")}));
    TestFalse(TEXT("Export traversal"), Valid(TEXT("PF.ExportTestReport"), {TEXT("../escape")}));
    TestTrue(TEXT("World slot identifier"), Valid(TEXT("PF.SaveWorld"), {TEXT("Automation_Example")}));
    for (const TCHAR* Slot : {TEXT("../escape"), TEXT("C:/World"), TEXT("bad-name"), TEXT("")})
    { TestFalse(TEXT("Invalid save/load slot"), Valid(TEXT("PF.LoadWorld"), {Slot})); }
    TestTrue(TEXT("Help through console"), IConsoleManager::Get().ProcessUserConsoleInput(TEXT("PF.Help"), *GLog, nullptr));
    TestEqual(TEXT("Help execution recorded"), CommandHistory().Last().Command, FString(TEXT("PF.Help")));
    TestFalse(TEXT("Timestamp recorded"), CommandHistory().Last().StartedUtc.IsEmpty());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFMissingSystemsTest, "PF.PrimalAgentTools.MissingSystemsAreBlocked",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)
bool FPFMissingSystemsTest::RunTest(const FString&)
{
    for (const FCommandSpec& S : CommandSpecs())
    {
        if (S.Blocker.IsEmpty()) { continue; }
        TArray<FString> Args;
        if (S.Name == TEXT("PF.GiveItem")) { Args = {TEXT("Item_Wood"), TEXT("20")}; }
        else if (S.Name == TEXT("PF.SpawnCreature")) { Args = {TEXT("BP_TestWolf")}; }
        else if (S.MinArgs == 1) { Args = {TEXT("10")}; }
        TestEqual(*S.Name, ExecuteCommand(S.Name, Args, nullptr, false).Status(), FString(TEXT("NOT IMPLEMENTED")));
    }
    TestTrue(TEXT("Item grant without authoritative world rejected"), ExecuteCommand(TEXT("PF.GiveItem"), {TEXT("Item_Unknown"), TEXT("1")}, nullptr, false).HasErrors());
    TestTrue(TEXT("Creature spawning requires authoritative world"), ExecuteCommand(TEXT("PF.SpawnCreature"), {TEXT("UnknownCreature")}, nullptr, false).HasErrors());
    TestTrue(TEXT("Null gameplay world cannot teleport"), ExecuteCommand(TEXT("PF.Teleport"), {TEXT("0"), TEXT("0"), TEXT("200")}, nullptr, false).HasErrors());
    for (const TCHAR* Name : {TEXT("PF.SaveWorld"), TEXT("PF.LoadWorld"), TEXT("PF.TestPersistence")})
    { TestTrue(TEXT("Persistence requires authoritative world"), ExecuteCommand(Name, {}, nullptr, false).HasErrors()); }
    AddInfo(TEXT("These assertions verify honest blockers, not item grants, spawning, clamping or persistence. Those integration tests require the missing gameplay APIs."));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFTeleportTest, "PF.PrimalAgentTools.TeleportAndRuntimeReset",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)
bool FPFTeleportTest::RunTest(const FString&)
{
    FTestWorldWrapper Fixture;
    if (!TestTrue(TEXT("Create isolated runtime world"), Fixture.CreateTestWorld(EWorldType::Game))) { return false; }
    UWorld* World = Fixture.GetTestWorld();
    AActor* Floor = World->SpawnActor<AActor>();
    UBoxComponent* Box = NewObject<UBoxComponent>(Floor);
    Floor->SetRootComponent(Box); Box->SetBoxExtent(FVector(2000, 2000, 25));
    Box->SetCollisionProfileName(TEXT("BlockAll")); Box->RegisterComponent();
    Floor->SetActorLocation(FVector(0, 0, -25));
    if (!TestTrue(TEXT("Begin play with project GameMode"), Fixture.BeginPlayInTestWorld())) { return false; }
    APlayerController* PC = World->SpawnActor<APlayerController>();
    ACharacter* Pawn = World->SpawnActor<ACharacter>(FVector(0, 0, 200), FRotator::ZeroRotator);
    if (!PC || !Pawn) { AddError(TEXT("Failed to create controlled character fixture")); return false; }
    PC->Possess(Pawn);
    const auto Go = [&](const TArray<FString>& Args) { return ExecuteCommand(TEXT("PF.Teleport"), Args, World, false); };
    TestFalse(TEXT("Real server teleport succeeds"), Go({TEXT("500"), TEXT("300"), TEXT("200")}).HasErrors());
    TestTrue(TEXT("Resulting position verified"), Pawn->GetActorLocation().Equals(FVector(500, 300, 200), 1));
    TestTrue(TEXT("Collision rejects destination inside floor"), Go({TEXT("500"), TEXT("300"), TEXT("0")}).HasErrors());
    TestTrue(TEXT("Missing ground rejected"), Go({TEXT("5000"), TEXT("300"), TEXT("200")}).HasErrors());
    TestTrue(TEXT("Rejected commands preserve position"), Pawn->GetActorLocation().Equals(FVector(500, 300, 200), 1));
    TestEqual(TEXT("Runtime reset has no fabricated implementation"), ExecuteCommand(TEXT("PF.ResetTestWorld"), {}, World, false).Status(), FString(TEXT("NOT IMPLEMENTED")));
    TestTrue(TEXT("Reset preserved pawn"), IsValid(Pawn));
    APlayerController* Second = World->SpawnActor<APlayerController>();
    TestNotNull(TEXT("Second controller"), Second);
    TestTrue(TEXT("Ambiguous multiplayer target rejected"), Go({TEXT("0"), TEXT("0"), TEXT("200")}).HasErrors());
    const FResult Export = ExecuteCommand(TEXT("PF.ExportTestReport"), {TEXT("CommandVerification")}, World, false);
    TestFalse(TEXT("Execution history exported"), Export.HasErrors());
    Fixture.ForwardErrorMessages(this);
    return true;
}
#endif
