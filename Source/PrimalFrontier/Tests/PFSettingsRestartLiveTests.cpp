// Opt-in actual menu writes followed by a separate-process read. Never personal config.
#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Settings/PFGameUserSettings.h"
#include "Settings/PFSettingsMenu.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFPauseMenu.h"
#include "Survival/PFSurvivorCharacter.h"
#include "Survival/PFSurvivalHUD.h"
#include "Inventory/PFInventoryComponent.h"
#include "Camera/CameraComponent.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "UnrealClient.h"
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"
#include "HAL/IConsoleManager.h"
#include "Framework/Application/SlateApplication.h"

namespace
{
template<class T> T* RestartWidget(UWorld* World)
{
    TArray<UUserWidget*> Widgets;UWidgetBlueprintLibrary::GetAllWidgetsOfClass(World,Widgets,T::StaticClass(),true);
    return Widgets.Num()==1?Cast<T>(Widgets[0]):nullptr;
}
void RestartKey(FKey Key)
{FSlateApplication::Get().ProcessKeyDownEvent(FKeyEvent(Key,FModifierKeysState(),uint32(0),false,0,0));}
FString RestartCaption(UPFSettingsMenu* Menu,const TCHAR* Prefix)
{
    FString Found;
    auto Read=[&](UWidget* W){if(auto* T=Cast<UTextBlock>(W);T && T->GetText().ToString().StartsWith(Prefix)){Found=T->GetText().ToString();}};
    Menu->WidgetTree->ForEachWidget([&](UWidget* W){Read(W);if(auto* Row=Cast<UPFSettingsRow>(W)){Row->WidgetTree->ForEachWidget(Read);}});
    return Found;
}
FPFLocalPreferences RestartExpected()
{
    FPFLocalPreferences Expected;Expected.FieldOfView=95;Expected.bMotionBlur=true;
    Expected.MasterVolume=0.9f;Expected.HUDScale=1.25f;return Expected;
}
class FSettingsRestartExercise final : public IAutomationLatentCommand
{
public:
    FSettingsRestartExercise(FAutomationTestBase* T,FString D,FString F,bool V):Test(T),Directory(MoveTemp(D)),Filename(MoveTemp(F)),bVerify(V){}
    ~FSettingsRestartExercise(){if(PC.IsValid()){PC->SetPauseMenuOpen(false);}}
    bool Update() override
    {
        const double Now=FPlatformTime::Seconds();
        if(Now-Started>60){Test->AddError(FString::Printf(TEXT("Settings restart timeout stage%d"),Stage));return true;}
        if(Now<Until || (!Shot.IsEmpty() && IFileManager::Get().FileSize(*Shot)<=0)){return false;}
        auto* Live=UPFGameUserSettings::Get();
        if(Stage==0)
        {
            for(const auto& C:GEngine->GetWorldContexts())
            {
                auto* W=C.World();if(!W || !W->IsGameWorld() || W->GetNetMode()!=NM_Standalone){continue;}
                auto* Candidate=Cast<APFSurvivalPlayerController>(W->GetFirstPlayerController());
                if(Candidate && Candidate->GetLocalPlayer() && Cast<APFSurvivorCharacter>(Candidate->GetPawn()) && Candidate->GetInventory()){PC=Candidate;break;}
            }
            if(!PC.IsValid() || !Live){return false;}
            if(bVerify)
            {
                // Assert startup-loaded data before any fixture Apply, LoadSettings or mutation.
                CheckPreferences();
                if(!Test->TestTrue(TEXT("Read source INI loaded by fresh process"),FFileHelper::LoadFileToString(SourceBefore,*Filename))){return true;}
            }
            else
            {
                const FPFLocalPreferences Defaults;
                if(!Test->TestTrue(TEXT("Unique preparation starts with independent known defaults"),FPFLocalPreferences::StaticStruct()->CompareScriptStruct(&Defaults,&Live->Preferences,0))){return true;}
            }
            Quality=Live->ScalabilityQuality;Resolution=Live->GetScreenResolution();Mode=Live->GetFullscreenMode();
            if(!Test->TestTrue(TEXT("Trusted modal fixture seeds three wood"),PC->GetInventory()->Grant(TEXT("Item_Wood"),3))){return true;}
            Wood=PC->GetInventory()->Count(TEXT("Item_Wood"));
            PC->SetPauseMenuOpen(true);Pause=RestartWidget<UPFPauseMenu>(PC->GetWorld());
            if(!Test->TestNotNull(TEXT("Actual Pause exists"),Pause.Get())){return true;}
            Stage=1;Until=Now+0.5;return false;
        }
        if(!PC.IsValid() || !Pause.IsValid() || !Live){Test->AddError(TEXT("Lost settings restart fixture"));return true;}
        if(Stage==1)
        {
            RestartKey(EKeys::Gamepad_DPad_Down);RestartKey(EKeys::Gamepad_FaceButton_Bottom);
            Menu=RestartWidget<UPFSettingsMenu>(PC->GetWorld());
            if(!Test->TestNotNull(TEXT("Actual Pause input opens Settings"),Menu.Get())){return true;}
            Stage=2;Until=Now+0.5;return false;
        }
        if(Stage>=2 && Stage<=7 && !Menu.IsValid()){Test->AddError(TEXT("Lost settings restart menu"));return true;}
        if(Stage==2)
        {
            if(!bVerify)
            {
                RestartKey(EKeys::Right);RestartKey(EKeys::Gamepad_RightShoulder);
                for(int32 I=0;I<6;++I){RestartKey(EKeys::Down);}RestartKey(EKeys::Right);
                RestartKey(EKeys::Gamepad_RightShoulder);RestartKey(EKeys::Left);
                RestartKey(EKeys::Gamepad_RightShoulder);RestartKey(EKeys::Right);
                const FPFLocalPreferences Defaults;
                Test->TestTrue(TEXT("Draft changes do not mutate live preferences"),FPFLocalPreferences::StaticStruct()->CompareScriptStruct(&Defaults,&Live->Preferences,0));
                RestartKey(EKeys::Gamepad_FaceButton_Bottom);CheckPreferences();
                Test->TestTrue(TEXT("Actual Apply reports saved"),!RestartCaption(Menu.Get(),TEXT("Settings applied and saved.")).IsEmpty());
                Test->TestTrue(TEXT("Actual menu creates isolated INI"),IFileManager::Get().FileSize(*Filename)>0);
                Capture(TEXT("prepared.png"));Stage=7;return false;
            }
            Test->TestEqual(TEXT("Fresh Game tab shows disk FOV"),RestartCaption(Menu.Get(),TEXT("Field of view:")),FString(TEXT("Field of view: 95")));
            Capture(TEXT("restart_game.png"));Stage=3;return false;
        }
        if(Stage==3)
        {
            RestartKey(EKeys::Gamepad_RightShoulder);
            Test->TestEqual(TEXT("Fresh Graphics tab shows disk blur"),RestartCaption(Menu.Get(),TEXT("Motion blur:")),FString(TEXT("Motion blur: On")));
            Capture(TEXT("restart_graphics.png"));Stage=4;return false;
        }
        if(Stage==4)
        {
            RestartKey(EKeys::Gamepad_RightShoulder);
            Test->TestEqual(TEXT("Fresh Audio tab shows disk master volume"),RestartCaption(Menu.Get(),TEXT("Master volume:")),FString(TEXT("Master volume: 90%")));
            Capture(TEXT("restart_audio.png"));Stage=5;return false;
        }
        if(Stage==5)
        {
            RestartKey(EKeys::Gamepad_RightShoulder);
            Test->TestEqual(TEXT("Fresh Accessibility tab shows disk HUD scale"),RestartCaption(Menu.Get(),TEXT("HUD text scale:")),FString(TEXT("HUD text scale: 125%")));
            Capture(TEXT("restart_accessibility.png"));Stage=6;return false;
        }
        if(Stage==6)
        {
            // A changed second-process draft must not rewrite the first-process INI.
            RestartKey(EKeys::Left);CheckPreferences();Stage=7;Until=Now+0.3;return false;
        }
        if(Stage==7)
        {
            RestartKey(EKeys::Escape);
            Test->TestTrue(TEXT("Back restores actual Pause focus"),PC->IsPauseMenuOpen() && Pause->HasKeyboardFocus());
            PC->SetPauseMenuOpen(false);Stage=8;Until=Now+0.5;return false;
        }
        if(Stage==8)
        {
            CheckPreferences();
            auto* Character=Cast<APFSurvivorCharacter>(PC->GetPawn());
            if(!Test->TestNotNull(TEXT("First-person survivor remains possessed"),Character)){return true;}
            Test->TestEqual(TEXT("Actual camera consumes persisted FOV"),Character->GetFirstPersonCameraComponent()->FieldOfView,95.f);
            Test->TestEqual(TEXT("Actual camera consumes persisted blur"),Character->GetFirstPersonCameraComponent()->PostProcessSettings.MotionBlurAmount,0.5f);
            auto* HUD=RestartWidget<UPFSurvivalHUD>(PC->GetWorld());
            auto* Label=HUD?Cast<UTextBlock>(HUD->WidgetTree->FindWidget(TEXT("PF_HealthLabel"))):nullptr;
            if(Test->TestNotNull(TEXT("Actual health label exists"),Label)){Test->TestEqual(TEXT("Actual HUD consumes persisted scale"),Label->GetFont().Size,float(FMath::RoundToInt(22*1.25f)));}
            Test->TestFalse(TEXT("Resume restores movement"),PC->IsMoveInputIgnored());
            Test->TestEqual(TEXT("Modal inputs conserve inventory"),PC->GetInventory()->Count(TEXT("Item_Wood")),Wood);
            Test->TestTrue(TEXT("No quality/resolution/mode changes"),Quality==Live->ScalabilityQuality && Resolution==Live->GetScreenResolution() && Mode==Live->GetFullscreenMode());
            Capture(TEXT("resumed.png"));Stage=9;return false;
        }
        if(Stage==9)
        {
            if(bVerify)
            {
                FString After;Test->TestTrue(TEXT("Fresh read and Cancel do not rewrite source INI"),FFileHelper::LoadFileToString(After,*Filename) && After==SourceBefore);
            }
            Test->AddInfo(bVerify?TEXT("[PrimalSettings] Fresh-process startup, four actual categories, renderer/camera/HUD and Cancel verified; not audio audibility/controller/display/human acceptance."):TEXT("[PrimalSettings] Actual menu prepared isolated settings for a separate-process verification; this preparation alone is not restart proof."));return true;
        }
        Test->AddError(TEXT("Unexpected settings restart stage"));return true;
    }
private:
    void Capture(const TCHAR* Name){Shot=Directory/Name;FScreenshotRequest::RequestScreenshot(Shot,true,false);Until=FPlatformTime::Seconds()+0.8;}
    void CheckPreferences()
    {
        auto* Live=UPFGameUserSettings::Get();const auto Expected=RestartExpected();
        Test->TestTrue(TEXT("All independent expected preferences match actual live object"),FPFLocalPreferences::StaticStruct()->CompareScriptStruct(&Expected,&Live->Preferences,0));
        Test->TestEqual(TEXT("Renderer consumes persisted blur"),IConsoleManager::Get().FindConsoleVariable(TEXT("r.MotionBlurQuality"))->GetInt(),3);
        Test->TestTrue(TEXT("Settings test remains genuinely unlimited with VSync off"),Live->GetFrameRateLimit()==0 && !Live->IsVSyncEnabled() &&
            IConsoleManager::Get().FindConsoleVariable(TEXT("r.VSync"))->GetInt()==0 && IConsoleManager::Get().FindConsoleVariable(TEXT("t.MaxFPS"))->GetFloat()==0);
    }
    FAutomationTestBase* Test;FString Directory,Filename,Shot,SourceBefore;bool bVerify=false;
    int32 Stage=0,Wood=0;double Started=FPlatformTime::Seconds(),Until=0;
    Scalability::FQualityLevels Quality;FIntPoint Resolution;EWindowMode::Type Mode=EWindowMode::Windowed;
    TWeakObjectPtr<APFSurvivalPlayerController> PC;TWeakObjectPtr<UPFPauseMenu> Pause;TWeakObjectPtr<UPFSettingsMenu> Menu;
};
bool StartRestartTest(FAutomationTestBase* Test,bool bVerify)
{
    FString Label,Profile,Source;FParse::Value(FCommandLine::Get(),TEXT("PFControlsEvidence="),Label);FParse::Value(FCommandLine::Get(),TEXT("PFIdentityProfile="),Profile);
    FParse::Value(FCommandLine::Get(),TEXT("PFSettingsRestartSource="),Source);
    if(!FParse::Param(FCommandLine::Get(),bVerify?TEXT("PFRunSettingsRestartVerify"):TEXT("PFRunSettingsRestartPrepare")) ||
        !FParse::Param(FCommandLine::Get(),TEXT("PFRunControlsUITest")) || FParse::Param(FCommandLine::Get(),TEXT("nullrhi")) || GIsEditor ||
        (!bVerify && FParse::Param(FCommandLine::Get(),TEXT("NoSaveConfig"))) || (bVerify && !FParse::Param(FCommandLine::Get(),TEXT("NoSaveConfig"))) ||
        !Label.StartsWith(bVerify?TEXT("M11RestartV"):TEXT("M11RestartP")) || Label.Len()>48 ||
        !Profile.StartsWith(TEXT("UI")) || Profile.Len()!=14 || (bVerify && (!Source.StartsWith(TEXT("M11RestartP")) || Source.Len()>48)) || (!bVerify && !Source.IsEmpty()))
    {Test->AddError(TEXT("Use guarded RunSettingsRestartAutomation.ps1; never personal config"));return false;}
    for(TCHAR C:Label+Profile+Source){if(!FChar::IsAlnum(C) && C!=TEXT('_')){Test->AddError(TEXT("Invalid isolated restart label/profile/source"));return false;}}
    const FString Target=FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()/TEXT("AutomationReports")/(bVerify?Source:Label)/TEXT("GameUserSettings.ini"));
    const auto* Branch=GConfig?GConfig->FindBranch(FName(TEXT("GameUserSettings")),GGameUserSettingsIni):nullptr;
    if(!Test->TestTrue(TEXT("Actual settings branch points exclusively to run-owned INI"),Branch && FPaths::IsSamePath(FPaths::ConvertRelativePathToFull(Branch->IniPath),Target))){return false;}
    if(bVerify && !Test->TestTrue(TEXT("Prior-process settings exist before verification"),IFileManager::Get().FileSize(*Target)>0)){return false;}
    const FString Directory=FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()/TEXT("AutomationReports/ControlsUI")/Label);
    if(IFileManager::Get().DirectoryExists(*Directory)){Test->AddError(TEXT("Refusing reused restart evidence directory"));return false;}
    IFileManager::Get().MakeDirectory(*Directory,true);IFileManager::Get().MakeDirectory(*FPaths::GetPath(Target),true);
    ADD_LATENT_AUTOMATION_COMMAND(FSettingsRestartExercise(Test,Directory,Target,bVerify));return true;
}
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFSettingsRestartPrepareLiveTest,"PF.UI.SettingsRestartPrepareLive",EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)
bool FPFSettingsRestartPrepareLiveTest::RunTest(const FString&){return StartRestartTest(this,false);}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFSettingsRestartVerifyLiveTest,"PF.UI.SettingsRestartVerifyLive",EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)
bool FPFSettingsRestartVerifyLiveTest::RunTest(const FString&){return StartRestartTest(this,true);}
#endif
