#include "Settings/PFGameUserSettings.h"
#include "AudioDevice.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "Misc/App.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"
#include "Scalability.h"

void FPFLocalPreferences::Sanitize()
{
    auto Bound=[](float Value,float Min,float Max,float Default){return FMath::IsFinite(Value)?FMath::Clamp(Value,Min,Max):Default;};
    FieldOfView=Bound(FieldOfView,70,110,90);
    LookSensitivity=Bound(LookSensitivity,0.25f,3,1);
    MasterVolume=Bound(MasterVolume,0,1,1);
    MusicVolume=Bound(MusicVolume,0,1,1);
    EffectsVolume=Bound(EffectsVolume,0,1,1);
    UIVolume=Bound(UIVolume,0,1,1);
    HUDScale=Bound(HUDScale,0.75f,1.5f,1);
    CrosshairScale=Bound(CrosshairScale,0.5f,2,1);
    ColorVisionMode=FMath::Clamp(ColorVisionMode,0,3);
}
UPFGameUserSettings* UPFGameUserSettings::Get(){return Cast<UPFGameUserSettings>(UGameUserSettings::GetGameUserSettings());}
UPFGameUserSettings* UPFGameUserSettings::CreateDraft(UObject* Owner) const
{
    auto* Draft=DuplicateObject<UPFGameUserSettings>(this,Owner);
    // Native scalability data is not a reflected property.
    Draft->ScalabilityQuality=ScalabilityQuality;
    return Draft;
}
void UPFGameUserSettings::SetToDefaults()
{
    Super::SetToDefaults();
    Preferences=FPFLocalPreferences();
    SetOverallScalabilityLevel(1);
    SetTextureQuality(2);
    SetResolutionScaleValueEx(75);
    SetVSyncEnabled(false);
    SetFrameRateLimit(0);
}
void UPFGameUserSettings::LoadSettings(bool bForceReload)
{
    Super::LoadSettings(bForceReload);
    if(GIsEditor)
    {
        // PIE uses the player's game choices, not the Editor viewport profile.
        Scalability::LoadState(GGameUserSettingsIni);
        ScalabilityQuality=Scalability::GetQualityLevels();
    }
    if(PreferenceVersion<1)
    {
        // Migrate once. Subsequent launches retain the player's choices.
        Preferences=FPFLocalPreferences();
        SetOverallScalabilityLevel(1);
        SetTextureQuality(2);
        SetResolutionScaleValueEx(75);
        PreferenceVersion=1;
    }
    Preferences.Sanitize();
}
void UPFGameUserSettings::SaveSettings()
{
    // Unreal's Editor path otherwise writes quality only to EditorSettings.ini.
    if(GIsEditor){Scalability::SaveState(GGameUserSettingsIni);}
    Super::SaveSettings();
}
void UPFGameUserSettings::ValidateSettings(){Super::ValidateSettings();Preferences.Sanitize();}
void UPFGameUserSettings::ApplyNonResolutionSettings()
{
    // Validate first: the base implementation may reload old engine settings.
    ValidateSettings();
    const bool bMigrated=PreferenceVersion<2;
    if(bMigrated)
    {
        SetOverallScalabilityLevel(1);
        SetTextureQuality(2);
        SetResolutionScaleValueEx(75);
        PreferenceVersion=2;
    }
    Super::ApplyNonResolutionSettings();
    Preferences.Sanitize();
    if(auto* Var=IConsoleManager::Get().FindConsoleVariable(TEXT("r.MotionBlurQuality"))){Var->Set(Preferences.bMotionBlur?3:0,ECVF_SetByGameSetting);}
    if(auto* Var=IConsoleManager::Get().FindConsoleVariable(TEXT("r.DepthOfFieldQuality"))){Var->Set(Preferences.bDepthOfField?2:0,ECVF_SetByGameSetting);}
    FApp::SetUnfocusedVolumeMultiplier(Preferences.bMuteUnfocused?0:1);
    if(!IsRunningDedicatedServer() && !IsRunningCommandlet())
    {
        UWidgetBlueprintLibrary::SetColorVisionDeficiencyType(static_cast<EColorVisionDeficiency>(Preferences.ColorVisionMode),10,true,false);
    }
    UE_LOG(LogTemp,Display,TEXT("[PrimalSettings] Applied: FOV=%.0f blur=%d renderScale=%.0f master=%.2f"),Preferences.FieldOfView,Preferences.bMotionBlur,ScalabilityQuality.ResolutionQuality,Preferences.MasterVolume);
    if(bMigrated){SaveSettings();}
}
void UPFGameUserSettings::ApplyToWorld(UWorld* World) const
{
    if(World && World->GetNetMode()!=NM_DedicatedServer)
    {
        if(FAudioDeviceHandle Device=World->GetAudioDevice()){Device->SetTransientPrimaryVolume(Preferences.MasterVolume);}
        TSoftObjectPtr<USoundMix> MixRef(FSoftObjectPath(TEXT("/Game/PrimalFrontier/Audio/SMX_Preferences.SMX_Preferences")));
        if(auto* Mix=MixRef.LoadSynchronous())
        {
            const TCHAR* Names[]={TEXT("SC_Music"),TEXT("SC_Effects"),TEXT("SC_UI")};
            const float Volumes[]={Preferences.MusicVolume,Preferences.EffectsVolume,Preferences.UIVolume};
            UGameplayStatics::SetBaseSoundMix(World,Mix);
            for(int32 I=0;I<3;++I)
            {
                TSoftObjectPtr<USoundClass> ClassRef(FSoftObjectPath(FString::Printf(TEXT("/Game/PrimalFrontier/Audio/%s.%s"),Names[I],Names[I])));
                if(auto* SoundClass=ClassRef.LoadSynchronous()){UGameplayStatics::SetSoundMixClassOverride(World,Mix,SoundClass,Volumes[I],1,0,true);}
            }
        }
    }
}
