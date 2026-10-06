// PFCreatureTests.cpp
//
// Automation test: PF.Creatures.Lifecycle   (M6, 0c2d935)
// Covers: catalog validation, perception of a living survivor and blocking by an
// obstacle, passive flee / hostile chase decisions, damage authority and clamping,
// death state, exactly one perishable loot drop, and the 8-creature world cap.
// Real navmesh movement is covered separately by the live test PF.Creatures.Live.
// Run: Session Frontend > Automation, filter "PF.Creatures".

#include "Creatures/PFCreature.h"
#include "Creatures/PFCreatureSpawner.h"
#include "Survival/PFSurvivalGameMode.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFSurvivorCharacter.h"
#include "Inventory/PFItemPickup.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerStart.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFCreatureTest,"PF.Creatures.Lifecycle",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFCreatureTest::RunTest(const FString&)
{
    AddExpectedMessage(TEXT("GetSocketInfoByName.*No SkeletalMesh for Component"),EAutomationExpectedMessageFlags::Contains,0);
    FTestWorldWrapper F;
    if(!F.CreateTestWorld(EWorldType::Game)){return false;}
    auto* W=F.GetTestWorld();
    W->SetGameMode(FURL(nullptr,TEXT("/Engine/Maps/Entry?game=/Script/PrimalFrontier.PFSurvivalGameMode"),TRAVEL_Absolute));
    W->SpawnActor<APlayerStart>(FVector(-200,0,100),FRotator::ZeroRotator);
    if(!F.BeginPlayInTestWorld()){return false;}
    auto* PC=W->SpawnActor<APFSurvivalPlayerController>();
    W->GetAuthGameMode<APFSurvivalGameMode>()->RestartPlayer(PC);
    auto* P=Cast<APFSurvivorCharacter>(PC->GetPawn());
    if(!P){return false;}
    P->GetCharacterMovement()->DisableMovement();

    // Catalog validation (including a deliberately invalid loot value).
    auto* Catalog=NewObject<UPFCreatureCatalog>(W);
    TestNotNull(TEXT("Passive definition"),Catalog->Find(TEXT("Creature_Forager")));
    TestNotNull(TEXT("Hostile definition"),Catalog->Find(TEXT("Creature_Prowler")));
    TestNull(TEXT("Unknown definition"),Catalog->Find(TEXT("Creature_Unknown")));
    Catalog->Creatures[0].FoodLoot=-1;
    TestNull(TEXT("Invalid loot rejected"),Catalog->Find(TEXT("Creature_Forager")));
    Catalog->Creatures[0].FoodLoot=2;

    // Spawn helper: creatures are frozen in place (no navmesh in this fixture).
    auto Make=[&](FName Id,FVector At)
    {
        FTransform T(At);
        auto* C=W->SpawnActorDeferred<APFCreature>(APFCreature::StaticClass(),T,nullptr,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
        C->CreatureId=Id;
        C->Catalog=Catalog;
        C->FinishSpawning(T);
        C->GetCharacterMovement()->DisableMovement();
        return C;
    };

    // Perception and decisions.
    auto* Passive=Make(TEXT("Creature_Forager"),FVector(100,0,100));
    TestEqual(TEXT("Initial health"),Passive->Health,60.f);
    TestTrue(TEXT("Living player perceived"),Passive->CanSee(P));
    Passive->Think();
    TestEqual(TEXT("Passive flees"),Passive->State.ToString(),FString(TEXT("Creature.State.Flee")));
    auto* Obstacle=W->SpawnActor<AActor>(FVector(-50,0,100),FRotator::ZeroRotator);
    auto* Box=NewObject<UBoxComponent>(Obstacle);
    Obstacle->SetRootComponent(Box);
    Box->SetBoxExtent(FVector(30,200,200));
    Box->SetCollisionProfileName(TEXT("BlockAll"));
    Box->RegisterComponent();
    Obstacle->SetActorLocation(FVector(-50,0,100));
    TestFalse(TEXT("Solid obstacle blocks perception"),Passive->CanSee(P));
    Obstacle->Destroy();
    auto* Hostile=Make(TEXT("Creature_Prowler"),FVector(100,300,100));
    Hostile->Think();
    TestEqual(TEXT("Hostile chases"),Hostile->State.ToString(),FString(TEXT("Creature.State.Chase")));

    // Damage authority, clamping and death.
    TestEqual(TEXT("Negative damage rejected"),Passive->TakeDamage(-1,FDamageEvent(),PC,P),0.f);
    Passive->SetRole(ROLE_SimulatedProxy);
    TestEqual(TEXT("Client damage rejected"),Passive->TakeDamage(20,FDamageEvent(),PC,P),0.f);
    Passive->SetRole(ROLE_Authority);
    TestEqual(TEXT("Server damage"),Passive->TakeDamage(20,FDamageEvent(),PC,P),20.f);
    TestEqual(TEXT("Health reduced"),Passive->Health,40.f);
    TestEqual(TEXT("Lethal damage clamped"),Passive->TakeDamage(1000,FDamageEvent(),PC,P),40.f);
    TestEqual(TEXT("Death state"),Passive->State.ToString(),FString(TEXT("Creature.State.Dead")));
    TestEqual(TEXT("Repeated kill ignored"),Passive->TakeDamage(100,FDamageEvent(),PC,P),0.f);

    // Exactly one loot pickup with the forager's 2 food and a future deadline.
    int32 LootCount=0;
    for(TActorIterator<APFItemPickup> It(W);It;++It)
    {
        ++LootCount;
        TestEqual(TEXT("Finite food loot"),It->GetContents().Quantity,2);
        TestTrue(TEXT("Loot expires"),It->GetContents().ExpiresAt>UPFInventoryComponent::ServerTime(W));
    }
    TestEqual(TEXT("One death produces one drop"),LootCount,1);

    // World cap: 2 existing + 6 more = 8 (the corpse counts); a ninth is refused.
    for(int32 N=0;N<6;++N){Make(TEXT("Creature_Forager"),FVector(1000,N*150,100));}
    TestEqual(TEXT("Eight total creatures"),APFCreatureSpawner::Count(W),8);
    TestNull(TEXT("Cap prevents ninth spawn"),APFCreatureSpawner::Spawn(W,TEXT("Creature_Forager"),FVector(0,1000,0),Catalog));

    AddInfo(TEXT("[PrimalCreatures] Catalog, perception, state, damage authority, death, unique perishable loot and spawn cap verified; live navigation is a separate test."));
    F.ForwardErrorMessages(this);
    return true;
}
#endif
