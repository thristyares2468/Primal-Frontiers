#include "Voice/PFVoiceAuthority.h"
#include "Survival/PFSurvivalGameMode.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFSurvivorCharacter.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Inventory/PFInventoryPlayerState.h"
#include "Engine/World.h"
#include "Engine/NetConnection.h"

namespace
{
void Apply(const FPFVoicePreferences& Pref,FPFVoiceParticipant& P)
{
    P.bTransmitConsent=Pref.bTransmitConsent;P.bReceiveConsent=Pref.bReceiveConsent;
    P.bPushToTalk=Pref.bPushToTalk;P.ReceiveVolume=Pref.ReceiveVolume;P.Muted=Pref.Muted;P.Blocked=Pref.Blocked;
}
bool Check(const FPFVoicePolicyConfig& C,const TArray<FPFVoiceParticipant>& P,FString& Error)
{
    TArray<FPFVoiceRecipient> Ignore;return FPFProximityVoicePolicy::Evaluate(C,P,P[0].PlayerId,Ignore,Error);
}
}
void UPFVoiceAuthority::Initialize(FSubsystemCollectionBase& Collection)
{Super::Initialize(Collection);Space=FGuid::NewGuid();}
void UPFVoiceAuthority::Deinitialize()
{Preferences.Reset();Space.Invalidate();Super::Deinitialize();}
bool UPFVoiceAuthority::DoesSupportWorldType(EWorldType::Type Type) const
{return Type==EWorldType::Game || Type==EWorldType::PIE;}
bool UPFVoiceAuthority::IsServer(FString& Error) const
{
    Error.Reset();const auto* W=GetWorld();
    if(!W || !DoesSupportWorldType(W->WorldType) || !W->HasBegunPlay() || W->GetNetMode()==NM_Client ||
        !W->GetAuthGameMode<APFSurvivalGameMode>() || !Space.IsValid())
    {Error=TEXT("Voice metadata requires an active survival server world");return false;}return true;
}
bool UPFVoiceAuthority::SetConfig(const FPFVoicePolicyConfig& In,FString& Error)
{
    if(!IsServer(Error)){return false;}
    FPFVoiceParticipant Dummy;Dummy.PlayerId=FGuid(0,0,0,1);Dummy.SpaceId=Space;
    if(!Check(In,{Dummy},Error)){return false;}Config=In;return true;
}
bool UPFVoiceAuthority::BuildSnapshot(TArray<FPFVoiceParticipant>& Out,FString& Error) const
{
    if(!IsServer(Error)){return false;}
    const auto* W=GetWorld();TArray<FPFVoiceParticipant> Candidate;
    for(auto It=W->GetPlayerControllerIterator();It;++It)
    {
        auto* PC=Cast<APFSurvivalPlayerController>(It->Get());
        if(!PC || PC->IsActorBeingDestroyed()){continue;}
        auto* PS=PC->GetPlayerState<APFInventoryPlayerState>();
        if(!PC->HasAuthority() || !PS || !PS->HasAuthority() || PS->GetOwner()!=PC || PS->IsInactive())
        {Error=TEXT("Inconsistent voice player ownership");return false;}
        FPFVoiceParticipant P;P.PlayerId=PS->PersistentPlayerId;P.SpaceId=Space;
        const auto* Connection=PC->GetNetConnection();
        P.bConnected=W->GetNetMode()==NM_Standalone || PC->IsLocalController() ||
            (Connection && Connection->GetConnectionState()==USOCK_Open);
        if(APawn* Pawn=PC->GetPawn())
        {
            if(!Pawn->HasAuthority() || Pawn->GetWorld()!=W || Pawn->GetController()!=PC || Pawn->GetPlayerState()!=PS)
            {Error=TEXT("Inconsistent voice pawn ownership");return false;}
            P.Position=Pawn->GetActorLocation();
            const auto* Survivor=Cast<APFSurvivorCharacter>(Pawn);
            P.bCanParticipate=Survivor && Survivor->Survival && !Survivor->IsActorBeingDestroyed() && !Survivor->Survival->IsDead();
        }
        if(const auto* Pref=Preferences.Find(PC)){Apply(*Pref,P);}
        Candidate.Add(MoveTemp(P));
        if(Candidate.Num()>FPFProximityVoicePolicy::MaxParticipants){Error=TEXT("Voice roster exceeds bounded capacity");return false;}
    }
    if(Candidate.IsEmpty()){Error=TEXT("No connected survival voice roster");return false;}
    if(!Check(Config,Candidate,Error)){return false;}
    Out=MoveTemp(Candidate);return true;
}
bool UPFVoiceAuthority::SetPreferences(APFSurvivalPlayerController* Owner,const FPFVoicePreferences& In,FString& Error)
{
    if(!IsServer(Error)){return false;}
    if(!Owner || Owner->GetWorld()!=GetWorld() || !Owner->HasAuthority() || Owner->IsActorBeingDestroyed())
    {Error=TEXT("Voice preferences require their actual server controller");return false;}
    TArray<FPFVoiceParticipant> P;if(!BuildSnapshot(P,Error)){return false;}
    const auto* PS=Owner->GetPlayerState<APFInventoryPlayerState>();
    auto* Match=PS?P.FindByPredicate([&](const auto& Item){return Item.PlayerId==PS->PersistentPlayerId;}):nullptr;
    if(!Match || !Match->bConnected){Error=TEXT("Voice preference owner is not connected");return false;}
    Apply(In,*Match);if(!Check(Config,P,Error)){return false;}
    // Disconnected/destroyed controller preferences never transfer through a reconnect GUID.
    for(auto It=Preferences.CreateIterator();It;++It)
    {if(!It.Key().IsValid() || It.Key()->IsActorBeingDestroyed()){It.RemoveCurrent();}}
    Preferences.Add(Owner,In);return true;
}
bool UPFVoiceAuthority::Evaluate(APFSurvivalPlayerController* Speaker,TArray<FPFVoiceRecipient>& Out,FString& Error) const
{
    if(!IsServer(Error)){return false;}
    if(!Speaker || Speaker->GetWorld()!=GetWorld() || !Speaker->HasAuthority() || Speaker->IsActorBeingDestroyed())
    {Error=TEXT("Voice speaker requires its actual server controller");return false;}
    TArray<FPFVoiceParticipant> P;if(!BuildSnapshot(P,Error)){return false;}
    const auto* PS=Speaker->GetPlayerState<APFInventoryPlayerState>();
    if(!PS){Error=TEXT("Voice speaker has no server player state");return false;}
    return FPFProximityVoicePolicy::Evaluate(Config,P,PS->PersistentPlayerId,Out,Error);
}
