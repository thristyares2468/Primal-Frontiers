#pragma once
#include "CoreMinimal.h"

/** Transport-independent rules. These are trusted server snapshots, never client claims. */
struct FPFVoiceParticipant
{
    FGuid PlayerId;
    FGuid SpaceId;
    FVector Position=FVector::ZeroVector;
    bool bConnected=false;
    bool bCanParticipate=false;
    bool bTransmitConsent=false;
    bool bReceiveConsent=false;
    bool bPushToTalk=false;
    float ReceiveVolume=1.f;
    TArray<FGuid> Muted;
    TArray<FGuid> Blocked;
};

struct FPFVoicePolicyConfig
{
    bool bEnabled=false;
    double FullVolumeDistanceCm=300;
    double MaxDistanceCm=1500;
};

enum class EPFVoiceRoute : uint8
{
    Allowed, Disabled, Unavailable, DifferentSpace, NotTransmitting,
    NotReceiving, Muted, Blocked, OutOfRange
};

struct FPFVoiceRecipient
{
    FGuid PlayerId;
    EPFVoiceRoute Result=EPFVoiceRoute::Disabled;
    float Gain=0;
};

/** Metadata only: does not capture, send, decode or play audio, or change gameplay. */
class PRIMALFRONTIER_API FPFProximityVoicePolicy
{
public:
    static constexpr int32 MaxParticipants=32;
    static constexpr double MaxCoordinateCm=1.e12;
    /** Invalid input leaves Out unchanged. Valid denied routes always have zero gain.
     *  No self route; sorted by public identity. Unknown mute/block IDs are inert. */
    static bool Evaluate(const FPFVoicePolicyConfig& Config,const TArray<FPFVoiceParticipant>& Participants,
        FGuid Speaker,TArray<FPFVoiceRecipient>& Out,FString& Error);
};
