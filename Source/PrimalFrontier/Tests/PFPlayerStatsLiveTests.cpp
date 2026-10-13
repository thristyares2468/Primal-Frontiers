// Explicit trusted XP boundaries isolate owned spending and actual new-process persistence.
#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Progression/PFProgressionComponent.h"
#include "Inventory/PFInventoryPlayerState.h"
#include "Inventory/PFInventoryComponent.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFSurvivorCharacter.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Persistence/PFWorldPersistence.h"
#include "Persistence/PFSaveFileStore.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

namespace
{
class FPlayerStatsLive : public IAutomationLatentCommand
{
public:
    explicit FPlayerStatsLive(FAutomationTestBase* T):Test(T)
    {FParse::Value(FCommandLine::Get(),TEXT("PFExpectedPlayers="),Expected);FParse::Value(FCommandLine::Get(),TEXT("PFSaveSlot="),Slot);Restart=FParse::Param(FCommandLine::Get(),TEXT("PFLoadSave"));}
    bool Update() override
    {
        const double Now=FPlatformTime::Seconds();if(Done){return Now-Done>(Client?5:15);}
        if(Now-Started>110){Test->AddError(FString::Printf(TEXT("Stats live timeout stage%d request%d client%d"),Stage,Request,Client));return true;}
        UWorld* W=nullptr;for(const auto& Context:GEngine->GetWorldContexts()){if(Context.World() && Context.World()->IsGameWorld()){W=Context.World();break;}}if(!W){return false;}
        Client=W->GetNetMode()==NM_Client;
        TArray<APFSurvivalPlayerController*> Players;
        for(auto It=W->GetPlayerControllerIterator();It;++It)
        {
            auto* PC=Cast<APFSurvivalPlayerController>(It->Get());if(PC && PC->GetPawn() && PC->GetInventory() && Progress(PC))
            {Players.Add(PC);if(!Client){auto* V=PC->GetPawn()->FindComponentByClass<UPFPlayerSurvivalComponent>();V->HungerDrainPerSecond=0;V->ThirstDrainPerSecond=0;V->StaminaRecoveryPerSecond=0;}}
        }
        if(Players.Num()!=(Client?1:Expected)){return false;}
        Players.Sort([](const auto& A,const auto& B){return A.PlayerState->GetPlayerId()<B.PlayerState->GetPlayerId();});
        auto* PC=Players[0];auto* G=Progress(PC);FString Error;
        if(!Client)
        {
            if(!Restart && Stage==0)
            {
                int32 Role=0;for(auto* P:Players)
                {
                    if(!P->GetInventory()->GetStacks().IsEmpty() || Progress(P)->GetExperience()!=0){Test->AddError(TEXT("Stats fixture requires fresh players"));return true;}
                    auto* Pawn=CastChecked<APFSurvivorCharacter>(P->GetPawn());Pawn->GetCharacterMovement()->DisableMovement();Pawn->SetActorLocation(FVector(-1200,Role*400,100));
                    Test->TestTrue(TEXT("Trusted role marker is not XP reward"),P->GetInventory()->Grant(TEXT("Item_Stone"),++Role));
                    FPFProgressionRecord Seed;Seed.Experience=1000;Test->TestTrue(TEXT("Explicit trusted level6 seed, not earned pacing"),Progress(P)->Restore(Seed,Error));
                }
                Stage=1;return false;
            }
            if(!Restart && Stage==1)
            {
                for(auto* P:Players){if(!Matches(P) || !Progress(P)->GetStatFeedback().StartsWith(TEXT("Refused:"))){return false;}}
                for(auto* P:Players)
                {
                    auto* V=P->GetPawn()->FindComponentByClass<UPFPlayerSurvivalComponent>();const int32 Role=Marker(P);
                    V->ChangeStamina((Role==1?135:95)-V->GetVitals().Stamina);V->SetHealth(Role==1?91:92);Check(P);
                }
                if(!Test->TestTrue(TEXT("Actual isolated upgraded world save"),W->GetSubsystem<UPFWorldPersistence>()->Save(Slot,Error))){Test->AddError(Error);return true;}
                for(auto* P:Players){P->GetPawn()->FindComponentByClass<UPFPlayerSurvivalComponent>()->SetHealth(88);}Stage=2;return false;
            }
            for(auto* P:Players)
            {
                if(!Matches(P) || (Restart && !Progress(P)->GetStatFeedback().StartsWith(TEXT("Refused:"))) || FMath::Abs(FRotator::NormalizeAxis(P->GetControlRotation().Yaw-123))>1){return false;}
            }
            for(auto* P:Players){Check(P);}
            Test->AddInfo(Restart?TEXT("[PrimalAgentTools] Stats restart server restored independent allocations and capacities"):TEXT("[PrimalAgentTools] Stats server independent allocations, exact budgets and upgraded save verified"));Done=Now;return false;
        }
        if(Restart)
        {
            if(!Matches(PC) || PC->GetPlayerSetupStatus()!=EPFPlayerSetupStatus::Restored){return false;}
            if(Stage==0)
            {Check(PC);DirectRefusals(PC,W);PC->ServerUpgradeStat(static_cast<EPFPlayerStat>(255));Stage=1;Changed=Now;return false;}
            if(Now-Changed<.6 || !G->GetStatFeedback().StartsWith(TEXT("Refused:")) || !Privacy(W,PC)){return false;}
            Check(PC);PC->SetControlRotation(FRotator(0,123,0));Test->AddInfo(TEXT("[PrimalAgentTools] Stats restart client identity, private ranks, capacities and refusal verified"));Done=Now;return false;
        }
        if(Stage==0)
        {
            if(G->GetExperience()!=1000 || Marker(PC)<1 || PC->GetPlayerSetupStatus()==EPFPlayerSetupStatus::Unconfirmed){return false;}
            DirectRefusals(PC,W);Stage=1;Changed=Now-.7;
        }
        const int32 Count=Marker(PC)==1?5:1;
        const EPFPlayerStat Order[]={EPFPlayerStat::Attack,EPFPlayerStat::Stamina,EPFPlayerStat::Weight,EPFPlayerStat::CraftingSpeed,EPFPlayerStat::Stamina};
        if(Stage==1 && Now-Changed>.6)
        {
            if(Request>0 && G->GetAvailableStatPoints()!=5-Request){return false;}
            if(Request<Count)
            {const auto Stat=Marker(PC)==1?Order[Request]:EPFPlayerStat::Weight;PC->ServerUpgradeStat(Stat);PC->ServerUpgradeStat(Stat);++Request;Changed=Now;return false;}
            PC->ServerUpgradeStat(static_cast<EPFPlayerStat>(255));Stage=2;Changed=Now;return false;
        }
        if(Stage==2 && Now-Changed>.6 && Matches(PC) && PC->GetPawn()->FindComponentByClass<UPFPlayerSurvivalComponent>()->GetVitals().Health==88)
        {
            if(!Privacy(W,PC)){return false;}Check(PC);PC->SetControlRotation(FRotator(0,123,0));
            Test->AddInfo(TEXT("[PrimalAgentTools] Stats owned RPC, duplicate cooldown, separate tech budget, privacy and client refusal verified"));Done=Now;
        }
        return false;
    }
private:
    UPFProgressionComponent* Progress(APFSurvivalPlayerController* P){auto* PS=P->GetPlayerState<APFInventoryPlayerState>();return PS?PS->Progression.Get():nullptr;}
    int32 Marker(APFSurvivalPlayerController* P){return P->GetInventory()->Count(TEXT("Item_Stone"));}
    bool Matches(APFSurvivalPlayerController* P)
    {const auto* G=Progress(P);const int32 Role=Marker(P);FPFPlayerStatRanks Desired;if(Role==1){Desired.Attack=1;Desired.Stamina=2;Desired.Weight=1;Desired.CraftingSpeed=1;}else if(Role==2){Desired.Weight=1;}else{return false;}return G->GetExperience()==1000 && G->GetRecord().Stats==Desired;}
    void Check(APFSurvivalPlayerController* P)
    {
        auto* G=Progress(P);const int32 Role=Marker(P);const auto V=P->GetPawn()->FindComponentByClass<UPFPlayerSurvivalComponent>()->GetVitals();
        Test->TestTrue(TEXT("Exact independent owner ranks"),Matches(P));Test->TestEqual(TEXT("Technology unaffected"),G->GetAvailablePoints(),15);Test->TestEqual(TEXT("Independent remaining stat budget"),G->GetAvailableStatPoints(),Role==1?0:4);
        Test->TestTrue(TEXT("No knowledge/craft credit or items generated by allocation"),G->GetRecord().Knowledge.IsEmpty() && G->GetRecord().CreditedCrafts.IsEmpty() && P->GetInventory()->GetStacks().Num()==1);
        Test->TestEqual(TEXT("Replicated/restored stamina capacity"),V.MaxStamina,Role==1?140.f:100.f);Test->TestEqual(TEXT("Replicated/restored weight threshold"),P->GetInventory()->WeightLimit,40.f);
        Test->TestTrue(TEXT("Saved non-default stamina preserved within normal recovery bounds"),V.Stamina>=(Role==1?135.f:95.f) && V.Stamina<=V.MaxStamina);
        // Logout intentionally captures changes after the manual save. Health88
        // was the final creation acknowledgement, so restart must recover that
        // latest state rather than the earlier91/92 manual-save snapshot.
        if(Restart){Test->TestEqual(TEXT("Restart restores latest disconnect health, not fresh100"),V.Health,88.f);}
    }
    void DirectRefusals(APFSurvivalPlayerController* PC,UWorld* W)
    {
        auto* G=Progress(PC);FString Error;const auto Before=G->GetRecord().Stats;const auto Feedback=G->GetStatFeedback();
        Test->TestFalse(TEXT("Client direct spend refuses"),G->RequestStat(EPFPlayerStat::Attack,PC->GetPawn(),Error));Test->TestFalse(TEXT("Client rank restore refuses"),G->Restore({},Error));
        Test->TestTrue(TEXT("Client cannot forge allocation/feedback"),G->GetRecord().Stats==Before && G->GetStatFeedback()==Feedback);
        Test->TestFalse(TEXT("Client cannot publish world save"),W->GetSubsystem<UPFWorldPersistence>()->Save(Slot,Error));
    }
    bool Privacy(UWorld* W,APFSurvivalPlayerController* PC)
    {
        int32 Others=0;for(TActorIterator<APFInventoryPlayerState> It(W);It;++It){if(*It!=PC->PlayerState){++Others;auto* G=It->Progression.Get();if(!G){return false;}Test->TestTrue(TEXT("Foreign progression and stat feedback are owner-only"),G->GetExperience()==0 && G->GetRecord().Stats==FPFPlayerStatRanks{} && G->GetStatFeedback().IsEmpty());}}
        return Others==Expected-1;
    }
    FAutomationTestBase* Test;FString Slot;double Started=FPlatformTime::Seconds(),Changed=0,Done=0;int32 Expected=1,Stage=0,Request=0;bool Restart=false,Client=false;
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFPlayerStatsLiveTest,"PF.Progression.StatsLive",EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)
bool FPFPlayerStatsLiveTest::RunTest(const FString&)
{
    FString Slot;int32 Expected=0;FParse::Value(FCommandLine::Get(),TEXT("PFExpectedPlayers="),Expected);
    if(!FParse::Param(FCommandLine::Get(),TEXT("PFRunStatsTests")) || !FParse::Param(FCommandLine::Get(),TEXT("nullrhi")) || (Expected!=1 && Expected!=2) ||
       !FParse::Value(FCommandLine::Get(),TEXT("PFSaveSlot="),Slot) || !Slot.StartsWith(TEXT("AutomationM12StatsLive")) || !FPFSaveFileStore::ValidSlot(Slot))
    {AddError(TEXT("Requires isolated NullRHI Stats runner, fresh automation slot and1-2 players"));return false;}
    ADD_LATENT_AUTOMATION_COMMAND(FPlayerStatsLive(this));return true;
}
#endif
