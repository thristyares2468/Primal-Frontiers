#include "PFCommands.h"
#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Crafting/PFCraftingComponent.h"
#include "Crafting/PFResourceNode.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "GameFramework/PlayerState.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
class FPFCraftingLiveExercise final : public IAutomationLatentCommand
{
public:
    explicit FPFCraftingLiveExercise(FAutomationTestBase* In):Test(In),Started(FPlatformTime::Seconds())
    {FParse::Value(FCommandLine::Get(),TEXT("PFExpectedPlayers="),Expected);}
    bool Update() override
    {
        const double Now=FPlatformTime::Seconds();
        if(Stage==99){return Now-Changed>(bClientSession?3:12);}
        if(Now-Started>180){Test->AddError(FString::Printf(TEXT("[PrimalCrafting] Live timeout stage=%d"),Stage));return true;}
        UWorld* W=nullptr;for(const auto& Context:GEngine->GetWorldContexts()){if(Context.World() && Context.World()->IsGameWorld()){W=Context.World();break;}}
        if(!W){return false;}const bool Client=W->GetNetMode()==NM_Client;bClientSession=Client;
        TArray<APFSurvivalPlayerController*> Players;
        for(auto It=W->GetPlayerControllerIterator();It;++It){auto* PC=Cast<APFSurvivalPlayerController>(It->Get());if(PC && PC->GetPawn() && PC->GetInventory() && PC->GetCrafting()){Players.Add(PC);}}
        if(Players.Num()!=(Client?1:Expected)){return false;}
        auto* PC=Players[0];auto* I=PC->GetInventory();auto* C=PC->GetCrafting();APFResourceNode* Node=nullptr;
        for(TActorIterator<APFResourceNode> It(W);It;++It){if(It->GetOwner()==PC->GetPawn()){Node=*It;break;}}
        auto Advance=[&](int32 Next){Stage=Next;Changed=Now;Test->AddInfo(FString::Printf(TEXT("[PrimalCrafting] %s stage=%d wood=%d tool=%d cooked=%d"),Client?TEXT("Client"):TEXT("Server"),Stage,I->Count(TEXT("Item_Wood")),I->Count(TEXT("Item_Tool")),I->Count(TEXT("Item_CookedFood"))));};
        if(!Client)
        {
            if(Stage==0)
            {
                int32 Index=0;
                for(auto* P:Players)
                {
                    auto* Needs=P->GetPawn()->FindComponentByClass<UPFPlayerSurvivalComponent>();Needs->HungerDrainPerSecond=0;Needs->ThirstDrainPerSecond=0;
                    const float Y=600.f+400.f*Index++;P->GetPawn()->SetActorLocation(FVector(-400,Y,100));P->SetControlRotation(FRotator::ZeroRotator);
                    FActorSpawnParameters Params;Params.Owner=P->GetPawn();W->SpawnActor<APFResourceNode>(FVector(-220,Y,130),FRotator::ZeroRotator,Params);
                    Test->TestTrue(TEXT("Server supplies stone fixture"),P->GetInventory()->Grant(TEXT("Item_Stone"),2));
                    Test->TestTrue(TEXT("Server supplies food fixture"),P->GetInventory()->Grant(TEXT("Item_Food"),2));
                }
                Advance(1);
            }
            else if(Stage==1)
            {
                for(auto* P:Players){auto* Bag=P->GetInventory();if(Bag->Count(TEXT("Item_Tool"))!=1 || Bag->Count(TEXT("Item_CookedFood"))!=1){return false;}}
                for(auto* P:Players)
                {Test->TestEqual(TEXT("Actual server RPC loop wood"),P->GetInventory()->Count(TEXT("Item_Wood")),2);Test->TestEqual(TEXT("Actual server food consumed once"),P->GetInventory()->Count(TEXT("Item_Food")),1);}
                Test->TestFalse(TEXT("Gathering integration command"),PF::AgentTools::ExecuteCommand(TEXT("PF.TestGathering"),{},W).HasErrors());
                Test->TestFalse(TEXT("Crafting integration command"),PF::AgentTools::ExecuteCommand(TEXT("PF.TestCrafting"),{},W).HasErrors());Advance(2);
            }
            else if(Stage==2)
            {
                for(TActorIterator<APFResourceNode> It(W);It;++It){if(It->GetOwner() && (It->HitsRemaining!=3 || It->RespawnAt!=0)){return false;}}
                Test->TestFalse(TEXT("Server export"),PF::AgentTools::ExecuteCommand(TEXT("PF.ExportTestReport"),{TEXT("M4LiveServer")},W).HasErrors());Advance(99);
            }
        }
        else
        {
            if(!Node){return false;}
            if(Stage==0)
            {
                if(I->Count(TEXT("Item_Stone"))!=2 || I->Count(TEXT("Item_Food"))!=2){return false;}
                Test->TestFalse(TEXT("Client direct gathering rejected"),Node->Gather(PC->GetPawn()));
                Test->TestFalse(TEXT("Client direct crafting rejected"),C->Start(TEXT("Recipe_Tool"),PC->GetPawn()));
                const auto Denied=PF::AgentTools::ExecuteCommand(TEXT("PF.Craft"),{TEXT("Recipe_Tool")},W,false);
                Test->TestTrue(TEXT("Client developer mutation rejected"),Denied.Issues.ContainsByPredicate([](const auto& E){return E.Code==TEXT("NotAuthority");}));
                PC->ServerCraftAction(TEXT("Recipe_Unknown"),false);Advance(1);
            }
            else if(Stage==1 && Now-Changed>0.6)
            {Test->TestTrue(TEXT("Invalid recipe stays idle"),C->ActiveRecipe.IsNone());PC->Interact();Advance(2);}
            else if(Stage==2 && I->Count(TEXT("Item_Wood"))==2 && Now-Changed>0.6)
            {PC->Interact();Advance(3);}
            else if(Stage==3 && I->Count(TEXT("Item_Wood"))==4)
            {PC->ServerCraftAction(TEXT("Recipe_Tool"),false);Advance(4);}
            else if(Stage==4 && C->ActiveRecipe==TEXT("Recipe_Tool") && Now-Changed>0.6)
            {PC->ServerCraftAction(TEXT("Recipe_Tool"),false);Advance(5);}
            else if(Stage==5 && I->Count(TEXT("Item_Tool"))==1 && C->ActiveRecipe.IsNone())
            {Test->TestEqual(TEXT("Tool consumed wood"),I->Count(TEXT("Item_Wood")),1);Test->TestEqual(TEXT("Tool consumed stone"),I->Count(TEXT("Item_Stone")),0);PC->Interact();Advance(6);}
            else if(Stage==6 && I->Count(TEXT("Item_Wood"))==3 && Node->HitsRemaining==0)
            {Test->TestTrue(TEXT("Replicated depletion deadline"),Node->RespawnAt>0);PC->ServerCraftAction(TEXT("Recipe_Cook"),false);Advance(7);}
            else if(Stage==7 && C->ActiveRecipe==TEXT("Recipe_Cook") && Now-Changed>0.6)
            {PC->ServerCraftAction(NAME_None,true);Advance(8);}
            else if(Stage==8 && C->ActiveRecipe.IsNone() && Now-Changed>0.6)
            {Test->TestEqual(TEXT("Cancel no cooked output"),I->Count(TEXT("Item_CookedFood")),0);Test->TestEqual(TEXT("Cancel keeps food"),I->Count(TEXT("Item_Food")),2);PC->ServerCraftAction(TEXT("Recipe_Cook"),false);Advance(9);}
            else if(Stage==9 && I->Count(TEXT("Item_CookedFood"))==1 && C->ActiveRecipe.IsNone())
            {Test->TestEqual(TEXT("Cook consumes fuel once"),I->Count(TEXT("Item_Wood")),2);Test->TestEqual(TEXT("Cook consumes food once"),I->Count(TEXT("Item_Food")),1);Advance(10);}
            else if(Stage==10 && Node->HitsRemaining==3 && Node->RespawnAt==0)
            {
                int32 OtherNodes=0;for(TActorIterator<APFResourceNode> It(W);It;++It){if(It->GetOwner() && It->GetOwner()!=PC->GetPawn()){++OtherNodes;}}
                Test->TestEqual(TEXT("Other player's resource actor replicated"),OtherNodes,Expected-1);
                Test->TestFalse(TEXT("Client export"),PF::AgentTools::ExecuteCommand(TEXT("PF.ExportTestReport"),{TEXT("M4LiveClient")},W).HasErrors());Advance(99);
            }
        }
        return false;
    }
private:
    FAutomationTestBase* Test;double Started,Changed=0;int32 Stage=0,Expected=1;bool bClientSession=false;
};
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFCraftingLiveTest,"PF.Crafting.Live",EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)
bool FPFCraftingLiveTest::RunTest(const FString&)
{
    if(!FParse::Param(FCommandLine::Get(),TEXT("PFRunCraftingLiveTests"))){AddError(TEXT("Requires isolated M4 session -PFRunCraftingLiveTests -PFExpectedPlayers=1|2"));return false;}
    ADD_LATENT_AUTOMATION_COMMAND(FPFCraftingLiveExercise(this));return true;
}
#endif
