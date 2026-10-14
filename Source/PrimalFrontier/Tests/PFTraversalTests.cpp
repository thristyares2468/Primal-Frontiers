#if WITH_DEV_AUTOMATION_TESTS
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFTraversalRulesTest,"PF.Traversal.Rules",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFTraversalRulesTest::RunTest(const FString&)
{
    AddExpectedMessage(TEXT("GetSocketInfoByName.*No SkeletalMesh for Component"),EAutomationExpectedMessageFlags::Contains,0);
    FTestWorldWrapper F;if(!F.CreateTestWorld(EWorldType::Game)){return false;}auto* W=F.GetTestWorld();
    auto Box=[&](FVector At,FVector Extent)
    {auto* A=W->SpawnActor<AActor>();auto* B=NewObject<UBoxComponent>(A);A->SetRootComponent(B);B->SetBoxExtent(Extent);B->SetCollisionProfileName(TEXT("BlockAll"));B->RegisterComponent();A->SetActorLocation(At);return A;};
    Box(FVector(0,0,-25),FVector(6000,6000,25));
    W->SetGameMode(FURL(nullptr,TEXT("/Engine/Maps/Entry?game=/Script/PrimalFrontier.PFSurvivalGameMode"),TRAVEL_Absolute));
    W->SpawnActor<APlayerStart>(FVector(-500,0,120),FRotator::ZeroRotator);if(!F.BeginPlayInTestWorld()){return false;}
    auto* PC=W->SpawnActor<APFSurvivalPlayerController>();PC->SetAsLocalPlayerController();W->GetAuthGameMode<APFSurvivalGameMode>()->RestartPlayer(PC);
    auto* P=Cast<APFSurvivorCharacter>(PC->GetPawn());if(!P){return false;}auto* M=P->Traversal();auto* I=PC->GetInventory();
    P->Survival->HungerDrainPerSecond=0;P->Survival->ThirstDrainPerSecond=0;
    auto Tick=[&](int32 N,bool Move=false){for(int32 J=0;J<N;++J){if(Move){P->AddMovementInput(FVector(1,0,0));}F.TickTestWorld(.05f);}};
    Tick(10);TestTrue(TEXT("Real pawn uses custom movement and lands"),M->IsMovingOnGround());const float Walk=M->GetMaxSpeed();
    TestTrue(TEXT("Exact weight limit grant"),I->Grant(TEXT("Item_Wood"),60));P->SetSprintHeld(true);
    TestTrue(TEXT("At limit sprint allowed"),M->IsSprinting());
    TestTrue(TEXT("First excess pickup accepted"),I->Grant(TEXT("Item_Wood"),1));TestFalse(TEXT("Any excess immediately disables sprint"),M->IsSprinting());
    TestTrue(TEXT("First excess slows walk"),M->GetMaxSpeed()<Walk);TestTrue(TEXT("Small excess still allows stamina-gated jump"),P->CanJump());
    I->Grant(TEXT("Item_Wood"),5);TestFalse(TEXT("110 percent weight blocks jump"),P->CanJump());const float FirstSlow=M->GetMaxSpeed();
    I->Grant(TEXT("Item_Wood"),60);TestTrue(TEXT("Further excess progressively slower"),M->GetMaxSpeed()<FirstSlow);
    I->RemoveItem(TEXT("Item_Wood"),I->Count(TEXT("Item_Wood")));TestTrue(TEXT("Unloading immediately restores sprint"),M->IsSprinting());
    Tick(20,true);TestTrue(TEXT("Real movement attains sprint speed"),M->Velocity.Size2D()>Walk*1.3f);
    TestTrue(TEXT("Server actually drains sprint stamina"),P->Survival->GetVitals().Stamina<95);
    P->SetSprintHeld(false);TestEqual(TEXT("Release restores walking speed"),M->GetMaxSpeed(),Walk);
    // Actual saved-move allocation/capture/replay/flags and combine boundaries.
    auto* Prediction=static_cast<FNetworkPredictionData_Client_Character*>(M->GetPredictionData_Client());auto Move=Prediction->AllocateNewMove();
    M->bWantsSprint=true;M->bWantsGlide=true;Move->SetMoveFor(P,.05f,FVector(1,0,0),*Prediction);
    TestTrue(TEXT("Sprint/glide compressed intentions recorded"),(Move->GetCompressedFlags()&0x30)==0x30);
    M->ClearTraversal();Move->PrepMoveFor(P);TestTrue(TEXT("Correction replay restores intentions"),M->bWantsSprint && M->bWantsGlide);
    auto Other=Prediction->AllocateNewMove();Other->SetMoveFor(P,.05f,FVector(1,0,0),*Prediction);M->ClearTraversal();Other->SetMoveFor(P,.05f,FVector(1,0,0),*Prediction);
    TestFalse(TEXT("Different intents never combine"),Move->CanCombineWith(Other,P,.1f));
    // Real falling physics, descent cap, expenditure, cancellation and normal landing.
    P->SetActorLocation(FVector(-500,1000,1000));M->SetMovementMode(MOVE_Falling);M->Velocity=FVector(0,0,-600);P->ToggleGlide();
    const float Before=P->Survival->GetVitals().Stamina;const float NormalGravity=W->GetGravityZ()*M->GravityScale;
    Tick(8);TestTrue(TEXT("Glide remains falling"),M->IsGliding());TestTrue(TEXT("Glide caps actual descent"),M->Velocity.Z>=-M->GlideDescentSpeed-1);
    TestTrue(TEXT("Glide spends authoritative stamina"),P->Survival->GetVitals().Stamina<Before);P->ToggleGlide();
    TestFalse(TEXT("Toggle stops glide"),M->IsGliding());TestEqual(TEXT("Gravity restored without permanent setting edits"),M->GetGravityZ(),NormalGravity);
    P->ToggleGlide();P->Survival->ChangeStamina(-100);TestFalse(TEXT("Zero stamina cannot glide"),M->IsGliding());Tick(45);TestFalse(TEXT("Landing clears glide intent"),M->bWantsGlide);
    P->Survival->ChangeStamina(100);M->ClearTraversal();M->StopMovementImmediately();P->SetActorLocation(FVector(-500,0,120));Tick(8);PC->SetControlRotation(FRotator::ZeroRotator);
    auto* Ledge=Box(FVector(-280,0,40),FVector(150,180,40));auto* Roof=Box(FVector(-450,0,240),FVector(200,200,15));FString Reason;
    const float ClimbBefore=P->Survival->GetVitals().Stamina;
    TestTrue(TEXT("Contact attachment does not require preflight top"),P->TryClimb(Reason));TestEqual(TEXT("Attachment spends no upfront stamina"),P->Survival->GetVitals().Stamina,ClimbBefore);
    TestFalse(TEXT("Duplicate native attachment refused"),P->TryClimb(Reason));P->CancelTraversal();Roof->Destroy();Tick(15);
    if(!TestTrue(TEXT("Clear actual low ledge attaches"),P->TryClimb(Reason))){AddError(Reason);return false;}
    for(int32 N=0;N<26;++N){if(M->IsWallClimbing()){P->AddMovementInput(FVector::UpVector);}F.TickTestWorld(.05f);}
    TestTrue(TEXT("Continuous climb consumes elapsed stamina and reaches top"),P->Survival->GetVitals().Stamina<ClimbBefore && P->GetActorLocation().Z>170 && P->GetActorLocation().X>-430 && M->IsMovingOnGround());
    Ledge->Destroy();M->StopMovementImmediately();P->SetActorLocation(FVector(-500,1000,120));Tick(10);
    PC->SetCraftingMenuOpen(true);P->SetSprintHeld(true);TestFalse(TEXT("Modal menu blocks sprint"),M->IsSprinting());TestFalse(TEXT("Modal menu blocks jump"),P->CanJump());TestFalse(TEXT("Modal menu blocks climb"),P->TryClimb(Reason));PC->SetCraftingMenuOpen(false);
    P->SetRole(ROLE_AutonomousProxy);TestFalse(TEXT("Client cannot execute climb authority directly"),P->TryClimb(Reason));P->SetRole(ROLE_Authority);
    P->SetSprintHeld(true);P->LaunchCharacter(FVector(100,0,500),true,true);P->Survival->ApplyDamage(1000);TestFalse(TEXT("Death clears sprint"),M->bWantsSprint);TestFalse(TEXT("Dead pawn cannot climb"),P->TryClimb(Reason));
    TestFalse(TEXT("Death clears queued climb launch"),M->HandlePendingLaunch());
    P->SetRole(ROLE_AutonomousProxy);P->ClientWallClimbState_Implementation(FVector(-1,0,0),100,true,false);
    TestTrue(TEXT("Late continuous-climb owner acknowledgment cannot move corpse"),!M->IsWallClimbing() && M->MovementMode==MOVE_None);P->SetRole(ROLE_Authority);
    P->SetRole(ROLE_AutonomousProxy);P->ClientClimbImpulse_Implementation(FVector(100,0,500),false,1.5f,FVector::ZeroVector);TestFalse(TEXT("Late owner climb acknowledgment cannot relaunch corpse"),M->HandlePendingLaunch());P->SetRole(ROLE_Authority);
    P->SetRole(ROLE_AutonomousProxy);P->ClientClimbImpulse_Implementation(FVector(100,0,500),true,1.5f,FVector::ZeroVector);TestFalse(TEXT("Late automatic acknowledgment cannot relaunch corpse"),M->HandlePendingLaunch());TestTrue(TEXT("Dead owner has no automatic drive"),M->AutomaticClimbDrive.IsNearlyZero());P->SetRole(ROLE_Authority);
    F.ForwardErrorMessages(this);AddInfo(TEXT("[PrimalTraversal] Real weight boundaries, movement/stamina, saved-move replay, falling glide, swept ledge climb, menus/death/client refusal checked."));return true;
}
#endif
