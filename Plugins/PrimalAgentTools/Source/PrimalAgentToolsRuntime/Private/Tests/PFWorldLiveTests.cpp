// PFWorldLiveTests.cpp
//
// Live network test: PF.World.Live   (M7, b5a3165)
// Runs on L_M7SurvivalArena in a real server plus 1-2 clients. Waits until the map
// has loaded 9 resource nodes and 2 creatures, then:
//
//   Server stage 0: two PlayerStarts face into the arena; one clock, one hazard;
//     navigation connects the zones and the rise; set the clock to 22:00 and slow
//     it to a 24 h day; disable needs drain; move the player into the hazard.
//   Server 1: exposure applies; move out next to a wood node.
//   Server 2: exposure clears; gather the node three times (6 wood).
//   Server 3: gather stone (2) and start crafting the tool from gathered inputs.
//   Server 4: tool completed; place a foundation with the remaining wood.
//   Server 5: PF.SetTimeOfDay 9 succeeds; export a report.
//   Client 0: sees night 22:00+, one replicated clock and hazard; can't set the
//     clock directly or through the PF command; the pause menu opens without
//     pausing the multiplayer world.
//   Client 1: sees day 09:00 and the server-built foundation; export a report.
//
// Launch with: -PFRunWorldLiveTests -PFExpectedPlayers=1|2.

#include "World/PFWorldClock.h"
#include "Crafting/PFResourceNode.h"
#include "Crafting/PFCraftingComponent.h"
#include "Creatures/PFCreature.h"
#include "Building/PFBuildPiece.h"
#include "Building/PFBuildingComponent.h"
#include "Survival/PFSurvivalHazard.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "PFCommands.h"
#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "GameFramework/PlayerStart.h"
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
class FPFWorldLiveExercise : public IAutomationLatentCommand
{
public:
    explicit FPFWorldLiveExercise(FAutomationTestBase* T):Test(T),Started(FPlatformTime::Seconds())
    {
        FParse::Value(FCommandLine::Get(),TEXT("PFExpectedPlayers="),Expected);
    }

