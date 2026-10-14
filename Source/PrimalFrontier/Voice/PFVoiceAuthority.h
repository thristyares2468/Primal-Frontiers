#pragma once
#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Voice/PFProximityVoicePolicy.h"
#include "PFVoiceAuthority.generated.h"
class APFSurvivalPlayerController;

/** Session-local, default-off metadata; no positions, identities or audio supplied by preferences. */
struct FPFVoicePreferences
{
    bool bTransmitConsent=false;
    bool bReceiveConsent=false;
    bool bPushToTalk=false;
    float ReceiveVolume=1.f;
    TArray<FGuid> Muted;
    TArray<FGuid> Blocked;
};

/** Server-only snapshot bridge. No audio transport, automatic capture, RPC or save state. */
UCLASS()
class PRIMALFRONTIER_API UPFVoiceAuthority : public UWorldSubsystem
{
    GENERATED_BODY()
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool DoesSupportWorldType(EWorldType::Type Type) const override;
    bool SetConfig(const FPFVoicePolicyConfig& In,FString& Error);
    /** Trusted native callers only. Future owning-controller RPC must validate/rate-limit intent separately. */
    bool SetPreferences(APFSurvivalPlayerController* Owner,const FPFVoicePreferences& In,FString& Error);
    bool BuildSnapshot(TArray<FPFVoiceParticipant>& Out,FString& Error) const;
    bool Evaluate(APFSurvivalPlayerController* Speaker,TArray<FPFVoiceRecipient>& Out,FString& Error) const;
private:
    bool IsServer(FString& Error) const;
    FGuid Space;
    FPFVoicePolicyConfig Config;
    TMap<TWeakObjectPtr<APFSurvivalPlayerController>,FPFVoicePreferences> Preferences;
};
