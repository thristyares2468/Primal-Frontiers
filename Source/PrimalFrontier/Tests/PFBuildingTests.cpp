// PFBuildingTests.cpp
//
// Automation test: PF.Building.PlacementAndStorage   (M5, 5702d4b)
// Covers: missing cost, unknown piece, bad rotation, grid-snapped foundation and
// exact wood charge, duplicate footprint, supported wall/door/roof, door ownership,
// storage open/deposit/withdraw with no duplication and unchanged freshness,
// full-bag refusal, occupied/unauthorized demolition, owner damage, rebuild,
// client-role refusal and out-of-range refusal.
// Runs in a transient game world with the real survival GameMode/controller/pawn.
// Run: Session Frontend > Automation, filter "PF.Building" (or -ExecCmds="Automation RunTests PF.Building").

#include "Building/PFBuildingComponent.h"
#include "Building/PFBuildPiece.h"
#include "Inventory/PFInventoryComponent.h"
#include "Inventory/PFInventoryPlayerState.h"
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
    // The native survivor has no skeletal mesh in this fixture, so socket lookups warn harmlessly.
    AddExpectedMessage(TEXT("GetSocketInfoByName.*No SkeletalMesh for Component"),EAutomationExpectedMessageFlags::Contains,0);

    // Fixture: game world, flat 40 m ground, one PlayerStart, one possessed survivor.
    FTestWorldWrapper F;
    if(!F.CreateTestWorld(EWorldType::Game)){return false;}
    auto* W=F.GetTestWorld();
    W->SetGameMode(FURL(nullptr,TEXT("/Engine/Maps/Entry?game=/Script/PrimalFrontier.PFSurvivalGameMode"),TRAVEL_Absolute));
    auto* Ground=W->SpawnActor<AStaticMeshActor>(FVector(0,0,-25),FRotator::ZeroRotator);
    Ground->GetStaticMeshComponent()->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube")));
    Ground->SetActorScale3D(FVector(40,40,0.5));
    Ground->GetStaticMeshComponent()->SetCollisionProfileName(TEXT("BlockAll"));
    W->SpawnActor<APlayerStart>(FVector(-450,0,100),FRotator::ZeroRotator);
    if(!F.BeginPlayInTestWorld()){return false;}
    auto* PC=W->SpawnActor<APFSurvivalPlayerController>();
    W->GetAuthGameMode<APFSurvivalGameMode>()->RestartPlayer(PC);
    auto* P=Cast<APFSurvivorCharacter>(PC->GetPawn());
    auto* B=PC->Building.Get();
    auto* I=PC->GetInventory();
    if(!P || !I){return false;}
    P->GetCharacterMovement()->DisableMovement();
    // Fresh in-memory catalogs so the test doesn't depend on saved asset edits.
    B->Catalog=NewObject<UPFBuildingCatalog>(B);
    I->Catalog=NewObject<UPFItemCatalog>(I);
    // Aim the survivor's view at a world point (placement is derived from the view).
    auto Aim=[&](FVector At)
    {
        FVector Eye;
        FRotator R;
        P->GetActorEyesViewPoint(Eye,R);
        PC->SetControlRotation((At-Eye).Rotation());
    };

    // Costs, IDs and rotation validation.
    Aim(FVector(0,0,0));
    TestNull(TEXT("Missing costs rejects"),B->Place(TEXT("Build_Foundation"),0));
    I->Grant(TEXT("Item_Wood"),30);
    TestNull(TEXT("Unknown piece rejects"),B->Place(TEXT("Build_Invalid"),0));
    TestNull(TEXT("Invalid rotation rejects"),B->Place(TEXT("Build_Foundation"),4));

    // Foundation, then a supported wall and a rotated door.
    auto* Base=B->Place(TEXT("Build_Foundation"),0);
    if(!TestNotNull(TEXT("Grounded foundation placed"),Base)){AddError(B->Feedback);return false;}
    TestEqual(TEXT("Cost paid exactly"),I->Count(TEXT("Item_Wood")),28);
    TestNull(TEXT("Duplicate footprint rejected"),B->Place(TEXT("Build_Foundation"),0));
    Aim(FVector(0,0,20));
    auto* Wall=B->Place(TEXT("Build_Wall"),0);
    if(!TestNotNull(TEXT("Supported wall placed"),Wall)){AddError(B->Feedback);return false;}
    Aim(FVector(0,0,20));
    TestFalse(TEXT("Supporting foundation cannot demolish"),B->Demolish(Base));
    auto* Door=B->Place(TEXT("Build_Door"),1);
    if(!TestNotNull(TEXT("Rotated door/frame placed"),Door)){AddError(B->Feedback);return false;}
    TestTrue(TEXT("Owned door opens"),Door->ToggleDoor(PC->PlayerState));
    TestTrue(TEXT("Door state changed"),Door->bDoorOpen);
    auto* Other=W->SpawnActor<APFInventoryPlayerState>();
    TestTrue(TEXT("Separate server-issued ownership identities"),
        Other->PersistentPlayerId.IsValid() && Other->PersistentPlayerId != PC->GetPlayerState<APFInventoryPlayerState>()->PersistentPlayerId);
    TestFalse(TEXT("Other player cannot open"),Door->ToggleDoor(Other));

    // Roof snaps 320 cm above the wall's platform (foundation at z=10 -> roof at z=330).
    Aim(Wall->GetActorLocation());
    auto* Roof=B->Place(TEXT("Build_Ceiling"),0);
    TestNotNull(TEXT("Roof supported by wall"),Roof);
    if(Roof)
    {
        TestEqual(TEXT("Roof elevation"),Roof->GetActorLocation().Z,330.0);
        Roof->Destroy();
    }

    // Storage: open, deposit, conservation and freshness.
    Aim(FVector(0,0,20));
    auto* Chest=B->Place(TEXT("Build_Storage"),0);
    if(!TestNotNull(TEXT("Storage on foundation"),Chest)){AddError(B->Feedback);return false;}
    Aim(Chest->GetActorLocation());
    TestTrue(TEXT("Open owned storage"),B->Interact(Chest));
    I->Grant(TEXT("Item_Food"),2);
    const auto Food=*I->GetStacks().FindByPredicate([](const auto& S){return S.ItemId==TEXT("Item_Food");});
    TestFalse(TEXT("Invalid quantity"),B->Transfer(true,Food.StackId,-1));
    TestTrue(TEXT("Deposit one"),B->Transfer(true,Food.StackId,1));
    TestEqual(TEXT("No duplication bag"),I->Count(TEXT("Item_Food")),1);
    TestEqual(TEXT("No duplication storage"),Chest->Storage->Count(TEXT("Item_Food")),1);
    TestEqual(TEXT("Freshness unchanged"),Chest->Storage->GetStacks()[0].ExpiresAt,Food.ExpiresAt);
    TestFalse(TEXT("Occupied storage cannot demolish"),B->Demolish(Chest));

    // Withdraw: a full bag refuses without losing the stored item; replay is rejected.
    const auto Stored=Chest->Storage->GetStacks()[0];
    I->WeightLimit=0.1f;
    TestFalse(TEXT("Full bag leaves storage intact"),B->Transfer(false,Stored.StackId,1));
    I->WeightLimit=30;
    TestEqual(TEXT("Failed transfer conserves item"),Chest->Storage->Count(TEXT("Item_Food")),1);
    TestTrue(TEXT("Withdraw"),B->Transfer(false,Stored.StackId,1));
    TestFalse(TEXT("Replay withdrawal rejected"),B->Transfer(false,Stored.StackId,1));

    // Ownership checks: hand over both the stable ownership key and current presentation pointer.
    const FGuid OriginalOwnerId=Chest->PersistentOwnerId;
    Chest->Builder=Other;
    Chest->PersistentOwnerId=Other->PersistentPlayerId;
    TestFalse(TEXT("Unauthorized demolition"),B->Demolish(Chest));
    TestFalse(TEXT("Unauthorized storage"),B->Transfer(true,Food.StackId,1));
    TestEqual(TEXT("Unauthorized damage"),Chest->TakeDamage(25,FDamageEvent(),PC,P),0.f);
    Chest->Builder=PC->PlayerState;
    Chest->PersistentOwnerId=OriginalOwnerId;

    // A reconnect creates a new PlayerState but preserves its server-assigned ownership key.
    auto* Reconnected=W->SpawnActor<APFInventoryPlayerState>();
    Reconnected->PersistentPlayerId=OriginalOwnerId;
    TestTrue(TEXT("Ownership survives replacement PlayerState"),Chest->IsOwnedBy(Reconnected));
    TestFalse(TEXT("Another stable identity is refused"),Chest->IsOwnedBy(Other));
    Chest->BindPersistentOwner(Reconnected);
    TestEqual(TEXT("Reconnect rebinds builder"),Chest->Builder.Get(),static_cast<APlayerState*>(Reconnected));
    Chest->BindPersistentOwner(Other);
    TestEqual(TEXT("Wrong identity cannot rebind"),Chest->Builder.Get(),static_cast<APlayerState*>(Reconnected));
    Chest->BindPersistentOwner(PC->PlayerState);

    // Owner damage, demolition of empty storage, and rebuilding in the same spot.
    TestEqual(TEXT("Owner damage"),Chest->TakeDamage(25,FDamageEvent(),PC,P),25.f);
    TestEqual(TEXT("Health replicated value"),Chest->Health,75.f);
    TestTrue(TEXT("Empty storage demolishes"),B->Demolish(Chest));
    Aim(FVector(0,0,20));
    TestNotNull(TEXT("Rebuild same location"),B->Place(TEXT("Build_Storage"),0));

    // Authority and reach.
    PC->SetRole(ROLE_AutonomousProxy);
    TestNull(TEXT("Client placement denied"),B->Place(TEXT("Build_Foundation"),0));
    TestFalse(TEXT("Client demolition denied"),B->Demolish(Wall));
    PC->SetRole(ROLE_Authority);
    Aim(FVector(1500,0,0));
    TestNull(TEXT("Out of range denied"),B->Place(TEXT("Build_Foundation"),0));

    AddInfo(TEXT("[PrimalBuilding] Placement, snapping, rotation, cost, support, ownership, doors, damage, demolition and storage conservation checked."));
    F.ForwardErrorMessages(this);
    return true;
}
#endif
