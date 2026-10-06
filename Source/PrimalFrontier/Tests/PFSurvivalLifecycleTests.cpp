// PFSurvivalLifecycleTests.cpp
//
// Automation test: PF.Survival.Lifecycle   (M1, f7ed11d)
// Real GameMode + PlayerStart: spawn position, first-person camera, server damage
// pipeline, jump stamina cost, client-role refusal, and three timed
// death -> respawn cycles (old pawn destroyed, vitals and collision restored).
// Run: Session Frontend > Automation, filter "PF.Survival".

#include "Survival/PFSurvivorCharacter.h"
#include "Survival/PFSurvivalGameMode.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFSurvivalLifecycleTest, "PF.Survival.Lifecycle",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)
bool FPFSurvivalLifecycleTest::RunTest(const FString&)
{
    // This native fixture deliberately has no presentation assets. The inherited
    // template camera's head attachment is exercised by the Blueprint playtest.
    AddExpectedMessage(TEXT("GetSocketInfoByName.*No SkeletalMesh for Component"),
        EAutomationExpectedMessageFlags::Contains, 0);
    FTestWorldWrapper Fixture;
    if (!Fixture.CreateTestWorld(EWorldType::Game)) { AddError(TEXT("World creation failed.")); return false; }
    UWorld* World = Fixture.GetTestWorld();
    World->SetGameMode(FURL(nullptr, TEXT("/Engine/Maps/Entry?game=/Script/PrimalFrontier.PFSurvivalGameMode"), TRAVEL_Absolute));
    AActor* Floor = World->SpawnActor<AActor>();
    UBoxComponent* Box = NewObject<UBoxComponent>(Floor);
    Floor->SetRootComponent(Box); Box->SetBoxExtent(FVector(2000, 2000, 25));
    Box->SetCollisionProfileName(TEXT("BlockAll")); Box->RegisterComponent(); Floor->SetActorLocation(FVector(0, 0, -25));
    APlayerStart* Start = World->SpawnActor<APlayerStart>(FVector(100, 200, 120), FRotator::ZeroRotator);
    if (!TestTrue(TEXT("Begin play"), Fixture.BeginPlayInTestWorld())) { return false; }
    APFSurvivalGameMode* Mode = World->GetAuthGameMode<APFSurvivalGameMode>();
    if (!TestNotNull(TEXT("Survival GameMode"), Mode)) { return false; }
    APFSurvivalPlayerController* PC = World->SpawnActor<APFSurvivalPlayerController>();
    Mode->RestartPlayer(PC);
    APFSurvivorCharacter* Pawn = Cast<APFSurvivorCharacter>(PC->GetPawn());
    if (!TestNotNull(TEXT("Possessed survivor"), Pawn)) { return false; }
    TestTrue(TEXT("Spawn at PlayerStart XY"), FVector2D(Pawn->GetActorLocation()).Equals(FVector2D(Start->GetActorLocation()), 1));
    TestTrue(TEXT("Spawn height within capsule grounding adjustment"),
        FMath::Abs(Pawn->GetActorLocation().Z - Start->GetActorLocation().Z) <= Pawn->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
    TestTrue(TEXT("First-person camera active"), Pawn->GetFirstPersonCameraComponent()->IsActive());
    TestEqual(TEXT("Server damage pipeline"), Pawn->TakeDamage(25, FDamageEvent(), PC, nullptr), 25.f);
    TestEqual(TEXT("Damage changed health"), Pawn->Survival->GetVitals().Health, 75.f);
    Pawn->OnJumped_Implementation();
    TestEqual(TEXT("Server jump consumes stamina"), Pawn->Survival->GetVitals().Stamina, 80.f);
    Pawn->SetRole(ROLE_AutonomousProxy);
    TestEqual(TEXT("Client cannot apply damage"), Pawn->TakeDamage(20, FDamageEvent(), PC, nullptr), 0.f);
    Pawn->OnJumped_Implementation();
    TestEqual(TEXT("Client cannot spend replicated stamina"), Pawn->Survival->GetVitals().Stamina, 80.f);
    Pawn->SetRole(ROLE_Authority);
    TestFalse(TEXT("Living pawn cannot force respawn"), Mode->RespawnPlayer(PC));
    Mode->RespawnDelay = 0.1f;
    for (int32 Cycle = 0; Cycle < 3; ++Cycle)
    {
        TWeakObjectPtr<APFSurvivorCharacter> Previous(Pawn);
        Pawn->TakeDamage(1000, FDamageEvent(), PC, nullptr);
        TestTrue(TEXT("Death state"), Pawn->Survival->IsDead());
        TestTrue(TEXT("Dead movement disabled"), Pawn->GetCharacterMovement()->MovementMode == MOVE_None);
        TestFalse(TEXT("Dead collision disabled"), Pawn->GetActorEnableCollision());
        for (int32 I = 0; I < 5; ++I) { Fixture.TickTestWorld(0.1f); }
        Pawn = Cast<APFSurvivorCharacter>(PC->GetPawn());
        if (!TestNotNull(TEXT("Automatic respawn"), Pawn)) { return false; }
        TestFalse(TEXT("Old pawn destroyed"), Previous.IsValid());
        TestEqual(TEXT("Respawn health reset"), Pawn->Survival->GetVitals().Health, 100.f);
        TestEqual(TEXT("Respawn stamina reset"), Pawn->Survival->GetVitals().Stamina, 100.f);
        TestTrue(TEXT("Respawn retains start XY"), FVector2D(Pawn->GetActorLocation()).Equals(FVector2D(Start->GetActorLocation()), 1));
        TestTrue(TEXT("Respawn collision restored"), Pawn->GetActorEnableCollision());
    }
    AddInfo(TEXT("[PrimalSurvival] Three server death/respawn cycles validated at a real PlayerStart."));
    Fixture.ForwardErrorMessages(this);
    return true;
}
#endif
