#include "PFCommands.h"
#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Survival/PFSurvivorCharacter.h"
#include "Survival/PFSurvivalGameMode.h"
#include "Survival/PFPlayerSurvivalComponent.h"

// Opt-in integration test: actual possession and socket replication, no simulated roles.
class FPFSurvivalLiveExercise final : public IAutomationLatentCommand
{
public:
    explicit FPFSurvivalLiveExercise(FAutomationTestBase* InTest) : Test(InTest), Started(FPlatformTime::Seconds()) {}
    bool Update() override
    {
        UWorld* World = nullptr;
        for (const FWorldContext& Context : GEngine->GetWorldContexts())
        { if (Context.World() && Context.World()->IsGameWorld()) { World = Context.World(); break; } }
        const double Now = FPlatformTime::Seconds();
        if (Now - Started > 150) { Test->AddError(TEXT("[PrimalSurvival] Live test timed out waiting for possession/replicated lifecycle.")); return true; }
        APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
        APFSurvivorCharacter* Pawn = PC ? Cast<APFSurvivorCharacter>(PC->GetPawn()) : nullptr;
        if (!Pawn) { return false; }
        const bool Client = World->GetNetMode() == NM_Client;
        const FPFPlayerVitals V = Pawn->Survival->GetVitals();
        if (Stage == 0)
        {
            if (V.Health != 100 || V.Stamina != 100) { return false; }
            Test->TestEqual(TEXT("Real initial health"), V.Health, 100.f);
            Test->TestEqual(TEXT("Real initial stamina"), V.Stamina, 100.f);
            Test->TestEqual(TEXT("Authority matches network role"), Pawn->HasAuthority(), !Client);
            PF::AgentTools::ExecuteCommand(TEXT("PF.Help"), {}, World);
            OriginalPawn = Pawn;
            if (Client)
            {
                for (const TCHAR* Name : {TEXT("PF.SetHealth"), TEXT("PF.SetStamina"), TEXT("PF.Damage"), TEXT("PF.Kill"), TEXT("PF.Respawn")})
                {
                    const bool HasArgument = FString(Name) != TEXT("PF.Kill") && FString(Name) != TEXT("PF.Respawn");
                    const auto R = PF::AgentTools::ExecuteCommand(Name, HasArgument ? TArray<FString>{TEXT("25")} : TArray<FString>{}, World, false);
                    Test->TestTrue(TEXT("Client command rejected by authority guard"), R.Issues.ContainsByPredicate([](const PF::AgentTools::FIssue& I) { return I.Code == TEXT("NotAuthority"); }));
                }
                Test->TestFalse(TEXT("Client component rejects damage"), Pawn->Survival->ApplyDamage(50));
                Test->TestFalse(TEXT("Client component rejects stamina mutation"), Pawn->Survival->ChangeStamina(-50));
            }
            Stage = 1; Changed = Now;
            Test->AddInfo(TEXT("[PrimalSurvival] Live initial values and authority checked."));
        }
        else if (Stage == 1 && (Client ? V.Health == 75 && V.Stamina == 20 : Now - Changed > 20))
        {
            if (!Client)
            {
                Pawn->Survival->StaminaRecoveryPerSecond = 0; // Hold a stable replication sample in this isolated test.
                Test->TestFalse(TEXT("Damage command succeeds"), PF::AgentTools::ExecuteCommand(TEXT("PF.Damage"), {TEXT("25")}, World).HasErrors());
                Test->TestFalse(TEXT("Stamina command succeeds"), PF::AgentTools::ExecuteCommand(TEXT("PF.SetStamina"), {TEXT("20")}, World).HasErrors());
                Test->TestEqual(TEXT("Damage result"), Pawn->Survival->GetVitals().Health, 75.f);
                Test->TestEqual(TEXT("Stamina result"), Pawn->Survival->GetVitals().Stamina, 20.f);
            }
            Test->AddInfo(TEXT("[PrimalSurvival] Live damage/stamina values observed: Health=75 Stamina=20."));
            Stage = 2; Changed = Now;
        }
        else if (Stage == 2 && (Client ? Pawn->Survival->IsDead() : Now - Changed > 15))
        {
            if (!Client)
            {
                APFSurvivalGameMode* Mode = World->GetAuthGameMode<APFSurvivalGameMode>();
                if (!Mode) { Test->AddError(TEXT("Survival GameMode missing.")); return true; }
                Mode->RespawnDelay = 8; // Leave time for a remote client to observe death.
                Test->TestFalse(TEXT("Kill command succeeds"), PF::AgentTools::ExecuteCommand(TEXT("PF.Kill"), {}, World).HasErrors());
            }
            Test->TestTrue(TEXT("Death observed"), Pawn->Survival->IsDead());
            Test->AddInfo(TEXT("[PrimalSurvival] Live death observed."));
            Stage = 3;
        }
        else if (Stage == 3 && Pawn != OriginalPawn.Get() && V.Health == 100 && V.Stamina == 100)
        {
            Test->TestFalse(TEXT("Replacement pawn alive"), Pawn->Survival->IsDead());
            Test->TestTrue(TEXT("Replacement pawn collision restored"), Pawn->GetActorEnableCollision());
            Test->AddInfo(TEXT("[PrimalSurvival] Live replacement possession and reset attributes observed."));
            const auto Report = PF::AgentTools::ExecuteCommand(TEXT("PF.ExportTestReport"), {Client ? TEXT("M1LiveClient") : TEXT("M1LiveAuthority")}, World);
            Test->TestFalse(TEXT("Structured developer report exported"), Report.HasErrors());
            Stage = 4; Changed = Now;
        }
        // Allow the server's final possession/attribute updates to reach the client before TestExit.
        return Stage == 4 && Now - Changed > 10;
    }
private:
    FAutomationTestBase* Test;
    TWeakObjectPtr<APFSurvivorCharacter> OriginalPawn;
    double Started, Changed = 0;
    int32 Stage = 0;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFSurvivalLiveTest, "PF.Survival.Live",
    EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)
bool FPFSurvivalLiveTest::RunTest(const FString&)
{
    if (!FParse::Param(FCommandLine::Get(), TEXT("PFRunSurvivalLiveTests")))
    { AddError(TEXT("Requires isolated -game/-server -PFRunSurvivalLiveTests session on L_M1Survival.")); return false; }
    ADD_LATENT_AUTOMATION_COMMAND(FPFSurvivalLiveExercise(this));
    return true;
}
#endif
