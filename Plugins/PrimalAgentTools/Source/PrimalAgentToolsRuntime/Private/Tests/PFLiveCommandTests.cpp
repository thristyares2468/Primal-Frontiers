// PFLiveCommandTests.cpp
//
// Live network test: PF.LiveDeveloperCommands   (added in 78e4f30, after M1)
// Runs the PF.* developer commands in a live dedicated server + 2 client session
// (or standalone). Sequence: wait 30 s for players, exercise, wait 20 s, snapshot.
//   Client: PF.Teleport/GiveItem/SaveWorld/LoadWorld/ResetTestWorld are refused
//     with NotAuthority and the pawn does not move.
//   Server: teleports every real character by PlayerId and checks the result,
//     runs every other command once as a smoke test, and rejects a negative
//     quantity safely.
//   Snapshot: logs every character's replicated role/location and exports a
//     report; the dedicated server lingers 30 s for late clients.
// See Plugins/PrimalAgentTools/DEVELOPER_COMMANDS.md.
// Launch with: -PFRunLiveTests (never in a normal gameplay session).

#include "PFCommands.h"
#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Tests/AutomationCommon.h"
using namespace PF::AgentTools;

static UWorld* LiveWorld()
{
    for (const FWorldContext& C : GEngine->GetWorldContexts())
    { if (C.World() && C.World()->IsGameWorld()) { return C.World(); } }
    return nullptr;
}
DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FPFLiveExercise, FAutomationTestBase*, Test);
bool FPFLiveExercise::Update()
{
    UWorld* World = LiveWorld();
    if (!World) { Test->AddError(TEXT("No live gameplay world.")); return true; }
    ExecuteCommand(TEXT("PF.Help"), {}, World);
    if (World->GetNetMode() == NM_Client)
    {
        APlayerController* PC = World->GetFirstPlayerController();
        APawn* Pawn = PC ? PC->GetPawn() : nullptr;
        if (!Pawn) { Test->AddError(TEXT("Client has no possessed pawn.")); return true; }
        const FVector Before = Pawn->GetActorLocation();
        for (const TCHAR* Name : {TEXT("PF.Teleport"), TEXT("PF.GiveItem"), TEXT("PF.SaveWorld"), TEXT("PF.LoadWorld"), TEXT("PF.ResetTestWorld")})
        {
            TArray<FString> Args;
            if (FString(Name) == TEXT("PF.Teleport")) { Args = {TEXT("500"), TEXT("300"), TEXT("200")}; }
            if (FString(Name) == TEXT("PF.GiveItem")) { Args = {TEXT("Item_Wood"), TEXT("20")}; }
            const FResult R = ExecuteCommand(Name, Args, World, false);
            Test->TestTrue(TEXT("Client mutation explicitly rejected"), R.Issues.ContainsByPredicate([](const FIssue& I) { return I.Code == TEXT("NotAuthority"); }));
        }
        Test->TestTrue(TEXT("Client rejection preserves immediate pawn position"), Pawn->GetActorLocation().Equals(Before, 0.01));
    }
    else
    {
        const bool bNetwork = World->GetNetMode() != NM_Standalone;
        if (bNetwork && World->GetNumPlayerControllers() != 2) { Test->AddError(TEXT("Network verification requires exactly two connected players.")); return true; }
        int32 Verified = 0;
        for (auto It = World->GetPlayerControllerIterator(); It; ++It)
        {
            APlayerController* PC = It->Get();
            ACharacter* Pawn = PC ? Cast<ACharacter>(PC->GetPawn()) : nullptr;
            if (!Pawn || !PC->PlayerState) { Test->AddError(TEXT("Server player lacks character or PlayerState.")); continue; }
            const FVector Before = Pawn->GetActorLocation();
            const FVector Destination = Before + FVector(150, 0, 100);
            const FResult R = ExecuteCommand(TEXT("PF.Teleport"), {FString::SanitizeFloat(Destination.X), FString::SanitizeFloat(Destination.Y), FString::SanitizeFloat(Destination.Z), FString::FromInt(PC->PlayerState->GetPlayerId())}, World);
            Test->TestFalse(TEXT("Existing first-person pawn teleports on server"), R.HasErrors());
            Test->TestTrue(TEXT("Server resulting position matches destination"), Pawn->GetActorLocation().Equals(Destination, 1));
            Verified += !R.HasErrors();
        }
        Test->TestTrue(TEXT("At least one real project character tested"), Verified > 0);
        for (const FCommandSpec& S : CommandSpecs())
        {
            if (S.Name == TEXT("PF.Help") || S.Name == TEXT("PF.Teleport")) { continue; }
            TArray<FString> Args;
            if (S.Name == TEXT("PF.GiveItem")) { Args = {TEXT("Item_Wood"), TEXT("20")}; }
            else if (S.Name == TEXT("PF.SpawnCreature")) { Args = {TEXT("BP_TestWolf")}; }
            else if (S.MinArgs == 1) { Args = {TEXT("10")}; }
            ExecuteCommand(S.Name, Args, World);
        }
        Test->TestTrue(TEXT("Malformed command rejected safely"), ExecuteCommand(TEXT("PF.GiveItem"), {TEXT("Item_Wood"), TEXT("-1")}, World, false).HasErrors());
    }
    return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FPFLiveSnapshot, FAutomationTestBase*, Test);
bool FPFLiveSnapshot::Update()
{
    UWorld* World = LiveWorld();
    if (!World) { Test->AddError(TEXT("Lost live gameplay world.")); return true; }
    for (TActorIterator<ACharacter> It(World); It; ++It)
    {
        if (const APlayerState* PS = It->GetPlayerState())
        {
            UE_LOG(LogPrimalAgentTools, Display, TEXT("[PrimalAgentTools] ReplicationSnapshot PlayerId=%d Role=%d Location=%s"), PS->GetPlayerId(), static_cast<int32>(It->GetLocalRole()), *It->GetActorLocation().ToString());
        }
    }
    ExecuteCommand(TEXT("PF.ExportTestReport"), {World->GetNetMode() == NM_Client ? TEXT("LiveClient") : TEXT("LiveServer")}, World);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFLiveTest, "PF.LiveDeveloperCommands",
    EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)
bool FPFLiveTest::RunTest(const FString&)
{
    if (!FParse::Param(FCommandLine::Get(), TEXT("PFRunLiveTests")))
    { AddError(TEXT("Requires isolated Development -game/-server with -PFRunLiveTests. Never run in a user gameplay session.")); return false; }
    ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(30));
    ADD_LATENT_AUTOMATION_COMMAND(FPFLiveExercise(this));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(20));
    ADD_LATENT_AUTOMATION_COMMAND(FPFLiveSnapshot(this));
    // Keep the server alive while later-starting clients take their snapshots.
    if (LiveWorld() && LiveWorld()->GetNetMode() == NM_DedicatedServer) { ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(30)); }
    return true;
}
#endif
