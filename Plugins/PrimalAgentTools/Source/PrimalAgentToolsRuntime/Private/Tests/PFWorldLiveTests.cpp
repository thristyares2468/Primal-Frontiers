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
    explicit FPFWorldLiveExercise(FAutomationTestBase* T):Test(T),Started(FPlatformTime::Seconds()){FParse::Value(FCommandLine::Get(),TEXT("PFExpectedPlayers="),Expected);}
    bool Update() override
    {
        const double Now=FPlatformTime::Seconds();if(Stage==99){return Now-Changed>(bClient?3:15);}
        if(Now-Started>120){Test->AddError(FString::Printf(TEXT("[PrimalWorld] Live timeout stage=%d"),Stage));return true;}
        UWorld* W=nullptr;for(const auto& Context:GEngine->GetWorldContexts()){if(Context.World() && Context.World()->IsGameWorld()){W=Context.World();break;}}if(!W){return false;}
        bClient=W->GetNetMode()==NM_Client;TArray<APFSurvivalPlayerController*> Players;for(auto It=W->GetPlayerControllerIterator();It;++It){auto* P=Cast<APFSurvivalPlayerController>(It->Get());if(P && P->GetPawn() && P->GetInventory()){Players.Add(P);}}
        if(Players.Num()!=(bClient?1:Expected)){return false;}
        APFWorldClock* Clock=nullptr;int32 Clocks=0,Nodes=0,Hazards=0,Creatures=0,Buildings=0;APFResourceNode* Wood=nullptr;APFResourceNode* Stone=nullptr;
        TArray<APFResourceNode*> FibreNodes,WaterNodes;TSet<FName> ResourceKinds;
        for(TActorIterator<APFWorldClock> It(W);It;++It){Clock=*It;++Clocks;}
        for(TActorIterator<APFResourceNode> It(W);It;++It){++Nodes;if(It->GetActorLocation().Y<0){if(It->ResourceId==TEXT("Node_Wood")){Wood=*It;}if(It->ResourceId==TEXT("Node_Stone")){Stone=*It;}}}
        for(TActorIterator<APFResourceNode> It(W);It;++It){ResourceKinds.Add(It->ResourceId);if(It->ResourceId==TEXT("Node_Fibre")){FibreNodes.Add(*It);}if(It->ResourceId==TEXT("Node_Water")){WaterNodes.Add(*It);}}
        FibreNodes.Sort([](const APFResourceNode& A,const APFResourceNode& B){return A.GetActorLocation().X<B.GetActorLocation().X;});
        WaterNodes.Sort([](const APFResourceNode& A,const APFResourceNode& B){return A.GetActorLocation().X<B.GetActorLocation().X;});
        for(TActorIterator<APFSurvivalHazard> It(W);It;++It){++Hazards;}for(TActorIterator<APFCreature> It(W);It;++It){++Creatures;}for(TActorIterator<APFBuildPiece> It(W);It;++It){++Buildings;}
        if(Now-LastDiagnostic>5){LastDiagnostic=Now;Test->AddInfo(FString::Printf(TEXT("[PrimalWorld] Observe client=%d stage=%d clocks=%d nodes=%d creatures=%d buildings=%d hour=%.2f"),bClient,Stage,Clocks,Nodes,Creatures,Buildings,Clock?Clock->Hour:-1));}
        if(!Clock || Nodes!=15 || Creatures!=2 || FibreNodes.Num()!=2 || WaterNodes.Num()!=2){return false;}
        auto* PC=Players[0];auto* V=PC->GetPawn()->FindComponentByClass<UPFPlayerSurvivalComponent>();if(!V){return false;}
        auto Advance=[&](int32 S){Stage=S;Changed=Now;Test->AddInfo(FString::Printf(TEXT("[PrimalWorld] %s stage=%d"),bClient?TEXT("Client"):TEXT("Server"),S));};
        auto Aim=[&](FVector Target){FVector Eye;FRotator Look;PC->GetPawn()->GetActorEyesViewPoint(Eye,Look);PC->SetControlRotation((Target-Eye).Rotation());};
        if(bClient)
        {
            if(Stage==0 && Clock->Hour>=22 && Clock->Phase.ToString()==TEXT("World.Time.Night"))
            {
                Test->TestEqual(TEXT("Unique replicated map clock"),Clocks,1);Test->TestEqual(TEXT("Hazard loaded"),Hazards,1);Test->TestEqual(TEXT("Five resource kinds replicated"),ResourceKinds.Num(),5);
                Test->TestFalse(TEXT("Direct client clock mutation denied"),Clock->SetHour(1));
                PC->SetPauseMenuOpen(true);Test->TestTrue(TEXT("Client menu opens"),PC->IsPauseMenuOpen());Test->TestFalse(TEXT("Multiplayer world remains unpaused"),W->IsPaused());PC->SetPauseMenuOpen(false);Test->TestFalse(TEXT("Client menu resumes"),PC->IsPauseMenuOpen());
                Test->TestTrue(TEXT("Developer clock mutation denied on client"),PF::AgentTools::ExecuteCommand(TEXT("PF.SetTimeOfDay"),{TEXT("1")},W,false).HasErrors());Advance(1);
            }
            if(Stage==1 && Clock->Hour>=9 && Clock->Hour<10 && Clock->Phase.ToString()==TEXT("World.Time.Day") && Buildings==1 && PC->GetInventory()->Count(TEXT("Item_Water"))>=2 && FMath::IsNearlyEqual(V->GetVitals().Thirst,10.f))
            {
                const auto* Stack=PC->GetInventory()->GetStacks().FindByPredicate([](const FPFItemStack& S){return S.ItemId==TEXT("Item_Water");});
                if(!Stack){return false;}WaterId=Stack->StackId;InitialWater=PC->GetInventory()->Count(TEXT("Item_Water"));
                Test->TestTrue(TEXT("Fibre replicated to owner"),PC->GetInventory()->Count(TEXT("Item_Fibre"))>=2);
                Test->TestFalse(TEXT("Client direct water consume denied"),PC->GetInventory()->Consume(WaterId,PC->GetPawn()));
                PC->ServerInventoryAction(WaterId,2,2);Advance(2);
            }
            else if(Stage==2 && Now-Changed>0.6)
            {Test->TestEqual(TEXT("Invalid consume quantity did not spend water"),PC->GetInventory()->Count(TEXT("Item_Water")),InitialWater);PC->ServerInventoryAction(WaterId,2,1);Advance(3);}
            else if(Stage==3 && PC->GetInventory()->Count(TEXT("Item_Water"))==InitialWater-1 && FMath::IsNearlyEqual(V->GetVitals().Thirst,45.f))
            {Test->AddInfo(TEXT("[PrimalWorld] Fifteen resources/five kinds, zones, replicated clock/building/fibre/water and validated drink RPC verified."));PF::AgentTools::ExecuteCommand(TEXT("PF.ExportTestReport"),{TEXT("M7WorldClient")},W);Advance(99);}
            return false;
        }
        if(Stage==0)
        {
            int32 Starts=0;for(TActorIterator<APlayerStart> It(W);It;++It){++Starts;Test->TestTrue(TEXT("Start faces horizontally into arena"),FMath::IsNearlyZero(It->GetActorRotation().Pitch) && FMath::IsNearlyEqual(It->GetActorRotation().Yaw,90.f));}Test->TestEqual(TEXT("Two valid starts"),Starts,2);
            Test->TestEqual(TEXT("Unique map clock"),Clocks,1);Test->TestEqual(TEXT("One exposure region"),Hazards,1);
            for(FVector End:{FVector(0,1800,10),FVector(1000,4400,10),FVector(-1800,2800,90),FVector(-1950,4300,10),FVector(1500,1400,10),FVector(-1300,700,10)})
            {auto* Path=UNavigationSystemV1::FindPathToLocationSynchronously(W,FVector(0,-1100,10),End);Test->TestTrue(TEXT("Connected navigation between zones and rise"),Path && Path->IsValid() && !Path->IsPartial());}
            Clock->DayLengthSeconds=86400;Clock->SetHour(22);V->HungerDrainPerSecond=0;V->ThirstDrainPerSecond=0;
            PC->GetPawn()->SetActorLocation(FVector(0,3400,100));Advance(1);return false;
        }
        if(Stage==1 && Now-Changed>2)
        {Test->TestTrue(TEXT("Map danger applies exposure"),V->GetVitals().Exposure>0);PC->GetPawn()->SetActorLocation(FVector(-400,-800,100));Aim(Wood->GetActorLocation());Advance(2);return false;}
        if(Stage==2 && Now-Changed>0.7)
        {
            Test->TestEqual(TEXT("Leaving hazard clears exposure"),V->GetVitals().Exposure,0.f);Aim(Wood->GetActorLocation());Test->TestTrue(TEXT("Server gathers map wood"),Wood->Gather(PC->GetPawn()));++Hits;Changed=Now;
            if(Hits==3){Test->TestEqual(TEXT("Gathered six wood from map"),PC->GetInventory()->Count(TEXT("Item_Wood")),6);PC->GetPawn()->SetActorLocation(FVector(0,-700,100));Aim(Stone->GetActorLocation());Advance(3);}return false;
        }
        if(Stage==3 && Now-Changed>0.7)
        {Aim(Stone->GetActorLocation());Test->TestTrue(TEXT("Server gathers map stone"),Stone->Gather(PC->GetPawn()));Test->TestEqual(TEXT("Gathered two stone from map"),PC->GetInventory()->Count(TEXT("Item_Stone")),2);Test->TestTrue(TEXT("Craft using gathered inputs"),PC->GetCrafting()->Start(TEXT("Recipe_Tool"),PC->GetPawn()));Advance(4);return false;}
        if(Stage==4 && Now-Changed>6)
        {
            Test->TestEqual(TEXT("Tool completed"),PC->GetInventory()->Count(TEXT("Item_Tool")),1);
            PC->GetPawn()->SetActorLocation(FVector(-1200,0,100));Aim(FVector(-800,0,0));
            Test->TestNotNull(TEXT("Build using remaining gathered wood"),PC->Building->Place(TEXT("Build_Foundation"),0));Advance(5);return false;
        }
        if(Stage==5 && Now-Changed>2)
        {
            for(int32 Index=0;Index<Players.Num();++Index)
            {
                auto* P=Players[Index];auto* Needs=P->GetPawn()->FindComponentByClass<UPFPlayerSurvivalComponent>();Needs->HungerDrainPerSecond=0;Needs->ThirstDrainPerSecond=0;
                auto* Node=FibreNodes[Index];P->GetPawn()->SetActorLocation(Node->GetActorLocation()+FVector(0,-180,40));
                FVector Eye;FRotator Look;P->GetPawn()->GetActorEyesViewPoint(Eye,Look);P->SetControlRotation((Node->GetActorLocation()-Eye).Rotation());
                Test->TestTrue(TEXT("Gather fibre in woodland"),Node->Gather(P->GetPawn()));
            }
            Advance(6);return false;
        }
        if(Stage==6 && Now-Changed>0.7)
        {
            for(int32 Index=0;Index<Players.Num();++Index)
            {
                auto* P=Players[Index];auto* Node=WaterNodes[Index];P->GetPawn()->SetActorLocation(Node->GetActorLocation()+FVector(0,-180,40));
                FVector Eye;FRotator Look;P->GetPawn()->GetActorEyesViewPoint(Eye,Look);P->SetControlRotation((Node->GetActorLocation()-Eye).Rotation());
                Test->TestTrue(TEXT("Collect water from world"),Node->Gather(P->GetPawn()));P->GetPawn()->FindComponentByClass<UPFPlayerSurvivalComponent>()->SetThirst(10);
            }
            Test->TestTrue(TEXT("Server time command succeeds"),!PF::AgentTools::ExecuteCommand(TEXT("PF.SetTimeOfDay"),{TEXT("9")},W).HasErrors());Advance(7);return false;
        }
        if(Stage==7)
        {
            for(auto* P:Players){const int32 ExpectedWater=P->GetInventory()->Count(TEXT("Item_Tool"))>0?3:1;if(P->GetInventory()->Count(TEXT("Item_Water"))!=ExpectedWater || !FMath::IsNearlyEqual(P->GetPawn()->FindComponentByClass<UPFPlayerSurvivalComponent>()->GetVitals().Thirst,45.f)){return false;}}
            Test->AddInfo(TEXT("[PrimalWorld] All owners consumed exactly one collected water portion through validated RPCs."));PF::AgentTools::ExecuteCommand(TEXT("PF.ExportTestReport"),{TEXT("M7WorldServer")},W);Advance(99);
        }
        return false;
    }
private:
    FAutomationTestBase* Test;double Started,Changed=0,LastDiagnostic=0;int32 Stage=0,Expected=1,Hits=0,InitialWater=0;FGuid WaterId;bool bClient=false;
};
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFWorldLiveTest,"PF.World.Live",EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)
bool FPFWorldLiveTest::RunTest(const FString&)
{if(!FParse::Param(FCommandLine::Get(),TEXT("PFRunWorldLiveTests"))){AddError(TEXT("Requires isolated L_M7SurvivalArena -PFRunWorldLiveTests -PFExpectedPlayers=1|2"));return false;}ADD_LATENT_AUTOMATION_COMMAND(FPFWorldLiveExercise(this));return true;}
#endif
