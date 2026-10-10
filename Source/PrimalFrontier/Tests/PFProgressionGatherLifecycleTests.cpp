#if WITH_DEV_AUTOMATION_TESTS
#include "Progression/PFProgressionComponent.h"
#include "Inventory/PFInventoryPlayerState.h"
#include "Inventory/PFInventoryComponent.h"
#include "Crafting/PFResourceNode.h"
#include "Crafting/PFCraftingCatalog.h"
#include "Survival/PFSurvivalGameMode.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFSurvivorCharacter.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Survival/PFInteraction.h"
#include "Persistence/PFWorldPersistence.h"
#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/BoxComponent.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "Tests/AutomationCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFGatherLifecycleTest,"PF.Progression.GatherLifecycle",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFGatherLifecycleTest::RunTest(const FString&)
{
    AddExpectedMessage(TEXT("GetSocketInfoByName.*No SkeletalMesh for Component"),EAutomationExpectedMessageFlags::Contains,0);
    FTestWorldWrapper Fixture;
    if(!Fixture.CreateTestWorld(EWorldType::Game)){return false;}
    auto* W=Fixture.GetTestWorld();
    W->SetGameMode(FURL(nullptr,TEXT("/Engine/Maps/Entry?game=/Script/PrimalFrontier.PFSurvivalGameMode"),TRAVEL_Absolute));
    auto* Floor=W->SpawnActor<AActor>();
    auto* Box=NewObject<UBoxComponent>(Floor);
    Floor->SetRootComponent(Box);Box->SetBoxExtent(FVector(4000,4000,25));
    Box->SetCollisionProfileName(TEXT("BlockAll"));Box->RegisterComponent();Floor->SetActorLocation(FVector(0,0,-25));
    W->SpawnActor<APlayerStart>(FVector(-500,0,120),FRotator::ZeroRotator);
    if(!Fixture.BeginPlayInTestWorld()){return false;}
    auto* PC=W->SpawnActor<APFSurvivalPlayerController>();
    W->GetAuthGameMode<APFSurvivalGameMode>()->RestartPlayer(PC);
    auto* PS=PC->GetPlayerState<APFInventoryPlayerState>();
    auto* P=Cast<APFSurvivorCharacter>(PC->GetPawn());
    if(!PS || !P || !PS->Progression || !PS->Inventory){return false;}
    auto* G=PS->Progression.Get();auto* I=PS->Inventory.Get();FString Error;
    // Isolate the timer test from movement/starvation. Keep normal catalogs, bag limits,
    // gather cooldown, regrowth and reward duration; no grants, record seeds or clock writes.
    P->GetCharacterMovement()->DisableMovement();
    P->Survival->HungerDrainPerSecond=0;P->Survival->ThirstDrainPerSecond=0;
    PC->SetControlRotation(FRotator::ZeroRotator);
    W->GetSubsystem<UPFWorldPersistence>()->ActiveSlot.Reset();
    ON_SCOPE_EXIT{W->GetWorldSettings()->SetPauserPlayerState(nullptr);};
    FVector Eye;FRotator Look;P->GetActorEyesViewPoint(Eye,Look);
    const FTransform At(FRotator::ZeroRotator,Eye+Look.Vector()*150);
    auto* Node=W->SpawnActorDeferred<APFResourceNode>(APFResourceNode::StaticClass(),At,P,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
    if(!Node){return false;}
    Node->ResourceId=TEXT("Node_Wood");Node->FinishSpawning(At);
    const auto* Definition=Node->Catalog?Node->Catalog->Resource(Node->ResourceId,Node->Items):nullptr;
    if(!TestNotNull(TEXT("Normal wood definition loaded"),Definition)){return false;}
    if(!TestTrue(TEXT("Default three hits, two yield, twenty-second regrowth"),Definition->Hits==3 && Definition->YieldPerHit==2 && Definition->RespawnSeconds==20)){return false;}
    auto Cooldown=[&](){for(int32 N=0;N<6;++N){Fixture.TickTestWorld(0.1f);}};
    auto TickUntil=[&](double Deadline)
    {
        // Exercise regular UWorld/actor timers in bounded steps, not TimeSeconds assignment.
        // Unreal clamps large frame deltas in WorldSettings. Use normal sub-clamp
        // frames and budget enough ticks for the full default 1800-second window.
        for(int32 N=0;N<20000 && W->GetTimeSeconds()<Deadline;++N)
        {
            const double Before=W->GetTimeSeconds();
            const float Step=static_cast<float>(FMath::Clamp(Deadline-Before,0.01,0.2));
            Fixture.TickTestWorld(Step);
            if(!TestTrue(TEXT("Unpaused world tick advances game time"),W->GetTimeSeconds()>Before)){return false;}
        }
        return TestTrue(TEXT("Ordinary ticks reached deadline"),W->GetTimeSeconds()>=Deadline);
    };
    auto Gather=[&](int32 XP,int32 Wood,int32 Hits)
    {
        return TestTrue(TEXT("Normal owned trace selects node"),PFInteraction::FindTarget(P)==Node) &&
            TestTrue(TEXT("Normal finite gathering accepts"),Node->Gather(P)) &&
            TestTrue(TEXT("Exact earned XP, inventory and node hits"),G->GetExperience()==XP && I->Count(TEXT("Item_Wood"))==Wood && Node->HitsRemaining==Hits);
    };
    Cooldown();
    TestTrue(TEXT("Fresh progression and bag"),G->GetExperience()==0 && I->GetStacks().IsEmpty() && G->GetRecord().GatherWindows.IsEmpty());
    const double FirstAt=W->GetTimeSeconds();
    if(!Gather(5,2,2)){return false;}
    TestFalse(TEXT("Immediate cooldown gives no duplicate credit"),Node->Gather(P));
    TestTrue(TEXT("Refused cooldown conserves items/hits/XP"),G->GetExperience()==5 && I->Count(TEXT("Item_Wood"))==2 && Node->HitsRemaining==2);
    Cooldown();if(!Gather(10,4,1)){return false;}
    Cooldown();if(!Gather(15,6,0)){return false;}
    const double RegrowthAt=Node->RespawnAt;
    TestTrue(TEXT("Depletion starts unchanged twenty-second deadline"),FMath::IsNearlyEqual(RegrowthAt-UPFInventoryComponent::ServerTime(W),20.0,0.00001));
    TestFalse(TEXT("Depleted node cannot yield or reward"),Node->Gather(P));
    FPFProgressionRecord BeforePause,AfterPause;
    if(!TestTrue(TEXT("Capture genuinely earned budget"),G->Capture(BeforePause,Error))){AddError(Error);return false;}
    const double PauseAt=W->GetTimeSeconds();
    W->GetWorldSettings()->SetPauserPlayerState(PS);
    for(int32 N=0;N<40;++N){Fixture.TickTestWorld(1.0f);}
    TestEqual(TEXT("Paused Unreal game time stays frozen"),W->GetTimeSeconds(),PauseAt);
    TestTrue(TEXT("Paused duration cannot trigger regrowth or inventory"),Node->HitsRemaining==0 && Node->RespawnAt==RegrowthAt && I->Count(TEXT("Item_Wood"))==6);
    TestTrue(TEXT("Pause preserves captured earned credit/duration"),G->Capture(AfterPause,Error) && AfterPause.Experience==15 && AfterPause.GatherWindows==BeforePause.GatherWindows);
    W->GetWorldSettings()->SetPauserPlayerState(nullptr);
    if(!TickUntil(RegrowthAt-0.5)){return false;}
    TestTrue(TEXT("No early regrowth"),Node->HitsRemaining==0 && Node->RespawnAt==RegrowthAt);
    if(!TickUntil(RegrowthAt+0.5)){return false;}
    TestTrue(TEXT("Normal actor tick refills finite hits once without XP/items"),Node->HitsRemaining==3 && Node->RespawnAt==0 && G->GetExperience()==15 && I->Count(TEXT("Item_Wood"))==6);
    if(!Gather(20,8,2)){return false;}
    Cooldown();if(!Gather(25,10,1)){return false;}
    Cooldown();if(!Gather(25,12,0)){return false;}
    FPFProgressionRecord Exhausted;
    if(!TestTrue(TEXT("Five credits exhausted, sixth action still yields"),G->Capture(Exhausted,Error) && Exhausted.GatherWindows.Num()==1 && Exhausted.GatherWindows[0].Rewards==5)){return false;}
    TestTrue(TEXT("Regrowth never refreshes original reward window"),FMath::IsNearlyEqual(Exhausted.GatherWindows[0].RemainingSeconds,1800-(W->GetTimeSeconds()-FirstAt),0.00001));
    if(!TickUntil(Node->RespawnAt+0.5)){return false;}
    TestTrue(TEXT("Second natural refill does not grant XP/items"),Node->HitsRemaining==3 && G->GetExperience()==25 && I->Count(TEXT("Item_Wood"))==12);
    if(!TickUntil(FirstAt+1799)){return false;}
    FPFProgressionRecord LastSecond;
    if(!TestTrue(TEXT("Budget remains exhausted just before default thirty minutes"),G->Capture(LastSecond,Error) && LastSecond.GatherWindows.Num()==1 && LastSecond.GatherWindows[0].Rewards==5 && LastSecond.GatherWindows[0].RemainingSeconds>0 && LastSecond.GatherWindows[0].RemainingSeconds<=1.01)){return false;}
    if(!Gather(25,14,2)){return false;}
    if(!TickUntil(FirstAt+1800.25)){return false;}
    FPFProgressionRecord Expired;
    TestTrue(TEXT("Ordinary thirty-minute active ticks expire only budget, not XP"),G->Capture(Expired,Error) && Expired.GatherWindows.IsEmpty() && Expired.Experience==25 && Expired.Knowledge.IsEmpty() && Expired.CreditedCrafts.IsEmpty());
    if(!Gather(30,16,1)){return false;}
    FPFProgressionRecord Renewed;
    if(!TestTrue(TEXT("First accepted gather after real expiry earns one fresh credit"),G->Capture(Renewed,Error) && Renewed.GatherWindows.Num()==1 && Renewed.GatherWindows[0].Rewards==1 && Renewed.GatherWindows[0].RemainingSeconds==1800)){return false;}
    TestFalse(TEXT("Fresh window cooldown still rejects duplicate"),Node->Gather(P));
    TestTrue(TEXT("Fresh refusal preserves exact window/items/hits"),G->Capture(AfterPause,Error) && AfterPause.GatherWindows==Renewed.GatherWindows && G->GetExperience()==30 && I->Count(TEXT("Item_Wood"))==16 && Node->HitsRemaining==1);
    const double RenewedAt=W->GetTimeSeconds();
    Cooldown();if(!Gather(35,18,0)){return false;}
    TestTrue(TEXT("Second credit ages instead of extending fresh window"),G->Capture(AfterPause,Error) && AfterPause.GatherWindows.Num()==1 && AfterPause.GatherWindows[0].Rewards==2 && FMath::IsNearlyEqual(AfterPause.GatherWindows[0].RemainingSeconds,1800-(W->GetTimeSeconds()-RenewedAt),0.00001));
    TestTrue(TEXT("Only normal wood actions supplied the entire bag"),I->GetStacks().Num()==1 && I->GetStacks()[0].ItemId==TEXT("Item_Wood") && I->GetStacks()[0].Quantity==18 && I->GetStacks()[0].ExpiresAt==0 && G->GetRecord().CreditedCrafts.IsEmpty() && G->GetRecord().Knowledge.IsEmpty());
    Fixture.ForwardErrorMessages(this);
    AddInfo(TEXT("[PrimalAgentTools] GatherLifecycle: real empty-bag/zero-XP owned gathering, default20s natural regrowth, actual40 paused world ticks, ordinary1800 active game seconds, exhausted finite yield and fresh5XP credit/cooldown conservation. No grants/seeded records/direct clock writes/shortened timers/private files; accelerated native time is not human pacing or multiplayer acceptance."));
    return true;
}
#endif
