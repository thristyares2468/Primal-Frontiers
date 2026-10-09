// Real windowed display confirmation/rollback. No physical fullscreen switch.
#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Settings/PFGameUserSettings.h"
#include "Settings/PFSettingsMenu.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFPauseMenu.h"
#include "Inventory/PFInventoryComponent.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameViewportClient.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UnrealClient.h"
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Misc/ConfigCacheIni.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformTime.h"
#include "HAL/IConsoleManager.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/SWindow.h"

namespace
{
template<class T> T* DisplayWidget(UWorld* World)
{
    TArray<UUserWidget*> Widgets;UWidgetBlueprintLibrary::GetAllWidgetsOfClass(World,Widgets,T::StaticClass(),true);
    return Widgets.Num()==1?Cast<T>(Widgets[0]):nullptr;
}
void DisplayKey(FKey Key)
{FSlateApplication::Get().ProcessKeyDownEvent(FKeyEvent(Key,FModifierKeysState(),uint32(0),false,0,0));}
FString DisplayCaption(UPFSettingsMenu* Menu,const TCHAR* Prefix)
{
    FString Found;auto Read=[&](UWidget* W){if(auto* T=Cast<UTextBlock>(W);T && T->GetText().ToString().StartsWith(Prefix)){Found=T->GetText().ToString();}};
    Menu->WidgetTree->ForEachWidget([&](UWidget* W){Read(W);if(auto* Row=Cast<UPFSettingsRow>(W)){Row->WidgetTree->ForEachWidget(Read);}});return Found;
}
FString DisplayDestination()
{
    const auto* Branch=GConfig?GConfig->FindBranch(FName(TEXT("GameUserSettings")),GGameUserSettingsIni):nullptr;
    return Branch?Branch->IniPath:FString();
}
class FSettingsDisplayExercise final : public IAutomationLatentCommand
{
public:
    FSettingsDisplayExercise(FAutomationTestBase* T,FString D,FString F):Test(T),Directory(MoveTemp(D)),Filename(MoveTemp(F)){}
    ~FSettingsDisplayExercise(){if(PC.IsValid()){PC->SetPauseMenuOpen(false);}}
    bool Update() override
    {
        const double Now=FPlatformTime::Seconds();if(Now-Started>60){Test->AddError(TEXT("Windowed display fixture timed out"));return true;}
        if(Now<Until){return false;}auto* Live=UPFGameUserSettings::Get();
        if(Phase==0)
        {
            for(const auto& C:GEngine->GetWorldContexts())
            {
                auto* W=C.World();if(!W || !W->IsGameWorld() || W->GetNetMode()!=NM_Standalone){continue;}
                auto* Candidate=Cast<APFSurvivalPlayerController>(W->GetFirstPlayerController());
                if(Candidate && Candidate->GetLocalPlayer() && Candidate->GetPawn() && Candidate->GetInventory()){PC=Candidate;break;}
            }
            if(!PC.IsValid() || !Live || !GEngine->GameViewport || !GEngine->GameViewport->Viewport){return false;}
            Launch=GEngine->GameViewport->Viewport->GetSizeXY();Preferences=Live->Preferences;Quality=Live->ScalabilityQuality;
            UKismetSystemLibrary::GetSupportedFullscreenResolutions(Modes);
            for(const auto R:Modes)
            {
                if(R.X>=800 && R.Y>=600 && R.X<=Launch.X && R.Y<=Launch.Y && R!=Launch && int64(R.X)*R.Y>int64(Smaller.X)*Smaller.Y){Smaller=R;}
            }
            if(!Test->TestTrue(TEXT("Hardware offers a supported smaller bounded window resolution"),Smaller.X>0 && Smaller.Y>0)){return true;}
            if(!Test->TestTrue(TEXT("Launch resolution is supported for the rollback exercise"),Modes.Contains(Launch))){return true;}
            if(!Test->TestTrue(TEXT("Seed inventory for display modal conservation"),PC->GetInventory()->Grant(TEXT("Item_Wood"),3))){return true;}
            Wood=PC->GetInventory()->Count(TEXT("Item_Wood"));PC->SetPauseMenuOpen(true);Pause=DisplayWidget<UPFPauseMenu>(PC->GetWorld());
            if(!Test->TestNotNull(TEXT("Pause exists"),Pause.Get())){return true;}
            Test->AddInfo(FString::Printf(TEXT("[PrimalSettings] Windowed launch %dx%d; bounded confirmation target %dx%d."),Launch.X,Launch.Y,Smaller.X,Smaller.Y));
            Phase=1;Until=Now+0.5;return false;
        }
        if(!PC.IsValid() || !Pause.IsValid() || !Live){Test->AddError(TEXT("Lost display fixture"));return true;}
        if(Phase==1)
        {
            DisplayKey(EKeys::Gamepad_DPad_Down);DisplayKey(EKeys::Gamepad_FaceButton_Bottom);Menu=DisplayWidget<UPFSettingsMenu>(PC->GetWorld());
            if(!Test->TestNotNull(TEXT("Real Pause input opens Settings"),Menu.Get())){return true;}
            Phase=2;Until=Now+0.5;return false;
        }
        if(!Menu.IsValid()){Test->AddError(TEXT("Lost display menu"));return true;}
        if(Phase==2)
        {
            DisplayKey(EKeys::Gamepad_RightShoulder);DisplayKey(EKeys::Down); // Graphics/display mode.
            for(int32 I=0;I<3 && DisplayCaption(Menu.Get(),TEXT("Display:"))!=TEXT("Display: Windowed");++I){DisplayKey(EKeys::Right);}
            if(!Test->TestTrue(TEXT("Only a Windowed draft will be applied"),DisplayCaption(Menu.Get(),TEXT("Display:"))==TEXT("Display: Windowed"))){return true;}
            DisplayKey(EKeys::Down);
            if(!SetDraftResolution(Smaller)){return true;}
            DisplayKey(EKeys::Enter);Phase=3;Until=Now+0.75;return false;
        }
        if(Phase==3)
        {
            CheckWindow(Smaller);Test->TestTrue(TEXT("Actual pending confirmation displayed"),DisplayCaption(Menu.Get(),TEXT("Keep display?")).Len()>0);
            Shot=Directory/TEXT("pending.png");FScreenshotRequest::RequestScreenshot(Shot,true,false);Phase=4;Until=Now+1.5;return false;
        }
        if(Phase==4)
        {
            if(IFileManager::Get().FileSize(*Shot)<=0){return false;}
            DisplayKey(EKeys::Gamepad_FaceButton_Bottom);
            Test->TestTrue(TEXT("Real Keep input confirms display"),DisplayCaption(Menu.Get(),TEXT("Display confirmed. Settings saved.")).Len()>0);
            CheckConfirmed();Shot=Directory/TEXT("confirmed.png");FScreenshotRequest::RequestScreenshot(Shot,true,false);Phase=5;Until=Now+1.5;return false;
        }
        if(Phase==5)
        {
            if(IFileManager::Get().FileSize(*Shot)<=0){return false;}
            if(!SetDraftResolution(Launch)){return true;}DisplayKey(EKeys::Enter);Phase=6;Until=Now+0.75;return false;
        }
        if(Phase==6)
        {
            CheckWindow(Launch);DisplayKey(EKeys::Escape);
            Test->TestTrue(TEXT("Back removes Settings and restores Pause focus"),!Menu->IsInViewport() && Pause->HasKeyboardFocus());
            Phase=7;Until=Now+0.75;return false;
        }
        if(Phase==7)
        {
            CheckWindow(Smaller);CheckConfirmed();DisplayKey(EKeys::Enter);Menu=DisplayWidget<UPFSettingsMenu>(PC->GetWorld());
            if(!Test->TestNotNull(TEXT("Settings reopens after display cancellation"),Menu.Get())){return true;}
            DisplayKey(EKeys::Gamepad_RightShoulder);DisplayKey(EKeys::Down);DisplayKey(EKeys::Down);
            if(!SetDraftResolution(Launch)){return true;}DisplayKey(EKeys::Enter);TimeoutStarted=Now;Phase=8;Until=Now+0.75;return false;
        }
        if(Phase==8)
        {
            CheckWindow(Launch);Test->TestTrue(TEXT("Timeout path starts with a real pending display"),DisplayCaption(Menu.Get(),TEXT("Keep display?")).Len()>0);
            Test->TestTrue(TEXT("World remains paused while real-time timer runs"),PC->IsPaused());
            Phase=9;Until=TimeoutStarted+16;return false; // Actual shipping UI's 15-second timer; no shortcut.
        }
        if(Phase==9)
        {
            Test->TestTrue(TEXT("Display timeout really waited at least 15 seconds"),Now-TimeoutStarted>=15);
            Test->TestTrue(TEXT("Real timeout message visible"),DisplayCaption(Menu.Get(),TEXT("Display reverted after timeout.")).Len()>0);
            CheckWindow(Smaller);CheckConfirmed();
            Test->TestTrue(TEXT("Timed-out menu retains focus"),Menu->HasKeyboardFocus());
            Shot=Directory/TEXT("timeout.png");FScreenshotRequest::RequestScreenshot(Shot,true,false);Phase=10;Until=Now+1.5;return false;
        }
        if(Phase==10)
        {
            if(IFileManager::Get().FileSize(*Shot)<=0){return false;}
            // Guarded engine reload proves saved confirmed state rather than a UI-only change.
            Test->TestTrue(TEXT("Confirmed display has a real disposable config"),IFileManager::Get().FileSize(*Filename)>0);
            Live->SetScreenResolution(Launch);Live->LoadSettings(true);CheckConfirmed();
            DisplayKey(EKeys::Escape);Test->TestTrue(TEXT("Final Back restores Pause focus"),Pause->HasKeyboardFocus());PC->SetPauseMenuOpen(false);
            Test->TestFalse(TEXT("Resume restores movement"),PC->IsMoveInputIgnored());
            Test->TestEqual(TEXT("Display input conserves inventory"),PC->GetInventory()->Count(TEXT("Item_Wood")),Wood);
            Test->AddInfo(TEXT("[PrimalSettings] Actual smaller window confirmed; Back and real 15-second paused timeout restored confirmed viewport/file state. No physical fullscreen/monitor/HDR or manual gameplay certification."));return true;
        }
        Test->AddError(TEXT("Unexpected display fixture phase"));return true;
    }
private:
    bool SetDraftResolution(FIntPoint R)
    {
        const FString Expected=FString::Printf(TEXT("Resolution: %d x %d"),R.X,R.Y);
        for(int32 I=0;I<=Modes.Num() && DisplayCaption(Menu.Get(),TEXT("Resolution:"))!=Expected;++I){DisplayKey(EKeys::Left);}
        return Test->TestTrue(TEXT("Actual resolution-row input selects bounded target"),DisplayCaption(Menu.Get(),TEXT("Resolution:"))==Expected);
    }
    void CheckWindow(FIntPoint R)
    {
        Test->TestTrue(TEXT("Actual rendered viewport matches requested size"),GEngine->GameViewport->Viewport->GetSizeXY()==R);
        const auto Window=GEngine->GameViewport->GetWindow();
        Test->TestTrue(TEXT("Actual window remains Windowed"),Window.IsValid() && Window->GetWindowMode()==EWindowMode::Windowed);
        auto* Live=UPFGameUserSettings::Get();Test->TestTrue(TEXT("Settings match requested window size/mode"),Live->GetScreenResolution()==R && Live->GetFullscreenMode()==EWindowMode::Windowed);
        Test->TestTrue(TEXT("Display changes conserve all local non-display preferences/quality"),FPFLocalPreferences::StaticStruct()->CompareScriptStruct(&Preferences,&Live->Preferences,0) && Quality==Live->ScalabilityQuality);
        Test->TestTrue(TEXT("Display exercise really remains uncapped"),Live->GetFrameRateLimit()==0 && !Live->IsVSyncEnabled() && IConsoleManager::Get().FindConsoleVariable(TEXT("r.VSync"))->GetInt()==0 && IConsoleManager::Get().FindConsoleVariable(TEXT("t.MaxFPS"))->GetFloat()==0);
    }
    void CheckConfirmed()
    {
        auto* Live=UPFGameUserSettings::Get();Test->TestTrue(TEXT("Confirmed resolution is the kept smaller window"),Live->GetScreenResolution()==Smaller && Live->GetLastConfirmedScreenResolution()==Smaller);
        Test->TestTrue(TEXT("Confirmed mode remains Windowed"),Live->GetFullscreenMode()==EWindowMode::Windowed && Live->GetLastConfirmedFullscreenMode()==EWindowMode::Windowed);
    }
    FAutomationTestBase* Test;FString Directory,Filename,Shot;TArray<FIntPoint> Modes;FIntPoint Launch,Smaller=FIntPoint::ZeroValue;
    FPFLocalPreferences Preferences;Scalability::FQualityLevels Quality;int32 Phase=0,Wood=0;double Started=FPlatformTime::Seconds(),Until=0,TimeoutStarted=0;
    TWeakObjectPtr<APFSurvivalPlayerController> PC;TWeakObjectPtr<UPFPauseMenu> Pause;TWeakObjectPtr<UPFSettingsMenu> Menu;
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFSettingsDisplayLiveTest,"PF.UI.SettingsDisplayLive",EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)
bool FPFSettingsDisplayLiveTest::RunTest(const FString&)
{
    FString Label,Profile;FParse::Value(FCommandLine::Get(),TEXT("PFControlsEvidence="),Label);FParse::Value(FCommandLine::Get(),TEXT("PFIdentityProfile="),Profile);
    if(!FParse::Param(FCommandLine::Get(),TEXT("PFRunSettingsDisplayTest")) || !FParse::Param(FCommandLine::Get(),TEXT("PFRunControlsUITest")) ||
        FParse::Param(FCommandLine::Get(),TEXT("nullrhi")) || FParse::Param(FCommandLine::Get(),TEXT("NoSaveConfig")) || GIsEditor ||
        !Label.StartsWith(TEXT("M11Display")) || Label.Len()>48 || !Profile.StartsWith(TEXT("UI")) || Profile.Len()!=14)
    {AddError(TEXT("Use isolated rendered RunControlsAutomation.ps1 -TestCase Display"));return false;}
    for(TCHAR C:Label+Profile){if(!FChar::IsAlnum(C) && C!=TEXT('_')){AddError(TEXT("Invalid display label/profile"));return false;}}
    const FString Target=FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()/TEXT("AutomationReports")/Label/TEXT("GameUserSettings.ini"));const FString Destination=DisplayDestination();
    if(!TestTrue(TEXT("Display save destination belongs only to this run"),!Destination.IsEmpty() && FPaths::IsSamePath(FPaths::ConvertRelativePathToFull(Destination),Target))){return false;}
    const FString Directory=FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()/TEXT("AutomationReports/ControlsUI")/Label);
    if(IFileManager::Get().DirectoryExists(*Directory)){AddError(TEXT("Refusing reused display evidence directory"));return false;}
    IFileManager::Get().MakeDirectory(*Directory,true);IFileManager::Get().MakeDirectory(*FPaths::GetPath(Target),true);
    ADD_LATENT_AUTOMATION_COMMAND(FSettingsDisplayExercise(this,Directory,Target));return true;
}
#endif
