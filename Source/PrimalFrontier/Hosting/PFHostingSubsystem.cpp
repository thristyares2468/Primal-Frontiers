#include "Hosting/PFHostingSubsystem.h"
#include "Hosting/PFHostingConfig.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Persistence/PFWorldPersistence.h"
#if UE_SERVER
#include "PlayFabGSDK.h"
#include "Engine/NetDriver.h"
#include "IPAddress.h"
#include "Async/Async.h"
#endif

DEFINE_LOG_CATEGORY_STATIC(LogPFHosting,Log,All);

bool UPFHostingSubsystem::IsHostedRequested()
{return FParse::Param(FCommandLine::Get(),TEXT("PFPlayFabHosted"));}

void UPFHostingSubsystem::FailHosting(const TCHAR* Reason)
{
    bStopping=true;
    UE_LOG(LogPFHosting,Error,TEXT("[PrimalAgentTools] [PFHosting] Refused: %s"),Reason);
    FPlatformMisc::RequestExitWithStatus(false,2);
}

void UPFHostingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    if(!IsHostedRequested()){return;}
#if UE_SERVER
    FString Ignored;
    if(!IsRunningDedicatedServer() || FParse::Param(FCommandLine::Get(),TEXT("noGSDK")) ||
        FParse::Value(FCommandLine::Get(),TEXT("port="),Ignored) ||
        FParse::Value(FCommandLine::Get(),TEXT("PFSaveSlot="),Ignored) || FParse::Param(FCommandLine::Get(),TEXT("PFLoadSave")))
    {FailHosting(TEXT("Hosted lifecycle requires dedicated Server, agent-selected port and no local save override"));return;}
    const FString ConfigFile=FPlatformMisc::GetEnvironmentVariable(TEXT("GSDK_CONFIG_FILE"));
    FString Json,Error;FPFHostingAgentConfig Config;
    const int64 Size=IFileManager::Get().FileSize(*ConfigFile);
    if(Size<=0 || Size>65536 || !FFileHelper::LoadFileToString(Json,*ConfigFile) || !FPFHostingAgentConfig::Parse(Json,Config,Error))
    {FailHosting(TEXT("Valid bounded GSDK_CONFIG_FILE required; no agent/secret values logged"));return;}
    AgentPort=Config.ListenPort;
    auto& Sdk=FModuleManager::LoadModuleChecked<FPlayFabGSDKModule>(TEXT("PlayFabGSDK"));
    bSdkLoaded=true;
    Sdk.OnHealthCheck.BindLambda([]()
    {
        const bool bHealthy=!IsEngineExitRequested();
        UE_LOG(LogPFHosting,Verbose,TEXT("[PrimalAgentTools] [PFHosting] Health callback healthy=%d"),bHealthy);
        return bHealthy;
    });
    Sdk.OnReadyForPlayers.BindLambda([](){UE_LOG(LogPFHosting,Display,TEXT("[PrimalAgentTools] [PFHosting] StandingBy; gameplay admission CLOSED"));});
    Sdk.OnServerActive.BindWeakLambda(this,[this]()
    {
        if(bStopping){return;}
        bActive=true;
        UE_LOG(LogPFHosting,Display,TEXT("[PrimalAgentTools] [PFHosting] Active; gameplay admission CLOSED (durable restore/identity NOT IMPLEMENTED)"));
    });
    const TWeakObjectPtr<UPFHostingSubsystem> WeakThis(this);
    // Vendor shutdown callback is on a worker thread. UObject/lifecycle work stays on GameThread.
    Sdk.OnShutdown.BindLambda([WeakThis]()
    {AsyncTask(ENamedThreads::GameThread,[WeakThis](){if(WeakThis.IsValid()){WeakThis->StopHosting();}});});
    // Same supported FURL setting as UGSDKUtils::SetDefaultServerHostPort,
    // after validating the vendor config, before the world/netdriver is created.
    FURL::UrlConfig.DefaultPort=AgentPort;
    StartedAt=FPlatformTime::Seconds();
    TickHandle=FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this,&UPFHostingSubsystem::TickHosting),0.25f);
    UE_LOG(LogPFHosting,Display,TEXT("[PrimalAgentTools] [PFHosting] Initializing; agent UDP gameport=%d; no local/remote save restoration"),AgentPort);
#else
    FailHosting(TEXT("PFPlayFabHosted requires a compiled dedicated Server target"));
#endif
}

bool UPFHostingSubsystem::TickHosting(float)
{
#if UE_SERVER
    if(bStopping){return false;}
    if(bReady){return true;}
    if(FPlatformTime::Seconds()-StartedAt>60){FailHosting(TEXT("World/listen/catalog readiness timed out"));return false;}
    auto* World=GetGameInstance()->GetWorld();
    if(!World || !World->HasBegunPlay() || World->GetNetMode()!=NM_DedicatedServer){return true;}
    if(World->GetOutermost()->GetName()!=TEXT("/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld"))
    {FailHosting(TEXT("Only the verified open-world package is supported by this lifecycle gate"));return false;}
    auto* Driver=World->GetNetDriver();const auto Address=Driver?Driver->GetLocalAddr():nullptr;
    if(!Address.IsValid()){return true;}
    if(Address->GetPort()!=AgentPort){FailHosting(TEXT("Actual bound port differs from agent gameport"));return false;}
    FPFWorldSaveData Snapshot;FString Error;
    if(!World->GetSubsystem<UPFWorldPersistence>()->Capture(Snapshot,Error) || Snapshot.Resources.IsEmpty()){return true;}
    bReady=true;
    FPlayFabGSDKModule::Get().ReadyForPlayers();
#endif
    return true;
}

void UPFHostingSubsystem::StopHosting()
{
    if(bStopping){return;}
    bStopping=true;
    UE_LOG(LogPFHosting,Display,TEXT("[PrimalAgentTools] [PFHosting] Terminating; active=%d; gameplay admission CLOSED; no durability acknowledgment"),bActive);
    FPlatformMisc::RequestExitWithStatus(false,0);
}

void UPFHostingSubsystem::Deinitialize()
{
    bStopping=true;
    if(TickHandle.IsValid()){FTSTicker::GetCoreTicker().RemoveTicker(TickHandle);TickHandle.Reset();}
#if UE_SERVER
    if(bSdkLoaded)
    {
        auto& Sdk=FPlayFabGSDKModule::Get();
        // These two callbacks run on GameThread. Leave the worker-thread
        // delegates immutable: health captures no UObject and shutdown uses a
        // weak reference plus GameThread dispatch, which observes bStopping.
        Sdk.OnReadyForPlayers.Unbind();Sdk.OnServerActive.Unbind();
    }
#endif
    Super::Deinitialize();
}
