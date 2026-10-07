#include "Settings/PFGameUserSettings.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "Sound/AudioSettings.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"
#include "Misc/ConfigCacheIni.h"
#include <limits>
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFSettingsTest,"PF.Settings.Preferences",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFSettingsTest::RunTest(const FString&)
{
    auto* ActiveSettings=UPFGameUserSettings::Get();
    if(!TestNotNull(TEXT("Engine uses project settings class"),ActiveSettings)){return false;}
    float SavedRenderScale=0;
    if(GConfig->GetFloat(TEXT("ScalabilityGroups"),TEXT("sg.ResolutionQuality"),SavedRenderScale,GGameUserSettingsIni))
    {
        TestEqual(TEXT("Editor retains saved game render scale"),ActiveSettings->ScalabilityQuality.ResolutionQuality,SavedRenderScale);
    }
    TestEqual(TEXT("New sounds default to project effects class"),GetDefault<UAudioSettings>()->DefaultSoundClassName.ToString(),FString(TEXT("/Game/PrimalFrontier/Audio/SC_Effects.SC_Effects")));
    TestNotNull(TEXT("Project preference mix loads"),LoadObject<USoundMix>(nullptr,TEXT("/Game/PrimalFrontier/Audio/SMX_Preferences.SMX_Preferences")));
    for(const TCHAR* Name:{TEXT("SC_Music"),TEXT("SC_Effects"),TEXT("SC_UI")})
    {
        TestNotNull(TEXT("Project audio category loads"),LoadObject<USoundClass>(nullptr,*FString::Printf(TEXT("/Game/PrimalFrontier/Audio/%s.%s"),Name,Name)));
    }
    auto* Settings=NewObject<UPFGameUserSettings>();Settings->SetToDefaults();
    TestFalse(TEXT("Motion blur defaults off"),Settings->Preferences.bMotionBlur);
    TestEqual(TEXT("Default medium shadows"),Settings->GetShadowQuality(),1);
    auto& P=Settings->Preferences;
    P.FieldOfView=10000;P.LookSensitivity=-5;P.MasterVolume=std::numeric_limits<float>::quiet_NaN();P.HUDScale=999;P.ColorVisionMode=-1;
    P.Sanitize();
    TestEqual(TEXT("FOV bounded"),P.FieldOfView,110.f);
    TestEqual(TEXT("Sensitivity bounded"),P.LookSensitivity,0.25f);
    TestEqual(TEXT("NaN volume repaired"),P.MasterVolume,1.f);
    TestEqual(TEXT("HUD size bounded"),P.HUDScale,1.5f);
    TestEqual(TEXT("Colour mode bounded"),P.ColorVisionMode,0);
    auto* Draft=Settings->CreateDraft(GetTransientPackage());
    TestEqual(TEXT("Draft retains native scalability"),Draft->GetShadowQuality(),Settings->GetShadowQuality());
    TestEqual(TEXT("Draft retains render scale"),Draft->ScalabilityQuality.ResolutionQuality,Settings->ScalabilityQuality.ResolutionQuality);
    Draft->Preferences.FieldOfView=75;
    TestEqual(TEXT("Editing draft leaves active value unchanged"),Settings->Preferences.FieldOfView,110.f);
    const FString File=FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()/TEXT("AutomationReports/SettingsRoundTrip.ini"));
    IFileManager::Get().MakeDirectory(*FPaths::GetPath(File),true);
    Settings->PreferenceVersion=1;Settings->SaveConfig(CPF_Config,*File);
    auto* Reloaded=NewObject<UPFGameUserSettings>();Reloaded->LoadConfig(UPFGameUserSettings::StaticClass(),*File);
    TestEqual(TEXT("Preferences survive config round trip"),Reloaded->Preferences.FieldOfView,110.f);
    TestEqual(TEXT("Version survives config round trip"),Reloaded->PreferenceVersion,1);
    AddInfo(TEXT("[PrimalSettings] Defaults, malformed input, draft isolation and engine config persistence checked."));
    return true;
}
#endif
