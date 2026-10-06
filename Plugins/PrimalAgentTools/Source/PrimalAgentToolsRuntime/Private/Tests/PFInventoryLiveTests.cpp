// PFInventoryLiveTests.cpp
//
// Live network test: PF.Inventory.Live   (M3, 340c538)
// Runs in a real dedicated server plus 1-2 client processes on the M3 map. The same
// latent command runs in every process; server and client walk matching stages:
//
//   Stage 0  Server: disable needs drain, place players, grant 6 wood and 2 food
//              that expire in 80 s.
//            Client: direct Grant()/RemoveItem() and PF.GiveItem are refused; send
//              a garbage inventory action (unknown code 255, huge quantity).
//   Stage 1  Client: after 2 s, the garbage action changed nothing; split 3 wood off.
//            Server: once every player has two wood stacks, grant 1 stone as a marker.
//   Stage 2  Client: drop 1 food (action 1).
//            Server: after 20 s, food is back to 2 (drop + pickup) and 1 wood is removed.
//   Stage 3  Client: the dropped pickup keeps the food's freshness deadline; pick it
//              up with Interact.
//            Server: wait until every player's food has expired.
//   Stage 4  Client: the round trip kept the original deadline.
//            Server: 5 s later, set an 8 s respawn delay and kill every player.
//   Stage 5  Both: wait for death (client) and respawn (server).
//   Stage 6  Both: wood (5) and stone (1) survived death; the client also checks that
//              other players' inventories are not replicated to it (owner-only).
//   Stage 7  Linger 10 s, then finish.
//
// Launch with: -PFRunInventoryLiveTests -PFExpectedPlayers=1|2.
// Inventory action codes used here: 0 split, 1 drop (see UPFInventoryComponent).

#include "PFCommands.h"
#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Inventory/PFInventoryComponent.h"
#include "Inventory/PFInventoryPlayerState.h"
#include "Inventory/PFItemPickup.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFSurvivalGameMode.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
class FPFInventoryLiveExercise final : public IAutomationLatentCommand
{
public:
    explicit FPFInventoryLiveExercise(FAutomationTestBase* In):Test(In),Started(FPlatformTime::Seconds())
    {
        FParse::Value(FCommandLine::Get(),TEXT("PFExpectedPlayers="),Expected);
    }

