#include "Voice/PFProximityVoicePolicy.h"

namespace
{
bool ValidIds(const TArray<FGuid>& Ids,FGuid Self)
{
    if(Ids.Num()>FPFProximityVoicePolicy::MaxParticipants){return false;}
    TSet<FGuid> Seen;
    for(const auto& Id:Ids){if(!Id.IsValid() || Id==Self || Seen.Contains(Id)){return false;}Seen.Add(Id);}
    return true;
}
}

bool FPFProximityVoicePolicy::Evaluate(const FPFVoicePolicyConfig& Config,const TArray<FPFVoiceParticipant>& Participants,
    FGuid Speaker,TArray<FPFVoiceRecipient>& Out,FString& Error)
{
    Error.Reset();
    if(!FMath::IsFinite(Config.FullVolumeDistanceCm) || !FMath::IsFinite(Config.MaxDistanceCm) ||
        Config.FullVolumeDistanceCm<0 || Config.MaxDistanceCm<=Config.FullVolumeDistanceCm || Config.MaxDistanceCm>10000 ||
        !Speaker.IsValid() || Participants.IsEmpty() || Participants.Num()>MaxParticipants)
    {Error=TEXT("Invalid bounded voice policy input");return false;}
    TSet<FGuid> Seen;
    for(const auto& P:Participants)
    {
        if(!P.PlayerId.IsValid() || !P.SpaceId.IsValid() || Seen.Contains(P.PlayerId) ||
            !FMath::IsFinite(P.Position.X) || !FMath::IsFinite(P.Position.Y) || !FMath::IsFinite(P.Position.Z) ||
            FMath::Abs(P.Position.X)>MaxCoordinateCm || FMath::Abs(P.Position.Y)>MaxCoordinateCm || FMath::Abs(P.Position.Z)>MaxCoordinateCm ||
            !FMath::IsFinite(P.ReceiveVolume) || P.ReceiveVolume<0 || P.ReceiveVolume>1 ||
            !ValidIds(P.Muted,P.PlayerId) || !ValidIds(P.Blocked,P.PlayerId))
        {Error=TEXT("Invalid voice participant snapshot");return false;}
        Seen.Add(P.PlayerId);
    }
    const auto* Sender=Participants.FindByPredicate([&](const auto& P){return P.PlayerId==Speaker;});
    if(!Sender){Error=TEXT("Unknown voice speaker");return false;}
    TArray<FPFVoiceRecipient> Candidate;Candidate.Reserve(Participants.Num()-1);
    for(const auto& Receiver:Participants)
    {
        if(Receiver.PlayerId==Speaker){continue;}
        FPFVoiceRecipient R;R.PlayerId=Receiver.PlayerId;
        if(!Config.bEnabled){R.Result=EPFVoiceRoute::Disabled;}
        else if(!Sender->bConnected || !Receiver.bConnected || !Sender->bCanParticipate || !Receiver.bCanParticipate){R.Result=EPFVoiceRoute::Unavailable;}
        else if(Sender->SpaceId!=Receiver.SpaceId){R.Result=EPFVoiceRoute::DifferentSpace;}
        else if(Sender->Blocked.Contains(Receiver.PlayerId) || Receiver.Blocked.Contains(Speaker)){R.Result=EPFVoiceRoute::Blocked;}
        else if(!Sender->bTransmitConsent || !Sender->bPushToTalk){R.Result=EPFVoiceRoute::NotTransmitting;}
        else if(!Receiver.bReceiveConsent){R.Result=EPFVoiceRoute::NotReceiving;}
        else if(Receiver.ReceiveVolume<=0 || Receiver.Muted.Contains(Speaker)){R.Result=EPFVoiceRoute::Muted;}
        else
        {
            const double Distance=FVector::Distance(Sender->Position,Receiver.Position);
            if(Distance>=Config.MaxDistanceCm){R.Result=EPFVoiceRoute::OutOfRange;}
            else
            {
                R.Result=EPFVoiceRoute::Allowed;
                R.Gain=Receiver.ReceiveVolume*float(Distance<=Config.FullVolumeDistanceCm?1.:
                    (Config.MaxDistanceCm-Distance)/(Config.MaxDistanceCm-Config.FullVolumeDistanceCm));
            }
        }
        Candidate.Add(R);
    }
    Candidate.Sort([](const auto& A,const auto& B){return A.PlayerId.ToString(EGuidFormats::Digits)<B.PlayerId.ToString(EGuidFormats::Digits);});
    Out=MoveTemp(Candidate);return true;
}
