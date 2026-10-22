#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Crafting/PFCraftingComponent.h"
#include "Crafting/PFCraftingStation.h"
#include "Building/PFBuildPiece.h"
#include "Inventory/PFInventoryPlayerState.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

namespace
{
class FBatchLive final : public IAutomationLatentCommand
{
public:
    explicit FBatchLive(FAutomationTestBase* T):Test(T){FParse::Value(FCommandLine::Get(),TEXT("PFExpectedPlayers="),Expected);}
    bool Update() override
    {
        const double Now=FPlatformTime::Seconds();if(Stage==99){return Now-Changed>(bClient?3:12);}
        if(Now-Started>120){Test->AddError(FString::Printf(TEXT("Batch live timeout stage%d client%d"),Stage,bClient));return true;}
        UWorld* W=nullptr;for(const auto& X:GEngine->GetWorldContexts()){if(X.World() && X.World()->IsGameWorld()){W=X.World();break;}}if(!W){return false;}
        bClient=W->GetNetMode()==NM_Client;TArray<APFSurvivalPlayerController*> Players;
        for(auto It=W->GetPlayerControllerIterator();It;++It){auto* P=Cast<APFSurvivalPlayerController>(It->Get());if(P && P->GetPawn() && P->GetInventory() && P->GetCrafting()){Players.Add(P);}}
        if(Players.Num()!=(bClient?1:Expected)){return false;}
        auto* PC=Players[0];auto* C=PC->GetCrafting();auto* I=PC->GetInventory();const FName Batch=TEXT("Recipe_CopperBatch");
        if(!bClient)
        {
            if(Stage==0)
            {
                int32 N=0;for(auto* P:Players)
                {
                    auto* Needs=P->GetPawn()->FindComponentByClass<UPFPlayerSurvivalComponent>();Needs->HungerDrainPerSecond=0;Needs->ThirstDrainPerSecond=0;
                    P->GetPawn()->SetActorLocation(FVector(-400,1200+650*N++,100));
                    if(!Test->TestTrue(TEXT("Disposable server batch inputs"),P->GetInventory()->GetStacks().IsEmpty() && P->GetInventory()->Grant(TEXT("Item_CopperOre"),8) && P->GetInventory()->Grant(TEXT("Item_Wood"),2))){return true;}
                }
                Next(Now);return false;
            }
            if(Stage==1)
            {
                for(auto* P:Players){if(!P->GetCrafting()->Feedback.Contains(TEXT("Requires refinery"))){return false;}}
                for(auto* P:Players)
                {
                    auto* Catalog=GetDefault<UPFBuildingCatalog>();const FVector Location=P->GetPawn()->GetActorLocation()+FVector(0,140,-90);
                    const FTransform BaseT(Location),RefineT(Location+FVector(0,0,45));
                    auto* Base=W->SpawnActorDeferred<APFBuildPiece>(APFBuildPiece::StaticClass(),BaseT);Base->Initialize(*Catalog->Find(TEXT("Build_Foundation")),P->PlayerState,nullptr);Base->FinishSpawning(BaseT);
                    auto* Refinery=W->SpawnActorDeferred<APFBuildPiece>(APFBuildPiece::StaticClass(),RefineT);Refinery->Initialize(*Catalog->Find(TEXT("Build_Refinery")),P->PlayerState,Base);Refinery->FinishSpawning(RefineT);
                }
                Next(Now);return false;
            }
            if(Stage==2)
            {
                for(auto* P:Players)
                {
                    if(P->GetCrafting()->ActiveRecipe==Batch && P->GetCrafting()->ActiveStationMultiplier==1.5f){Seen.Add(P);}
                    if(P->GetCrafting()->Feedback.StartsWith(TEXT("Cancelled"))){Cancelled.Add(P);}
                }
                for(auto* P:Players){if(P->GetInventory()->Count(TEXT("Item_CopperIngot"))!=2 || !P->GetCrafting()->ActiveRecipe.IsNone()){return false;}}
                for(auto* P:Players)
                {
                    Test->TestEqual(TEXT("Server exact remaining ore"),P->GetInventory()->Count(TEXT("Item_CopperOre")),4);
                    Test->TestEqual(TEXT("Server consumes only one wood after cancel/retry/duplicate"),P->GetInventory()->Count(TEXT("Item_Wood")),1);
                }
                Test->TestEqual(TEXT("Every owned server job used refinery"),Seen.Num(),Expected);Test->TestEqual(TEXT("Every cancellation observed on server"),Cancelled.Num(),Expected);
                Test->AddInfo(TEXT("Batch server required refinery, exact fuel, cancel and owned RPC conservation verified"));Stage=99;Changed=Now;
            }
            return false;
        }
        if(Now-Changed<.7){return false;}
        if(Stage==0)
        {
            if(I->Count(TEXT("Item_CopperOre"))!=8 || I->Count(TEXT("Item_Wood"))!=2){return false;}
            Test->TestFalse(TEXT("Direct client cannot craft batch"),C->Start(Batch,PC->GetPawn()));PC->ServerCraftAction(Batch,false);Next(Now);return false;
        }
        if(Stage==1)
        {
            if(!C->Feedback.Contains(TEXT("Requires refinery"))){return false;}
            Test->TestTrue(TEXT("Server missing-station refusal preserves inputs"),C->ActiveRecipe.IsNone() && I->Count(TEXT("Item_Wood"))==2 && I->Count(TEXT("Item_CopperOre"))==8);Next(Now);return false;
        }
        if(Stage==2)
        {
            const auto* Recipe=C->Catalog?C->Catalog->Recipe(Batch,I->Catalog):nullptr;auto* Station=Recipe?APFCraftingStation::FindBest(*Recipe,PC->GetPawn(),false):nullptr;
            if(!Station || !Station->bOwnedProvider || !IsValid(Station->ProviderPiece)){return false;}
            Test->TestTrue(TEXT("Replicated actual owner provider"),Station->ProviderPiece->IsOwnedBy(PC->PlayerState));
            Test->TestFalse(TEXT("Client station cannot authorize itself"),Station->CanUse(*Recipe,PC->GetPawn(),true));PC->ServerCraftAction(Batch,false);Next(Now);return false;
        }
        if(Stage==3)
        {
            if(C->ActiveRecipe!=Batch){return false;}Test->TestEqual(TEXT("Required station snapshot replicated"),C->ActiveStation,FString(TEXT("Owned refinery")));Test->TestEqual(TEXT("Server multiplier replicated"),C->ActiveStationMultiplier,1.5f);
            Test->TestEqual(TEXT("Fuel remains until completion"),I->Count(TEXT("Item_Wood")),2);PC->ServerCraftAction(NAME_None,true);Next(Now);return false;
        }
        if(Stage==4)
        {
            if(!C->ActiveRecipe.IsNone() || !C->Feedback.StartsWith(TEXT("Cancelled"))){return false;}
            Test->TestTrue(TEXT("Cancel gives no output and keeps fuel/ore"),I->Count(TEXT("Item_Wood"))==2 && I->Count(TEXT("Item_CopperOre"))==8 && I->Count(TEXT("Item_CopperIngot"))==0);PC->ServerCraftAction(Batch,false);Next(Now);return false;
        }
        if(Stage==5)
        {
            if(C->ActiveRecipe!=Batch){return false;}PC->ServerCraftAction(Batch,false);Next(Now);return false;
        }
        if(Stage==6)
        {
            if(!C->ActiveRecipe.IsNone() || I->Count(TEXT("Item_CopperIngot"))!=2){return false;}
            Test->TestTrue(TEXT("Owning client exact final batches"),I->Count(TEXT("Item_CopperOre"))==4 && I->Count(TEXT("Item_Wood"))==1);
            int32 Others=0;for(TActorIterator<APFInventoryPlayerState> It(W);It;++It){if(*It!=PC->PlayerState && !It->IsActorBeingDestroyed()){++Others;Test->TestTrue(TEXT("Foreign inventory stays private"),It->Inventory->GetStacks().IsEmpty());}}
            if(Others!=Expected-1){return false;}
            Test->AddInfo(TEXT("Batch client required station, fuel, private inventory and owned RPC refusal verified"));Stage=99;Changed=Now;return false;
        }
        return false;
    }
private:
    void Next(double Now){++Stage;Changed=Now;}
    FAutomationTestBase* Test;TSet<APFSurvivalPlayerController*> Seen,Cancelled;int32 Expected=0,Stage=0;bool bClient=false;double Started=FPlatformTime::Seconds(),Changed=0;
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFBatchLiveTest,"PF.Crafting.BatchLive",EAutomationTestFlags::ClientContext|EAutomationTestFlags::ServerContext|EAutomationTestFlags::EngineFilter)
bool FPFBatchLiveTest::RunTest(const FString&)
{
    int32 Players=0;FParse::Value(FCommandLine::Get(),TEXT("PFExpectedPlayers="),Players);
    if(!FParse::Param(FCommandLine::Get(),TEXT("PFRunBatchRefiningTests")) || (Players!=1 && Players!=2)){AddError(TEXT("Requires isolated batch runner and one/two clients"));return false;}
    ADD_LATENT_AUTOMATION_COMMAND(FBatchLive(this));return true;
}
#endif
