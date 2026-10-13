#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Persistence/PFSaveFileStore.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/ScopeExit.h"
#include "HAL/FileManager.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFOriginalBackupTest,"PF.Persistence.OriginalBackup",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFOriginalBackupTest::RunTest(const FString&)
{
    const FString Slot=TEXT("AutomationOriginal")+FGuid::NewGuid().ToString(EGuidFormats::Digits),Edge=TEXT("TestKnownEdge");
    const FString Original=FPFSaveFileStore::OriginalPath(Slot,Edge),Blocked=FPFSaveFileStore::OriginalPath(Slot,TEXT("Blocked"));
    ON_SCOPE_EXIT
    {
        // Only uniquely owned explicit fixture files; never recursively delete directories.
        for(bool Second:{false,true}){IFileManager::Get().Delete(*FPFSaveFileStore::Path(Slot,Second));IFileManager::Get().Delete(*(FPFSaveFileStore::Path(Slot,Second)+TEXT(".pending")));}
        IFileManager::Get().Delete(*Original);IFileManager::Get().DeleteDirectory(*Blocked,false,false);
    };
    FString Error;const TArray<uint8> First={1,2,3},Second={4,5,6},Third={7,8,9},Bad={77};
    FPFSavedFile Out;Out.Payload=Bad;Out.Generation=777;
    TestTrue(TEXT("Invalid slot and edge never produce a path"),FPFSaveFileStore::OriginalPath(TEXT("../bad"),Edge).IsEmpty() && FPFSaveFileStore::OriginalPath(Slot,TEXT("../bad")).IsEmpty());
    TestFalse(TEXT("Missing source refuses without backup"),FPFSaveFileStore::PreserveOriginal(Slot,Edge,Error));
    TestFalse(TEXT("Missing original read refuses"),FPFSaveFileStore::ReadOriginal(Slot,Edge,Out,Error));
    TestTrue(TEXT("Failed read preserves output"),Out.Payload==Bad && Out.Generation==777);
    if(!TestTrue(TEXT("First source write"),FPFSaveFileStore::Write(Slot,First,Error)) || !TestTrue(TEXT("Second source write"),FPFSaveFileStore::Write(Slot,Second,Error))){return false;}
    FPFSavedFile Latest;TestTrue(TEXT("Selected source exists"),FPFSaveFileStore::Read(Slot,Latest,Error));
    TArray<uint8> SourceBytes,SourceA,SourceB;FFileHelper::LoadFileToArray(SourceBytes,*FPFSaveFileStore::Path(Slot,true));
    FFileHelper::LoadFileToArray(SourceA,*FPFSaveFileStore::Path(Slot,false));FFileHelper::LoadFileToArray(SourceB,*FPFSaveFileStore::Path(Slot,true));
    TestTrue(TEXT("Preserve selected original"),FPFSaveFileStore::PreserveOriginal(Slot,Edge,Error));
    TestTrue(TEXT("Read original"),FPFSaveFileStore::ReadOriginal(Slot,Edge,Out,Error));
    TestTrue(TEXT("Exact payload/generation/time preserved"),Out.Payload==Latest.Payload && Out.Generation==Latest.Generation && Out.SavedUtc==Latest.SavedUtc && !Out.bRecoveredBackup);
    TArray<uint8> Bytes;FFileHelper::LoadFileToArray(Bytes,*Original);TestTrue(TEXT("Exact source envelope bytes preserved"),Bytes==SourceBytes);
    TestTrue(TEXT("Repeat is idempotent"),FPFSaveFileStore::PreserveOriginal(Slot,Edge,Error));
    TestTrue(TEXT("Later A/B rotation"),FPFSaveFileStore::Write(Slot,Third,Error));TestTrue(TEXT("Repeated preservation after rotation"),FPFSaveFileStore::PreserveOriginal(Slot,Edge,Error));
    TArray<uint8> After;FFileHelper::LoadFileToArray(After,*Original);TestTrue(TEXT("First original survives ordinary rotation byte-for-byte"),After==Bytes);
    TestTrue(TEXT("Original remains previous selected generation"),FPFSaveFileStore::ReadOriginal(Slot,Edge,Out,Error) && Out.Payload==Second && Out.Generation==2);
    // Non-file destination must refuse without damaging either source generation.
    TestTrue(TEXT("Create blocked fixture destination"),IFileManager::Get().MakeDirectory(*Blocked,true));
    FFileHelper::LoadFileToArray(SourceA,*FPFSaveFileStore::Path(Slot,false));FFileHelper::LoadFileToArray(SourceB,*FPFSaveFileStore::Path(Slot,true));
    TestFalse(TEXT("Blocked backup destination refuses"),FPFSaveFileStore::PreserveOriginal(Slot,TEXT("Blocked"),Error));
    FFileHelper::LoadFileToArray(After,*FPFSaveFileStore::Path(Slot,false));TestTrue(TEXT("Source A unchanged on destination refusal"),After==SourceA);
    FFileHelper::LoadFileToArray(After,*FPFSaveFileStore::Path(Slot,true));TestTrue(TEXT("Source B unchanged on destination refusal"),After==SourceB);
    TestTrue(TEXT("Corrupt original fixture"),FFileHelper::SaveArrayToFile(Bad,*Original));
    TestFalse(TEXT("Never overwrite corrupt original"),FPFSaveFileStore::PreserveOriginal(Slot,Edge,Error));
    Out.Payload=First;Out.Generation=55;TestFalse(TEXT("Corrupt original read refuses"),FPFSaveFileStore::ReadOriginal(Slot,Edge,Out,Error));
    TestTrue(TEXT("Corrupt read preserves outputs"),Out.Payload==First && Out.Generation==55);
    FFileHelper::LoadFileToArray(After,*Original);TestTrue(TEXT("Corrupt original bytes retained"),After==Bad);
    TestTrue(TEXT("Corrupt source fixture"),FFileHelper::SaveArrayToFile(Bad,*FPFSaveFileStore::Path(Slot,true)));
    TestFalse(TEXT("Corrupt sibling blocks a new migration original"),FPFSaveFileStore::PreserveOriginal(Slot,TEXT("OtherEdge"),Error));
    TestFalse(TEXT("No candidate original from corrupt source pair"),IFileManager::Get().FileExists(*FPFSaveFileStore::OriginalPath(Slot,TEXT("OtherEdge"))));
    TestFalse(TEXT("Traversal edge refuses"),FPFSaveFileStore::PreserveOriginal(Slot,TEXT("../escape"),Error));
    AddInfo(TEXT("[PrimalAgentTools] OriginalBackup: real selected envelope preserved, exact byte/generation/UTC, no-overwrite rotation/idempotence, missing/corrupt/blocked/path refusal and unchanged outputs/source. Unique native slots only; no migration/load integration or private save."));return true;
}
#endif
