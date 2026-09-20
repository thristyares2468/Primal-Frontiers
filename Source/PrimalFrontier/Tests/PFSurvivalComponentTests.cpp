#include "Survival/PFPlayerSurvivalComponent.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include <limits>

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFSurvivalComponentTest, "PF.Survival.Component",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)
bool FPFSurvivalComponentTest::RunTest(const FString&)
{
    FTestWorldWrapper Fixture;
    if (!TestTrue(TEXT("Create lightweight test world"), Fixture.CreateTestWorld(EWorldType::Game))) { return false; }
    AActor* Owner = Fixture.GetTestWorld()->SpawnActor<AActor>();
    Owner->SetReplicates(true);
    UPFPlayerSurvivalComponent* Survival = NewObject<UPFPlayerSurvivalComponent>(Owner);
    Owner->AddInstanceComponent(Survival);
    Survival->RegisterComponent();
    if (!TestTrue(TEXT("Begin play"), Fixture.BeginPlayInTestWorld())) { return false; }
    TestEqual(TEXT("Initial health"), Survival->GetVitals().Health, 100.f);
    TestEqual(TEXT("Initial stamina"), Survival->GetVitals().Stamina, 100.f);
    TestTrue(TEXT("Component replication enabled"), Survival->GetIsReplicated());
    TestTrue(TEXT("Authority damage accepted"), Survival->ApplyDamage(25.f));
    TestEqual(TEXT("Damage deducted"), Survival->GetVitals().Health, 75.f);
    TestFalse(TEXT("Negative damage rejected"), Survival->ApplyDamage(-10.f));
    TestFalse(TEXT("NaN health rejected"), Survival->SetHealth(std::numeric_limits<float>::quiet_NaN()));
    TestTrue(TEXT("Stamina spent"), Survival->SpendStamina(20.f));
    TestEqual(TEXT("Stamina deducted"), Survival->GetVitals().Stamina, 80.f);
    TestFalse(TEXT("Overspending rejected"), Survival->SpendStamina(81.f));
    TestTrue(TEXT("Stamina lower clamp"), Survival->ChangeStamina(-1000.f));
    TestEqual(TEXT("Stamina zero"), Survival->GetVitals().Stamina, 0.f);
    Survival->ChangeStamina(1000.f);
    TestEqual(TEXT("Stamina upper clamp"), Survival->GetVitals().Stamina, 100.f);
    Survival->SpendStamina(20.f);
    for (int32 I = 0; I < 40; ++I) { Fixture.TickTestWorld(0.1f); }
    TestEqual(TEXT("Server stamina recovery"), Survival->GetVitals().Stamina, 100.f);

    Owner->SetRole(ROLE_SimulatedProxy);
    TestFalse(TEXT("Client damage refused"), Survival->ApplyDamage(10.f));
    TestFalse(TEXT("Client health assignment refused"), Survival->SetHealth(1.f));
    TestFalse(TEXT("Client stamina assignment refused"), Survival->ChangeStamina(-10.f));
    TestEqual(TEXT("Rejected client mutation preserved health"), Survival->GetVitals().Health, 75.f);
    Owner->SetRole(ROLE_Authority);
    TestTrue(TEXT("Lethal damage accepted"), Survival->ApplyDamage(1000.f));
    TestTrue(TEXT("Death"), Survival->IsDead());
    TestEqual(TEXT("Dead stamina zero"), Survival->GetVitals().Stamina, 0.f);
    TestEqual(TEXT("Death gameplay tag"), Survival->GetLifeState().ToString(), FString(TEXT("State.Survival.Dead")));
    TestFalse(TEXT("Cannot revive by assigning health"), Survival->SetHealth(100.f));
    TestFalse(TEXT("Dead stamina change rejected"), Survival->ChangeStamina(10.f));
    TestFalse(TEXT("Hunger remains planned"), Survival->SupportsStat(FGameplayTag::RequestGameplayTag(TEXT("Attribute.Survival.Hunger"))));
    TestFalse(TEXT("Thirst remains planned"), Survival->SupportsStat(FGameplayTag::RequestGameplayTag(TEXT("Attribute.Survival.Thirst"))));
    AddInfo(TEXT("[PrimalSurvival] Component assertions cover authority guards; socket replication is a separate live test."));
    Fixture.ForwardErrorMessages(this);
    return true;
}
#endif
