#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Voice/PFProximityVoicePolicy.h"
#include "Misc/AutomationTest.h"
#include <limits>

namespace
{
FPFVoiceParticipant Participant(uint32 Id,double X)
{
    FPFVoiceParticipant P;P.PlayerId=FGuid(0,0,0,Id);P.SpaceId=FGuid(0,0,1,1);P.Position.X=X;
    P.bConnected=true;P.bCanParticipate=true;P.bTransmitConsent=true;P.bReceiveConsent=true;P.bPushToTalk=true;return P;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFVoicePolicyTest,"PF.Voice.Policy",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFVoicePolicyTest::RunTest(const FString&)
{
    FPFVoicePolicyConfig Config;TArray<FPFVoiceParticipant> P={Participant(1,0),Participant(2,300),Participant(3,900),Participant(4,1500)};
    TArray<FPFVoiceRecipient> Routes;FString Error;
    const auto Run=[&](){return TestTrue(TEXT("Valid policy evaluates"),FPFProximityVoicePolicy::Evaluate(Config,P,P[0].PlayerId,Routes,Error));};
    if(!Run()){return false;}TestEqual(TEXT("No self route"),Routes.Num(),3);
    for(const auto& R:Routes){TestTrue(TEXT("Default off with zero gain"),R.Result==EPFVoiceRoute::Disabled && R.Gain==0);}
    Config.bEnabled=true;if(!Run()){return false;}
    TestEqual(TEXT("Full gain at near boundary"),Routes[0].Gain,1.f);TestEqual(TEXT("Half gain at mid range"),Routes[1].Gain,.5f);TestTrue(TEXT("Exact far boundary inaudible"),Routes[2].Result==EPFVoiceRoute::OutOfRange && Routes[2].Gain==0);
    P[0].bPushToTalk=false;Run();TestTrue(TEXT("No PTT means no send"),Routes[0].Result==EPFVoiceRoute::NotTransmitting && Routes[0].Gain==0);P[0].bPushToTalk=true;
    P[0].bTransmitConsent=false;Run();TestEqual(TEXT("No microphone consent"),Routes[0].Result,EPFVoiceRoute::NotTransmitting);P[0].bTransmitConsent=true;
    P[1].bReceiveConsent=false;Run();TestEqual(TEXT("Recipient consent independently required"),Routes[0].Result,EPFVoiceRoute::NotReceiving);P[1].bReceiveConsent=true;
    P[1].bTransmitConsent=false;Run();TestEqual(TEXT("Receiving does not require microphone consent"),Routes[0].Result,EPFVoiceRoute::Allowed);
    P[1].ReceiveVolume=.25f;Run();TestEqual(TEXT("Receiver volume bounds gain"),Routes[0].Gain,.25f);P[1].ReceiveVolume=0;Run();TestEqual(TEXT("Zero volume mutes"),Routes[0].Result,EPFVoiceRoute::Muted);P[1].ReceiveVolume=1;
    P[1].Muted.Add(P[0].PlayerId);Run();TestTrue(TEXT("Recipient mute suppresses"),Routes[0].Result==EPFVoiceRoute::Muted && Routes[0].Gain==0);P[1].Muted.Reset();
    P[0].Muted.Add(P[1].PlayerId);Run();TestEqual(TEXT("Listening mute is not a bidirectional block"),Routes[0].Result,EPFVoiceRoute::Allowed);P[0].Muted.Reset();
    P[0].Blocked.Add(P[1].PlayerId);Run();TestEqual(TEXT("Speaker block suppresses outgoing"),Routes[0].Result,EPFVoiceRoute::Blocked);P[0].Blocked.Reset();P[1].Blocked.Add(P[0].PlayerId);Run();TestEqual(TEXT("Recipient block suppresses incoming"),Routes[0].Result,EPFVoiceRoute::Blocked);P[1].Blocked.Reset();
    P[1].SpaceId=FGuid(0,0,2,2);Run();TestEqual(TEXT("No cross-space voice"),Routes[0].Result,EPFVoiceRoute::DifferentSpace);P[1].SpaceId=P[0].SpaceId;
    P[1].bConnected=false;Run();TestEqual(TEXT("Disconnected receiver excluded"),Routes[0].Result,EPFVoiceRoute::Unavailable);P[1].bConnected=true;
    P[0].bCanParticipate=false;Run();TestEqual(TEXT("Absent/dead speaker denied"),Routes[0].Result,EPFVoiceRoute::Unavailable);P[0].bCanParticipate=true;
    P[1].Muted.Add(FGuid(0,0,0,99));Run();TestEqual(TEXT("Valid offline preference ID inert"),Routes[0].Result,EPFVoiceRoute::Allowed);
    float Previous=1;for(int32 D=0;D<=1600;D+=25){P[1].Position.X=D;if(!Run()){return false;}TestTrue(TEXT("Gain finite bounded monotonic"),FMath::IsFinite(Routes[0].Gain) && Routes[0].Gain>=0 && Routes[0].Gain<=Previous);Previous=Routes[0].Gain;}
    P.Swap(1,2);Run();TestEqual(TEXT("Routes stable by identity, not roster order"),Routes[0].PlayerId,FGuid(0,0,0,2));
    AddInfo(TEXT("[PrimalVoice] Metadata policy consent/PTT/range/volume/mute/block/space/connectivity checked; no audio transport or listening pass."));return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFVoicePolicyValidationTest,"PF.Voice.PolicyValidation",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFVoicePolicyValidationTest::RunTest(const FString&)
{
    FPFVoicePolicyConfig Config;Config.bEnabled=true;const TArray<FPFVoiceParticipant> Good={Participant(1,0),Participant(2,600)};
    TArray<FPFVoiceRecipient> Out;FPFVoiceRecipient Sentinel;Sentinel.PlayerId=FGuid(0,0,8,8);Sentinel.Gain=.75f;Sentinel.Result=EPFVoiceRoute::Blocked;Out.Add(Sentinel);FString Error;
    const auto Reject=[&](const FPFVoicePolicyConfig& C,const TArray<FPFVoiceParticipant>& P,FGuid Speaker)
    {TestFalse(TEXT("Malformed input refuses"),FPFProximityVoicePolicy::Evaluate(C,P,Speaker,Out,Error));TestTrue(TEXT("Refusal preserves output and supplies reason"),Out.Num()==1 && Out[0].PlayerId==Sentinel.PlayerId && Out[0].Gain==Sentinel.Gain && Out[0].Result==Sentinel.Result && !Error.IsEmpty());};
    Reject(Config,Good,FGuid());Reject(Config,Good,FGuid(0,0,0,99));Reject(Config,{},Good[0].PlayerId);
    auto BadConfig=Config;BadConfig.MaxDistanceCm=0;Reject(BadConfig,Good,Good[0].PlayerId);BadConfig=Config;BadConfig.FullVolumeDistanceCm=1500;Reject(BadConfig,Good,Good[0].PlayerId);BadConfig=Config;BadConfig.MaxDistanceCm=10001;Reject(BadConfig,Good,Good[0].PlayerId);BadConfig=Config;BadConfig.FullVolumeDistanceCm=-1;Reject(BadConfig,Good,Good[0].PlayerId);BadConfig=Config;BadConfig.MaxDistanceCm=std::numeric_limits<double>::quiet_NaN();Reject(BadConfig,Good,Good[0].PlayerId);
    auto Bad=Good;Bad[1].PlayerId=Bad[0].PlayerId;Reject(Config,Bad,Good[0].PlayerId);Bad=Good;Bad[1].SpaceId.Invalidate();Reject(Config,Bad,Good[0].PlayerId);Bad=Good;Bad[1].Position.X=std::numeric_limits<double>::infinity();Reject(Config,Bad,Good[0].PlayerId);Bad=Good;Bad[1].Position.Y=std::numeric_limits<double>::quiet_NaN();Reject(Config,Bad,Good[0].PlayerId);Bad=Good;Bad[1].Position.Z=FPFProximityVoicePolicy::MaxCoordinateCm+1;Reject(Config,Bad,Good[0].PlayerId);
    for(float Volume:{-1.f,1.01f,std::numeric_limits<float>::infinity()}){Bad=Good;Bad[1].ReceiveVolume=Volume;Reject(Config,Bad,Good[0].PlayerId);}
    Bad=Good;Bad[1].Muted={Good[0].PlayerId,Good[0].PlayerId};Reject(Config,Bad,Good[0].PlayerId);Bad=Good;Bad[1].Blocked.Add(Bad[1].PlayerId);Reject(Config,Bad,Good[0].PlayerId);Bad=Good;Bad[1].Muted.Add(FGuid());Reject(Config,Bad,Good[0].PlayerId);
    Bad.Reset();for(uint32 N=1;N<=33;++N){Bad.Add(Participant(N,N));}Reject(Config,Bad,Good[0].PlayerId);Bad=Good;for(uint32 N=10;N<43;++N){Bad[1].Blocked.Add(FGuid(0,0,0,N));}Reject(Config,Bad,Good[0].PlayerId);
    Bad.Reset();for(uint32 N=1;N<=32;++N){Bad.Add(Participant(N,0));}TestTrue(TEXT("Exact32 participant bound accepted"),FPFProximityVoicePolicy::Evaluate(Config,Bad,Bad[0].PlayerId,Out,Error));TestEqual(TEXT("At most31 recipients"),Out.Num(),31);
    AddInfo(TEXT("[PrimalVoice] Invalid identity/config/finite bounds/preferences/roster refuse atomically; metadata only."));return true;
}
#endif
