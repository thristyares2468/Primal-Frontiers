// Opt-in real owning-client request test. Catalog overrides are disposable only.
#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Crafting/PFHarvestCapabilities.h"
#include "Crafting/PFCraftingCatalog.h"
#include "Crafting/PFResourceNode.h"
#include "Inventory/PFInventoryComponent.h"
#include "Inventory/PFInventoryPlayerState.h"
#include "Inventory/PFItemCatalog.h"
#include "Progression/PFProgressionComponent.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFSurvivorCharacter.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Survival/PFInteraction.h"
#include "Creatures/PFCreature.h"
#include "Creatures/PFCreatureSpawner.h"
#include "World/PFWorldClock.h"
#include "PFRequestCodes.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

namespace
{
class FPFHarvestLiveExercise : public IAutomationLatentCommand
{
public:
    FPFHarvestLiveExercise(FAutomationTestBase* T,int32 Count):Test(T),Expected(Count){}
    bool Update() override
    {
        const double Now=FPlatformTime::Seconds();if(Now-Started>100){Test->AddError(FString::Printf(TEXT("[PrimalAgentTools] Harvest live timeout phase=%d"),Stage));return true;}
        UWorld* W=nullptr;for(const auto& Context:GEngine->GetWorldContexts()){if(Context.World() && Context.World()->IsGameWorld()){W=Context.World();break;}}
        if(!W){return false;}
        if(UWorld::RemovePIEPrefix(W->GetMapName())!=TEXT("L_M7SurvivalArena") || W->GetNetMode()==NM_Standalone)
        {Test->AddError(TEXT("Harvest live requires isolated dedicated L_M7SurvivalArena and real clients"));return true;}
        const bool Client=W->GetNetMode()==NM_Client;
        if(Stage==99){return Now-Changed>(Client?3:12);}
        TArray<APFSurvivalPlayerController*> Players;
        for(auto It=W->GetPlayerControllerIterator();It;++It){auto* PC=Cast<APFSurvivalPlayerController>(It->Get());if(PC && Cast<APFSurvivorCharacter>(PC->GetPawn()) && PC->GetInventory() && PC->GetInventory()->Catalog){Players.Add(PC);}}
        if(Players.Num()!=(Client?1:Expected)){return false;}
        APFWorldClock* Clock=nullptr;for(TActorIterator<APFWorldClock> It(W);It;++It){Clock=*It;}
        return Client?UpdateClient(W,Players[0],Clock,Now):UpdateServer(W,Players,Clock,Now);
    }
private:
    bool Check(bool Value,const TCHAR* Name){return Test->TestTrue(Name,Value);}
    void Phase(APFWorldClock* Clock,const TArray<APFSurvivalPlayerController*>& Players,int32 Next,double Now)
    {for(auto* PC:Players){CastChecked<APFSurvivorCharacter>(PC->GetPawn())->Survival->SetThirst(10);}Clock->SetHour(float(Next));Stage=Next;Changed=Now;}
    void Finish(double Now){Stage=99;Changed=Now;}
    bool UpdateServer(UWorld* W,const TArray<APFSurvivalPlayerController*>& Players,APFWorldClock* Clock,double Now)
    {
        if(Stage==0)
        {
            for(TActorIterator<APFCreatureSpawner> It(W);It;++It){It->bAutoSpawn=false;}
            for(TActorIterator<APFCreature> It(W);It;++It){It->Destroy();}
            for(TActorIterator<APFResourceNode> It(W);It;++It){It->Destroy();}
            if(!Clock){Clock=W->SpawnActor<APFWorldClock>();}if(!Check(IsValid(Clock),TEXT("Replicated fixture clock"))){return true;}Clock->SetActorTickEnabled(false);
            int32 Lane=0;
            for(auto* PC:Players)
            {
                auto* I=PC->GetInventory();if(!Check(I->GetStacks().IsEmpty(),TEXT("Fresh disposable bag"))){return true;}
                auto* P=CastChecked<APFSurvivorCharacter>(PC->GetPawn());P->SetActorLocation(FVector(-2200,1000+600*Lane++,100));P->GetCharacterMovement()->DisableMovement();P->Survival->HungerDrainPerSecond=0;P->Survival->ThirstDrainPerSecond=0;
                PC->SetControlRotation(FRotator::ZeroRotator);PC->ClientSetRotation(FRotator::ZeroRotator,false);
                I->Catalog=DuplicateObject<UPFItemCatalog>(I->Catalog,I);
                for(auto& D:I->Catalog->Items)
                {
                    if(D.Id==TEXT("Item_Tool")){D.HarvestFamilies.AddTag(PFHarvest::Pick());D.HarvestGrade=EPFHarvestGrade::Stone;}
                    if(D.Id==TEXT("Item_BoundTool")){D.HarvestFamilies.AddTag(PFHarvest::Axe());D.HarvestGrade=EPFHarvestGrade::Iron;D.GatheringHits=8;}
                }
                if(!Check(I->Grant(TEXT("Item_Tool"),1) && I->Grant(TEXT("Item_BoundTool"),1) && I->Grant(TEXT("Item_Water"),6),TEXT("Trusted boundary tools and phase acknowledgements"))){return true;}
                auto* Craft=NewObject<UPFCraftingCatalog>();auto* D=Craft->Resources.FindByPredicate([](const auto& R){return R.Id==TEXT("Node_Stone");});D->HarvestRequirement.Family=PFHarvest::Pick();D->HarvestRequirement.MinimumGrade=EPFHarvestGrade::Iron;
                FVector Eye;FRotator Look;P->GetActorEyesViewPoint(Eye,Look);const FTransform At(FRotator::ZeroRotator,Eye+FVector(150,0,0));
                auto* Node=W->SpawnActorDeferred<APFResourceNode>(APFResourceNode::StaticClass(),At,nullptr,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
                Node->Catalog=Craft;Node->Items=I->Catalog;Node->ResourceId=TEXT("Node_Stone");Node->FinishSpawning(At);Nodes.Add(PC,Node);
                if(!Check(PFInteraction::FindTarget(P)==Node,TEXT("Authoritative aim reaches private lane"))){return true;}
            }
            Phase(Clock,Players,8,Now);return false;
        }
        for(auto* PC:Players){if(PC->GetInventory()->Count(TEXT("Item_Water"))!=13-Stage){return false;}}
        if(Now-Changed<2){return false;}
        for(auto* PC:Players)
        {
            auto* I=PC->GetInventory();auto* PS=PC->GetPlayerState<APFInventoryPlayerState>();auto* Node=Nodes.FindRef(PC).Get();
            const int32 Stone=Stage>=12?6:Stage>=10?4:0,Hits=Stage>=12?0:Stage>=10?1:3,XP=Stage>=12?10:Stage>=10?5:0;
            if(!Check(Node && Node->HitsRemaining==Hits && I->Count(TEXT("Item_Stone"))==Stone && PS->Progression->GetExperience()==XP,TEXT("Every acknowledged phase has exact node/yield/XP"))){return true;}
            if(Stage==9){for(auto& D:I->Catalog->Items){if(D.Id==TEXT("Item_Tool")){D.HarvestGrade=EPFHarvestGrade::Iron;}}}
            if(Stage==10){if(!Check(I->RemoveItem(TEXT("Item_Tool"),1),TEXT("Server removes access tool"))){return true;}}
            if(Stage==11){if(!Check(I->Grant(TEXT("Item_Tool"),1),TEXT("Server returns exact tool"))){return true;}}
        }
        if(Stage==13)
        {Test->AddInfo(TEXT("[PrimalAgentTools] Harvest server exact owned RPC phases, under/wrong/removed tool refusal, finite yield and all acknowledgements verified"));Clock->SetHour(14);Finish(Now);return false;}
        Phase(Clock,Players,Stage+1,Now);return false;
    }
    bool UpdateClient(UWorld* W,APFSurvivalPlayerController* PC,APFWorldClock* Clock,double Now)
    {
        if(!Clock){return false;}const int32 PhaseId=FMath::RoundToInt(Clock->Hour);
        if(PhaseId==14 && Stage==13){Finish(Now);return false;}
        if(PhaseId<8 || PhaseId>13 || PhaseId<=Stage){return false;}
        // The first readiness acknowledgement can arrive long after phase start.
        // Give each newly observed phase its own interval before the next normal
        // consume RPC; do not weaken the controller's inventory anti-spam guard.
        if(ObservedPhase!=PhaseId){ObservedPhase=PhaseId;ObservedAt=Now;return false;}
        if(Now-ObservedAt<.5){return false;}
        auto* I=PC->GetInventory();auto* P=CastChecked<APFSurvivorCharacter>(PC->GetPawn());auto* PS=PC->GetPlayerState<APFInventoryPlayerState>();
        if(!PS || !PS->Progression || I->Count(TEXT("Item_Water"))!=14-PhaseId || !FMath::IsNearlyEqual(P->Survival->GetVitals().Thirst,10.f,.01f)){return false;}
        PC->SetControlRotation(FRotator::ZeroRotator);auto* Node=Cast<APFResourceNode>(PFInteraction::FindTarget(P));if(!Node){return false;}
        if(PhaseId==8)
        {Check(!Node->Gather(P),TEXT("Direct client node mutation refused"));Check(!I->Grant(TEXT("Item_Stone"),1),TEXT("Direct client grant refused"));}
        if(PhaseId>=10)
        {
            const int32 Quantity=PhaseId>=13?6:PhaseId>=11?4:0,XP=PhaseId>=13?10:PhaseId>=11?5:0;
            if(I->Count(TEXT("Item_Stone"))!=Quantity || PS->Progression->GetExperience()!=XP){return false;}
        }
        if(PhaseId==9 || PhaseId==10 || PhaseId==11 || PhaseId==12)
        {
            // Reflected call dispatches the existing protected owning-controller
            // RPC normally; do not expose a gameplay API merely for the fixture.
            UFunction* Interact=PC->FindFunctionChecked(TEXT("ServerInteract"));
            PC->ProcessEvent(Interact,nullptr);PC->ProcessEvent(Interact,nullptr);
        } // repeated owned RPC must not duplicate finite yield.
        if(PhaseId==13)
        {
            Check(Node->HitsRemaining==0 && I->Count(TEXT("Item_Stone"))==6 && I->Count(TEXT("Item_Tool"))==1 && I->Count(TEXT("Item_BoundTool"))==1,TEXT("Owner sees exact depleted yield/tools"));
            for(TActorIterator<APFInventoryPlayerState> It(W);It;++It){if(*It!=PS){Check(It->Inventory->GetStacks().IsEmpty() && It->Progression->GetExperience()==0,TEXT("Foreign bag and XP remain private"));}}
            Test->AddInfo(TEXT("[PrimalAgentTools] Harvest owner ordered RPC phases, direct mutation refusal, exact private yield and depletion verified"));
        }
        const auto* Water=I->GetStacks().FindByPredicate([](const auto& S){return S.ItemId==TEXT("Item_Water");});if(!Water){return false;}
        PC->ServerInventoryAction(Water->StackId,PFInventoryAction::Consume,1);Stage=PhaseId;
        Test->AddInfo(FString::Printf(TEXT("[PrimalAgentTools] Harvest client acknowledged phase=%d"),Stage));return false;
    }
    FAutomationTestBase* Test;int32 Expected=1,Stage=0,ObservedPhase=0;double Started=FPlatformTime::Seconds(),Changed=0,ObservedAt=0;
    TMap<TWeakObjectPtr<APFSurvivalPlayerController>,TWeakObjectPtr<APFResourceNode>> Nodes;
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFHarvestLiveTest,"PF.Harvest.Live",EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)
bool FPFHarvestLiveTest::RunTest(const FString&)
{
    int32 Players=0;FParse::Value(FCommandLine::Get(),TEXT("PFExpectedPlayers="),Players);
    if(!FParse::Param(FCommandLine::Get(),TEXT("PFRunHarvestLiveTests")) || !FParse::Param(FCommandLine::Get(),TEXT("nullrhi")) || (Players!=1 && Players!=2))
    {AddError(TEXT("Requires opt-in isolated NullRHI harvest runner with one/two clients"));return false;}
    ADD_LATENT_AUTOMATION_COMMAND(FPFHarvestLiveExercise(this,Players));return true;
}
#endif
