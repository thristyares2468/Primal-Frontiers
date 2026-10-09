// Isolated real menu travel/save/load smoke. Never uses a personal save slot.
#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "UI/PFMainMenu.h"
#include "Persistence/PFSessionGameInstance.h"
#include "Persistence/PFWorldPersistence.h"
#include "Persistence/PFWorldMenuModel.h"
#include "Persistence/PFSaveFileStore.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFSurvivorCharacter.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Survival/PFPauseMenu.h"
#include "Settings/PFSettingsMenu.h"
#include "Inventory/PFInventoryComponent.h"
#include "Building/PFBuildPiece.h"
#include "Building/PFBuildingCatalog.h"
#include "World/PFWorldClock.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/ComboBoxString.h"
#include "Components/TextBlock.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "UnrealClient.h"
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformTime.h"
#include "Framework/Application/SlateApplication.h"

namespace
{
template<class T>T* WorldMenuWidget(UWorld* W)
{TArray<UUserWidget*> Found;UWidgetBlueprintLibrary::GetAllWidgetsOfClass(W,Found,T::StaticClass(),true);return Found.Num()==1?Cast<T>(Found[0]):nullptr;}
void MenuKey(FKey Key)
{
    auto& Slate=FSlateApplication::Get();const FKeyEvent E(Key,FModifierKeysState(),0,false,0,0);
    Slate.ProcessKeyDownEvent(E);Slate.ProcessKeyUpEvent(E);
}
bool MenuClick(UUserWidget* Menu,const TCHAR* Name)
{
    auto* B=Cast<UButton>(Menu->WidgetTree->FindWidget(FName(Name)));if(!B){return false;}
    // Use supported focus plus actual Slate accept key events. Synthesized
    // pointer hover was unreliable on this background window; handling a down
    // event did not prove a click. No OnClicked broadcast or direct callback.
    B->SetKeyboardFocus();const bool Focused=B->HasKeyboardFocus();MenuKey(EKeys::SpaceBar);return Focused;
}
class FWorldMenuExercise final:public IAutomationLatentCommand
{
public:
    FWorldMenuExercise(FAutomationTestBase* T,FString S,FString D):Test(T),Slot(MoveTemp(S)),Directory(MoveTemp(D)){}
    ~FWorldMenuExercise()
    {
        if(PC.IsValid()){if(auto* P=PC->GetWorld()->GetSubsystem<UPFWorldPersistence>();P && P->ActiveSlot==Slot){P->ActiveSlot.Reset();}PC->SetPauseMenuOpen(false);}
        if(bOwnSlot){for(bool B:{false,true}){IFileManager::Get().Delete(*FPFSaveFileStore::Path(Slot,B));IFileManager::Get().Delete(*FPFSaveFileStore::Path(PFWorldMenuModel::NamesSlot(),B));}}
        if(bOwnOther){for(bool B:{false,true}){IFileManager::Get().Delete(*FPFSaveFileStore::Path(Slot+TEXT("_Other"),B));}}
    }
    bool Update()override
    {
        const double Now=FPlatformTime::Seconds();if(Now-Started>150){Test->AddError(FString::Printf(TEXT("World menu exercise timed out in phase %d"),Phase));return true;}if(Now<Until){return false;}
        UWorld* W=nullptr;for(const auto& C:GEngine->GetWorldContexts()){if(C.World() && C.World()->IsGameWorld()){W=C.World();break;}}
        if(!W){return false;}
        auto* GI=W->GetGameInstance<UPFSessionGameInstance>();if(!GI){Test->AddError(TEXT("Project session GameInstance missing"));return true;}
        if(Phase==0)
        {
            auto* Main=WorldMenuWidget<UPFMainMenu>(W);if(!Main){return false;}
            if(!Test->TestEqual(TEXT("Menu starts in the lightweight Entry map"),UWorld::RemovePIEPrefix(W->GetMapName()),FString(TEXT("Entry")))){return true;}
            FString Error;if(!Test->TestTrue(TEXT("Test owns a fresh slot"),PFWorldMenuModel::CanCreate(Slot,Error))){return true;}
            bOwnSlot=true;Phase=1;Until=Now+0.5;return false;
        }
        if(Phase==1)
        {
            auto* Main=WorldMenuWidget<UPFMainMenu>(W);if(!Main){return false;}
            auto* Name=Cast<UEditableTextBox>(Main->WidgetTree->FindWidget(TEXT("PF_NewWorldName")));
            if(!Test->TestNotNull(TEXT("New world name input"),Name)){return true;}
            FScreenshotRequest::RequestScreenshot(Directory/TEXT("main-menu.png"),true,false);
            Name->SetText(FText::FromString(Slot));Phase=12;Until=Now+2;return false;
        }
        if(Phase==12)
        {
            auto* Main=WorldMenuWidget<UPFMainMenu>(W);if(!Main){return false;}
            Test->TestTrue(TEXT("Main menu screenshot exists"),IFileManager::Get().FileSize(*(Directory/TEXT("main-menu.png")))>0);
            Test->TestTrue(TEXT("Keyboard opens Settings from main menu"),MenuClick(Main,TEXT("PF_MenuSettings")));Phase=17;Until=Now+0.5;return false;
        }
        if(Phase==17)
        {
            auto* Settings=WorldMenuWidget<UPFSettingsMenu>(W);if(!Test->TestNotNull(TEXT("Main-menu settings visible"),Settings)){return true;}
            MenuKey(EKeys::Escape);Phase=18;Until=Now+0.5;return false;
        }
        if(Phase==18)
        {
            auto* Main=WorldMenuWidget<UPFMainMenu>(W);if(!Main){return false;}
            Test->TestTrue(TEXT("Settings cancel returns menu focus"),Main->HasKeyboardFocus());
            Test->TestTrue(TEXT("Keyboard opens Multiplayer screen"),MenuClick(Main,TEXT("PF_Multiplayer")));Phase=13;Until=Now+0.5;return false;
        }
        if(Phase==13)
        {
            FScreenshotRequest::RequestScreenshot(Directory/TEXT("multiplayer-menu.png"),true,false);Phase=14;Until=Now+2;return false;
        }
        if(Phase==14)
        {
            auto* Main=WorldMenuWidget<UPFMainMenu>(W);if(!Main){return false;}
            Test->TestTrue(TEXT("Multiplayer screenshot exists"),IFileManager::Get().FileSize(*(Directory/TEXT("multiplayer-menu.png")))>0);
            MenuKey(EKeys::Escape);Test->TestTrue(TEXT("Keyboard opens Single player screen"),MenuClick(Main,TEXT("PF_SinglePlayer")));Phase=15;Until=Now+0.5;return false;
        }
        if(Phase==15)
        {
            auto* Main=WorldMenuWidget<UPFMainMenu>(W);if(!Main){return false;}
            FString Error;Test->TestFalse(TEXT("Path traversal is refused without travel"),GI->StartWorld(TEXT("../unsafe"),false,Error));
            Test->TestFalse(TEXT("Reserved profile is refused"),GI->StartWorld(TEXT("Identity_MenuTest"),false,Error));
            Test->TestFalse(TEXT("Missing world load is refused without fresh start"),GI->StartWorld(Slot,true,Error));
            Test->TestTrue(TEXT("Keyboard activates New world"),MenuClick(Main,TEXT("PF_NewWorld")));Phase=2;Until=Now+1;return false;
        }
        if(Phase==2)
        {
            auto* C=Cast<APFSurvivalPlayerController>(W->GetFirstPlayerController());
            if(!C || !C->GetPawn() || !C->GetInventory() || !C->GetPlayerSetupStatusText().ToString().Contains(TEXT("new survivor"))){return false;}
            PC=C;Pawn=Cast<APFSurvivorCharacter>(C->GetPawn());if(!Test->TestNotNull(TEXT("First-person survivor spawned"),Pawn.Get())){return true;}
            Test->TestEqual(TEXT("New world request sets selected slot"),W->GetSubsystem<UPFWorldPersistence>()->ActiveSlot,Slot);
            Test->TestTrue(TEXT("Seed bounded save inventory"),C->GetInventory()->Grant(TEXT("Item_Wood"),4));Pawn->Survival->SetHealth(65);Position=Pawn->GetActorLocation();
            auto* Catalog=NewObject<UPFBuildingCatalog>();const FTransform At(Position+FVector(1000,0,-90));
            auto* Base=W->SpawnActorDeferred<APFBuildPiece>(APFBuildPiece::StaticClass(),At);Base->Initialize(*Catalog->Find(TEXT("Build_Foundation")),C->PlayerState,nullptr);Base->FinishSpawning(At);Base->Health=75;Structure=Base;StructureId=Base->PersistentId;
            for(TActorIterator<APFWorldClock> It(W);It;++It){It->SetHour(22);}
            FSlateApplication::Get().SetAllUserFocusToGameViewport();Phase=3;Until=Now+0.5;return false;
        }
        if(Phase==3){MenuKey(EKeys::P);Phase=4;Until=Now+0.5;return false;}
        if(Phase==4)
        {
            auto* Pause=WorldMenuWidget<UPFPauseMenu>(W);
            if(!Test->TestNotNull(TEXT("P opens Pause after menu travel"),Pause)){return true;}
            Position=Pawn->GetActorLocation(); // Capture settled position immediately before the save action.
            for(int32 I=0;I<4;++I){MenuKey(EKeys::Down);}MenuKey(EKeys::Enter);
            Test->TestTrue(TEXT("Pause Save acknowledges actual success"),PC->GetWorldSaveFeedback().StartsWith(TEXT("Saved world: ")));
            Test->TestTrue(TEXT("Save retains live pawn and structure"),PC->GetPawn()==Pawn.Get() && Structure.IsValid());
            Test->TestEqual(TEXT("Save retains inventory"),PC->GetInventory()->Count(TEXT("Item_Wood")),4);
            Test->TestEqual(TEXT("Save retains health"),Pawn->Survival->GetVitals().Health,65.f);
            Test->TestTrue(TEXT("Save retains position"),Pawn->GetActorLocation().Equals(Position,0.1));
            FPFWorldMenuEntry E;Test->TestTrue(TEXT("Saved slot is validated and loadable"),PFWorldMenuModel::Inspect(Slot,E));
            FString Error;Test->TestFalse(TEXT("New world cannot replace this save"),PFWorldMenuModel::CanCreate(Slot,Error));
            FScreenshotRequest::RequestScreenshot(Directory/TEXT("saved-pause.png"),true,false);Phase=5;Until=Now+2;return false;
        }
        if(Phase==5)
        {
            if(!Test->TestTrue(TEXT("Saved Pause screenshot exists"),IFileManager::Get().FileSize(*(Directory/TEXT("saved-pause.png")))>0)){return true;}
            // A different solo world can replace the shared endpoint profile.
            // Exercise that real isolated write before reopening this world.
            PC->ClientRememberReconnectCredential(FGuid::NewGuid());
            MenuKey(EKeys::Up);MenuKey(EKeys::Enter); // End session: first confirmation.
            Test->TestTrue(TEXT("End session requires confirmation"),PC->IsPauseMenuOpen());
            MenuKey(EKeys::Enter);Phase=6;Until=Now+1;return false;
        }
        if(Phase==6)
        {
            auto* Main=WorldMenuWidget<UPFMainMenu>(W);if(!Main){return false;}
            Test->TestEqual(TEXT("Ending returns to Main menu"),UWorld::RemovePIEPrefix(W->GetMapName()),FString(TEXT("Entry")));
            Test->TestTrue(TEXT("Single player opens after return"),MenuClick(Main,TEXT("PF_SinglePlayer")));
            auto* Selector=Cast<UComboBoxString>(Main->WidgetTree->FindWidget(TEXT("PF_SavedWorlds")));
            if(!Test->TestNotNull(TEXT("Saved world selector exists"),Selector)){return true;}
            Selector->SetSelectedOption(Slot);Test->TestEqual(TEXT("Saved world available for explicit selection"),Selector->GetSelectedOption(),Slot);
            Phase=19;Until=Now+0.5;return false;
        }
        if(Phase==19)
        {
            auto* Main=WorldMenuWidget<UPFMainMenu>(W);if(!Main){return false;}
            FPFSavedFile Before,After;FString Error;
            if(!Test->TestTrue(TEXT("Read world before rename"),FPFSaveFileStore::Read(Slot,Before,Error))){return true;}
            Test->TestFalse(TEXT("Empty display name refused"),PFWorldMenuModel::Rename(Slot,TEXT("  "),Error));
            Test->TestFalse(TEXT("Control characters refused"),PFWorldMenuModel::Rename(Slot,TEXT("Bad\nName"),Error));
            Test->TestFalse(TEXT("Excessive display length refused"),PFWorldMenuModel::Rename(Slot,FString::ChrN(65,TEXT('A')),Error));
            Test->TestFalse(TEXT("Metadata cannot be used as gameplay save"),W->GetSubsystem<UPFWorldPersistence>()->Save(PFWorldMenuModel::NamesSlot(),Error));
            auto* Input=Cast<UEditableTextBox>(Main->WidgetTree->FindWidget(TEXT("PF_RenameWorldName")));
            if(!Test->TestNotNull(TEXT("Rename input exists"),Input)){return true;}
            Input->SetText(FText::FromString(TEXT("Renamed menu test world")));
            Test->TestTrue(TEXT("Keyboard activates Rename"),MenuClick(Main,TEXT("PF_RenameWorld")));
            Test->TestEqual(TEXT("New display name read back from disk"),PFWorldMenuModel::NameFor(Slot),FString(TEXT("Renamed menu test world")));
            Test->TestTrue(TEXT("Read unchanged world after rename"),FPFSaveFileStore::Read(Slot,After,Error));
            Test->TestEqual(TEXT("Rename preserves world generation"),After.Generation,Before.Generation);
            Test->TestTrue(TEXT("Rename preserves exact gameplay payload"),After.Payload==Before.Payload);
            const FString Other=Slot+TEXT("_Other");
            if(Test->TestTrue(TEXT("Duplicate-name fixture owns a fresh world"),PFWorldMenuModel::CanCreate(Other,Error)))
            {
                bOwnOther=true;
                Test->TestTrue(TEXT("Create isolated duplicate-name fixture"),FPFSaveFileStore::Write(Other,Before.Payload,Error));
                Test->TestFalse(TEXT("Duplicate display name refused case-insensitively"),PFWorldMenuModel::Rename(Other,TEXT("RENAMED MENU TEST WORLD"),Error));
                for(bool B:{false,true}){IFileManager::Get().Delete(*FPFSaveFileStore::Path(Other,B));}bOwnOther=false;
            }
            FPFSavedFile NamesBefore,Corrupt,NamesAfter;
            Test->TestTrue(TEXT("Read owned name registry"),FPFSaveFileStore::Read(PFWorldMenuModel::NamesSlot(),NamesBefore,Error));
            const TArray<uint8> InvalidNames={uint8('{'),uint8('!')};
            Test->TestTrue(TEXT("Publish isolated invalid metadata fixture"),FPFSaveFileStore::Write(PFWorldMenuModel::NamesSlot(),InvalidNames,Error));
            Test->TestTrue(TEXT("Read invalid metadata generation"),FPFSaveFileStore::Read(PFWorldMenuModel::NamesSlot(),Corrupt,Error));
            Test->TestFalse(TEXT("Corrupt name metadata refuses rename"),PFWorldMenuModel::Rename(Slot,TEXT("Must not overwrite"),Error));
            Test->TestTrue(TEXT("Read preserved invalid metadata"),FPFSaveFileStore::Read(PFWorldMenuModel::NamesSlot(),NamesAfter,Error));
            Test->TestTrue(TEXT("Refusal preserves metadata bytes and generation"),NamesAfter.Generation==Corrupt.Generation && NamesAfter.Payload==Corrupt.Payload);
            Test->TestTrue(TEXT("Restore owned valid metadata for loading"),FPFSaveFileStore::Write(PFWorldMenuModel::NamesSlot(),NamesBefore.Payload,Error));
            Main->RefreshWorlds();Test->TestEqual(TEXT("Refresh retains persisted label"),PFWorldMenuModel::NameFor(Slot),FString(TEXT("Renamed menu test world")));
            Phase=7;Until=Now+0.5;return false;
        }
        if(Phase==7)
        {
            auto* Main=WorldMenuWidget<UPFMainMenu>(W);if(!Main){return false;}
            FScreenshotRequest::RequestScreenshot(Directory/TEXT("load-menu.png"),true,false);
            Phase=16;Until=Now+2;return false;
        }
        if(Phase==16)
        {
            auto* Main=WorldMenuWidget<UPFMainMenu>(W);if(!Main){return false;}
            Test->TestTrue(TEXT("Load menu screenshot exists"),IFileManager::Get().FileSize(*(Directory/TEXT("load-menu.png")))>0);
            Test->TestTrue(TEXT("Keyboard activates Load selected world"),MenuClick(Main,TEXT("PF_LoadWorld")));Phase=8;Until=Now+1;return false;
        }
        if(Phase==8)
        {
            auto* C=Cast<APFSurvivalPlayerController>(W->GetFirstPlayerController());
            if(!C || !C->GetPawn() || !C->GetInventory() || !C->GetPlayerSetupStatusText().ToString().Contains(TEXT("restored"))){return false;}
            PC=C;Pawn=Cast<APFSurvivorCharacter>(C->GetPawn());
            Test->TestEqual(TEXT("Load restores inventory"),C->GetInventory()->Count(TEXT("Item_Wood")),4);
            Test->TestEqual(TEXT("Load restores player health"),Pawn->Survival->GetVitals().Health,65.f);
            Test->TestTrue(TEXT("Load restores position"),Pawn->GetActorLocation().Equals(Position,2));
            int32 Found=0;for(TActorIterator<APFBuildPiece> It(W);It;++It){if(It->PersistentId==StructureId){++Found;Test->TestEqual(TEXT("Load restores structure health"),It->Health,75.f);Test->TestTrue(TEXT("Load restores structure ownership"),It->IsOwnedBy(C->PlayerState));}}
            Test->TestEqual(TEXT("Structure restored exactly once"),Found,1);
            for(TActorIterator<APFWorldClock> It(W);It;++It){Test->TestTrue(TEXT("Load restores night"),It->Hour>=22 && It->Hour<22.1f);}
            FSlateApplication::Get().SetAllUserFocusToGameViewport();Phase=9;Until=Now+0.5;return false;
        }
        if(Phase==9){MenuKey(EKeys::P);Phase=10;Until=Now+0.5;return false;}
        if(Phase==10)
        {
            Test->TestTrue(TEXT("P works after loading"),PC->IsPauseMenuOpen());FScreenshotRequest::RequestScreenshot(Directory/TEXT("restored-pause.png"),true,false);Phase=11;Until=Now+2;return false;
        }
        Test->TestTrue(TEXT("Restored screenshot exists"),IFileManager::Get().FileSize(*(Directory/TEXT("restored-pause.png")))>0);
        Test->AddInfo(TEXT("[PrimalUI] Real menu keyboard activation, New/Save/End/Rename/Load travel and live-state conservation verified in one process; invalid/duplicate names and corrupt metadata refused without overwrites. Mouse/physical controller, process restart and Personal full-loop acceptance remain separate."));return true;
    }
private:
    FAutomationTestBase* Test;FString Slot,Directory;int32 Phase=0;bool bOwnSlot=false,bOwnOther=false;double Started=FPlatformTime::Seconds(),Until=0;
    TWeakObjectPtr<APFSurvivalPlayerController> PC;TWeakObjectPtr<APFSurvivorCharacter> Pawn;TWeakObjectPtr<APFBuildPiece> Structure;FGuid StructureId;FVector Position;
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFWorldMenuLiveTest,"PF.UI.WorldMenuLive",EAutomationTestFlags::ClientContext|EAutomationTestFlags::EngineFilter)
bool FPFWorldMenuLiveTest::RunTest(const FString&)
{
    FString Slot,Label;FParse::Value(FCommandLine::Get(),TEXT("PFWorldMenuTestSlot="),Slot);FParse::Value(FCommandLine::Get(),TEXT("PFControlsEvidence="),Label);
    if(!FParse::Param(FCommandLine::Get(),TEXT("PFRunWorldMenuTest")) || FParse::Param(FCommandLine::Get(),TEXT("nullrhi")) || !Slot.StartsWith(TEXT("UIWorld_")) || !FPFSaveFileStore::ValidSlot(Slot))
    {AddError(TEXT("Requires isolated rendered runner with a fresh UIWorld_ slot"));return false;}
    const FString Names=PFWorldMenuModel::NamesSlot();
    if(!Names.StartsWith(TEXT("Metadata_UIWorld_")) || IFileManager::Get().FileExists(*FPFSaveFileStore::Path(Names,false)) || IFileManager::Get().FileExists(*FPFSaveFileStore::Path(Names,true)))
    {AddError(TEXT("Requires a fresh isolated world-name registry"));return false;}
    bool bLabelValid=!Label.IsEmpty() && Label.Len()<=48;for(TCHAR C:Label){bLabelValid&=FChar::IsAlnum(C) || C==TEXT('_');}
    if(!bLabelValid)
    {AddError(TEXT("Invalid unique evidence label"));return false;}
    // Existing uncooked map/renderer baseline, individually reviewed. Travel
    // brings these into the test interval; do not suppress unrelated warnings.
    AddExpectedMessage(TEXT("Failed to find script package for import object 'Package /Script/WorldPartitionHLODUtilities'"),EAutomationExpectedMessageFlags::Contains,1);
    AddExpectedMessage(TEXT("Unable to load HLODBuilderMeshMergeSettings_1"),EAutomationExpectedMessageFlags::Contains,1);
    AddExpectedMessage(TEXT("Unable to load HLODBuilderInstancingSettings_0"),EAutomationExpectedMessageFlags::Contains,1);
    AddExpectedMessage(TEXT("Unable to find RecastNavMesh instance while trying to create UCrowdManager instance"),EAutomationExpectedMessageFlags::Contains,1);
    AddExpectedMessage(TEXT("Setting the console variable 'r.MotionBlurQuality' with 'SetByScalability' was ignored"),EAutomationExpectedMessageFlags::Contains,1);
    AddExpectedMessage(TEXT("Setting the console variable 'r.DepthOfFieldQuality' with 'SetByScalability' was ignored"),EAutomationExpectedMessageFlags::Contains,1);
    const FString Directory=FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()/TEXT("AutomationReports/ControlsUI")/Label);
    if(IFileManager::Get().DirectoryExists(*Directory)){AddError(TEXT("Refusing reused evidence directory"));return false;}
    IFileManager::Get().MakeDirectory(*Directory,true);ADD_LATENT_AUTOMATION_COMMAND(FWorldMenuExercise(this,Slot,Directory));return true;
}
#endif
