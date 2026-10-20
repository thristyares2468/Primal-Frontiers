// Explicit disposable loopback probe; never run against a normal player's world.
#if WITH_DEV_AUTOMATION_TESTS && !UE_SERVER
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

struct FPFHostedAdmissionProbe
{
    FAutomationTestBase* Test=nullptr;
    FDelegateHandle FailureHandle;
    double StartedAt=0;
    bool bReceived=false;
    FString Error;
};

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FPFWaitHostedRefusal,TSharedPtr<FPFHostedAdmissionProbe>,Probe);
bool FPFWaitHostedRefusal::Update()
{
    if(!Probe->bReceived && FPlatformTime::Seconds()-Probe->StartedAt<25){return false;}
    GEngine->OnNetworkFailure().Remove(Probe->FailureHandle);
    Probe->Test->TestTrue(TEXT("Real loopback handshake received a network refusal"),Probe->bReceived);
    Probe->Test->TestTrue(TEXT("Server explicitly denies hosted gameplay until durable identity/restore"),
        Probe->Error.Contains(TEXT("Hosted gameplay admission NOT IMPLEMENTED")));
    if(Probe->bReceived){Probe->Test->AddInfo(TEXT("[PrimalAgentTools] Hosted admission probe inspected actual server refusal"));}
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFHostedAdmissionLiveTest,"PF.Hosting.ClosedAdmissionLive",
    EAutomationTestFlags::ClientContext|EAutomationTestFlags::EngineFilter)
bool FPFHostedAdmissionLiveTest::RunTest(const FString&)
{
    int32 Port=0;
    if(!FParse::Param(FCommandLine::Get(),TEXT("PFRunHostingAdmissionProbe")) ||
        !FParse::Value(FCommandLine::Get(),TEXT("PFHostingProbePort="),Port) || Port<1024 || Port>65535)
    {AddError(TEXT("Requires explicit disposable PFRunHostingAdmissionProbe and loopback PFHostingProbePort"));return false;}
    UWorld* World=nullptr;
    for(const auto& Context:GEngine->GetWorldContexts())
    {if(Context.WorldType==EWorldType::Game && Context.World()){World=Context.World();break;}}
    if(!World || World->GetNetMode()!=NM_Standalone)
    {AddError(TEXT("Probe starts in an isolated standalone Entry world"));return false;}
    auto Probe=MakeShared<FPFHostedAdmissionProbe>();Probe->Test=this;Probe->StartedAt=FPlatformTime::Seconds();
    // Expected network rejection stays in raw logs. Only this exact refusal is expected;
    // timeout, unknown failure and unrelated engine errors must still fail the probe.
    AddExpectedError(TEXT("Hosted gameplay admission NOT IMPLEMENTED"),EAutomationExpectedErrorFlags::Contains,0);
    Probe->FailureHandle=GEngine->OnNetworkFailure().AddLambda([Probe](UWorld*,UNetDriver*,ENetworkFailure::Type,const FString& Error)
    {Probe->bReceived=true;Probe->Error=Error;});
    GEngine->SetClientTravel(World,*FString::Printf(TEXT("127.0.0.1:%d"),Port),TRAVEL_Absolute);
    ADD_LATENT_AUTOMATION_COMMAND(FPFWaitHostedRefusal(Probe));
    return true;
}
#endif
