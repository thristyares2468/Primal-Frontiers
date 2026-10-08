// Real draft/navigation/cancel smoke. Never applies preferences or changes display.
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
template<class T> T* FindSettingsWidget(UWorld* World)
{
    TArray<UUserWidget*> Widgets;UWidgetBlueprintLibrary::GetAllWidgetsOfClass(World,Widgets,T::StaticClass(),true);
    return Widgets.Num()==1?Cast<T>(Widgets[0]):nullptr;
}
void SettingsKey(FKey Key)
{FSlateApplication::Get().ProcessKeyDownEvent(FKeyEvent(Key,FModifierKeysState(),uint32(0),false,0,0));}
FString SettingsCaption(UPFSettingsMenu* Menu,const TCHAR* Prefix)
{
    FString Found;
    auto Read=[&](UWidget* W){if(auto* T=Cast<UTextBlock>(W);T && T->GetText().ToString().StartsWith(Prefix)){Found=T->GetText().ToString();}};
    Menu->WidgetTree->ForEachWidget([&](UWidget* W)
    {
        Read(W);
        // Each live value row owns its own tree; traverse those without stale removed rows.
        if(auto* Row=Cast<UPFSettingsRow>(W)){Row->WidgetTree->ForEachWidget(Read);}
    });
    return Found;
}
class FSettingsCancelExercise final : public IAutomationLatentCommand
{
public:
    FSettingsCancelExercise(FAutomationTestBase* T,FString D):Test(T),Directory(MoveTemp(D)){}
    ~FSettingsCancelExercise(){if(PC.IsValid()){PC->SetPauseMenuOpen(false);}}
    bool Update() override
    {
        const double Now=FPlatformTime::Seconds();
        if(Now-Started>45){Test->AddError(TEXT("Settings cancel UI timed out"));return true;}
        if(Now<Until){return false;}
        if(Phase==0)
        {
            for(const auto& C:GEngine->GetWorldContexts())
            {
                auto* W=C.World();if(!W || !W->IsGameWorld() || W->GetNetMode()!=NM_Standalone){continue;}
                auto* Candidate=Cast<APFSurvivalPlayerController>(W->GetFirstPlayerController());
                if(Candidate && Candidate->GetLocalPlayer() && Candidate->GetPawn() && Candidate->GetInventory()){PC=Candidate;break;}
            }
            auto* Live=UPFGameUserSettings::Get();if(!PC.IsValid() || !Live){return false;}
            Before=Live->Preferences;BeforeQuality=Live->ScalabilityQuality;Resolution=Live->GetScreenResolution();Mode=Live->GetFullscreenMode();
            FPS=Live->GetFrameRateLimit();bVSync=Live->IsVSyncEnabled();
            Blur=IConsoleManager::Get().FindConsoleVariable(TEXT("r.MotionBlurQuality"))->GetInt();
            bConfigExisted=IFileManager::Get().FileExists(*GGameUserSettingsIni);
            if(bConfigExisted && !FFileHelper::LoadFileToString(ConfigBefore,*GGameUserSettingsIni)){Test->AddError(TEXT("Unable to read existing config for preservation check"));return true;}
            if(!Test->TestTrue(TEXT("Fixture seeds real inventory for UI input conservation"),PC->GetInventory()->Grant(TEXT("Item_Wood"),3))){return true;}
            Wood=PC->GetInventory()->Count(TEXT("Item_Wood"));
            PC->SetPauseMenuOpen(true);Pause=FindSettingsWidget<UPFPauseMenu>(PC->GetWorld());
            if(!Test->TestNotNull(TEXT("Pause exists"),Pause.Get())){return true;}
            Phase=1;Until=Now+0.5;return false;
        }
        if(!PC.IsValid() || !Pause.IsValid()){Test->AddError(TEXT("Lost settings fixture"));return true;}
        if(Phase==1)
        {
            SettingsKey(EKeys::Gamepad_DPad_Down);SettingsKey(EKeys::Gamepad_FaceButton_Bottom);
            Menu=FindSettingsWidget<UPFSettingsMenu>(PC->GetWorld());
            if(!Test->TestNotNull(TEXT("Real Pause navigation opens Settings"),Menu.Get())){return true;}
            Test->TestTrue(TEXT("Settings return target is Pause"),Menu->ReturnFocus==Pause.Get());
            Phase=2;Until=Now+0.5;return false;
        }
        if(!Menu.IsValid()){Test->AddError(TEXT("Lost settings menu"));return true;}
        if(Phase>=2 && Phase<=5)
        {
            const TCHAR* Prefixes[]={TEXT("Field of view:"),TEXT("Motion blur:"),TEXT("Master volume:"),TEXT("HUD text scale:")};
            const TCHAR* Shots[]={TEXT("game_draft.png"),TEXT("graphics_draft.png"),TEXT("audio_draft.png"),TEXT("accessibility_draft.png")};
            const int32 Category=Phase-2;
            if(Category>0){SettingsKey(EKeys::Gamepad_RightShoulder);}
            if(Category==1){for(int32 I=0;I<6;++I){SettingsKey(EKeys::Gamepad_DPad_Down);}}
            const FString Old=SettingsCaption(Menu.Get(),Prefixes[Category]);
            Test->TestTrue(TEXT("Actual category row present"),!Old.IsEmpty());
            const bool bDown=Category==2?Before.MasterVolume>0.8f:Category==3?Before.HUDScale>1.f:Before.FieldOfView>100.f;
            SettingsKey(bDown?EKeys::Gamepad_DPad_Left:EKeys::Gamepad_DPad_Right);
            Test->TestTrue(TEXT("Input changes displayed draft value"),SettingsCaption(Menu.Get(),Prefixes[Category])!=Old);
            CheckLive();SettingsKey(EKeys::Gamepad_FaceButton_Left); // Contextual interact/consume must not leak through the modal.
            Shot=Directory/Shots[Category];FScreenshotRequest::RequestScreenshot(Shot,true,false);
            Phase+=10;Until=Now+1.5;return false;
        }
        if(Phase>=12 && Phase<=15)
        {
            if(IFileManager::Get().FileSize(*Shot)<=0){return false;}
            CheckBounds();Phase-=9;return false;
        }
        if(Phase==6)
        {
            SettingsKey(EKeys::Gamepad_FaceButton_Top); // Defaults edits the draft, never the applied object.
            Test->TestTrue(TEXT("Defaults displayed with Apply/Cancel explanation"),SettingsCaption(Menu.Get(),TEXT("Defaults selected.")).Contains(TEXT("Cancel to discard")));
            CheckLive();SettingsKey(EKeys::Gamepad_FaceButton_Right);
            Test->TestFalse(TEXT("Cancel removes Settings"),Menu->IsInViewport());
            Test->TestTrue(TEXT("Cancel retains Pause and restores focus"),PC->IsPauseMenuOpen() && Pause->HasKeyboardFocus());
            Phase=7;Until=Now+0.5;return false;
        }
        if(Phase==7)
        {
            CheckLive();SettingsKey(EKeys::Enter); // Settings remains selected; reopen through real focus.
            Menu=FindSettingsWidget<UPFSettingsMenu>(PC->GetWorld());
            if(!Test->TestNotNull(TEXT("Settings reopens after Cancel"),Menu.Get())){return true;}
            Test->TestTrue(TEXT("Reopen drops unapplied draft"),SettingsCaption(Menu.Get(),TEXT("Field of view:"))==FString::Printf(TEXT("Field of view: %.0f"),Before.FieldOfView));
            Phase=8;Until=Now+0.5;return false;
        }
        if(Phase==8)
        {
            SettingsKey(EKeys::Escape);Test->TestFalse(TEXT("Keyboard Escape cancels Settings"),Menu->IsInViewport());
            Test->TestTrue(TEXT("Keyboard Cancel restores Pause focus"),Pause->HasKeyboardFocus());
            SettingsKey(EKeys::Enter);Menu=FindSettingsWidget<UPFSettingsMenu>(PC->GetWorld());
            Test->TestNotNull(TEXT("Settings can reopen for teardown"),Menu.Get());PC->SetPauseMenuOpen(false);
            if(Menu.IsValid()){Test->TestFalse(TEXT("Closing Pause also removes Settings modal"),Menu->IsInViewport());}
            Test->TestFalse(TEXT("Resume restores movement"),PC->IsMoveInputIgnored());CheckLive();
            Test->TestEqual(TEXT("Settings input conserves seeded inventory"),PC->GetInventory()->Count(TEXT("Item_Wood")),Wood);
            FString After;const bool bExists=IFileManager::Get().FileExists(*GGameUserSettingsIni);
            Test->TestEqual(TEXT("Config file existence unchanged"),bExists,bConfigExisted);
            if(bExists){Test->TestTrue(TEXT("Read final config"),FFileHelper::LoadFileToString(After,*GGameUserSettingsIni));Test->TestTrue(TEXT("Cancel/defaults never write user's config"),After==ConfigBefore);}
            Test->AddInfo(TEXT("[PrimalSettings] Four actual category drafts, Defaults, pad/keyboard Cancel, focus, teardown and config preservation verified; no Apply/display/audio acceptance claimed."));
            return true;
        }
        Test->AddError(TEXT("Unexpected settings fixture phase"));return true;
    }
private:
    void CheckLive()
    {
        auto* Live=UPFGameUserSettings::Get();
        Test->TestTrue(TEXT("Draft leaves every applied local preference unchanged"),FPFLocalPreferences::StaticStruct()->CompareScriptStruct(&Before,&Live->Preferences,0));
        Test->TestTrue(TEXT("Draft leaves applied quality unchanged"),BeforeQuality==Live->ScalabilityQuality);
        Test->TestTrue(TEXT("Draft leaves display mode unchanged"),Resolution==Live->GetScreenResolution() && Mode==Live->GetFullscreenMode());
        Test->TestTrue(TEXT("Draft leaves FPS/VSync unchanged"),FPS==Live->GetFrameRateLimit() && bVSync==Live->IsVSyncEnabled());
        Test->TestEqual(TEXT("Draft blur toggle never applies renderer effect"),IConsoleManager::Get().FindConsoleVariable(TEXT("r.MotionBlurQuality"))->GetInt(),Blur);
    }
    void CheckBounds()
    {
        Test->TestTrue(TEXT("Settings owns keyboard focus"),Menu->HasKeyboardFocus());
        Menu->WidgetTree->ForEachWidget([&](UWidget* W)
        {
            if(auto* T=Cast<UTextBlock>(W))
            {
                const auto& G=T->GetCachedGeometry();const auto& Root=Menu->GetCachedGeometry();
                const auto Min=Root.AbsoluteToLocal(G.LocalToAbsolute(FVector2D::ZeroVector));const auto Max=Root.AbsoluteToLocal(G.LocalToAbsolute(G.GetLocalSize()));
                Test->TestTrue(TEXT("Settings title/actions/footer fit viewport"),Min.X>=-1 && Min.Y>=-1 && Max.X<=Root.GetLocalSize().X+1 && Max.Y<=Root.GetLocalSize().Y+1);
            }
        });
    }
    FAutomationTestBase* Test;FString Directory,Shot,ConfigBefore;FPFLocalPreferences Before;Scalability::FQualityLevels BeforeQuality;
    FIntPoint Resolution;EWindowMode::Type Mode=EWindowMode::Windowed;float FPS=0;bool bVSync=false,bConfigExisted=false;
    int32 Phase=0,Wood=0,Blur=0;double Started=FPlatformTime::Seconds(),Until=0;
    TWeakObjectPtr<APFSurvivalPlayerController> PC;TWeakObjectPtr<UPFPauseMenu> Pause;TWeakObjectPtr<UPFSettingsMenu> Menu;
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFSettingsCancelLiveTest,"PF.UI.SettingsCancelLive",EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)
bool FPFSettingsCancelLiveTest::RunTest(const FString&)
{
    FString Label,Profile;FParse::Value(FCommandLine::Get(),TEXT("PFControlsEvidence="),Label);FParse::Value(FCommandLine::Get(),TEXT("PFIdentityProfile="),Profile);
    if(!FParse::Param(FCommandLine::Get(),TEXT("PFRunControlsUITest")) || FParse::Param(FCommandLine::Get(),TEXT("nullrhi")) ||
        Label.IsEmpty() || Label.Len()>48 || !Profile.StartsWith(TEXT("UI")) || Profile.Len()!=14)
    {AddError(TEXT("Use isolated rendered RunControlsAutomation.ps1 -TestCase Settings with unique profile/evidence"));return false;}
    for(TCHAR C:Label+Profile){if(!FChar::IsAlnum(C) && C!=TEXT('_')){AddError(TEXT("Invalid profile/evidence label"));return false;}}
    const FString Directory=FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()/TEXT("AutomationReports/ControlsUI")/Label);
    if(IFileManager::Get().DirectoryExists(*Directory)){AddError(TEXT("Refusing reused settings evidence directory"));return false;}
    IFileManager::Get().MakeDirectory(*Directory,true);ADD_LATENT_AUTOMATION_COMMAND(FSettingsCancelExercise(this,Directory));return true;
}
#endif