    /** Called every frame by the automation framework; returning true ends the test. */
    bool Update() override
    {
        const double Now=FPlatformTime::Seconds();
        if(Now-Started>240){Test->AddError(FString::Printf(TEXT("[PrimalInventory] Live timeout stage %d"),Stage));return true;}

        // Find the running game world and wait for all expected players to be ready.
        UWorld* W=nullptr;
        for(const auto& C:GEngine->GetWorldContexts()){if(C.World() && C.World()->IsGameWorld()){W=C.World();break;}}
        if(!W){return false;}
        const bool Client=W->GetNetMode()==NM_Client;
        TArray<APFSurvivalPlayerController*> Players;
        for(auto It=W->GetPlayerControllerIterator();It;++It)
        {
            if(auto* PC=Cast<APFSurvivalPlayerController>(It->Get()))
            {
                if(PC->GetPawn() && PC->GetInventory()){Players.Add(PC);}
            }
        }
        if(Players.Num()!=(Client?1:Expected)){return false;}

        // True when Fn holds for every player's inventory.
        auto All=[&](auto Fn)
        {
            for(auto* PC:Players){if(!Fn(PC->GetInventory())){return false;}}
            return true;
        };
        auto* PC=Players[0];
        auto* I=PC->GetInventory();
        auto* S=PC->GetPawn()->FindComponentByClass<UPFPlayerSurvivalComponent>();
        if(!S){return false;}
        auto Log=[&](const TCHAR* Name)
        {
            Test->AddInfo(FString::Printf(TEXT("[PrimalInventory] %s stage=%s wood=%d food=%d stone=%d"),Client?TEXT("Client"):TEXT("Server"),Name,I->Count(TEXT("Item_Wood")),I->Count(TEXT("Item_Food")),I->Count(TEXT("Item_Stone"))));
        };

        if(Stage==0)
        {
            if(Client)
            {
                // Wait for the server grant, then probe client authority.
                if(I->Count(TEXT("Item_Wood"))!=6 || I->Count(TEXT("Item_Food"))!=2){return false;}
                Test->TestFalse(TEXT("Client cannot invent inventory"),I->Grant(TEXT("Item_Wood"),10));
                Test->TestFalse(TEXT("Client cannot remove items directly"),I->RemoveItem(TEXT("Item_Wood"),1));
                const auto Result=PF::AgentTools::ExecuteCommand(TEXT("PF.GiveItem"),{TEXT("Item_Wood"),TEXT("10")},W,false);
                Test->TestTrue(TEXT("Client developer grant rejected"),Result.Issues.ContainsByPredicate([](const auto& E){return E.Code==TEXT("NotAuthority");}));
                PC->ServerInventoryAction(FGuid::NewGuid(),255,MAX_int32);  // unknown stack, unknown action
                for(const auto& Entry:I->GetStacks()){if(Entry.ItemId==TEXT("Item_Food")){FoodDeadline=Entry.ExpiresAt;}}
            }
            else
            {
                int32 Index=0;
                for(auto* P:Players)
                {
                    auto* Needs=P->GetPawn()->FindComponentByClass<UPFPlayerSurvivalComponent>();
                    Needs->HungerDrainPerSecond=0;
                    Needs->ThirstDrainPerSecond=0;
                    P->GetPawn()->SetActorLocation(FVector(-400,800+Index++*350,120));
                    P->SetControlRotation(FRotator::ZeroRotator);
                    Test->TestTrue(TEXT("Server initial grant"),P->GetInventory()->Grant(TEXT("Item_Wood"),6));
                    Test->TestTrue(TEXT("Server fresh food batch"),P->GetInventory()->AddExisting(TEXT("Item_Food"),2,UPFInventoryComponent::ServerTime(W)+80));
                }
            }
            Log(TEXT("initial"));
            Stage=1;
            Changed=Now;
        }
        else if(Stage==1)
        {
            if(Client)
            {
                if(Now-Changed<2){return false;}
                Test->TestEqual(TEXT("Invalid request cannot create or destroy items"),I->Count(TEXT("Item_Wood")),6);
                for(const auto& Entry:I->GetStacks()){if(Entry.ItemId==TEXT("Item_Wood")){PC->ServerInventoryAction(Entry.StackId,0,3);break;}}
                Stage=2;
                Changed=Now;
            }
            else
            {
                // Every player must have split their wood into two stacks.
                if(!All([](auto* Inventory){int32 N=0;for(const auto& E:Inventory->GetStacks()){N+=E.ItemId==TEXT("Item_Wood");}return N==2;})){return false;}
                for(auto* P:Players){Test->TestTrue(TEXT("Marker after all real split RPCs"),P->GetInventory()->Grant(TEXT("Item_Stone"),1));}
                Log(TEXT("client-splits"));
                Stage=2;
                Changed=Now;
            }
        }
        else if(Stage==2)
        {
            if(Client)
            {
                if(I->Count(TEXT("Item_Stone"))!=1 || Now-Changed<0.5){return false;}
                for(const auto& Entry:I->GetStacks()){if(Entry.ItemId==TEXT("Item_Food")){PC->ServerInventoryAction(Entry.StackId,1,1);break;}}
                Stage=3;
            }
            else if(Now-Changed>20)
            {
                for(auto* P:Players)
                {
                    Test->TestEqual(TEXT("Client drop/pickup conserves food"),P->GetInventory()->Count(TEXT("Item_Food")),2);
                    P->GetInventory()->RemoveItem(TEXT("Item_Wood"),1);
                }
                Stage=3;
            }
        }
        else if(Stage==3)
        {
            if(Client)
            {
                // The dropped food must appear within 2 m and carry the same deadline.
                if(I->Count(TEXT("Item_Food"))!=1){return false;}
                bool Found=false;
                for(TActorIterator<APFItemPickup> It(W);It;++It)
                {
                    if(It->GetContents().ItemId==TEXT("Item_Food") && FVector::Dist(It->GetActorLocation(),PC->GetPawn()->GetActorLocation())<200)
                    {
                        Test->TestEqual(TEXT("World drop retains replicated deadline"),It->GetContents().ExpiresAt,FoodDeadline);
                        Found=true;
                    }
                }
                if(!Found){return false;}
                PC->Interact();
                Stage=4;
                Log(TEXT("drop-request"));
            }
            else if(All([](auto* Inventory){return Inventory->Count(TEXT("Item_Food"))==0;}))
            {
                // All food has now passed its 80 s deadline.
                Stage=4;
                Changed=Now;
            }
        }
        else if(Stage==4)
        {
            if(Client)
            {
                if(I->Count(TEXT("Item_Food"))!=2){return false;}
                Test->TestEqual(TEXT("Inventory round trip keeps deadline"),I->GetStacks().FindByPredicate([](const auto& E){return E.ItemId==TEXT("Item_Food");})->ExpiresAt,FoodDeadline);
                Log(TEXT("pickup-roundtrip"));
                Stage=5;
            }
            else if(Now-Changed>5)
            {
                // Kill everyone to prove the inventory survives death and respawn.
                W->GetAuthGameMode<APFSurvivalGameMode>()->RespawnDelay=8;
                for(auto* P:Players){P->GetPawn()->FindComponentByClass<UPFPlayerSurvivalComponent>()->ApplyDamage(1000);}
                Stage=5;
                Log(TEXT("expiry-and-death"));
            }
        }
        else if(Stage==5)
        {
            if(Client)
            {
                if(I->Count(TEXT("Item_Food"))!=0 || !S->IsDead()){return false;}
                Test->TestEqual(TEXT("All old food expired after transfers"),I->Count(TEXT("Item_Food")),0);
                Stage=6;
            }
            else
            {
                for(auto* P:Players){if(P->GetPawn()->FindComponentByClass<UPFPlayerSurvivalComponent>()->IsDead()){return false;}}
                Stage=6;
            }
        }
        else if(Stage==6)
        {
            // The inventory lives on the PlayerState, so it outlives the pawn.
            if(S->IsDead()){return false;}
            Test->TestEqual(TEXT("Wood retained after death/respawn"),I->Count(TEXT("Item_Wood")),5);
            Test->TestEqual(TEXT("Stone retained after death/respawn"),I->Count(TEXT("Item_Stone")),1);
            if(Client)
            {
                int32 Others=0;
                for(TActorIterator<APFInventoryPlayerState> It(W);It;++It)
                {
                    if(*It!=PC->PlayerState)
                    {
                        Test->TestEqual(TEXT("Other players' private inventory is not replicated"),It->Inventory->GetStacks().Num(),0);
                        ++Others;
                    }
                }
                Test->TestEqual(TEXT("Other player states present"),Others,Expected-1);
            }
            Log(TEXT("respawn-retained"));
            Test->TestFalse(TEXT("Report exported"),PF::AgentTools::ExecuteCommand(TEXT("PF.ExportTestReport"),{TEXT("M3Live")},W).HasErrors());
            Stage=7;
            Changed=Now;
        }
        return Stage==7 && Now-Changed>10;
    }

private:
    FAutomationTestBase* Test;
    double Started,Changed=0,FoodDeadline=0;
    int32 Stage=0,Expected=1;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFInventoryLiveTest,"PF.Inventory.Live",EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)
bool FPFInventoryLiveTest::RunTest(const FString&)
{
    // Opt-in only: running this in an ordinary editor session would mutate the world.
    if(!FParse::Param(FCommandLine::Get(),TEXT("PFRunInventoryLiveTests"))){AddError(TEXT("Requires isolated M3 session -PFRunInventoryLiveTests -PFExpectedPlayers=1|2"));return false;}
    ADD_LATENT_AUTOMATION_COMMAND(FPFInventoryLiveExercise(this));
    return true;
}
#endif
