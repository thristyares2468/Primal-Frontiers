#include "PFCommands.h"
#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Persistence/PFSaveFileStore.h"
#include "Persistence/PFWorldPersistence.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "HAL/IConsoleManager.h"
#include "HAL/FileManager.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/ScopeExit.h"
#include "Async/Async.h"
#include "Tests/AutomationCommon.h"
using namespace PF::AgentTools;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFOriginalCopyCommandTest,"PF.PrimalAgentTools.OriginalCopyCommand",
    EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FPFOriginalCopyCommandTest::RunTest(const FString&)
{
    const FString Token=FGuid::NewGuid().ToString(EGuidFormats::Digits);
    const FString Source=TEXT("AutomationCmdSource")+Token,Target=TEXT("AutomationCmdTarget")+Token;
    const FString ConsoleTarget=TEXT("AutomationCmdConsole")+Token,Denied=TEXT("AutomationCmdDenied")+Token,Edge=TEXT("CommandCopy");
    const FString Original=FPFSaveFileStore::OriginalPath(Source,Edge);
    ON_SCOPE_EXIT
    {
        IFileManager::Get().Delete(*Original);
        IFileManager::Get().DeleteDirectory(*FPaths::GetPath(Original),false,false);
        for(const FString& Slot:{Source,Target,ConsoleTarget,Denied})
        {for(bool Second:{false,true}){IFileManager::Get().Delete(*FPFSaveFileStore::Path(Slot,Second));}}
    };
    FString Why;FPFSavedFile Old;Old.Generation=37;Old.SavedUtc=FDateTime::UtcNow().ToUnixTimestamp()-172800;Old.Payload={1,0,2};
    TArray<uint8> Bytes,Read;
    if(!TestTrue(TEXT("Encode original fixture"),FPFSaveFileStore::EncodeEnvelope(Old,Bytes,Why)) ||
        !TestTrue(TEXT("Write source fixture"),FFileHelper::SaveArrayToFile(Bytes,*FPFSaveFileStore::Path(Source,false))) ||
        !TestTrue(TEXT("Preserve fixture original"),FPFSaveFileStore::PreserveOriginal(Source,Edge,Why))){return false;}
    auto Run=[&](UWorld* World,const FString& Destination){return ExecuteCommand(TEXT("PF.CopyOriginalSave"),{Source,Edge,Destination},World,false);};
    auto Code=[](const FResult& R,const TCHAR* Name){return R.Issues.ContainsByPredicate([Name](const auto& I){return I.Code==Name;});};
    TestTrue(TEXT("No world refuses before filesystem operation"),Code(Run(nullptr,Denied),TEXT("NotAuthority")));
    auto Worker=Async(EAsyncExecution::ThreadPool,[Source,Edge,Denied](){return ExecuteCommand(TEXT("PF.CopyOriginalSave"),{Source,Edge,Denied},nullptr,false);});
    TestTrue(TEXT("Off-thread command refused"),Code(Worker.Get(),TEXT("WrongThread")));
    FTestWorldWrapper Fixture;if(!Fixture.CreateTestWorld(EWorldType::Game)){return false;}auto* W=Fixture.GetTestWorld();
    TestTrue(TEXT("Uninitialized game world refuses"),Code(Run(W,Denied),TEXT("NotAuthority")));
    W->SetGameMode(FURL(nullptr,TEXT("/Engine/Maps/Entry?game=/Script/Engine.GameModeBase"),TRAVEL_Absolute));
    if(!TestTrue(TEXT("Begin authoritative disposable world"),Fixture.BeginPlayInTestWorld()) || !TestNotNull(TEXT("Authority game mode"),W->GetAuthGameMode())){return false;}
    auto* Persistence=W->GetSubsystem<UPFWorldPersistence>();const FString ActiveBefore=Persistence->ActiveSlot;const double TimeBefore=W->GetTimeSeconds();
    const auto Copied=Run(W,Target);
    TestTrue(TEXT("Real command invokes copy"),!Copied.HasErrors() && Code(Copied,TEXT("OriginalCopied")));
    TestTrue(TEXT("Result explicitly disclaims semantic validity and live load"),Copied.Issues.ContainsByPredicate([](const auto& I){return I.Message.Contains(TEXT("semantic validity is not certified")) && I.Message.Contains(TEXT("live world and active slot unchanged"));}));
    TestTrue(TEXT("Command output bytes are exact"),FFileHelper::LoadFileToArray(Read,*FPFSaveFileStore::Path(Target,false)) && Read==Bytes);
    FPFSavedFile Restored;TestTrue(TEXT("Generation/time not renewed"),FPFSaveFileStore::Read(Target,Restored,Why) && Restored.Generation==37 && Restored.SavedUtc==Old.SavedUtc && Restored.Payload==Old.Payload);
    TestTrue(TEXT("Repeat rejects occupied destination"),Code(Run(W,Target),TEXT("RecoveryCopyRejected")));
    TestTrue(TEXT("Refusal retains destination bytes"),FFileHelper::LoadFileToArray(Read,*FPFSaveFileStore::Path(Target,false)) && Read==Bytes);
    TestTrue(TEXT("Missing original edge refused"),ExecuteCommand(TEXT("PF.CopyOriginalSave"),{Source,TEXT("Missing"),Denied},W,false).HasErrors());
    const auto TypeBefore=W->WorldType;W->WorldType=EWorldType::Editor;
    TestTrue(TEXT("Editor world refuses"),Code(Run(W,Denied),TEXT("NotAuthority")));W->WorldType=TypeBefore;
    TestFalse(TEXT("Refusals created no destination"),IFileManager::Get().FileExists(*FPFSaveFileStore::Path(Denied,false)));
    const FString Line=FString::Printf(TEXT("PF.CopyOriginalSave %s %s %s"),*Source,*Edge,*ConsoleTarget);
    TestTrue(TEXT("Actual registered console dispatch"),IConsoleManager::Get().ProcessUserConsoleInput(*Line,*GLog,W));
    TestTrue(TEXT("Console result recorded in report history"),CommandHistory().Last().Command==TEXT("PF.CopyOriginalSave") && !CommandHistory().Last().HasErrors() && Code(CommandHistory().Last(),TEXT("OriginalCopied")));
    TestTrue(TEXT("Console copy exact"),FFileHelper::LoadFileToArray(Read,*FPFSaveFileStore::Path(ConsoleTarget,false)) && Read==Bytes);
    TestTrue(TEXT("Original and live source unchanged"),FFileHelper::LoadFileToArray(Read,*Original) && Read==Bytes && FFileHelper::LoadFileToArray(Read,*FPFSaveFileStore::Path(Source,false)) && Read==Bytes);
    TestTrue(TEXT("Active slot and running world unchanged"),Persistence->ActiveSlot==ActiveBefore && W->GetTimeSeconds()==TimeBefore && W->GetAuthGameMode()->GetClass()==AGameModeBase::StaticClass());
    AddInfo(TEXT("[PrimalAgentTools] OriginalCopyCommand: actual console/API copy, report history, exact old time/generation, occupied/authority/thread/editor refusal and unchanged live slot verified. Envelope-only; payload semantics intentionally not loaded."));
    Fixture.ForwardErrorMessages(this);return true;
}
#endif
