#include "PFReadyAutomation.h"
#include "Containers/Ticker.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "PFResults.h"

namespace
{
    FTSTicker::FDelegateHandle PFReadyPersistenceTicker;
}

void PFStartReadyPersistenceAutomation()
{
#if WITH_DEV_AUTOMATION_TESTS && !UE_SERVER
    if(!FParse::Param(FCommandLine::Get(),TEXT("PFRunPersistenceWhenReady"))){return;}
    if(!FParse::Param(FCommandLine::Get(),TEXT("PFRunPersistenceLiveTests")))
    {UE_LOG(LogTemp,Error,TEXT("[PrimalAgentTools] Readiness launch requires the explicit live persistence fixture"));FPlatformMisc::RequestExitWithStatus(false,2);return;}
    const double Started=FPlatformTime::Seconds();
    PFReadyPersistenceTicker=FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([Started,ReadyAt=0.0](float) mutable
    {
        UWorld* ReadyWorld=nullptr;
        if(GEngine){for(const auto& Context:GEngine->GetWorldContexts())
        {
            auto* World=Context.World();
            if(!World || !World->IsGameWorld() || !World->HasBegunPlay() || World->GetNetMode()!=NM_Client ||
                World->GetOutermost()->GetName()!=TEXT("/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld")){continue;}
            auto* PC=World->GetFirstPlayerController();
            if(PC && PC->IsLocalController() && PC->GetPawn()){ReadyWorld=World;break;}
        }}
        const double Now=FPlatformTime::Seconds();
        if(ReadyWorld)
        {
            if(ReadyAt==0){ReadyAt=Now;}
            if(Now-ReadyAt>=2)
            {
                UE_LOG(LogTemp,Display,TEXT("[PrimalAgentTools] Connected open-world pawn stable; queueing existing PF.Persistence.Live"));
                if(!GEngine->Exec(ReadyWorld,TEXT("Automation RunTests PF.Persistence.Live")))
                {UE_LOG(LogTemp,Error,TEXT("[PrimalAgentTools] Persistence automation queue refused"));FPlatformMisc::RequestExitWithStatus(false,2);}
                return false;
            }
        }
        else{ReadyAt=0;}
        if(Now-Started>90)
        {UE_LOG(LogTemp,Error,TEXT("[PrimalAgentTools] Connected-world readiness deadline exceeded; test not started"));FPlatformMisc::RequestExitWithStatus(false,2);return false;}
        return true;
    }),0.25f);
#endif
}

void PFStopReadyPersistenceAutomation()
{
    if(PFReadyPersistenceTicker.IsValid()){FTSTicker::GetCoreTicker().RemoveTicker(PFReadyPersistenceTicker);PFReadyPersistenceTicker.Reset();}
}
