// Opt-in real menu Apply/disk-reload test. Owns only its runner's disposable INI.
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
#include "HAL/PlatformTime.h"
#include "HAL/IConsoleManager.h"
#include "Framework/Application/SlateApplication.h"

namespace
{
template<class T> T* ApplyWidget(UWorld* World)
{
    TArray<UUserWidget*> Widgets;UWidgetBlueprintLibrary::GetAllWidgetsOfClass(World,Widgets,T::StaticClass(),true);
    return Widgets.Num()==1?Cast<T>(Widgets[0]):nullptr;
}
void ApplyKey(FKey Key)
{FSlateApplication::Get().ProcessKeyDownEvent(FKeyEvent(Key,FModifierKeysState(),uint32(0),false,0,0));}
FString ApplyCaption(UPFSettingsMenu* Menu,const TCHAR* Prefix)
{
    FString Found;
    auto Read=[&](UWidget* W){if(auto* T=Cast<UTextBlock>(W);T && T->GetText().ToString().StartsWith(Prefix)){Found=T->GetText().ToString();}};
    Menu->WidgetTree->ForEachWidget([&](UWidget* W){Read(W);if(auto* Row=Cast<UPFSettingsRow>(W)){Row->WidgetTree->ForEachWidget(Read);}});
    return Found;
}
FString ApplyDestination()
{
    const auto* Branch=GConfig?GConfig->FindBranch(FName(TEXT("GameUserSettings")),GGameUserSettingsIni):nullptr;
    return Branch?Branch->IniPath:FString();
}
class FSettingsApplyExercise final : public IAutomationLatentCommand
{
public:
    FSettingsApplyExercise(FAutomationTestBase* T,FString D,FString F):Test(T),Directory(MoveTemp(D)),Filename(MoveTemp(F)){}
    ~FSettingsApplyExercise(){if(PC.IsValid()){PC->SetPauseMenuOpen(false);}}
    bool Update() override
    {
        const double Now=FPlatformTime::Seconds();
        if(Now-Started>50){Test->AddError(TEXT("Settings Apply UI timed out"));return true;}
        if(Now<Until){return false;}
        auto* Live=UPFGameUserSettings::Get();
        if(Phase==0)
        {
            for(const auto& C:GEngine->GetWorldContexts())
            {
                auto* W=C.World();if(!W || !W->IsGameWorld() || W->GetNetMode()!=NM_Standalone){continue;}
                auto* Candidate=Cast<APFSurvivalPlayerController>(W->GetFirstPlayerController());
                if(Candidate && Candidate->GetLocalPlayer() && Cast<APFSurvivorCharacter>(Candidate->GetPawn()) && Candidate->GetInventory()){PC=Candidate;break;}
            }
            if(!PC.IsValid() || !Live){return false;}
            Before=Expected=Live->Preferences;Quality=Live->ScalabilityQuality;Resolution=Live->GetScreenResolution();Mode=Live->GetFullscreenMode();
            FPS=Live->GetFrameRateLimit();bVSync=Live->IsVSyncEnabled();
            if(!Test->TestTrue(TEXT("Seed real inventory for modal conservation"),PC->GetInventory()->Grant(TEXT("Item_Wood"),3))){return true;}
            Wood=PC->GetInventory()->Count(TEXT("Item_Wood"));
            PC->SetPauseMenuOpen(true);Pause=ApplyWidget<UPFPauseMenu>(PC->GetWorld());
            if(!Test->TestNotNull(TEXT("Pause exists"),Pause.Get())){return true;}
            Phase=1;Until=Now+0.5;return false;
        }
        if(!PC.IsValid() || !Pause.IsValid() || !Live){Test->AddError(TEXT("Lost Settings Apply fixture"));return true;}
        if(Phase==1)
        {
            ApplyKey(EKeys::Gamepad_DPad_Down);ApplyKey(EKeys::Gamepad_FaceButton_Bottom);
            Menu=ApplyWidget<UPFSettingsMenu>(PC->GetWorld());
            if(!Test->TestNotNull(TEXT("Pause input opens real Settings"),Menu.Get())){return true;}
            Phase=2;Until=Now+0.5;return false;
        }
        if(!Menu.IsValid()){Test->AddError(TEXT("Lost Settings Apply menu"));return true;}
        if(Phase==2)
        {
            const int32 FOVDirection=Before.FieldOfView>100?-1:1;
            ApplyKey(FOVDirection<0?EKeys::Left:EKeys::Right);Expected.FieldOfView+=5*FOVDirection;
            ApplyKey(EKeys::Gamepad_RightShoulder);for(int32 I=0;I<6;++I){ApplyKey(EKeys::Down);}ApplyKey(EKeys::Right);Expected.bMotionBlur=!Before.bMotionBlur;
            ApplyKey(EKeys::Gamepad_RightShoulder);const int32 VolumeDirection=Before.MasterVolume>0.8f?-1:1;
            ApplyKey(VolumeDirection<0?EKeys::Left:EKeys::Right);Expected.MasterVolume+=0.1f*VolumeDirection;
            ApplyKey(EKeys::Gamepad_RightShoulder);const int32 HUDDirection=Before.HUDScale>1?-1:1;
            ApplyKey(HUDDirection<0?EKeys::Left:EKeys::Right);Expected.HUDScale+=0.25f*HUDDirection;Expected.Sanitize();
            Test->TestTrue(TEXT("Four category drafts leave live preferences unchanged"),FPFLocalPreferences::StaticStruct()->CompareScriptStruct(&Before,&Live->Preferences,0));
            Test->TestTrue(TEXT("Accessibility draft has expected HUD scale"),ApplyCaption(Menu.Get(),TEXT("HUD text scale:"))==FString::Printf(TEXT("HUD text scale: %.0f%%"),Expected.HUDScale*100));
            ApplyKey(EKeys::Gamepad_FaceButton_Bottom);
            Test->TestTrue(TEXT("Real Apply reports applied and saved without display confirmation"),ApplyCaption(Menu.Get(),TEXT("Settings applied and saved.")).Len()>0);
            CheckApplied();
            Shot=Directory/TEXT("applied.png");FScreenshotRequest::RequestScreenshot(Shot,true,false);
            Phase=3;Until=Now+1.5;return false;
        }
        if(Phase==3)
        {
            if(IFileManager::Get().FileSize(*Shot)<=0){return false;}
            Test->TestTrue(TEXT("Apply creates real disposable INI"),IFileManager::Get().FileSize(*Filename)>0);
            FString SavedText;if(!Test->TestTrue(TEXT("Read actual saved INI"),FFileHelper::LoadFileToString(SavedText,*Filename))){return true;}
            // Change ONLY the live reflected object, then force the engine to read the disk branch.
            // Cached/in-memory values alone cannot satisfy this round trip.
            Live->Preferences=Before;
            Test->TestTrue(TEXT("In-memory alteration differs from applied preferences"),!FPFLocalPreferences::StaticStruct()->CompareScriptStruct(&Expected,&Live->Preferences,0));
            Live->LoadSettings(true);CheckApplied();
            FString After;Test->TestTrue(TEXT("Reload reads without rewriting saved file"),FFileHelper::LoadFileToString(After,*Filename) && After==SavedText);
            ApplyKey(EKeys::Escape);
            Test->TestTrue(TEXT("Back keeps Pause and restores focus"),PC->IsPauseMenuOpen() && Pause->HasKeyboardFocus());
            Phase=4;Until=Now+0.5;return false;
        }
        if(Phase==4)
        {
            ApplyKey(EKeys::Enter);Menu=ApplyWidget<UPFSettingsMenu>(PC->GetWorld());
            if(!Test->TestNotNull(TEXT("Settings reopens from restored Pause focus"),Menu.Get())){return true;}
            Test->TestTrue(TEXT("Reopened Game reflects applied disk FOV"),ApplyCaption(Menu.Get(),TEXT("Field of view:"))==FString::Printf(TEXT("Field of view: %.0f"),Expected.FieldOfView));
            ApplyKey(EKeys::Gamepad_RightShoulder);
            Test->TestTrue(TEXT("Reopened Graphics reflects applied blur"),ApplyCaption(Menu.Get(),TEXT("Motion blur:"))==FString(TEXT("Motion blur: "))+(Expected.bMotionBlur?TEXT("On"):TEXT("Off")));
            ApplyKey(EKeys::Gamepad_RightShoulder);
            Test->TestTrue(TEXT("Reopened Audio reflects applied master"),ApplyCaption(Menu.Get(),TEXT("Master volume:"))==FString::Printf(TEXT("Master volume: %.0f%%"),Expected.MasterVolume*100));
            ApplyKey(EKeys::Gamepad_RightShoulder);
            Test->TestTrue(TEXT("Reopened Accessibility reflects applied HUD scale"),ApplyCaption(Menu.Get(),TEXT("HUD text scale:"))==FString::Printf(TEXT("HUD text scale: %.0f%%"),Expected.HUDScale*100));
            Shot=Directory/TEXT("reloaded.png");FScreenshotRequest::RequestScreenshot(Shot,true,false);
            Phase=5;Until=Now+1.5;return false;
        }
        if(Phase==5)
        {
            if(IFileManager::Get().FileSize(*Shot)<=0){return false;}
            ApplyKey(EKeys::Escape);PC->SetPauseMenuOpen(false);Phase=6;Until=Now+0.5;return false;
        }
        if(Phase==6)
        {
            CheckApplied();
            auto* Character=Cast<APFSurvivorCharacter>(PC->GetPawn());
            if(!Test->TestNotNull(TEXT("Original first-person survivor remains possessed"),Character)){return true;}
            Test->TestEqual(TEXT("Real camera receives applied FOV after resume"),Character->GetFirstPersonCameraComponent()->FieldOfView,Expected.FieldOfView);
            Test->TestEqual(TEXT("Real camera receives applied motion blur"),Character->GetFirstPersonCameraComponent()->PostProcessSettings.MotionBlurAmount,Expected.bMotionBlur?0.5f:0.f);
            auto* HUD=ApplyWidget<UPFSurvivalHUD>(PC->GetWorld());
            auto* Label=HUD?Cast<UTextBlock>(HUD->WidgetTree->FindWidget(TEXT("PF_HealthLabel"))):nullptr;
            if(Test->TestNotNull(TEXT("Actual vitals label exists"),Label)){Test->TestEqual(TEXT("HUD tick uses applied text scale"),Label->GetFont().Size,float(FMath::RoundToInt(22*Expected.HUDScale)));}
            Test->TestFalse(TEXT("Resume restores movement"),PC->IsMoveInputIgnored());
            Test->TestEqual(TEXT("Modal inputs do not consume or drop inventory"),PC->GetInventory()->Count(TEXT("Item_Wood")),Wood);
            Shot=Directory/TEXT("resumed.png");FScreenshotRequest::RequestScreenshot(Shot,true,false);
            Phase=7;Until=Now+1.5;return false;
        }
        if(Phase==7)
        {
            if(IFileManager::Get().FileSize(*Shot)<=0){return false;}
            Test->AddInfo(TEXT("[PrimalSettings] Real non-resolution Apply, disposable disk reload, camera/HUD consumption, modal focus and inventory conservation verified; no process-restart/display/audio audibility acceptance."));return true;
        }
        Test->AddError(TEXT("Unexpected Settings Apply phase"));return true;
    }
private:
    void CheckApplied()
    {
        auto* Live=UPFGameUserSettings::Get();
        Test->TestTrue(TEXT("All reflected preferences equal intended applied values"),FPFLocalPreferences::StaticStruct()->CompareScriptStruct(&Expected,&Live->Preferences,0));
        Test->TestEqual(TEXT("Renderer receives actual applied blur"),IConsoleManager::Get().FindConsoleVariable(TEXT("r.MotionBlurQuality"))->GetInt(),Expected.bMotionBlur?3:0);
        Test->TestTrue(TEXT("Non-resolution Apply preserves quality"),Quality==Live->ScalabilityQuality);
        Test->TestTrue(TEXT("Non-resolution Apply preserves resolution and display mode"),Resolution==Live->GetScreenResolution() && Mode==Live->GetFullscreenMode());
        Test->TestTrue(TEXT("Non-resolution Apply preserves unlimited FPS and VSync"),FPS==Live->GetFrameRateLimit() && bVSync==Live->IsVSyncEnabled());
        Test->TestTrue(TEXT("Rendered test really remains uncapped without a console VSync override"),Live->GetFrameRateLimit()==0 && !Live->IsVSyncEnabled() &&
            IConsoleManager::Get().FindConsoleVariable(TEXT("r.VSync"))->GetInt()==0 && IConsoleManager::Get().FindConsoleVariable(TEXT("t.MaxFPS"))->GetFloat()==0);
    }
    FAutomationTestBase* Test;FString Directory,Filename,Shot;FPFLocalPreferences Before,Expected;Scalability::FQualityLevels Quality;
    FIntPoint Resolution;EWindowMode::Type Mode=EWindowMode::Windowed;float FPS=0;bool bVSync=false;
    int32 Phase=0,Wood=0;double Started=FPlatformTime::Seconds(),Until=0;
    TWeakObjectPtr<APFSurvivalPlayerController> PC;TWeakObjectPtr<UPFPauseMenu> Pause;TWeakObjectPtr<UPFSettingsMenu> Menu;
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFSettingsApplyLiveTest,"PF.UI.SettingsApplyLive",EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)
bool FPFSettingsApplyLiveTest::RunTest(const FString&)
{
    FString Label,Profile;FParse::Value(FCommandLine::Get(),TEXT("PFControlsEvidence="),Label);FParse::Value(FCommandLine::Get(),TEXT("PFIdentityProfile="),Profile);
    if(!FParse::Param(FCommandLine::Get(),TEXT("PFRunSettingsApplyTest")) || !FParse::Param(FCommandLine::Get(),TEXT("PFRunControlsUITest")) ||
        FParse::Param(FCommandLine::Get(),TEXT("nullrhi")) || FParse::Param(FCommandLine::Get(),TEXT("NoSaveConfig")) || GIsEditor ||
        !Label.StartsWith(TEXT("M11SettingsApply")) || Label.Len()>48 || !Profile.StartsWith(TEXT("UI")) || Profile.Len()!=14)
    {AddError(TEXT("Use isolated rendered RunControlsAutomation.ps1 -TestCase SettingsApply; never run with personal config"));return false;}
    for(TCHAR C:Label+Profile){if(!FChar::IsAlnum(C) && C!=TEXT('_')){AddError(TEXT("Invalid isolated Apply label/profile"));return false;}}
    const FString Target=FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()/TEXT("AutomationReports")/Label/TEXT("GameUserSettings.ini"));
    const FString Destination=ApplyDestination();
    if(!TestTrue(TEXT("Actual settings branch points exclusively to this run"),!Destination.IsEmpty() && FPaths::IsSamePath(FPaths::ConvertRelativePathToFull(Destination),Target))){return false;}
    const FString Directory=FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()/TEXT("AutomationReports/ControlsUI")/Label);
    if(IFileManager::Get().DirectoryExists(*Directory)){AddError(TEXT("Refusing reused Apply evidence directory"));return false;}
    IFileManager::Get().MakeDirectory(*Directory,true);IFileManager::Get().MakeDirectory(*FPaths::GetPath(Target),true);
    ADD_LATENT_AUTOMATION_COMMAND(FSettingsApplyExercise(this,Directory,Target));return true;
}
#endif
