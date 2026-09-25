#include "PFCommands.h"
#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerController.h"
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Survival/PFSurvivorCharacter.h"
#include "Survival/PFSurvivalGameMode.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Survival/PFRecoveryPickup.h"

class FPFNeedsLiveExercise final : public IAutomationLatentCommand
{
public:
    explicit FPFNeedsLiveExercise(FAutomationTestBase* InTest) : Test(InTest), Started(FPlatformTime::Seconds())
    { FParse::Value(FCommandLine::Get(),TEXT("PFExpectedPlayers="),Expected); }
    bool Update() override
    {
        const double Now = FPlatformTime::Seconds();
        if (Now-Started > 200) { Test->AddError(FString::Printf(TEXT("[PrimalSurvival] M2 live timed out in stage %d"),Stage)); return true; }
        UWorld* World = nullptr;
        for (const auto& C : GEngine->GetWorldContexts()) { if (C.World() && C.World()->IsGameWorld()) { World=C.World(); break; } }
        if (!World) { return false; }
        const bool Client = World->GetNetMode()==NM_Client;
        for (TActorIterator<APFRecoveryPickup> It(World);It;++It)
        {
            if (It->GetActorLocation().Equals(FVector(0,1000,100),1) && It->GetRemainingFreshSeconds()>0)
            { ExpiringFood=*It; bSawFreshFood=true; }
        }
        if (bSawFreshFood && !ExpiringFood.IsValid()) { bSawExpiredFood=true; }
        TArray<APFSurvivorCharacter*> Pawns;
        for (TActorIterator<APFSurvivorCharacter> It(World);It;++It) { if (It->GetPlayerState()) { Pawns.Add(*It); } }
        if (Pawns.Num()!=Expected) { return false; }
        auto All = [&](auto Predicate) { for (auto* P:Pawns) { if (!Predicate(P->Survival->GetVitals())) { return false; } } return true; };
        auto LogSample = [&](const TCHAR* Label)
        {
            for (auto* P:Pawns)
            {
                const auto V=P->Survival->GetVitals();
                Test->AddInfo(FString::Printf(TEXT("[PrimalSurvival] M2 %s PlayerId=%d Role=%d Health=%.1f Hunger=%.1f Thirst=%.1f Exposure=%.2f"),
                    Label,P->GetPlayerState()->GetPlayerId(),int32(P->GetLocalRole()),V.Health,V.Hunger,V.Thirst,V.Exposure));
            }
        };
        if (Stage==0)
        {
            if (!All([](const auto& V){return V.Health==100 && V.Hunger>70 && V.Thirst>70;})) { return false; }
            if (Client)
            {
                for (const TCHAR* Name:{TEXT("PF.SetHunger"),TEXT("PF.SetThirst"),TEXT("PF.SetExposure"),TEXT("PF.RecoverNeeds")})
                {
                    const auto R=PF::AgentTools::ExecuteCommand(Name,FString(Name)==TEXT("PF.RecoverNeeds") ? TArray<FString>{} : TArray<FString>{TEXT("0")},World,false);
                    Test->TestTrue(TEXT("M2 client command rejected"),R.Issues.ContainsByPredicate([](const auto& I){return I.Code==TEXT("NotAuthority");}));
                }
                for (auto* P:Pawns)
                {
                    Test->TestFalse(TEXT("Client cannot change own or remote food"),P->Survival->SetHunger(0));
                    Test->TestFalse(TEXT("Client cannot change own or remote water"),P->Survival->SetThirst(0));
                    Test->TestFalse(TEXT("Client cannot recover needs"),P->Survival->RecoverNeeds(35,35));
                    Test->TestFalse(TEXT("Client cannot assign exposure"),P->Survival->SetExposure(1));
                }
            }
            else
            {
                for (auto* P:Pawns)
                { P->Survival->HungerDrainPerSecond=0; P->Survival->ThirstDrainPerSecond=0; P->Survival->ExposureDamagePerSecond=0; }
            }
            LogSample(TEXT("initial")); Stage=1; Changed=Now;
        }
        else if (Stage==1 && (Client ? All([](const auto& V){return V.Hunger==40 && V.Thirst==50 && V.Exposure==0.25f;}) : Now-Changed>25))
        {
            if (!Client)
            {
                for(auto* P:Pawns){ P->Survival->SetHunger(40); P->Survival->SetThirst(50); P->Survival->SetExposure(0.25); }
                const FTransform Location(FRotator::ZeroRotator,FVector(0,1000,100));
                auto* Food=World->SpawnActorDeferred<APFRecoveryPickup>(APFRecoveryPickup::StaticClass(),Location);
                if (!Food) { Test->AddError(TEXT("Expiry fixture spawn failed")); return true; }
                Food->ShelfLifeSeconds=5; Food->FinishSpawning(Location);
            }
            LogSample(TEXT("authoritative-needs")); Stage=2; Changed=Now;
        }
        else if (Stage==2 && (Client ? All([](const auto& V){return V.Hunger==75 && V.Thirst==85 && V.Exposure==0;}) : Now-Changed>12))
        {
            if (!Client) { for(auto* P:Pawns){ P->Survival->RecoverNeeds(35,35); P->Survival->SetExposure(0); } }
            LogSample(TEXT("recovery")); Stage=3; Changed=Now;
        }
        else if (Stage==3 && (Client ? All([](const auto& V){return V.Hunger==0 && V.Thirst==0 && V.Health==90;}) : Now-Changed>12))
        {
            if (!Client)
            {
                for(auto* P:Pawns)
                {
                    P->Survival->SetHunger(0); P->Survival->SetThirst(0); P->Survival->AdvanceNeeds(2);
                    Test->TestEqual(TEXT("Actual server threshold damage"),P->Survival->GetVitals().Health,90.f);
                    P->Survival->StarvationDamagePerSecond=0; P->Survival->DehydrationDamagePerSecond=0;
                }
            }
            LogSample(TEXT("threshold-damage")); Stage=4; Changed=Now;
        }
        else if (Stage==4 && (Client ? All([](const auto& V){return V.Health==0;}) : Now-Changed>12))
        {
            if (!Client)
            {
                auto* Mode=World->GetAuthGameMode<APFSurvivalGameMode>();
                if (!Mode) { Test->AddError(TEXT("Missing survival GameMode")); return true; }
                Mode->RespawnDelay=8;
                for(auto* P:Pawns){ P->Survival->ApplyDamage(1000); }
            }
            LogSample(TEXT("death")); Stage=5;
        }
        else if (Stage==5 && All([](const auto& V){return V.Health==100 && V.Hunger>95 && V.Thirst>95 && V.Exposure==0;}))
        {
            LogSample(TEXT("respawn-reset"));
            Test->TestTrue(TEXT("Fresh food and its replicated expiry countdown observed"),bSawFreshFood);
            Test->TestTrue(TEXT("Expired food removal observed on this process"),bSawExpiredFood);
            const auto R=PF::AgentTools::ExecuteCommand(TEXT("PF.ExportTestReport"),{Client?TEXT("M2LiveClient"):TEXT("M2LiveServer")},World);
            Test->TestFalse(TEXT("M2 report exported"),R.HasErrors());
            Stage=6; Changed=Now;
        }
        return Stage==6 && Now-Changed>10;
    }
private:
    FAutomationTestBase* Test;
    double Started,Changed=0;
    int32 Stage=0,Expected=1;
    TWeakObjectPtr<APFRecoveryPickup> ExpiringFood;
    bool bSawFreshFood=false,bSawExpiredFood=false;
};
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFNeedsLiveTest,"PF.Survival.NeedsLive",
    EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)
bool FPFNeedsLiveTest::RunTest(const FString&)
{
    if (!FParse::Param(FCommandLine::Get(),TEXT("PFRunNeedsLiveTests")))
    { AddError(TEXT("Requires isolated M2 -game/-server -PFRunNeedsLiveTests [-PFExpectedPlayers=1|2].")); return false; }
    ADD_LATENT_AUTOMATION_COMMAND(FPFNeedsLiveExercise(this)); return true;
}
#endif
