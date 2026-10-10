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
#include "Creatures/PFCreatureSpawnCatalog.h"
#include "Survival/PFSurvivalGameMode.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFSurvivorCharacter.h"
#include "Inventory/PFItemPickup.h"
#if WITH_DEV_AUTOMATION_TESTS
#include <limits>
#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerStart.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFCreatureTest,"PF.Creatures.Lifecycle",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFCreatureSpawnPolicyTest,"PF.Creatures.SpawnPolicy",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFCreatureSpawnPolicyTest::RunTest(const FString&)
{
    auto* Creatures=NewObject<UPFCreatureCatalog>();
    auto* Spawns=NewObject<UPFCreatureSpawnCatalog>();
    const auto Original=Spawns->Policies;
    const auto Tag=[](const TCHAR* Name){return FGameplayTag::RequestGameplayTag(Name);};
    const auto Shore=Tag(TEXT("Ecology.Biome.Shore"));
    const auto Woodland=Tag(TEXT("Ecology.Biome.Woodland"));
    const auto Ridge=Tag(TEXT("Ecology.Biome.Ridge"));
    const auto Day=Tag(TEXT("World.Time.Day"));
    const auto Night=Tag(TEXT("World.Time.Night"));
    const FName Forager(TEXT("Creature_Forager")), Prowler(TEXT("Creature_Prowler"));
    FString Error;
    TestTrue(TEXT("Known creature policy defaults validate"),Spawns->Validate(*Creatures,Error));
    TestTrue(TEXT("Success clears error"),Error.IsEmpty());
    int32 Residents=0;
    for(const auto& Policy:Spawns->Policies){Residents+=Policy.MaximumResidents;}
    TestEqual(TEXT("Proposed policies reserve seven of eight slots"),Residents,7);
    for(int32 Roll=0;Roll<4;++Roll)
    {
        FName Id;
        TestTrue(TEXT("Day weighted choice"),Spawns->Choose(*Creatures,Woodland,Day,Roll,Id,Error));
        TestEqual(TEXT("Day three forager tickets and one prowler"),Id,Roll<3?Forager:Prowler);
        TestTrue(TEXT("Night weighted choice"),Spawns->Choose(*Creatures,Woodland,Night,Roll,Id,Error));
        TestEqual(TEXT("Night one forager ticket and three prowler"),Id,Roll<1?Forager:Prowler);
        FName Repeat;
        TestTrue(TEXT("Same policy/phase/roll repeats"),Spawns->Choose(*Creatures,Woodland,Night,Roll,Repeat,Error));
        TestEqual(TEXT("Deterministic choice has no mutable random state"),Repeat,Id);
    }
    FName Id;
    TestTrue(TEXT("Shore remains forager"),Spawns->Choose(*Creatures,Shore,Day,0,Id,Error));
    TestEqual(TEXT("Shore definition"),Id,Forager);
    TestTrue(TEXT("Ridge night prowler"),Spawns->Choose(*Creatures,Ridge,Night,0,Id,Error));
    TestEqual(TEXT("Ridge definition"),Id,Prowler);
    const FName Sentinel(TEXT("PreservedOutput"));
    const auto Refused=[&](FGameplayTag Biome,FGameplayTag Phase,int32 Roll)
    {
        Id=Sentinel;
        TestFalse(TEXT("Invalid or empty selection refuses"),Spawns->Choose(*Creatures,Biome,Phase,Roll,Id,Error));
        TestEqual(TEXT("Refusal preserves output"),Id,Sentinel);
        TestFalse(TEXT("Explicit refusal reason"),Error.IsEmpty());
    };
    Refused(Ridge,Day,0); // A dormant day phase is valid data, not a fallback spawn.
    Refused(Woodland,Day,-1); Refused(Woodland,Day,4); Refused(Woodland,Day,MAX_int32);
    Refused(FGameplayTag(),Day,0); Refused(Woodland,FGameplayTag(),0); Refused(Woodland,Shore,0);
    const auto Invalid=[&]()
    {
        TestFalse(TEXT("Whole invalid catalog rejected"),Spawns->Validate(*Creatures,Error));
        Refused(Woodland,Day,0); // Unrelated malformed rows cannot be hidden by selecting a valid row.
        Spawns->Policies=Original;
    };
    Spawns->Policies.Reset(); Invalid();
    Spawns->Policies.Add(Original[0]); Invalid();
    Spawns->Policies[0].Biome=Day; Invalid();
    Spawns->Policies[0].Biome=Tag(TEXT("Ecology.Biome")); Invalid();
    Spawns->Policies[0].Biome=FGameplayTag(); Invalid();
    Spawns->Policies[0].MaximumResidents=0; Invalid();
    Spawns->Policies[0].MaximumResidents=9; Invalid();
    Spawns->Policies[0].MaximumResidents=4; Invalid(); // Combined budget would be nine.
    Spawns->Policies[0].RespawnSeconds=-1; Invalid();
    Spawns->Policies[0].RespawnSeconds=301; Invalid();
    Spawns->Policies[0].RespawnSeconds=std::numeric_limits<float>::quiet_NaN(); Invalid();
    Spawns->Policies[0].Entries.Reset(); Invalid();
    Spawns->Policies[0].Entries.Add(Original[0].Entries[0]); Invalid();
    Spawns->Policies[0].Entries[0].CreatureId=NAME_None; Invalid();
    Spawns->Policies[0].Entries[0].CreatureId=TEXT("Creature_Unknown"); Invalid();
    Spawns->Policies[0].Entries[0].DayWeight=-1; Invalid();
    Spawns->Policies[0].Entries[0].NightWeight=101; Invalid();
    Spawns->Policies[0].Entries[0].DayWeight=0; Spawns->Policies[0].Entries[0].NightWeight=0; Invalid();
    Creatures->Creatures[0].FoodLoot=-1; Invalid();
    Creatures->Creatures[0].FoodLoot=2;
    TestTrue(TEXT("Original data still valid after refusals"),Spawns->Validate(*Creatures,Error));
    TestTrue(TEXT("No actor/map is required or modified"),Spawns->Choose(*Creatures,Woodland,Day,0,Id,Error));
    return !HasAnyErrors();
}

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
