#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Voice/PFVoiceAuthority.h"
#include "Survival/PFSurvivalGameMode.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFSurvivorCharacter.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Inventory/PFInventoryPlayerState.h"
#include "Tests/AutomationCommon.h"
#include "GameFramework/PlayerStart.h"
#include <limits>

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFVoiceAuthorityTest,"PF.Voice.AuthoritySnapshots",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFVoiceAuthorityTest::RunTest(const FString&)
{
    AddExpectedMessage(TEXT("GetSocketInfoByName.*No SkeletalMesh for Component"),EAutomationExpectedMessageFlags::Contains,0);
    FTestWorldWrapper Fixture;if(!Fixture.CreateTestWorld(EWorldType::Game)){return false;}
    auto* W=Fixture.GetTestWorld();
    W->SetGameMode(FURL(nullptr,TEXT("/Engine/Maps/Entry?game=/Script/PrimalFrontier.PFSurvivalGameMode"),TRAVEL_Absolute));
    W->SpawnActor<APlayerStart>(FVector(0,0,200),FRotator::ZeroRotator);
    auto* A=W->GetSubsystem<UPFVoiceAuthority>();FString Error;TArray<FPFVoiceParticipant> Snapshot;
    if(!TestNotNull(TEXT("Game subsystem exists"),A)){return false;}
    FPFVoiceParticipant Sentinel;Sentinel.PlayerId=FGuid(0,0,9,9);Snapshot.Add(Sentinel);
    TestFalse(TEXT("Pre-play metadata rejected"),A->BuildSnapshot(Snapshot,Error));
    TestEqual(TEXT("Refusal preserves snapshot"),Snapshot[0].PlayerId,Sentinel.PlayerId);
    if(!Fixture.BeginPlayInTestWorld()){return false;}
    TestFalse(TEXT("Empty server roster rejected"),A->BuildSnapshot(Snapshot,Error));
    auto Spawn=[&]()
    {auto* PC=W->SpawnActor<APFSurvivalPlayerController>();W->GetAuthGameMode<APFSurvivalGameMode>()->RestartPlayer(PC);return PC;};
    auto* Speaker=Spawn();auto* Listener=Spawn();
    auto* SP=Cast<APFSurvivorCharacter>(Speaker->GetPawn());auto* LP=Cast<APFSurvivorCharacter>(Listener->GetPawn());
    if(!TestNotNull(TEXT("Real speaker pawn"),SP) || !TestNotNull(TEXT("Real listener pawn"),LP)){return false;}
    SP->SetActorLocation(FVector(0,0,200));LP->SetActorLocation(FVector(900,0,200));
    if(!TestTrue(TEXT("Build actual server roster"),A->BuildSnapshot(Snapshot,Error))){AddError(Error);return false;}
    const auto* SState=Speaker->GetPlayerState<APFInventoryPlayerState>();auto* LState=Listener->GetPlayerState<APFInventoryPlayerState>();
    TestEqual(TEXT("Actual public identities only"),Snapshot.Num(),2);
    TestTrue(TEXT("Snapshot uses real pawn positions"),Snapshot.ContainsByPredicate([&](const auto& P){return P.PlayerId==LState->PersistentPlayerId && P.Position==LP->GetActorLocation();}));
    for(const auto& P:Snapshot){TestTrue(TEXT("Consent defaults off"),!P.bTransmitConsent && !P.bReceiveConsent && !P.bPushToTalk);}
    TArray<FPFVoiceRecipient> Routes;TestTrue(TEXT("Default routing evaluates"),A->Evaluate(Speaker,Routes,Error));TestEqual(TEXT("Default disabled"),Routes[0].Result,EPFVoiceRoute::Disabled);
    FPFVoicePolicyConfig Config;Config.bEnabled=true;TestTrue(TEXT("Native server enables policy"),A->SetConfig(Config,Error));
    FPFVoicePreferences Send;Send.bTransmitConsent=true;Send.bPushToTalk=true;FPFVoicePreferences Receive;Receive.bReceiveConsent=true;
    TestTrue(TEXT("Server speaker preference"),A->SetPreferences(Speaker,Send,Error));TestTrue(TEXT("Server listener preference"),A->SetPreferences(Listener,Receive,Error));
    TestTrue(TEXT("Actual range evaluation"),A->Evaluate(Speaker,Routes,Error));TestEqual(TEXT("Actual900cm gives half"),Routes[0].Gain,.5f);
    LP->SetActorLocation(FVector(1600,0,200));A->Evaluate(Speaker,Routes,Error);TestEqual(TEXT("New server position far"),Routes[0].Result,EPFVoiceRoute::OutOfRange);
    LP->SetActorLocation(FVector(200,0,200));Receive.Muted.Add(SState->PersistentPlayerId);A->SetPreferences(Listener,Receive,Error);A->Evaluate(Speaker,Routes,Error);TestEqual(TEXT("Actual recipient mute"),Routes[0].Result,EPFVoiceRoute::Muted);
    auto Bad=Receive;Bad.ReceiveVolume=std::numeric_limits<float>::quiet_NaN();TestFalse(TEXT("Malformed preference rejected"),A->SetPreferences(Listener,Bad,Error));A->Evaluate(Speaker,Routes,Error);TestEqual(TEXT("Earlier preference preserved"),Routes[0].Result,EPFVoiceRoute::Muted);
    Receive.Muted.Reset();Receive.Blocked.Add(SState->PersistentPlayerId);A->SetPreferences(Listener,Receive,Error);A->Evaluate(Speaker,Routes,Error);TestEqual(TEXT("Actual recipient block"),Routes[0].Result,EPFVoiceRoute::Blocked);
    Receive.Blocked.Reset();A->SetPreferences(Listener,Receive,Error);LP->Survival->ApplyDamage(100);A->Evaluate(Speaker,Routes,Error);TestEqual(TEXT("Actual death denied"),Routes[0].Result,EPFVoiceRoute::Unavailable);
    Listener->UnPossess();A->Evaluate(Speaker,Routes,Error);TestEqual(TEXT("Unpossessed listener denied"),Routes[0].Result,EPFVoiceRoute::Unavailable);
    const FGuid ListenerId=LState->PersistentPlayerId;LState->PersistentPlayerId=SState->PersistentPlayerId;TestFalse(TEXT("Duplicate actual identity rejected"),A->Evaluate(Speaker,Routes,Error));TestEqual(TEXT("Invalid roster preserves last result"),Routes[0].Result,EPFVoiceRoute::Unavailable);LState->PersistentPlayerId=ListenerId;
    auto* Replacement=W->SpawnActor<APFSurvivorCharacter>(FVector(200,0,200),FRotator::ZeroRotator);Listener->Possess(Replacement);A->Evaluate(Speaker,Routes,Error);TestEqual(TEXT("Pawn replacement uses new life"),Routes[0].Result,EPFVoiceRoute::Allowed);
    Config.MaxDistanceCm=std::numeric_limits<double>::infinity();TestFalse(TEXT("Bad config rejected atomically"),A->SetConfig(Config,Error));A->Evaluate(Speaker,Routes,Error);TestEqual(TEXT("Good config retained"),Routes[0].Result,EPFVoiceRoute::Allowed);
    TestFalse(TEXT("Null owner rejected"),A->SetPreferences(nullptr,Receive,Error));
    FTestWorldWrapper Other;if(!Other.CreateTestWorld(EWorldType::Game)){return false;}auto* Foreign=Other.GetTestWorld()->SpawnActor<APFSurvivalPlayerController>();
    TestFalse(TEXT("Foreign-world owner rejected"),A->SetPreferences(Foreign,Receive,Error));TestFalse(TEXT("Foreign-world speaker rejected"),A->Evaluate(Foreign,Routes,Error));
    Listener->Destroy();auto* Reconnected=Spawn();Reconnected->GetPlayerState<APFInventoryPlayerState>()->PersistentPlayerId=ListenerId;Reconnected->GetPawn()->SetActorLocation(FVector(200,0,200));
    TestTrue(TEXT("Reconnect metadata evaluates"),A->Evaluate(Speaker,Routes,Error));TestEqual(TEXT("New controller same ID consent resets"),Routes[0].Result,EPFVoiceRoute::NotReceiving);
    AddInfo(TEXT("[PrimalVoice] Real server actors: position/life/identity, ownership, default-off consent, mute/block, atomic refusal and reconnect reset; no audio."));return true;
}
#endif
