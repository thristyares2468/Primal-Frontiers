// PFSurvivalEnvironmentTests.cpp
//
// Automation test: PF.Survival.Environment   (M2, 8be2a14)
// Real overlaps with hazard zones (strongest intensity wins, removal and exit),
// ration consumption checks (authority, distance, line of sight, duplicates) and
// perishable ration expiry (refused at the deadline even before cleanup runs).
// Run: Session Frontend > Automation, filter "PF.Survival".

#include "Survival/PFPlayerSurvivalComponent.h"
#include "Survival/PFSurvivalHazard.h"
#include "Survival/PFRecoveryPickup.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFEnvironmentTest, "PF.Survival.Environment",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFEnvironmentTest::RunTest(const FString&)
{
    FTestWorldWrapper Fixture;
    if (!Fixture.CreateTestWorld(EWorldType::Game)) { return false; }
    UWorld* World = Fixture.GetTestWorld();
    auto* Pawn = World->SpawnActor<ACharacter>(FVector(0,0,100),FRotator::ZeroRotator);
    Pawn->GetCharacterMovement()->DisableMovement(); // Isolate overlap tests from gravity.
    auto* S = NewObject<UPFPlayerSurvivalComponent>(Pawn); Pawn->AddInstanceComponent(S); S->RegisterComponent();
    S->HungerDrainPerSecond = 0; S->ThirstDrainPerSecond = 0;
    Pawn->GetCapsuleComponent()->SetGenerateOverlapEvents(true);
    auto* Hazard = World->SpawnActor<APFSurvivalHazard>(FVector(500,0,100),FRotator::ZeroRotator);
    auto* Second = World->SpawnActor<APFSurvivalHazard>(FVector(500,0,100),FRotator::ZeroRotator); Second->Intensity = 0.5;
    if (!Fixture.BeginPlayInTestWorld()) { return false; }
    auto* PC = World->SpawnActor<APlayerController>(); PC->Possess(Pawn);
    Pawn->SetActorLocation(FVector(500,0,100));
    Fixture.TickTestWorld(0.2f); Fixture.TickTestWorld(0.2f);
    TestEqual(TEXT("Entering overlapping zones uses strongest exposure"), S->GetVitals().Exposure,1.f);
    TestTrue(TEXT("Hazard damages health"),S->GetVitals().Health < 100.f);
    Hazard->Destroy(); Fixture.TickTestWorld(0.2f);
    TestEqual(TEXT("Destroyed zone leaves remaining intensity"),S->GetVitals().Exposure,0.5f);
    Pawn->SetActorLocation(FVector(0,0,100)); Fixture.TickTestWorld(0.2f);
    TestEqual(TEXT("Leaving restores neutral exposure"),S->GetVitals().Exposure,0.f);
    S->SetHunger(10); S->SetThirst(20);
    auto* Pickup = World->SpawnActor<APFRecoveryPickup>(FVector(100,0,100),FRotator::ZeroRotator);
    Pawn->SetRole(ROLE_AutonomousProxy);
    TestFalse(TEXT("Client direct consumption refused"),Pickup->TryConsume(Pawn)); Pawn->SetRole(ROLE_Authority);
    Pickup->SetActorLocation(FVector(1000,0,100)); TestFalse(TEXT("Distant consumption refused"),Pickup->TryConsume(Pawn));
    Pickup->SetActorLocation(FVector(100,0,100));
    auto* Wall = World->SpawnActor<AActor>(); auto* Box = NewObject<UBoxComponent>(Wall);
    Wall->SetRootComponent(Box); Box->SetBoxExtent(FVector(10,100,500)); Box->SetCollisionProfileName(TEXT("BlockAll")); Box->RegisterComponent(); Wall->SetActorLocation(FVector(50,0,100));
    TestFalse(TEXT("Blocked line of sight refused"),Pickup->TryConsume(Pawn)); Wall->Destroy();
    TestTrue(TEXT("Nearby ration consumed"),Pickup->TryConsume(Pawn));
    TestEqual(TEXT("Food recovered"),S->GetVitals().Hunger,45.f);
    TestEqual(TEXT("Water recovered"),S->GetVitals().Thirst,55.f);
    TestFalse(TEXT("Duplicate consumption refused"),Pickup->TryConsume(Pawn));
    // Deadline is checked during consumption even before the next cleanup tick.
    const FTransform Location(FRotator::ZeroRotator,FVector(100,0,100));
    auto* Perishable = World->SpawnActorDeferred<APFRecoveryPickup>(APFRecoveryPickup::StaticClass(),Location);
    Perishable->ShelfLifeSeconds = 0.25f;
    Perishable->FinishSpawning(Location);
    Perishable->SetActorTickEnabled(false); // Hold cleanup to test deadline rejection independently.
    TestTrue(TEXT("Food starts fresh"),Perishable->GetRemainingFreshSeconds() > 0);
    TWeakObjectPtr<APFRecoveryPickup> WeakFood(Perishable);
    const auto BeforeExpiry = S->GetVitals();
    Fixture.TickTestWorld(0.5f);
    if (TestTrue(TEXT("Cleanup deliberately held"),WeakFood.IsValid()))
    {
        TestEqual(TEXT("Expiration deadline reached"),Perishable->GetRemainingFreshSeconds(),0.f);
        TestFalse(TEXT("Expired food cannot be consumed"),Perishable->TryConsume(Pawn));
        Perishable->SetActorTickEnabled(true);
    }
    for (int32 I=0; I<15; ++I) { Fixture.TickTestWorld(0.1f); }
    TestFalse(TEXT("Server removes spoiled food"),WeakFood.IsValid());
    TestEqual(TEXT("Spoiled food grants no hunger recovery"),S->GetVitals().Hunger,BeforeExpiry.Hunger);
    TestEqual(TEXT("Spoiled food grants no thirst recovery"),S->GetVitals().Thirst,BeforeExpiry.Thirst);
    AddInfo(TEXT("[PrimalSurvival] M2 real zone overlaps, exit/removal, authority, distance, obstruction and duplicate consumption tested."));
    Fixture.ForwardErrorMessages(this); return true;
}
#endif
