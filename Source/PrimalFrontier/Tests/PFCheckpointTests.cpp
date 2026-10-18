#include "Persistence/PFCheckpointProtocol.h"
#include "Persistence/PFCheckpointClient.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Misc/Base64.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/DateTime.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Tests/AutomationCommon.h"

namespace
{
    bool Frozen(FPFSavedFile& File,TArray<uint8>& Bytes,FString& Error)
    {
        FString Text; TSharedPtr<FJsonObject> Root; FString Encoded;
        return FFileHelper::LoadFileToString(Text,*FPaths::Combine(FPaths::ProjectDir(),TEXT("Tests/Fixtures/M8_c3d221f/fixture.json"))) &&
            FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Text),Root) && Root->TryGetStringField(TEXT("envelopeBase64"),Encoded) &&
            FBase64::Decode(Encoded,Bytes) && FPFSaveFileStore::DecodeEnvelope(Bytes,File,Error);
    }
    TSharedPtr<FJsonObject> Record(const FString& Upload)
    {
        TSharedPtr<FJsonObject> Root; FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Upload),Root); return Root;
    }
    FString Response(const TSharedPtr<FJsonObject>& Checkpoint)
    {
        auto Root=MakeShared<FJsonObject>(); Root->SetBoolField(TEXT("ok"),true); Root->SetObjectField(TEXT("checkpoint"),Checkpoint);
        FString Text; FJsonSerializer::Serialize(Root,TJsonWriterFactory<TCHAR,TCondensedJsonPrintPolicy<TCHAR>>::Create(&Text)); return Text;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFCheckpointRoundTrip,"PF.Checkpoints.FrozenEnvelopeRoundTrip",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FPFCheckpointRoundTrip::RunTest(const FString&)
{
    FPFSavedFile File,Returned; TArray<uint8> Bytes,Reencoded; FString Error,Upload;
    if (!TestTrue(TEXT("Read genuine historical writer envelope"),Frozen(File,Bytes,Error))) { return false; }
    TestTrue(TEXT("Exact historical file bytes preserved"),FPFSaveFileStore::EncodeEnvelope(File,Reencoded,Error) && Reencoded==Bytes);
    if (!TestTrue(TEXT("Build bounded API payload"),FPFCheckpointProtocol::BuildUpload(TEXT("test-world"),File,Upload,Error))) { AddError(Error); return false; }
    auto Data=Record(Upload);
    TestEqual(TEXT("SHA256 matches independent historical manifest"),Data->GetStringField(TEXT("checksum")),FString(TEXT("sha256:47d0a2035da74393511ea2f39463de0a4eea822fc0d822584e3799ab899a17ed")));
    TestTrue(TEXT("Acknowledgement matches exact upload"),FPFCheckpointProtocol::ReadAcknowledgement(TEXT("test-world"),File,Response(Data),Error));
    Data->SetStringField(TEXT("generation"),TEXT("1")); // Real Neon bigint shape.
    TestTrue(TEXT("Parse Neon bigint string"),FPFCheckpointProtocol::ReadDownload(TEXT("test-world"),1,Response(Data),Returned,Error));
    TestEqual(TEXT("Preserve original food-age UTC"),Returned.SavedUtc,File.SavedUtc);
    TestEqual(TEXT("Preserve generation"),Returned.Generation,File.Generation);
    TestTrue(TEXT("Preserve owner/item/resource payload bytes"),Returned.Payload==File.Payload);
    Data->RemoveField(TEXT("payload"));
    TestTrue(TEXT("Real POST acknowledgement without payload accepted"),FPFCheckpointProtocol::ReadAcknowledgement(TEXT("test-world"),File,Response(Data),Error));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFCheckpointRefusals,"PF.Checkpoints.ResponseRefusals",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FPFCheckpointRefusals::RunTest(const FString&)
{
    FPFSavedFile File; TArray<uint8> Bytes; FString Error,Upload;
    if (!Frozen(File,Bytes,Error) || !FPFCheckpointProtocol::BuildUpload(TEXT("test-world"),File,Upload,Error)) { AddError(Error); return false; }
    auto Refused=[&](const FString& Name,const FString& Json,uint64 Floor=1)
    {
        FPFSavedFile Out; Out.Generation=77; Out.SavedUtc=99; Out.Payload={5,6,7};
        TestFalse(*Name,FPFCheckpointProtocol::ReadDownload(TEXT("test-world"),Floor,Json,Out,Error));
        TestFalse(TEXT("Explicit refusal"),Error.IsEmpty());
        TestTrue(TEXT("Failure preserves caller data"),Out.Generation==77 && Out.SavedUtc==99 && Out.Payload==TArray<uint8>({5,6,7}));
    };
    Refused(TEXT("Malformed JSON"),TEXT("{bad"));
    Refused(TEXT("Server refusal"),TEXT("{\"ok\":false}"));
    Refused(TEXT("Rollback below caller floor"),Response(Record(Upload)),2);
    for (int32 Case=0;Case<9;++Case)
    {
        auto Data=Record(Upload);
        if (Case==0) { Data->SetStringField(TEXT("world_id"),TEXT("foreign-world")); }
        if (Case==1) { Data->SetNumberField(TEXT("envelope_version"),2); }
        if (Case==2) { Data->SetStringField(TEXT("checksum"),TEXT("sha256:bad")); }
        if (Case==3) { Data->SetStringField(TEXT("expires_at"),TEXT("2001-01-01T00:00:00Z")); }
        if (Case==4) { Data->SetStringField(TEXT("expires_at"),TEXT("bad")); }
        if (Case==5) { Data->RemoveField(TEXT("expires_at")); }
        if (Case==6) { Data->SetNumberField(TEXT("generation"),2); }
        if (Case==7) { Data->SetNumberField(TEXT("generation"),1.5); }
        if (Case==8) { Data->SetStringField(TEXT("generation"),TEXT("9007199254740992")); }
        Refused(FString::Printf(TEXT("Reject altered response case%d"),Case),Response(Data));
        if (Case!=6 && Case!=7 && Case!=8)
        { TestFalse(TEXT("Altered acknowledgement refused"),FPFCheckpointProtocol::ReadAcknowledgement(TEXT("test-world"),File,Response(Data),Error)); }
    }
    auto Data=Record(Upload); Data->SetStringField(TEXT("generation"),TEXT("-1")); Refused(TEXT("Negative generation"),Response(Data));
    auto Payload=Data->GetObjectField(TEXT("payload")); Payload->SetStringField(TEXT("envelope"),TEXT("not-base64"));
    Data->SetNumberField(TEXT("generation"),1); Refused(TEXT("Malformed envelope"),Response(Data));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFCheckpointBounds,"PF.Checkpoints.BoundsAndIntegrity",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FPFCheckpointBounds::RunTest(const FString&)
{
    FPFSavedFile File; TArray<uint8> Bytes; FString Error,Json=TEXT("sentinel");
    if (!Frozen(File,Bytes,Error)) { return false; }
    TestFalse(TEXT("URL/path injection refused"),FPFCheckpointProtocol::BuildUpload(TEXT("../world?token=x"),File,Json,Error));
    TestEqual(TEXT("Failed upload preserves output"),Json,FString(TEXT("sentinel")));
    File.Generation=FPFCheckpointProtocol::MaxGeneration+1;
    TestFalse(TEXT("Remote integer precision loss refused"),FPFCheckpointProtocol::BuildUpload(TEXT("world"),File,Json,Error));
    FPFSavedFile Out; Out.Generation=88;
    auto Corrupt=Bytes; Corrupt.Last()^=1;
    TestFalse(TEXT("Inner payload CRC checked"),FPFSaveFileStore::DecodeEnvelope(Corrupt,Out,Error));
    Corrupt=Bytes; Corrupt.Add(0);
    TestFalse(TEXT("Trailing envelope bytes refused"),FPFSaveFileStore::DecodeEnvelope(Corrupt,Out,Error));
    TestEqual(TEXT("Corrupt decode preserves generation"),Out.Generation,static_cast<uint64>(88));
    File.Generation=1; File.SavedUtc=FDateTime::UtcNow().ToUnixTimestamp()+1000; TArray<uint8> Encoded;
    TestFalse(TEXT("Future save timestamps refused"),FPFSaveFileStore::EncodeEnvelope(File,Encoded,Error));
    TestFalse(TEXT("No valid empty payload"),FPFSaveFileStore::EncodeEnvelope(FPFSavedFile(),Encoded,Error));
    File.Payload.SetNumZeroed(FPFSaveFileStore::MaxPayloadBytes+1);
    TestFalse(TEXT("Oversized payload refused before encoding"),FPFSaveFileStore::EncodeEnvelope(File,Encoded,Error));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFCheckpointAuthority,"PF.Checkpoints.ServerAuthority",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FPFCheckpointAuthority::RunTest(const FString&)
{
    FString Error; bool bCalled=false;
    auto Complete=[&](FPFCheckpointResult){ bCalled=true; };
    TestFalse(TEXT("Null world has no credentials or HTTP"),FPFCheckpointClient::Fetch(nullptr,TEXT("world"),1,Complete,Error));
    FTestWorldWrapper Fixture;
    if (!Fixture.CreateTestWorld(EWorldType::Game)) { return false; }
    TestFalse(TEXT("Ordinary standalone world denied"),FPFCheckpointClient::EligibleWorld(Fixture.GetTestWorld()));
    TestFalse(TEXT("Standalone cannot fetch"),FPFCheckpointClient::Fetch(Fixture.GetTestWorld(),TEXT("world"),1,Complete,Error));
    TestFalse(TEXT("Standalone cannot upload a save"),FPFCheckpointClient::UploadSavedSlot(Fixture.GetTestWorld(),TEXT("Survival"),TEXT("world"),Complete,Error));
    TestFalse(TEXT("Refused request never invokes success callback"),bCalled);
    TestFalse(TEXT("Authority refusal explained"),Error.IsEmpty());
    return true;
}
#endif
