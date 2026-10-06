// PFSurvivalNeedsTests.cpp
//
// Automation test: PF.Survival.Needs   (M2, 8be2a14)
// Deterministic needs simulation via AdvanceNeeds: drain, recovery clamping,
// threshold damage counted only for time actually spent empty (and independent
// of how the time is split into steps), exposure damage, optional fed healing,
// invalid input, client-role refusal and death from needs.
// Run: Session Frontend > Automation, filter "PF.Survival".

#include "Survival/PFPlayerSurvivalComponent.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include <limits>

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFNeedsTest, "PF.Survival.Needs",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFNeedsTest::RunTest(const FString&)
{
    FTestWorldWrapper Fixture;
    if (!Fixture.CreateTestWorld(EWorldType::Game)) { return false; }
    AActor* Owner = Fixture.GetTestWorld()->SpawnActor<AActor>();
    Owner->SetReplicates(true);
    auto* S = NewObject<UPFPlayerSurvivalComponent>(Owner);
    Owner->AddInstanceComponent(S); S->RegisterComponent();
    if (!Fixture.BeginPlayInTestWorld()) { return false; }
    TestEqual(TEXT("Initial hunger"), S->GetVitals().Hunger, 100.f);
    TestEqual(TEXT("Initial thirst"), S->GetVitals().Thirst, 100.f);
    TestEqual(TEXT("Neutral exposure"), S->GetVitals().Exposure, 0.f);
    S->HungerDrainPerSecond = 1; S->ThirstDrainPerSecond = 2;
    S->AdvanceNeeds(10);
    TestEqual(TEXT("Food drain"), S->GetVitals().Hunger, 90.f);
    TestEqual(TEXT("Water drain"), S->GetVitals().Thirst, 80.f);
    TestTrue(TEXT("Recovery accepted"), S->RecoverNeeds(25, 25));
    TestEqual(TEXT("Food recovery clamped"), S->GetVitals().Hunger, 100.f);
    TestEqual(TEXT("Water recovery clamped"), S->GetVitals().Thirst, 100.f);
    S->SetHunger(5); S->SetThirst(10); S->AdvanceNeeds(10);
    TestEqual(TEXT("Only five seconds of threshold damage"), S->GetVitals().Health, 75.f);
    S->SetHealth(100); S->SetHunger(5); S->SetThirst(10);
    for (int32 I=0; I<10; ++I) { S->AdvanceNeeds(1); }
    TestEqual(TEXT("Partition independent threshold integration"), S->GetVitals().Health, 75.f);
    S->RecoverNeeds(100,100); S->SetExposure(1); S->AdvanceNeeds(2);
    TestEqual(TEXT("Exposure damage"), S->GetVitals().Health, 65.f);
    S->SetExposure(0); S->FedHealthRecoveryPerSecond = 2; S->AdvanceNeeds(2);
    TestEqual(TEXT("Configured fed health recovery"), S->GetVitals().Health, 69.f);
    TestFalse(TEXT("Invalid recovery rejected"), S->RecoverNeeds(-1,10));
    TestFalse(TEXT("Oversized recovery rejected"), S->RecoverNeeds(101,10));
    TestFalse(TEXT("Nonfinite thirst rejected"), S->SetThirst(std::numeric_limits<float>::quiet_NaN()));
    TestFalse(TEXT("Exposure range rejected"), S->SetExposure(2));
    TestFalse(TEXT("Negative simulation time rejected"), S->AdvanceNeeds(-1));
    const auto Before = S->GetVitals();
    Owner->SetRole(ROLE_SimulatedProxy);
    TestFalse(TEXT("Client hunger rejected"), S->SetHunger(0));
    TestFalse(TEXT("Client thirst rejected"), S->SetThirst(0));
    TestFalse(TEXT("Client exposure rejected"), S->SetExposure(1));
    TestFalse(TEXT("Client recovery rejected"), S->RecoverNeeds(10,10));
    TestFalse(TEXT("Client simulation rejected"), S->AdvanceNeeds(100));
    TestEqual(TEXT("Client probes preserve health"), S->GetVitals().Health, Before.Health);
    TestEqual(TEXT("Client probes preserve food"), S->GetVitals().Hunger, Before.Hunger);
    Owner->SetRole(ROLE_Authority);
    S->SetHealth(1); S->SetHunger(0); S->SetThirst(0); S->AdvanceNeeds(1);
    TestTrue(TEXT("Needs can cause death"), S->IsDead());
    TestFalse(TEXT("Dead recovery rejected"), S->RecoverNeeds(100,100));
    TestFalse(TEXT("Dead simulation stopped"), S->AdvanceNeeds(1));
    AddInfo(TEXT("[PrimalSurvival] M2 deterministic drain, threshold timing, recovery, exposure, invalid input and role guards verified."));
    Fixture.ForwardErrorMessages(this); return true;
}
#endif
