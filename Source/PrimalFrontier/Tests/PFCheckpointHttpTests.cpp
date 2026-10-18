#include "Persistence/PFCheckpointClient.h"
#include "Persistence/PFCheckpointProtocol.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Misc/Base64.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "HAL/PlatformProcess.h"
#include "HAL/FileManager.h"

namespace
{
    struct FCheckpointHttpState
    {
        FProcHandle Process;
        FString PortFile;
        FPFSavedFile File;
        FString Url;
        TFunction<void(int32)> Issue;
        double Deadline=0;
        int32 Step=0;
        bool bWaiting=false,bFinished=false;
        ~FCheckpointHttpState()
        {
            if (Process.IsValid()) { if (FPlatformProcess::IsProcRunning(Process)) { FPlatformProcess::TerminateProc(Process,true); } FPlatformProcess::CloseProc(Process); }
            if (!PortFile.IsEmpty()) { IFileManager::Get().Delete(*PortFile,false,true); }
        }
    };

    class FWaitCheckpointHttp final : public IAutomationLatentCommand
    {
        TSharedPtr<FCheckpointHttpState> State;
        FAutomationTestBase* Test;
    public:
        FWaitCheckpointHttp(TSharedPtr<FCheckpointHttpState> In,FAutomationTestBase* Owner):State(In),Test(Owner){}
        bool Update() override
        {
            if (FPlatformTime::Seconds()>State->Deadline)
            { State->bFinished=true; Test->AddError(TEXT("Checkpoint loopback HTTP fixture timed out")); return true; }
            if (State->Url.IsEmpty())
            {
                FString Port;
                if (!IFileManager::Get().FileExists(*State->PortFile)) { return false; }
                if (!FFileHelper::LoadFileToString(Port,*State->PortFile)) { return false; }
                const int32 Value=FCString::Atoi(*Port);
                if (Value<1 || Value>65535) { Test->AddError(TEXT("Invalid fixture port")); return true; }
                State->Url=FString::Printf(TEXT("http://127.0.0.1:%d/v1/checkpoints"),Value);
            }
            if (State->Step==8) { State->bFinished=true; return true; }
            if (!State->bWaiting) { State->bWaiting=true; State->Issue(State->Step); }
            return false;
        }
    };
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFCheckpointHttp,"PF.Checkpoints.HttpRoundTrip",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FPFCheckpointHttp::RunTest(const FString&)
{
    // Only this friend test can inject loopback/fake credentials. Public production entrypoints still enforce dedicated authority.
    auto State=MakeShared<FCheckpointHttpState>();
    FString Text,Encoded,Error; TArray<uint8> Bytes; TSharedPtr<FJsonObject> Root;
    if (!FFileHelper::LoadFileToString(Text,*FPaths::Combine(FPaths::ProjectDir(),TEXT("Tests/Fixtures/M8_c3d221f/fixture.json"))) ||
        !FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Text),Root) || !Root->TryGetStringField(TEXT("envelopeBase64"),Encoded) ||
        !FBase64::Decode(Encoded,Bytes) || !FPFSaveFileStore::DecodeEnvelope(Bytes,State->File,Error))
    { AddError(TEXT("Historical HTTP test fixture unreadable")); return false; }
    State->PortFile=FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectSavedDir(),TEXT("AutomationReports"),TEXT("M15Http_")+FGuid::NewGuid().ToString(EGuidFormats::Digits)+TEXT(".port")));
    const FString Python=FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::EngineDir(),TEXT("Binaries/ThirdParty/Python3/Win64/python.exe")));
    const FString Script=FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectDir(),TEXT("Tests/CheckpointHttpFixture.py")));
    State->Process=FPlatformProcess::CreateProc(*Python,*FString::Printf(TEXT("\"%s\" \"%s\""),*Script,*State->PortFile),true,true,true,nullptr,0,nullptr,nullptr);
    if (!State->Process.IsValid()) { AddError(TEXT("Could not launch engine-bundled Python loopback fixture")); return false; }
    State->Deadline=FPlatformTime::Seconds()+70;
    TWeakPtr<FCheckpointHttpState> WeakState=State;
    State->Issue=[this,WeakState](int32 Step)
    {
        auto Current=WeakState.Pin(); if (!Current) { return; }
        const bool bUpload=Step==0 || Step==2;
        FString Url=Current->Url+(bUpload?TEXT(""):Step==4?TEXT("/corrupt"):Step==5?TEXT("/missing"):Step==6?TEXT("/unavailable"):TEXT("/test-world"));
        FString Why;
        const bool Started=FPFCheckpointClient::Send(Url,Step==3?TEXT("wrong-fixture-key"):TEXT("fixture-only-not-a-real-key"),TEXT("test-world"),
            bUpload?&Current->File:nullptr,1,[Step]{return Step!=7;},
            [this,Current,Step](FPFCheckpointResult Result)
            {
                if (Current->bFinished) { return; }
                const int32 ExpectedCodes[]={201,200,409,401,200,404,503,200};
                TestEqual(*FString::Printf(TEXT("HTTP step%d status"),Step),Result.HttpStatus,ExpectedCodes[Step]);
                TestEqual(*FString::Printf(TEXT("HTTP step%d success/refusal"),Step),Result.bSucceeded,Step<2);
                TestFalse(TEXT("Sanitized result details present"),Result.Details.IsEmpty());
                if (Step==1)
                {
                    TestTrue(TEXT("Real streamed HTTP preserves saved payload"),Result.File.Payload==Current->File.Payload);
                    TestEqual(TEXT("Real streamed HTTP preserves original UTC"),Result.File.SavedUtc,Current->File.SavedUtc);
                }
                if (Step==7) { TestTrue(TEXT("Ended server discards response"),Result.Details.Contains(TEXT("world ended"))); }
                Current->Step++; Current->bWaiting=false;
            },Why);
        if (!TestTrue(TEXT("Loopback request starts asynchronously"),Started)) { AddError(Why); Current->Step=8; Current->bWaiting=false; }
    };
    FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShared<FWaitCheckpointHttp>(State,this));
    return true;
}
#endif
