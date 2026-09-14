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
    TestEqual(TEXT("Unknown item cannot be granted"), ExecuteCommand(TEXT("PF.GiveItem"), {TEXT("Item_Unknown"), TEXT("1")}, nullptr, false).Status(), FString(TEXT("NOT IMPLEMENTED")));
    TestEqual(TEXT("Unknown creature cannot spawn"), ExecuteCommand(TEXT("PF.SpawnCreature"), {TEXT("UnknownCreature")}, nullptr, false).Status(), FString(TEXT("NOT IMPLEMENTED")));
    TestTrue(TEXT("Null gameplay world cannot teleport"), ExecuteCommand(TEXT("PF.Teleport"), {TEXT("0"), TEXT("0"), TEXT("200")}, nullptr, false).HasErrors());
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
