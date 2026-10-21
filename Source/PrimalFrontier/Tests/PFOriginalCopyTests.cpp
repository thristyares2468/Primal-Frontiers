#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Persistence/PFSaveFileStore.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/Base64.h"
#include "Misc/ScopeExit.h"
#include "HAL/FileManager.h"
#include "Async/Async.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace PFOriginalCopyTests
{
    struct FFiles
    {
        TArray<FString> Slots,Originals,Directories;
        FString Slot(const FString& Name)
        {
            FString Result=TEXT("AutomationCopy")+Name+FGuid::NewGuid().ToString(EGuidFormats::Digits);
            Slots.Add(Result);return Result;
        }
        FString Original(const FString& Slot,const FString& Edge)
        {
            const FString Result=FPFSaveFileStore::OriginalPath(Slot,Edge);
            Originals.Add(Result);
            IFileManager::Get().MakeDirectory(*FPaths::GetPath(Result),true);
            return Result;
        }
        ~FFiles()
        {
            for(const FString& Name:Originals){IFileManager::Get().Delete(*Name);}
            for(const FString& Name:Directories){IFileManager::Get().DeleteDirectory(*Name,false,false);}
            for(const FString& Slot:Slots)
            {
                for(bool Second:{false,true})
                {
                    const FString Name=FPFSaveFileStore::Path(Slot,Second);
                    IFileManager::Get().Delete(*Name);IFileManager::Get().Delete(*(Name+TEXT(".pending")));
                }
            }
            for(const FString& Name:Originals){IFileManager::Get().DeleteDirectory(*FPaths::GetPath(Name),false,false);}
        }
    };
    bool SameFile(const FString& Name,const TArray<uint8>& Expected)
    {TArray<uint8> Bytes;return FFileHelper::LoadFileToArray(Bytes,*Name) && Bytes==Expected;}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFOriginalCopyExactTest,"PF.Persistence.OriginalCopy.Exact",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFOriginalCopyExactTest::RunTest(const FString&)
{
    using namespace PFOriginalCopyTests;
    FFiles Files;const FString Source=Files.Slot(TEXT("Source")),Target=Files.Slot(TEXT("Target")),Frozen=Files.Slot(TEXT("Frozen"));
    const FString Edge=TEXT("Recovery"),Original=Files.Original(Source,Edge),Historical=Files.Original(Source,TEXT("Historical"));
    FString Error;FPFSavedFile Old;Old.Payload={1,5,9};Old.Generation=42;Old.SavedUtc=FDateTime::UtcNow().ToUnixTimestamp()-86400;
    TArray<uint8> Bytes;
    if(!TestTrue(TEXT("Encode old time-anchored source"),FPFSaveFileStore::EncodeEnvelope(Old,Bytes,Error)) ||
        !TestTrue(TEXT("Install source"),FFileHelper::SaveArrayToFile(Bytes,*FPFSaveFileStore::Path(Source,false))) ||
        !TestTrue(TEXT("Preserve original"),FPFSaveFileStore::PreserveOriginal(Source,Edge,Error))){return false;}
    TestTrue(TEXT("Advance source independently"),FPFSaveFileStore::Write(Source,{2,6,10},Error));
    TArray<uint8> SourceB;FFileHelper::LoadFileToArray(SourceB,*FPFSaveFileStore::Path(Source,true));
    TestTrue(TEXT("Copy original into unused slot"),FPFSaveFileStore::CopyOriginalToNewSlot(Source,Edge,Target,Error));
    TestTrue(TEXT("Recovery file is byte exact"),SameFile(FPFSaveFileStore::Path(Target,false),Bytes));
    TestFalse(TEXT("No second generation published"),IFileManager::Get().FileExists(*FPFSaveFileStore::Path(Target,true)));
    FPFSavedFile Out;
    TestTrue(TEXT("Recovery uses ordinary reader"),FPFSaveFileStore::Read(Target,Out,Error));
    TestTrue(TEXT("Generation, payload and old time anchor retained without renewed food time"),
        Out.Generation==42 && Out.SavedUtc==Old.SavedUtc && Out.Payload==Old.Payload && !Out.bRecoveredBackup);
    TestFalse(TEXT("Repeat refuses existing destination"),FPFSaveFileStore::CopyOriginalToNewSlot(Source,Edge,Target,Error));
    TestTrue(TEXT("Repeat preserves copied envelope"),SameFile(FPFSaveFileStore::Path(Target,false),Bytes));
    TestTrue(TEXT("Subsequent normal save advances recovery generation"),FPFSaveFileStore::Write(Target,{3,7,11},Error));
    TestTrue(TEXT("Normal reader selects new recovery generation"),FPFSaveFileStore::Read(Target,Out,Error) && Out.Generation==43);
    TestTrue(TEXT("Archived original and both source generations untouched"),SameFile(Original,Bytes) &&
        SameFile(FPFSaveFileStore::Path(Source,false),Bytes) && SameFile(FPFSaveFileStore::Path(Source,true),SourceB));
    const FString Salvaged=Files.Slot(TEXT("Salvaged"));const TArray<uint8> Broken={77};
    TestTrue(TEXT("Install damaged active A/B generations"),FFileHelper::SaveArrayToFile(Broken,*FPFSaveFileStore::Path(Source,false)) &&
        FFileHelper::SaveArrayToFile(Broken,*FPFSaveFileStore::Path(Source,true)));
    TestTrue(TEXT("Verified original remains recoverable when active A/B are corrupt"),FPFSaveFileStore::CopyOriginalToNewSlot(Source,Edge,Salvaged,Error));
    TestTrue(TEXT("Recovery leaves corrupt active evidence and archived original untouched"),SameFile(Original,Bytes) &&
        SameFile(FPFSaveFileStore::Path(Salvaged,false),Bytes) && SameFile(FPFSaveFileStore::Path(Source,false),Broken) &&
        SameFile(FPFSaveFileStore::Path(Source,true),Broken));

    FString Text;TSharedPtr<FJsonObject> Fixture;TArray<uint8> FrozenBytes;
    if(!TestTrue(TEXT("Read genuine archived writer fixture"),FFileHelper::LoadFileToString(Text,
        *(FPaths::ProjectDir()/TEXT("Tests/Fixtures/M8_c3d221f/fixture.json"))) &&
        FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Text),Fixture) && Fixture.IsValid() &&
        FBase64::Decode(Fixture->GetStringField(TEXT("envelopeBase64")),FrozenBytes))){return false;}
    TestTrue(TEXT("Install genuine historical original unchanged"),FFileHelper::SaveArrayToFile(FrozenBytes,*Historical));
    TestTrue(TEXT("Copy archived writer original"),FPFSaveFileStore::CopyOriginalToNewSlot(Source,TEXT("Historical"),Frozen,Error));
    TestTrue(TEXT("Historical envelope byte exact at both paths"),SameFile(Historical,FrozenBytes) && SameFile(FPFSaveFileStore::Path(Frozen,false),FrozenBytes));
    AddInfo(TEXT("[PrimalAgentTools] OriginalCopy.Exact: unchanged original bytes/generation/UTC, ordinary A/B continuation, repeat refusal and frozen old-writer copy; no live restore or private save."));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFOriginalCopyRefusalTest,"PF.Persistence.OriginalCopy.Refusal",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFOriginalCopyRefusalTest::RunTest(const FString&)
{
    using namespace PFOriginalCopyTests;
    FFiles Files;const FString Source=Files.Slot(TEXT("Source")),Target=Files.Slot(TEXT("Empty")),Edge=TEXT("Recovery");
    const FString Original=Files.Original(Source,Edge);FString Error;
    TestFalse(TEXT("Missing original refuses"),FPFSaveFileStore::CopyOriginalToNewSlot(Source,Edge,Target,Error));
    FPFSavedFile Seed;Seed.Payload={4,8,12};Seed.Generation=9;Seed.SavedUtc=FDateTime::UtcNow().ToUnixTimestamp()-3600;
    TArray<uint8> Good;TestTrue(TEXT("Encode fixture"),FPFSaveFileStore::EncodeEnvelope(Seed,Good,Error));
    TArray<uint8> Bad=Good;Bad.Last()^=0xFF;
    TArray<uint8> Future=Good;Future[4]=2;
    TArray<uint8> Oversize;Oversize.SetNumZeroed(FPFSaveFileStore::MaxPayloadBytes+33);
    for(const TArray<uint8>& Bytes:{TArray<uint8>{77},Bad,Future,Oversize})
    {
        TestTrue(TEXT("Install explicitly malformed original fixture"),FFileHelper::SaveArrayToFile(Bytes,*Original));
        TestFalse(TEXT("Malformed original refused"),FPFSaveFileStore::CopyOriginalToNewSlot(Source,Edge,Target,Error));
        TestTrue(TEXT("Malformed evidence retained"),SameFile(Original,Bytes));
        TestFalse(TEXT("No candidate file on source refusal"),IFileManager::Get().FileExists(*FPFSaveFileStore::Path(Target,false)));
    }
    TestTrue(TEXT("Install valid original"),FFileHelper::SaveArrayToFile(Good,*Original));
    for(const FString& Invalid:{FString(),FString(TEXT("../escape")),FString::ChrN(65,TEXT('x'))})
    {
        TestFalse(TEXT("Invalid destination refuses"),FPFSaveFileStore::CopyOriginalToNewSlot(Source,Edge,Invalid,Error));
        TestFalse(TEXT("Invalid source refuses"),FPFSaveFileStore::CopyOriginalToNewSlot(Invalid,Edge,Target,Error));
        TestFalse(TEXT("Invalid edge refuses"),FPFSaveFileStore::CopyOriginalToNewSlot(Source,Invalid,Target,Error));
    }
    TestFalse(TEXT("Same slot refuses"),FPFSaveFileStore::CopyOriginalToNewSlot(Source,Edge,Source,Error));
    TestFalse(TEXT("Case alias refuses even without source A/B"),FPFSaveFileStore::CopyOriginalToNewSlot(Source,Edge,Source.ToLower(),Error));
    TestFalse(TEXT("Off-thread recovery refuses"),Async(EAsyncExecution::ThreadPool,[&Source,&Edge,&Target]()
    {FString ThreadError;return FPFSaveFileStore::CopyOriginalToNewSlot(Source,Edge,Target,ThreadError);}).Get());
    for(bool Second:{false,true}) for(bool Pending:{false,true}) for(bool Directory:{false,true})
    {
        const FString Blocked=Files.Slot(TEXT("Occupied"));
        const FString Name=FPFSaveFileStore::Path(Blocked,Second)+(Pending?TEXT(".pending"):TEXT(""));
        if(Directory){TestTrue(TEXT("Create directory occupant"),IFileManager::Get().MakeDirectory(*Name,true));Files.Directories.Add(Name);}
        else{TestTrue(TEXT("Create file occupant"),FFileHelper::SaveArrayToFile(Bad,*Name));}
        TestFalse(TEXT("Every A/B and pending occupant refuses recovery"),FPFSaveFileStore::CopyOriginalToNewSlot(Source,Edge,Blocked,Error));
        TestTrue(TEXT("Occupant retained exactly"),Directory?IFileManager::Get().DirectoryExists(*Name):SameFile(Name,Bad));
    }
    const FString Archived=Files.Slot(TEXT("Archived")),Archive=Files.Original(Archived,TEXT("OtherEdge"));
    TestTrue(TEXT("Install destination original evidence"),FFileHelper::SaveArrayToFile(Bad,*Archive));
    TestFalse(TEXT("Existing destination original evidence refuses"),FPFSaveFileStore::CopyOriginalToNewSlot(Source,Edge,Archived,Error));
    TestTrue(TEXT("Existing archive retained"),SameFile(Archive,Bad));
    TestTrue(TEXT("Source original unchanged after all refusals"),SameFile(Original,Good));
    AddInfo(TEXT("[PrimalAgentTools] OriginalCopy.Refusal: invalid/case-alias/off-thread/missing/corrupt/future/oversize/occupied A/B/pending/archive evidence preserved; no user slots or live world."));
    return true;
}
#endif
