#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Survival/PFSurvivorCharacter.h"
#include "Survival/PFSurvivalGameMode.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Survival/PFTraversalMovement.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/PlayerStart.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFClimbContactTest,"PF.Traversal.Contact",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFClimbContactTest::RunTest(const FString&)
{
    AddExpectedMessage(TEXT("GetSocketInfoByName.*No SkeletalMesh for Component"),EAutomationExpectedMessageFlags::Contains,0);
    FTestWorldWrapper F;if(!F.CreateTestWorld(EWorldType::Game)){return false;}auto* W=F.GetTestWorld();
    auto Box=[&](FVector At,FVector Extent)
    {auto* A=W->SpawnActor<AActor>();auto* B=NewObject<UBoxComponent>(A);A->SetRootComponent(B);B->SetBoxExtent(Extent);B->SetCollisionProfileName(TEXT("BlockAll"));B->RegisterComponent();A->SetActorLocation(At);return A;};
    Box(FVector(0,0,-25),FVector(6000,6000,25));
    W->SetGameMode(FURL(nullptr,TEXT("/Engine/Maps/Entry?game=/Script/PrimalFrontier.PFSurvivalGameMode"),TRAVEL_Absolute));
    W->SpawnActor<APlayerStart>(FVector(-500,0,120),FRotator::ZeroRotator);if(!F.BeginPlayInTestWorld()){return false;}
    auto* PC=W->SpawnActor<APFSurvivalPlayerController>();PC->SetAsLocalPlayerController();W->GetAuthGameMode<APFSurvivalGameMode>()->RestartPlayer(PC);
    auto* P=Cast<APFSurvivorCharacter>(PC->GetPawn());if(!P){return false;}auto* M=P->Traversal();auto* V=P->Survival.Get();
    V->HungerDrainPerSecond=0;V->ThirstDrainPerSecond=0;V->StaminaRecoveryPerSecond=0;
    const float Half=P->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
    auto Tick=[&](int32 N,bool Forward=false)
    {for(int32 J=0;J<N;++J){if(Forward){P->AddMovementInput(P->GetActorForwardVector());}F.TickTestWorld(.025f);}};
    for(int32 Height:{80,110})
    {
        auto* Ledge=Box(FVector(220,0,Height*.5),FVector(150,180,Height*.5));
        for(bool Automatic:{false,true})
        {
            P->StopJumping();M->ClearTraversal();M->StopMovementImmediately();M->ClearAccumulatedForces();
            P->SetActorLocation(FVector(-500,1000,Half+3));M->SetMovementMode(MOVE_Falling);Tick(80);
            V->ChangeStamina(100);P->SetActorLocation(FVector(-200,0,Half+3));M->SetMovementMode(MOVE_Falling);
            P->SetActorRotation(FRotator::ZeroRotator);PC->SetControlRotation(FRotator::ZeroRotator);Tick(5);
            Tick(45,true);M->StopMovementImmediately();
            if(!TestTrue(TEXT("Ordinary forward approach stops grounded against wall"),M->IsMovingOnGround() && P->GetActorLocation().X>30 && P->GetActorLocation().X<40)){return false;}
            FString Reason;bool Accepted=true;
            if(Automatic){P->Jump();}else{Accepted=P->TryClimb(Reason);}
            TestTrue(TEXT("Grounded contact manual request accepted"),Accepted);if(!Accepted){AddInfo(Reason);}
            bool Landed=false;
            for(int32 J=0;J<80;++J)
            {
                if(J==6){P->StopJumping();}Tick(1,Automatic);
                if(M->IsMovingOnGround() && P->GetActorLocation().X>70 && FMath::Abs(P->GetActorLocation().Z-Half-Height)<5){Landed=true;break;}
            }
            AddInfo(FString::Printf(TEXT("[PrimalTraversal] Contact height=%d automatic=%d accepted=%d landed=%d at=%s"),Height,Automatic,Accepted,Landed,*P->GetActorLocation().ToString()));
            TestTrue(TEXT("Climb from actual wall contact lands on top"),Landed);
            TestEqual(TEXT("Contact climb charges exactly once"),V->GetVitals().Stamina,100-P->ClimbStaminaCost-(Automatic?P->JumpStaminaCost:0));
        }
        Ledge->Destroy();
    }
    F.ForwardErrorMessages(this);return true;
}
#endif