    bool Update() override
    {
        const double Now=FPlatformTime::Seconds();
        if(Stage==99){return Now-Changed>(bClient?3:15);}
        if(Now-Started>120){Test->AddError(FString::Printf(TEXT("[PrimalWorld] Live timeout stage=%d"),Stage));return true;}

        // World and players.
        UWorld* W=nullptr;
        for(const auto& Context:GEngine->GetWorldContexts()){if(Context.World() && Context.World()->IsGameWorld()){W=Context.World();break;}}
        if(!W){return false;}
        bClient=W->GetNetMode()==NM_Client;
        TArray<APFSurvivalPlayerController*> Players;
        for(auto It=W->GetPlayerControllerIterator();It;++It)
        {
            auto* P=Cast<APFSurvivalPlayerController>(It->Get());
            if(P && P->GetPawn() && P->GetInventory()){Players.Add(P);}
        }
        if(Players.Num()!=(bClient?1:Expected)){return false;}

        // Inventory of the arena contents (wood/stone nodes in the southern half, Y<0).
        APFWorldClock* Clock=nullptr;
        int32 Clocks=0,Nodes=0,Hazards=0,Creatures=0,Buildings=0;
        APFResourceNode* Wood=nullptr;
        APFResourceNode* Stone=nullptr;
        for(TActorIterator<APFWorldClock> It(W);It;++It){Clock=*It;++Clocks;}
        for(TActorIterator<APFResourceNode> It(W);It;++It)
        {
            ++Nodes;
            if(It->GetActorLocation().Y<0)
            {
                if(It->ResourceId==TEXT("Node_Wood")){Wood=*It;}
                if(It->ResourceId==TEXT("Node_Stone")){Stone=*It;}
            }
        }
        for(TActorIterator<APFSurvivalHazard> It(W);It;++It){++Hazards;}
        for(TActorIterator<APFCreature> It(W);It;++It){++Creatures;}
        for(TActorIterator<APFBuildPiece> It(W);It;++It){++Buildings;}
        if(Now-LastDiagnostic>5)
        {
            LastDiagnostic=Now;
            Test->AddInfo(FString::Printf(TEXT("[PrimalWorld] Observe client=%d stage=%d clocks=%d nodes=%d creatures=%d buildings=%d hour=%.2f"),bClient,Stage,Clocks,Nodes,Creatures,Buildings,Clock?Clock->Hour:-1));
        }
        if(!Clock || Nodes!=9 || Creatures!=2){return false;}  // wait for the full arena to replicate
        auto* PC=Players[0];
        auto* V=PC->GetPawn()->FindComponentByClass<UPFPlayerSurvivalComponent>();
        if(!V){return false;}
        auto Advance=[&](int32 S)
        {
            Stage=S;
            Changed=Now;
            Test->AddInfo(FString::Printf(TEXT("[PrimalWorld] %s stage=%d"),bClient?TEXT("Client"):TEXT("Server"),S));
        };
        auto Aim=[&](FVector Target)
        {
            FVector Eye;
            FRotator Look;
            PC->GetPawn()->GetActorEyesViewPoint(Eye,Look);
            PC->SetControlRotation((Target-Eye).Rotation());
        };

        // ---------------- Client ----------------
        if(bClient)
        {
            if(Stage==0 && Clock->Hour>=22 && Clock->Phase.ToString()==TEXT("World.Time.Night"))
            {
                Test->TestEqual(TEXT("Unique replicated map clock"),Clocks,1);
                Test->TestEqual(TEXT("Hazard loaded"),Hazards,1);
                Test->TestFalse(TEXT("Direct client clock mutation denied"),Clock->SetHour(1));
                PC->SetPauseMenuOpen(true);
                Test->TestTrue(TEXT("Client menu opens"),PC->IsPauseMenuOpen());
                Test->TestFalse(TEXT("Multiplayer world remains unpaused"),W->IsPaused());
                PC->SetPauseMenuOpen(false);
                Test->TestFalse(TEXT("Client menu resumes"),PC->IsPauseMenuOpen());
                Test->TestTrue(TEXT("Developer clock mutation denied on client"),PF::AgentTools::ExecuteCommand(TEXT("PF.SetTimeOfDay"),{TEXT("1")},W,false).HasErrors());
                Advance(1);
            }
            if(Stage==1 && Clock->Hour>=9 && Clock->Hour<10 && Clock->Phase.ToString()==TEXT("World.Time.Day") && Buildings==1)
            {
                Test->AddInfo(TEXT("[PrimalWorld] Loaded nine resources/two creatures; observed server night-to-day and gathered/crafted-funded foundation replication."));
                PF::AgentTools::ExecuteCommand(TEXT("PF.ExportTestReport"),{TEXT("M7WorldClient")},W);
                Advance(99);
            }
            return false;
        }

        // ---------------- Server ----------------
        if(Stage==0)
        {
            int32 Starts=0;
            for(TActorIterator<APlayerStart> It(W);It;++It)
            {
                ++Starts;
                Test->TestTrue(TEXT("Start faces horizontally into arena"),FMath::IsNearlyZero(It->GetActorRotation().Pitch) && FMath::IsNearlyEqual(It->GetActorRotation().Yaw,90.f));
            }
            Test->TestEqual(TEXT("Two valid starts"),Starts,2);
            Test->TestEqual(TEXT("Unique map clock"),Clocks,1);
            Test->TestEqual(TEXT("One exposure region"),Hazards,1);
            // Safe start -> central resources, northern danger, and the stepped rise must all be reachable.
            for(FVector End:{FVector(0,1800,10),FVector(1000,4400,10),FVector(-1800,2800,90)})
            {
                auto* Path=UNavigationSystemV1::FindPathToLocationSynchronously(W,FVector(0,-1100,10),End);
                Test->TestTrue(TEXT("Connected navigation between zones and rise"),Path && Path->IsValid() && !Path->IsPartial());
            }
            Clock->DayLengthSeconds=86400;  // effectively freeze time at 22:00 for the client check
            Clock->SetHour(22);
            V->HungerDrainPerSecond=0;
            V->ThirstDrainPerSecond=0;
            PC->GetPawn()->SetActorLocation(FVector(0,3400,100));  // inside the northern hazard
            Advance(1);
            return false;
        }
        if(Stage==1 && Now-Changed>2)
        {
            Test->TestTrue(TEXT("Map danger applies exposure"),V->GetVitals().Exposure>0);
            PC->GetPawn()->SetActorLocation(FVector(-400,-800,100));
            Aim(Wood->GetActorLocation());
            Advance(2);
            return false;
        }
        if(Stage==2 && Now-Changed>0.7)
        {
            // Three bare-hand hits of 2 wood each (0.7 s apart to respect the 0.5 s node cooldown).
            Test->TestEqual(TEXT("Leaving hazard clears exposure"),V->GetVitals().Exposure,0.f);
            Aim(Wood->GetActorLocation());
            Test->TestTrue(TEXT("Server gathers map wood"),Wood->Gather(PC->GetPawn()));
            ++Hits;
            Changed=Now;
            if(Hits==3)
            {
                Test->TestEqual(TEXT("Gathered six wood from map"),PC->GetInventory()->Count(TEXT("Item_Wood")),6);
                PC->GetPawn()->SetActorLocation(FVector(0,-700,100));
                Aim(Stone->GetActorLocation());
                Advance(3);
            }
            return false;
        }
        if(Stage==3 && Now-Changed>0.7)
        {
            Aim(Stone->GetActorLocation());
            Test->TestTrue(TEXT("Server gathers map stone"),Stone->Gather(PC->GetPawn()));
            Test->TestEqual(TEXT("Gathered two stone from map"),PC->GetInventory()->Count(TEXT("Item_Stone")),2);
            Test->TestTrue(TEXT("Craft using gathered inputs"),PC->GetCrafting()->Start(TEXT("Recipe_Tool"),PC->GetPawn()));
            Advance(4);
            return false;
        }
        if(Stage==4 && Now-Changed>6)
        {
            // Tool used 3 of the 6 wood; the foundation uses 2 of the remaining 3.
            Test->TestEqual(TEXT("Tool completed"),PC->GetInventory()->Count(TEXT("Item_Tool")),1);
            PC->GetPawn()->SetActorLocation(FVector(-1200,0,100));
            Aim(FVector(-800,0,0));
            Test->TestNotNull(TEXT("Build using remaining gathered wood"),PC->Building->Place(TEXT("Build_Foundation"),0));
            Advance(5);
            return false;
        }
        if(Stage==5 && Now-Changed>12)
        {
            Test->TestTrue(TEXT("Server time command succeeds"),!PF::AgentTools::ExecuteCommand(TEXT("PF.SetTimeOfDay"),{TEXT("9")},W).HasErrors());
            PF::AgentTools::ExecuteCommand(TEXT("PF.ExportTestReport"),{TEXT("M7WorldServer")},W);
            Advance(99);
        }
        return false;
    }

private:
    FAutomationTestBase* Test;
    double Started,Changed=0,LastDiagnostic=0;
    int32 Stage=0,Expected=1,Hits=0;
    bool bClient=false;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFWorldLiveTest,"PF.World.Live",EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)
bool FPFWorldLiveTest::RunTest(const FString&)
{
    if(!FParse::Param(FCommandLine::Get(),TEXT("PFRunWorldLiveTests"))){AddError(TEXT("Requires isolated L_M7SurvivalArena -PFRunWorldLiveTests -PFExpectedPlayers=1|2"));return false;}
    ADD_LATENT_AUTOMATION_COMMAND(FPFWorldLiveExercise(this));
    return true;
}
#endif
