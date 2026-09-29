#include "Building/PFBuildingComponent.h"
#include "Building/PFBuildPiece.h"
#include "Inventory/PFInventoryComponent.h"
#include "Inventory/PFItemCatalog.h"
#include "Survival/PFSurvivalGameMode.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFSurvivorCharacter.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerState.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFBuildingTest,"PF.Building.PlacementAndStorage",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFBuildingTest::RunTest(const FString&)
{
    AddExpectedMessage(TEXT("GetSocketInfoByName.*No SkeletalMesh for Component"),EAutomationExpectedMessageFlags::Contains,0);
    FTestWorldWrapper F;if(!F.CreateTestWorld(EWorldType::Game)){return false;}auto* W=F.GetTestWorld();
    W->SetGameMode(FURL(nullptr,TEXT("/Engine/Maps/Entry?game=/Script/PrimalFrontier.PFSurvivalGameMode"),TRAVEL_Absolute));
    auto* Ground=W->SpawnActor<AStaticMeshActor>(FVector(0,0,-25),FRotator::ZeroRotator);Ground->GetStaticMeshComponent()->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube")));Ground->SetActorScale3D(FVector(40,40,0.5));Ground->GetStaticMeshComponent()->SetCollisionProfileName(TEXT("BlockAll"));
    W->SpawnActor<APlayerStart>(FVector(-450,0,100),FRotator::ZeroRotator);if(!F.BeginPlayInTestWorld()){return false;}
    auto* PC=W->SpawnActor<APFSurvivalPlayerController>();W->GetAuthGameMode<APFSurvivalGameMode>()->RestartPlayer(PC);auto* P=Cast<APFSurvivorCharacter>(PC->GetPawn());auto* B=PC->Building.Get();auto* I=PC->GetInventory();if(!P || !I){return false;}
    P->GetCharacterMovement()->DisableMovement();B->Catalog=NewObject<UPFBuildingCatalog>(B);I->Catalog=NewObject<UPFItemCatalog>(I);
    auto Aim=[&](FVector At){FVector Eye;FRotator R;P->GetActorEyesViewPoint(Eye,R);PC->SetControlRotation((At-Eye).Rotation());};
    Aim(FVector(0,0,0));TestNull(TEXT("Missing costs rejects"),B->Place(TEXT("Build_Foundation"),0));I->Grant(TEXT("Item_Wood"),30);
    TestNull(TEXT("Unknown piece rejects"),B->Place(TEXT("Build_Invalid"),0));TestNull(TEXT("Invalid rotation rejects"),B->Place(TEXT("Build_Foundation"),4));
    auto* Base=B->Place(TEXT("Build_Foundation"),0);if(!TestNotNull(TEXT("Grounded foundation placed"),Base)){AddError(B->Feedback);return false;}
    TestEqual(TEXT("Cost paid exactly"),I->Count(TEXT("Item_Wood")),28);TestNull(TEXT("Duplicate footprint rejected"),B->Place(TEXT("Build_Foundation"),0));
    Aim(FVector(0,0,20));auto* Wall=B->Place(TEXT("Build_Wall"),0);if(!TestNotNull(TEXT("Supported wall placed"),Wall)){AddError(B->Feedback);return false;}
    Aim(FVector(0,0,20));TestFalse(TEXT("Supporting foundation cannot demolish"),B->Demolish(Base));
    auto* Door=B->Place(TEXT("Build_Door"),1);if(!TestNotNull(TEXT("Rotated door/frame placed"),Door)){AddError(B->Feedback);return false;}
    TestTrue(TEXT("Owned door opens"),Door->ToggleDoor(PC->PlayerState));TestTrue(TEXT("Door state changed"),Door->bDoorOpen);
    auto* Other=W->SpawnActor<APlayerState>();TestFalse(TEXT("Other player cannot open"),Door->ToggleDoor(Other));
    Aim(Wall->GetActorLocation());auto* Roof=B->Place(TEXT("Build_Ceiling"),0);TestNotNull(TEXT("Roof supported by wall"),Roof);
    if(Roof){TestEqual(TEXT("Roof elevation"),Roof->GetActorLocation().Z,330.0);Roof->Destroy();}
    Aim(FVector(0,0,20));auto* Chest=B->Place(TEXT("Build_Storage"),0);if(!TestNotNull(TEXT("Storage on foundation"),Chest)){AddError(B->Feedback);return false;}
    Aim(Chest->GetActorLocation());TestTrue(TEXT("Open owned storage"),B->Interact(Chest));
    I->Grant(TEXT("Item_Food"),2);const auto Food=*I->GetStacks().FindByPredicate([](const auto& S){return S.ItemId==TEXT("Item_Food");});
    TestFalse(TEXT("Invalid quantity"),B->Transfer(true,Food.StackId,-1));TestTrue(TEXT("Deposit one"),B->Transfer(true,Food.StackId,1));
    TestEqual(TEXT("No duplication bag"),I->Count(TEXT("Item_Food")),1);TestEqual(TEXT("No duplication storage"),Chest->Storage->Count(TEXT("Item_Food")),1);
    TestEqual(TEXT("Freshness unchanged"),Chest->Storage->GetStacks()[0].ExpiresAt,Food.ExpiresAt);TestFalse(TEXT("Occupied storage cannot demolish"),B->Demolish(Chest));
    const auto Stored=Chest->Storage->GetStacks()[0];I->WeightLimit=0.1f;TestFalse(TEXT("Full bag leaves storage intact"),B->Transfer(false,Stored.StackId,1));I->WeightLimit=30;
    TestEqual(TEXT("Failed transfer conserves item"),Chest->Storage->Count(TEXT("Item_Food")),1);TestTrue(TEXT("Withdraw"),B->Transfer(false,Stored.StackId,1));TestFalse(TEXT("Replay withdrawal rejected"),B->Transfer(false,Stored.StackId,1));
    Chest->Builder=Other;TestFalse(TEXT("Unauthorized demolition"),B->Demolish(Chest));TestFalse(TEXT("Unauthorized storage"),B->Transfer(true,Food.StackId,1));TestEqual(TEXT("Unauthorized damage"),Chest->TakeDamage(25,FDamageEvent(),PC,P),0.f);Chest->Builder=PC->PlayerState;
    TestEqual(TEXT("Owner damage"),Chest->TakeDamage(25,FDamageEvent(),PC,P),25.f);TestEqual(TEXT("Health replicated value"),Chest->Health,75.f);
    TestTrue(TEXT("Empty storage demolishes"),B->Demolish(Chest));Aim(FVector(0,0,20));TestNotNull(TEXT("Rebuild same location"),B->Place(TEXT("Build_Storage"),0));
    PC->SetRole(ROLE_AutonomousProxy);TestNull(TEXT("Client placement denied"),B->Place(TEXT("Build_Foundation"),0));TestFalse(TEXT("Client demolition denied"),B->Demolish(Wall));PC->SetRole(ROLE_Authority);
    Aim(FVector(1500,0,0));TestNull(TEXT("Out of range denied"),B->Place(TEXT("Build_Foundation"),0));
    AddInfo(TEXT("[PrimalBuilding] Placement, snapping, rotation, cost, support, ownership, doors, damage, demolition and storage conservation checked."));F.ForwardErrorMessages(this);return true;
}
#endif
