#if WITH_DEV_AUTOMATION_TESTS
#include "Progression/PFProgressionComponent.h"
#include "Inventory/PFInventoryPlayerState.h"
#include "Inventory/PFInventoryComponent.h"
#include "Crafting/PFCraftingComponent.h"
#include "Survival/PFSurvivalGameMode.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFSurvivorCharacter.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Persistence/PFWorldPersistence.h"
#include "World/PFWorldClock.h"
#include "Components/BoxComponent.h"
#include "GameFramework/PlayerStart.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFKnowledgeRequestTest,"PF.Progression.KnowledgeRequests",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFKnowledgeRequestTest::RunTest(const FString&)
{
    AddExpectedMessage(TEXT("GetSocketInfoByName.*No SkeletalMesh for Component"),EAutomationExpectedMessageFlags::Contains,0);
    FTestWorldWrapper Fixture;if(!Fixture.CreateTestWorld(EWorldType::Game)){return false;}auto* W=Fixture.GetTestWorld();
    W->SetGameMode(FURL(nullptr,TEXT("/Engine/Maps/Entry?game=/Script/PrimalFrontier.PFSurvivalGameMode"),TRAVEL_Absolute));
    auto* Floor=W->SpawnActor<AActor>();auto* Box=NewObject<UBoxComponent>(Floor);Floor->SetRootComponent(Box);Box->SetBoxExtent(FVector(4000,4000,25));Box->SetCollisionProfileName(TEXT("BlockAll"));Box->RegisterComponent();Floor->SetActorLocation(FVector(0,0,-25));
    W->SpawnActor<APlayerStart>(FVector(-500,0,120),FRotator::ZeroRotator);W->SpawnActor<APFWorldClock>();if(!Fixture.BeginPlayInTestWorld()){return false;}
    auto* PC=W->SpawnActor<APFSurvivalPlayerController>();auto* Other=W->SpawnActor<APFSurvivalPlayerController>();auto* GM=W->GetAuthGameMode<APFSurvivalGameMode>();GM->RestartPlayer(PC);GM->RestartPlayer(Other);
    auto* PS=PC->GetPlayerState<APFInventoryPlayerState>();auto* Pawn=Cast<APFSurvivorCharacter>(PC->GetPawn());
    if(!PS || !PS->Progression || !Pawn || !Other->GetPawn()){return false;}auto* G=PS->Progression.Get();FString Error;
    auto Advance=[&](){for(int32 N=0;N<4;++N){Fixture.TickTestWorld(0.1f);}};
    TestFalse(TEXT("Fresh player cannot bypass level/points"),G->RequestKnowledge(TEXT("Tech_FieldTools"),Pawn,Error));
    TestTrue(TEXT("Premature refusal unchanged"),G->GetExperience()==0 && G->GetRecord().Knowledge.IsEmpty());Advance();
    FPFProgressionRecord Seed;Seed.Experience=100;
    if(!TestTrue(TEXT("Trusted isolated100XP fixture"),G->Restore(Seed,Error))){AddError(Error);return false;}
    TestFalse(TEXT("Missing pawn"),G->RequestKnowledge(TEXT("Tech_FieldTools"),nullptr,Error));
    TestFalse(TEXT("Foreign pawn rejected"),G->RequestKnowledge(TEXT("Tech_FieldTools"),Other->GetPawn(),Error));
    TestFalse(TEXT("Unknown knowledge"),G->RequestKnowledge(TEXT("Tech_Unknown"),Pawn,Error));
    TestFalse(TEXT("Invalid request cannot skip cooldown"),G->RequestKnowledge(TEXT("Tech_FieldTools"),Pawn,Error));
    TestTrue(TEXT("Refusals preserve trustedXP,price and inventory"),G->GetExperience()==100 && G->GetAvailablePoints()==3 && G->GetRecord().Knowledge.IsEmpty() && PS->Inventory->GetStacks().IsEmpty());Advance();
    PS->SetRole(ROLE_AutonomousProxy);const FString Before=G->GetKnowledgeFeedback();
    TestFalse(TEXT("Direct client mutation refuses"),G->RequestKnowledge(TEXT("Tech_FieldTools"),Pawn,Error));TestEqual(TEXT("Client cannot even forge feedback"),G->GetKnowledgeFeedback(),Before);PS->SetRole(ROLE_Authority);
    // Actual controller boundary, not direct pure-record purchase.
    PC->ServerLearnKnowledge(TEXT("Tech_FieldTools"));
    TestTrue(TEXT("Owned server request spends two exactlyonce"),G->GetExperience()==100 && G->GetAvailablePoints()==1 && G->GetRecord().Knowledge==TArray<FName>{FName(TEXT("Tech_FieldTools"))});
    TestTrue(TEXT("Owner-only truthful pending-access feedback"),G->GetKnowledgeFeedback().Contains(TEXT("recipe access integration pending")));
    TestTrue(TEXT("No free item/craft credit"),PS->Inventory->GetStacks().IsEmpty() && G->GetRecord().CreditedCrafts.IsEmpty());Advance();
    TestFalse(TEXT("Duplicate cannot double-spend"),G->RequestKnowledge(TEXT("Tech_FieldTools"),Pawn,Error));TestEqual(TEXT("Duplicate preserves one point"),G->GetAvailablePoints(),1);Advance();
    TestFalse(TEXT("Empty knowledge rejected"),G->RequestKnowledge(NAME_None,Pawn,Error));Advance();
    Pawn->Survival->ApplyDamage(100);TestTrue(TEXT("Fixture dead"),Pawn->Survival->IsDead());
    TestFalse(TEXT("Dead pawn rejected even withpoints"),G->RequestKnowledge(TEXT("Tech_FieldTools"),Pawn,Error));
    TestEqual(TEXT("Other player state never changed"),Other->GetPlayerState<APFInventoryPlayerState>()->Progression->GetExperience(),0);
    TestEqual(TEXT("Other points unchanged"),Other->GetPlayerState<APFInventoryPlayerState>()->Progression->GetAvailablePoints(),0);
    W->GetSubsystem<UPFWorldPersistence>()->ActiveSlot.Reset();
    AddInfo(TEXT("[PrimalProgression] Owned living server request,level/unknown/cooldown/foreign/client/death refusal,atomic pricing/no item creation and duplicate protection checked;100XP is fixture seed."));
    Fixture.ForwardErrorMessages(this);return true;
}
#endif
