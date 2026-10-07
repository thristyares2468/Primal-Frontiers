#pragma once
#include "GameFramework/GameUserSettings.h"
#include "PFGameUserSettings.generated.h"

/** Local presentation preferences only. Never replicated or used for gameplay authority. */
USTRUCT(BlueprintType)
struct FPFLocalPreferences
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float FieldOfView=90;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float LookSensitivity=1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bInvertLook=false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bControlHints=true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bMotionBlur=false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bDepthOfField=false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float MasterVolume=1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float MusicVolume=1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float EffectsVolume=1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float UIVolume=1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bMuteUnfocused=true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float HUDScale=1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float CrosshairScale=1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bCrosshair=true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 ColorVisionMode=0;
    void Sanitize();
};

UCLASS(Config=GameUserSettings)
class PRIMALFRONTIER_API UPFGameUserSettings : public UGameUserSettings
{
    GENERATED_BODY()
public:
    UPROPERTY(Config) FPFLocalPreferences Preferences;
    UPROPERTY(Config) int32 PreferenceVersion=0;
    static UPFGameUserSettings* Get();
    UPFGameUserSettings* CreateDraft(UObject* Owner) const;
    virtual void SetToDefaults() override;
    virtual void LoadSettings(bool bForceReload=false) override;
    virtual void SaveSettings() override;
    virtual void ValidateSettings() override;
    virtual void ApplyNonResolutionSettings() override;
    /** Apply to this world's audio device (PIE has a separate device). */
    void ApplyToWorld(UWorld* World) const;
};
