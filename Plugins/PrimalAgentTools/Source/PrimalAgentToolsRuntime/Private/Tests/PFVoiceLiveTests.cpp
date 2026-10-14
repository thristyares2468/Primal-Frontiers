#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Voice/PFVoiceAuthority.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFSurvivorCharacter.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Inventory/PFInventoryPlayerState.h"
#include "Inventory/PFInventoryComponent.h"
#include "World/PFWorldClock.h"
#include "Creatures/PFCreature.h"
#include "Creatures/PFCreatureSpawner.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include <limits>

class FPFVoiceLiveExercise final : public IAutomationLatentCommand
{
public:
    FPFVoiceLiveExercise(FAutomationTestBase* In,int32 Count):Test(In),Expected(Count),Started(FPlatformTime::Seconds()){}
    bool Update() override
    {
        const double Now=FPlatformTime::Seconds();if(Stage==99){return Now-Changed>(Client?3:12);}
        // All owners acknowledged before phase4; keep server alive while clients finish,
        // without requiring a departed client's pawn to remain in the roster.
        if(Stage==4){return Now-Changed>15;}
        if(Now-Started>100){Test->AddError(FString::Printf(TEXT("[PrimalAgentTools] Voice metadata timeout stage=%d ready=%d water=%d hour=%.1f"),Stage,LastReady,LastWater,LastHour));return true;}
        UWorld* W=nullptr;for(const auto& C:GEngine->GetWorldContexts()){if(C.World() && C.World()->IsGameWorld()){W=C.World();break;}}
        if(!W){return false;}Client=W->GetNetMode()==NM_Client;
        if(UWorld::RemovePIEPrefix(W->GetMapName())!=TEXT("L_PrimalFrontier_OpenWorld") || W->GetNetMode()==NM_Standalone)
        {Test->AddError(TEXT("[PrimalAgentTools] Voice metadata requires dedicated OpenWorld and real clients"));return true;}
        TArray<APFSurvivalPlayerController*> Players;
        for(auto It=W->GetPlayerControllerIterator();It;++It)
        {auto* PC=Cast<APFSurvivalPlayerController>(It->Get());const auto* PS=PC?PC->GetPlayerState<APFInventoryPlayerState>():nullptr;if(PC && PC->GetPawn() && PC->GetInventory() && PS && PS->PersistentPlayerId.IsValid()){Players.Add(PC);}}
        LastReady=Players.Num();if(Players.Num()!=(Client?1:Expected)){return false;}
        auto* A=W->GetSubsystem<UPFVoiceAuthority>();if(!Check(A!=nullptr,TEXT("Actual world voice subsystem"))){return true;}
        APFWorldClock* Clock=nullptr;for(TActorIterator<APFWorldClock> It(W);It;++It){Clock=*It;}
        LastWater=Players[0]->GetInventory()->Count(TEXT("Item_Water"));LastHour=Clock?Clock->Hour:-1;
        if(Client){return RunClient(A,Players[0],Clock,Now);}return RunServer(W,A,Players,Clock,Now);
    }
private:
    bool Check(bool Value,const TCHAR* Name){return Test->TestTrue(Name,Value);}
    void Finish(double Now){Stage=99;Changed=Now;}
    void Phase(APFWorldClock* Clock,int32 Value,double Now)
    {Check(Clock->SetHour(float(Value)),TEXT("Server barrier hour accepted"));Clock->ForceNetUpdate();Stage=Value;Changed=Now;Test->AddInfo(FString::Printf(TEXT("[PrimalAgentTools] Voice metadata server phase=%d"),Value));}
    bool RunClient(UPFVoiceAuthority* A,APFSurvivalPlayerController* PC,APFWorldClock* Clock,double Now)
    {
        if(!Clock || Clock->Hour<1 || Clock->Hour>4){return false;}
        const int32 PhaseId=FMath::RoundToInt(Clock->Hour);
        if(PhaseId!=Stage)
        {
            // Replication can deliver the next barrier immediately after the preceding RPC.
            // Respect the real inventory cooldown instead of weakening its authority gate.
            if(PendingPhase!=PhaseId){PendingPhase=PhaseId;PendingSince=Now;}
            if(Now-PendingSince<.3){return false;}
            FString Error;FPFVoicePolicyConfig Config;Config.bEnabled=true;FPFVoicePreferences Pref;Pref.bTransmitConsent=true;Pref.bReceiveConsent=true;Pref.bPushToTalk=true;
            TArray<FPFVoiceParticipant> Snapshot;FPFVoiceParticipant Sentinel;Sentinel.PlayerId=FGuid(0,0,9,9);Snapshot.Add(Sentinel);
            TArray<FPFVoiceRecipient> Routes;FPFVoiceRecipient R;R.Gain=.5f;Routes.Add(R);
            Check(!A->SetConfig(Config,Error),TEXT("Actual client cannot enable server policy"));Check(!A->SetPreferences(PC,Pref,Error),TEXT("Actual client cannot mutate server preferences"));
            Check(!A->BuildSnapshot(Snapshot,Error) && Snapshot.Num()==1 && Snapshot[0].PlayerId==Sentinel.PlayerId,TEXT("Actual client snapshot refused atomically"));
            Check(!A->Evaluate(PC,Routes,Error) && Routes.Num()==1 && Routes[0].Gain==.5f,TEXT("Actual client cannot invent route"));
            if(PhaseId<=3)
            {
                const auto* Water=PC->GetInventory()->GetStacks().FindByPredicate([](const auto& S){return S.ItemId==TEXT("Item_Water");});
                if(!Water || Water->Quantity!=5-PhaseId){return false;}
                // Existing owned consumption RPC is an acknowledgement, never a voice preference RPC.
                PC->ServerInventoryAction(Water->StackId,2,1);Stage=PhaseId;
                Test->AddInfo(FString::Printf(TEXT("[PrimalAgentTools] Voice metadata client acknowledged phase=%d"),PhaseId));
            }
            else
            {Check(PC->GetInventory()->Count(TEXT("Item_Water"))==1,TEXT("Exactly three real owned acknowledgements"));Test->AddInfo(TEXT("[PrimalAgentTools] Voice metadata client authority refusal and three server barriers verified; no audio"));Finish(Now);}
        }return false;
    }
    bool RunServer(UWorld* W,UPFVoiceAuthority* A,const TArray<APFSurvivalPlayerController*>& Players,APFWorldClock* Clock,double Now)
    {
        FString Error;TArray<FPFVoiceRecipient> Routes;TArray<FPFVoiceParticipant> Snapshot;
        if(Stage==0)
        {
            for(TActorIterator<APFCreatureSpawner> It(W);It;++It){It->bAutoSpawn=false;}
            for(TActorIterator<APFCreature> It(W);It;++It){It->Destroy();}
            if(!Clock){Clock=W->SpawnActor<APFWorldClock>();}Clock->SetActorTickEnabled(false);
            FPFVoicePolicyConfig Config;Check(A->Evaluate(Players[0],Routes,Error),TEXT("Default server snapshot evaluates"));for(const auto& R:Routes){Check(R.Result==EPFVoiceRoute::Disabled && R.Gain==0,TEXT("Default real server policy disabled"));}
            Config.bEnabled=true;Check(A->SetConfig(Config,Error),TEXT("Real server config accepted"));
            int32 Index=0;
            for(auto* PC:Players)
            {
                PC->GetPawn()->SetActorLocation(FVector(-1000,Index++*900,500));CastChecked<ACharacter>(PC->GetPawn())->GetCharacterMovement()->DisableMovement();
                auto* V=PC->GetPawn()->FindComponentByClass<UPFPlayerSurvivalComponent>();V->HungerDrainPerSecond=0;V->ThirstDrainPerSecond=0;V->SetThirst(10);V->SetHunger(100);
                Check(PC->GetInventory()->GetStacks().IsEmpty(),TEXT("Fresh disposable bag"));Check(PC->GetInventory()->Grant(TEXT("Item_Water"),4),TEXT("Private owned acknowledgement items"));
                FPFVoicePreferences Pref;Pref.bTransmitConsent=true;Pref.bReceiveConsent=true;Pref.bPushToTalk=true;Check(A->SetPreferences(PC,Pref,Error),TEXT("Actual connected owner preferences"));
            }
            Check(A->BuildSnapshot(Snapshot,Error) && Snapshot.Num()==Expected,TEXT("Actual connected server roster count"));
            for(auto* PC:Players){Check(A->Evaluate(PC,Routes,Error) && Routes.Num()==Expected-1,TEXT("No self route in real server"));for(const auto& R:Routes){Check(R.Result==EPFVoiceRoute::Allowed && FMath::IsNearlyEqual(R.Gain,.5f),TEXT("Actual two-player near fade"));}}
            Phase(Clock,1,Now);return false;
        }
        if(Stage<4)
        {
            if(Now-Changed<.5){return false;}
            for(auto* PC:Players){if(PC->GetInventory()->Count(TEXT("Item_Water"))!=4-Stage){return false;}}
            if(Stage==1)
            {
                if(Expected==2){Players[1]->GetPawn()->SetActorLocation(FVector(-1000,1600,500));for(auto* PC:Players){Check(A->Evaluate(PC,Routes,Error),TEXT("Far reevaluates actual server positions"));Check(Routes[0].Result==EPFVoiceRoute::OutOfRange && Routes[0].Gain==0,TEXT("Actual far denied"));}}
                Phase(Clock,2,Now);return false;
            }
            if(Stage==2)
            {
                FPFVoicePreferences Pref;Pref.bTransmitConsent=true;Pref.bReceiveConsent=true;Pref.bPushToTalk=true;
                if(Expected==2)
                {
                    Players[1]->GetPawn()->SetActorLocation(FVector(-1000,200,500));Pref.Muted.Add(Players[0]->GetPlayerState<APFInventoryPlayerState>()->PersistentPlayerId);
                    Check(A->SetPreferences(Players[1],Pref,Error),TEXT("Actual listener mute set"));Check(A->Evaluate(Players[0],Routes,Error) && Routes[0].Result==EPFVoiceRoute::Muted,TEXT("Actual listener mute denied"));
                    Check(A->Evaluate(Players[1],Routes,Error) && Routes[0].Result==EPFVoiceRoute::Allowed,TEXT("Mute is listening direction only"));Pref.Blocked=Pref.Muted;Pref.Muted.Reset();A->SetPreferences(Players[1],Pref,Error);
                    for(auto* PC:Players){Check(A->Evaluate(PC,Routes,Error) && Routes[0].Result==EPFVoiceRoute::Blocked,TEXT("Actual bilateral block denied"));}
                    auto Bad=Pref;Bad.ReceiveVolume=std::numeric_limits<float>::infinity();Check(!A->SetPreferences(Players[1],Bad,Error),TEXT("Actual malformed preference refused"));
                    Check(A->Evaluate(Players[0],Routes,Error) && Routes[0].Result==EPFVoiceRoute::Blocked,TEXT("Prior block retained"));
                }
                else{Pref.bPushToTalk=false;A->SetPreferences(Players[0],Pref,Error);Check(A->BuildSnapshot(Snapshot,Error) && !Snapshot[0].bPushToTalk,TEXT("One-player PTT stop stored only on server"));}
                Phase(Clock,3,Now);return false;
            }
            Test->AddInfo(TEXT("[PrimalAgentTools] Voice metadata server connected identity/position/default consent/range/mute/block and all owner acknowledgements verified; no audio"));Phase(Clock,4,Now);return false;
        }
        return false;
    }
    FAutomationTestBase* Test;int32 Expected,Stage=0,LastReady=0,LastWater=0,PendingPhase=0;double Started,Changed=0,PendingSince=0;float LastHour=-1;bool Client=false;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFVoiceLiveTest,"PF.Voice.AuthorityLive",EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)
bool FPFVoiceLiveTest::RunTest(const FString&)
{
    int32 Expected=0;FParse::Value(FCommandLine::Get(),TEXT("PFExpectedPlayers="),Expected);
    if(!FParse::Param(FCommandLine::Get(),TEXT("PFRunVoiceMetadataTests")) || (Expected!=1 && Expected!=2))
    {AddError(TEXT("[PrimalAgentTools] Requires isolated VoiceMetadata runner and real dedicated OpenWorld/1–2clients; no audio"));return false;}
    ADD_LATENT_AUTOMATION_COMMAND(FPFVoiceLiveExercise(this,Expected));return true;
}
#endif
