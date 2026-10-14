#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Tests/PFWallClimbFixture.h"
#include "Inventory/PFInventoryComponent.h"
#include "Misc/AutomationTest.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFAutomaticClimbTest,"PF.Traversal.AutomaticClimb",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FPFAutomaticClimbTest::RunTest(const FString&)
{
    AddExpectedMessage(TEXT("GetSocketInfoByName.*No SkeletalMesh for Component"),EAutomationExpectedMessageFlags::Contains,0);FPFWallFixture F;if(!F.Start()){return false;}
    auto* Wall=F.Box({220,0,400},{150,500,400});F.Reset();F.P->Jump();bool Attached=false;
    for(int32 N=0;N<25;++N){if(N==6){F.P->StopJumping();}F.Tick(1,F.M->IsWallClimbing()?FVector::UpVector:FVector::ForwardVector);if(F.M->IsWallClimbing()){Attached=true;break;}}
    TestTrue(TEXT("Actual paid forward jump contact automatically attaches"),Attached);TestTrue(TEXT("Jump pays20 then climb pays only elapsed drain"),F.P->Survival->GetVitals().Stamina<=80 && F.P->Survival->GetVitals().Stamina>75);
    F.P->StopJumping();F.Tick(20,FVector::UpVector);TestTrue(TEXT("Attached jump transitions to continuous ascent"),F.M->IsWallClimbing() && F.P->Survival->GetVitals().Stamina<75);
    FString Reason;TestFalse(TEXT("One paid jump cannot attach twice"),F.P->TryAutomaticClimb(Reason));
    F.Reset(20);F.P->Jump();F.Tick(6,FVector::ForwardVector);F.P->StopJumping();F.Tick(10,FVector::ForwardVector);TestTrue(TEXT("Jump spending final stamina cannot attach"),!F.M->IsWallClimbing() && F.P->Survival->GetVitals().Stamina==0);
    F.Reset();F.M->SetMovementMode(MOVE_Falling);F.M->Velocity={0,0,100};TestFalse(TEXT("Walking off/rising without paid jump never arms automatic attach"),F.P->TryAutomaticClimb(Reason));
    F.Reset();F.PC->SetCraftingMenuOpen(true);F.P->Jump();F.Tick(10,FVector::ForwardVector);F.P->StopJumping();TestTrue(TEXT("Modal blocks paid jump and climb"),F.P->Survival->GetVitals().Stamina==100 && !F.M->IsWallClimbing());F.PC->SetCraftingMenuOpen(false);
    F.Reset();auto* I=F.PC->GetInventory();I->Grant(TEXT("Item_Wood"),66);F.P->Jump();F.Tick(10,FVector::ForwardVector);F.P->StopJumping();TestTrue(TEXT("Weight cutoff blocks attachment/jump"),F.P->Survival->GetVitals().Stamina==100 && !F.M->IsWallClimbing());I->RemoveItem(TEXT("Item_Wood"),66);
    F.Reset();F.P->Jump();F.Tick(1);F.P->StopJumping();F.P->SetRole(ROLE_AutonomousProxy);TestFalse(TEXT("Client cannot consume server automatic token"),F.P->TryAutomaticClimb(Reason));F.P->SetRole(ROLE_Authority);F.P->CancelTraversal();TestFalse(TEXT("Cancel clears pending token"),F.P->TryAutomaticClimb(Reason));
    F.Fixture.ForwardErrorMessages(this);AddInfo(TEXT("[PrimalAgentTools] Actual automatic paid-jump attach and continuous ascent; empty/menu/weight/client/token guards verified"));return true;
}
#endif
