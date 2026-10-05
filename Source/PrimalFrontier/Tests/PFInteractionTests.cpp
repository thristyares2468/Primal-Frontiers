#include "Survival/PFInteraction.h"
#include "Survival/PFSurvivalGameMode.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFSurvivorCharacter.h"
#include "Inventory/PFItemPickup.h"
#include "Building/PFBuildingComponent.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFInteractionTest,"PF.Interaction.TargetAndPickup",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFInteractionTest::RunTest(const FString&)
{
    AddExpectedMessage(TEXT("GetSocketInfoByName.*No SkeletalMesh for Component"),EAutomationExpectedMessageFlags::Contains,0);
    FTestWorldWrapper F;if(!F.CreateTestWorld(EWorldType::Game)){return false;}auto* W=F.GetTestWorld();
    W->SetGameMode(FURL(nullptr,TEXT("/Engine/Maps/Entry?game=/Script/PrimalFrontier.PFSurvivalGameMode"),TRAVEL_Absolute));W->SpawnActor<APlayerStart>(FVector(0,0,150),FRotator::ZeroRotator);if(!F.BeginPlayInTestWorld()){return false;}
    auto* PC=W->SpawnActor<APFSurvivalPlayerController>();W->GetAuthGameMode<APFSurvivalGameMode>()->RestartPlayer(PC);auto* P=Cast<APFSurvivorCharacter>(PC->GetPawn());if(!P || !PC->GetInventory()){return false;}
    P->GetCharacterMovement()->DisableMovement();PC->SetControlRotation(FRotator::ZeroRotator);FVector Eye;FRotator Look;P->GetActorEyesViewPoint(Eye,Look);
    TestTrue(TEXT("Rendered camera matches authoritative eye"),P->GetFirstPersonCameraComponent()->GetComponentLocation().Equals(Eye,1));
    auto* Item=W->SpawnActor<APFItemPickup>(Eye+FVector(150,25,0),FRotator::ZeroRotator);
    TestTrue(TEXT("Small pickup aim tolerance"),PFInteraction::FindTarget(P)==Item);
    auto* Wall=W->SpawnActor<AActor>();auto* Box=NewObject<UBoxComponent>(Wall);Wall->SetRootComponent(Box);Box->SetBoxExtent(FVector(10,200,200));Box->SetCollisionProfileName(TEXT("BlockAll"));Box->RegisterComponent();Wall->SetActorLocation(Eye+FVector(75,0,0));
    TestNull(TEXT("Occluder prevents interaction"),PFInteraction::FindTarget(P));Wall->Destroy();
    Item->SetActorLocation(Eye+FVector(400,0,0));TestNull(TEXT("Out of reach excluded"),PFInteraction::FindTarget(P));TestTrue(TEXT("Advisory farther target"),PFInteraction::FindTarget(P,500)==Item);
    Item->SetActorLocation(Eye+FVector(150,0,0));Item->SetRole(ROLE_SimulatedProxy);TestFalse(TEXT("Client direct pickup denied"),Item->TryPickup(P));Item->SetRole(ROLE_Authority);
    // This transient fixture has no viewport/ULocalPlayer. Mark the controller
    // local before exercising the local input route, as a real client would be.
    PC->SetAsLocalPlayerController();
    PC->Building->bBuildMode=true;PC->Interact();TestEqual(TEXT("E picks up while building overlay is active"),PC->GetInventory()->Count(TEXT("Item_Wood")),5);TestTrue(TEXT("Successful feedback visible without opening inventory"),PC->RecentInteractionMessage().Contains(TEXT("Picked up")));
    TestFalse(TEXT("Duplicate pickup denied"),Item->TryPickup(P));
    AddInfo(TEXT("[PrimalInteraction] Camera alignment, small-target tolerance, occlusion, range, authority and build-mode pickup verified."));F.ForwardErrorMessages(this);return true;
}
#endif
