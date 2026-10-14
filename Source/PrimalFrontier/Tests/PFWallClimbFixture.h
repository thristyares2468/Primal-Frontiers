#pragma once
#include "Survival/PFSurvivorCharacter.h"
#include "Survival/PFSurvivalGameMode.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Survival/PFTraversalMovement.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/PlayerStart.h"
#include "Engine/World.h"
#include "Tests/AutomationCommon.h"

struct FPFWallFixture
{
    FTestWorldWrapper Fixture;UWorld* W=nullptr;APFSurvivalPlayerController* PC=nullptr;APFSurvivorCharacter* P=nullptr;UPFTraversalMovement* M=nullptr;
    AActor* Box(FVector At,FVector Extent)
    {auto* A=W->SpawnActor<AActor>();auto* B=NewObject<UBoxComponent>(A);A->SetRootComponent(B);B->SetBoxExtent(Extent);B->SetCollisionProfileName(TEXT("BlockAll"));B->RegisterComponent();A->SetActorLocation(At);return A;}
    void Tick(int32 N,FVector Input=FVector::ZeroVector)
    {for(int32 J=0;J<N;++J){if(!Input.IsZero()){P->AddMovementInput(Input);}Fixture.TickTestWorld(.025f);}}
    bool Start()
    {
        if(!Fixture.CreateTestWorld(EWorldType::Game)){return false;}W=Fixture.GetTestWorld();Box({0,0,-25},{6000,6000,25});
        W->SetGameMode(FURL(nullptr,TEXT("/Engine/Maps/Entry?game=/Script/PrimalFrontier.PFSurvivalGameMode"),TRAVEL_Absolute));W->SpawnActor<APlayerStart>(FVector(-500,0,120),FRotator::ZeroRotator);
        if(!Fixture.BeginPlayInTestWorld()){return false;}PC=W->SpawnActor<APFSurvivalPlayerController>();PC->SetAsLocalPlayerController();W->GetAuthGameMode<APFSurvivalGameMode>()->RestartPlayer(PC);
        P=Cast<APFSurvivorCharacter>(PC->GetPawn());if(!P){return false;}M=P->Traversal();P->Survival->HungerDrainPerSecond=0;P->Survival->ThirstDrainPerSecond=0;P->Survival->StaminaRecoveryPerSecond=0;Reset();return true;
    }
    void Reset(float Stamina=100)
    {
        P->StopJumping();M->ClearTraversal();M->StopMovementImmediately();M->ClearAccumulatedForces();P->SetActorLocation(FVector(-500,1000,100));M->SetMovementMode(MOVE_Falling);Tick(60);
        P->Survival->ChangeStamina(-P->Survival->GetVitals().Stamina);P->Survival->ChangeStamina(Stamina);
        const float R=P->GetCapsuleComponent()->GetScaledCapsuleRadius(),H=P->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();P->SetActorLocation(FVector(70-R-4,0,H+3));
        P->SetActorRotation(FRotator::ZeroRotator);PC->SetControlRotation(FRotator::ZeroRotator);M->SetMovementMode(MOVE_Falling);Tick(5);
    }
};
