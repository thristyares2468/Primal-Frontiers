#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Tests/PFWallClimbFixture.h"
#include "Inventory/PFInventoryComponent.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFClimbContactTest,"PF.Traversal.Contact",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FPFClimbContactTest::RunTest(const FString&)
{
    AddExpectedMessage(TEXT("GetSocketInfoByName.*No SkeletalMesh for Component"),EAutomationExpectedMessageFlags::Contains,0);
    FPFWallFixture F;if(!F.Start()){return false;}FString Reason;const float Half=F.P->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
    for(int32 Height:{80,110,400,1000})
    {
        auto* Wall=F.Box(FVector(220,0,Height*.5f),FVector(150,180,Height*.5f));F.Reset();
        TestTrue(TEXT("Grounded F attaches at any wall height"),F.P->TryClimb(Reason));TestEqual(TEXT("Attachment never prepays full ascent"),F.P->Survival->GetVitals().Stamina,100.f);
        bool Landed=false;for(int32 N=0;N<400;++N){F.Tick(1,FVector::UpVector);if(F.M->IsMovingOnGround() && F.P->GetActorLocation().X>70 && FMath::Abs(F.P->GetActorLocation().Z-Half-Height)<5){Landed=true;break;}}
        TestTrue(TEXT("Continuous real swept movement reaches clear top"),Landed);TestTrue(TEXT("Only elapsed climbing drains stamina"),F.P->Survival->GetVitals().Stamina>0 && F.P->Survival->GetVitals().Stamina<100);
        AddInfo(FString::Printf(TEXT("[PrimalAgentTools] Continuous height%d at%s stamina%.2f landed%d"),Height,*F.P->GetActorLocation().ToString(),F.P->Survival->GetVitals().Stamina,Landed));Wall->Destroy();
    }
    auto* Wall=F.Box({220,0,1000},{150,500,1000});F.Reset(24);TestTrue(TEXT("Low stamina still attaches to20m wall"),F.P->TryClimb(Reason));
    F.Tick(20,FVector::UpVector);TestTrue(TEXT("Partial rise and gradual drain"),F.P->GetActorLocation().Z>Half+50 && FMath::IsNearlyEqual(F.P->Survival->GetVitals().Stamina,18.f,.1f));
    bool Fell=false;for(int32 N=0;N<85;++N){F.Tick(1,FVector::UpVector);if(F.M->IsFalling() && F.P->Survival->GetVitals().Stamina==0){Fell=true;break;}}
    TestTrue(TEXT("Exactly empty stamina drops before the top"),Fell && F.P->GetActorLocation().Z<Half+500);TestFalse(TEXT("Immediate regrab cannot create stamina"),F.P->TryClimb(Reason));F.Tick(80);TestTrue(TEXT("Normal gravity lands on ground after exhaustion"),F.M->IsMovingOnGround());
    F.Reset();TestTrue(TEXT("Hold fixture attaches"),F.P->TryClimb(Reason));F.P->Survival->StaminaRecoveryDelay=0;F.P->Survival->StaminaRecoveryPerSecond=80;const FVector At=F.P->GetActorLocation();
    F.Tick(40);TestTrue(TEXT("Holding drains4/s and never recovers on wall"),FMath::IsNearlyEqual(F.P->Survival->GetVitals().Stamina,96.f,.1f) && F.P->GetActorLocation().Equals(At,1));F.P->Survival->StaminaRecoveryPerSecond=0;
    F.Tick(20,F.M->WallRight());TestTrue(TEXT("Sideways real movement with12/s drain"),F.P->GetActorLocation().Y>At.Y+60 && FMath::IsNearlyEqual(F.P->Survival->GetVitals().Stamina,90.f,.1f));
    F.Tick(30,FVector::UpVector);const float Up=F.P->GetActorLocation().Z;F.Tick(10,-FVector::UpVector);TestTrue(TEXT("S descends along wall"),F.P->GetActorLocation().Z<Up-20);
    F.P->ServerClimb();TestTrue(TEXT("Second F releases without extra charge"),F.M->IsFalling());F.Tick(80);
    F.Reset();auto* Roof=F.Box({100,0,260},{250,600,15});TestTrue(TEXT("Blocked route can attach without preflight top claim"),F.P->TryClimb(Reason));F.Tick(80,FVector::UpVector);
    TestTrue(TEXT("Swept capsule cannot climb through ceiling"),F.P->GetActorLocation().Z+Half<247 && F.M->IsWallClimbing());F.P->CancelTraversal();Roof->Destroy();F.Reset();
    TestTrue(TEXT("Wall-loss fixture attaches"),F.P->TryClimb(Reason));F.Tick(30,FVector::UpVector);Wall->Destroy();F.Tick(1);TestTrue(TEXT("Missing contact drops into gravity"),F.M->IsFalling());
    Wall=F.Box({220,0,400},{150,500,400});F.Reset();F.P->SetRole(ROLE_AutonomousProxy);TestFalse(TEXT("Client direct attach cannot grant server mode"),F.P->TryClimb(Reason));TestFalse(TEXT("Client cannot authorize movement directly"),F.M->BeginWallClimb({-1,0,0}));F.P->SetRole(ROLE_Authority);
    F.M->SetMovementMode(MOVE_Custom,UPFTraversalMovement::WallClimbMode);F.Tick(1);
    TestFalse(TEXT("Forged custom movement mode lacks authoritative wall permission"),F.M->IsWallClimbing());F.Reset();
    TestTrue(TEXT("Cancellation fixture attaches"),F.P->TryClimb(Reason));TestTrue(TEXT("Climbing has native gameplay state tag"),F.M->ClimbingState().IsValid());
    F.M->WallClimbStaminaPerSecond=0;const float InvalidBefore=F.P->Survival->GetVitals().Stamina;F.Tick(1);
    TestTrue(TEXT("Invalid runtime drain tuning cancels without a free ascent"),!F.M->IsWallClimbing() && F.P->Survival->GetVitals().Stamina==InvalidBefore);
    F.M->WallClimbStaminaPerSecond=12;F.Reset();F.P->TryClimb(Reason);F.P->CancelTraversal();TestFalse(TEXT("Menu cancellation clears climbing"),F.M->IsWallClimbing());
    TestFalse(TEXT("Cancellation clears climbing state tag"),F.M->ClimbingState().IsValid());F.Reset();F.P->TryClimb(Reason);F.P->Survival->SetHealth(0);TestTrue(TEXT("Death clears wall mode and disables movement"),!F.M->IsWallClimbing() && F.M->MovementMode==MOVE_None);
    F.Fixture.ForwardErrorMessages(this);return true;
}
#endif
