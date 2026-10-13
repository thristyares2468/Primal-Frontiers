#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Survival/PFSurvivorCharacter.h"
#include "Survival/PFSurvivalGameMode.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Survival/PFTraversalMovement.h"
#include "Inventory/PFInventoryComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/PlayerStart.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFAutomaticClimbTest,"PF.Traversal.AutomaticClimb",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFAutomaticClimbTest::RunTest(const FString&)
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
    const float Radius=P->GetCapsuleComponent()->GetScaledCapsuleRadius(),Half=P->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
    auto Tick=[&](int32 N,bool Forward=false)
    {for(int32 J=0;J<N;++J){if(Forward){P->AddMovementInput(FVector(1,0,0));}F.TickTestWorld(.025f);}};
    auto Reset=[&]()
    {
        P->StopJumping();M->ClearTraversal();M->StopMovementImmediately();M->ClearAccumulatedForces();
        P->SetActorLocation(FVector(-500,1000,Half+3));M->SetMovementMode(MOVE_Falling);Tick(80);
        V->ChangeStamina(100);P->SetActorLocation(FVector(70-Radius-5,0,Half+3));M->SetMovementMode(MOVE_Falling);Tick(5);PC->SetControlRotation(FRotator::ZeroRotator);
    };
    auto* Ledge=Box(FVector(220,0,40),FVector(150,180,40));Reset();
    if(!TestTrue(TEXT("Automatic fixture starts grounded"),M->IsMovingOnGround())){return false;}
    P->Jump();bool Landed=false;
    for(int32 N=0;N<60;++N)
    {
        if(N==6){P->StopJumping();}Tick(1,true);
        if(N<20){AddInfo(FString::Printf(TEXT("[PrimalTraversal] Diagnostic frame=%d at=%s velocity=%s falling=%d climbUntil=%.3f now=%.3f"),N,*P->GetActorLocation().ToString(),*M->Velocity.ToString(),M->IsFalling(),M->ClimbUntil,W->GetTimeSeconds()));}
        if(P->GetActorLocation().X>70 && P->GetActorLocation().Z>Half+75 && M->IsMovingOnGround())
        {Landed=true;M->StopMovementImmediately();break;}
    }
    P->StopJumping();
    AddInfo(FString::Printf(TEXT("[PrimalTraversal] Automatic landing observed=%d position=%s grounded=%d"),Landed,*P->GetActorLocation().ToString(),M->IsMovingOnGround()));
    TestTrue(TEXT("Actual jump and wall contact automatically reach low ledge"),Landed);
    TestEqual(TEXT("Exactly one jump plus one climb stamina charge"),V->GetVitals().Stamina,100-P->JumpStaminaCost-P->ClimbStaminaCost);
    FString Reason;TestFalse(TEXT("Landed/used jump cannot trigger another automatic climb"),P->TryAutomaticClimb(Reason));
    Ledge->Destroy();auto* Tall=Box(FVector(220,0,100),FVector(150,180,100));Reset();
    P->Jump();Tick(6,true);P->StopJumping();Tick(54,true);
    TestEqual(TEXT("Tall wall cannot become low ledge from airborne feet"),V->GetVitals().Stamina,100-P->JumpStaminaCost);
    Tall->Destroy();Ledge=Box(FVector(220,0,40),FVector(150,180,40));
    auto* Roof=Box(FVector(100,0,260),FVector(250,200,15));Reset();
    P->Jump();Tick(6,true);P->StopJumping();Tick(54,true);
    TestEqual(TEXT("Blocked landing/overhead spends no automatic climb stamina"),V->GetVitals().Stamina,100-P->JumpStaminaCost);Roof->Destroy();
    Reset();V->ChangeStamina(-(100-P->JumpStaminaCost));P->Jump();Tick(6,true);P->StopJumping();Tick(54,true);
    TestEqual(TEXT("Jump affordable but additional climb not affordable"),V->GetVitals().Stamina,0.f);
    Reset();M->SetMovementMode(MOVE_Falling);M->Velocity=FVector(0,0,100);
    TestFalse(TEXT("Falling/rising without actual paid jump cannot climb"),P->TryAutomaticClimb(Reason));
    Reset();P->JumpMaxCount=2;M->SetMovementMode(MOVE_Falling);P->Jump();Tick(1);P->StopJumping();
    TestFalse(TEXT("Airborne extra jump never reanchors automatic ledge origin"),P->TryAutomaticClimb(Reason));P->JumpMaxCount=1;
    Reset();M->AutomaticClimbDrive=FVector(200,0,0);M->ClimbUntil=W->GetTimeSeconds()+1.5;P->Jump();Tick(1);P->StopJumping();
    TestTrue(TEXT("Ordinary jump clears previous approved mantle drive"),M->AutomaticClimbDrive.IsNearlyZero());
    Reset();PC->SetCraftingMenuOpen(true);P->Jump();Tick(6,true);P->StopJumping();
    TestEqual(TEXT("Modal UI never arms or charges a jump/climb"),V->GetVitals().Stamina,100.f);PC->SetCraftingMenuOpen(false);
    Reset();auto* I=PC->GetInventory();I->Grant(TEXT("Item_Wood"),66);P->Jump();Tick(6,true);P->StopJumping();
    TestEqual(TEXT("Jump cutoff weight prevents automatic climb and charges"),V->GetVitals().Stamina,100.f);I->RemoveItem(TEXT("Item_Wood"),66);
    Reset();P->Jump();Tick(1);P->StopJumping();P->SetRole(ROLE_AutonomousProxy);
    TestFalse(TEXT("Client cannot consume server automatic climb token"),P->TryAutomaticClimb(Reason));P->SetRole(ROLE_Authority);
    P->CancelTraversal();TestFalse(TEXT("Menu/cancel discards pending automatic climb"),P->TryAutomaticClimb(Reason));
    F.ForwardErrorMessages(this);AddInfo(TEXT("[PrimalTraversal] Actual automatic wall-contact mantle and original height/clearance/cost/airborne/menu/weight/authority/cancel gates verified"));return true;
}
#endif
