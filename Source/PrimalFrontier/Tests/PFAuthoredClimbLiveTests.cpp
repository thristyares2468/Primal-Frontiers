#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Survival/PFSurvivorCharacter.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Survival/PFTraversalMovement.h"
#include "Inventory/PFInventoryComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/PlayerStart.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "UObject/Package.h"
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "UnrealClient.h"
#include "InputKeyEventArgs.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"

namespace
{
struct FAuthoredClimbRecord {int32 Lane=0,Phase=0;float Stamina=100;double Landed=0;};
class FAuthoredClimbExercise : public IAutomationLatentCommand
{
public:
    FAuthoredClimbExercise(FAutomationTestBase* InTest,FString InDirectory):Test(InTest),Directory(MoveTemp(InDirectory))
    {FParse::Value(FCommandLine::Get(),TEXT("PFExpectedPlayers="),Expected);Manual=FParse::Param(FCommandLine::Get(),TEXT("PFManualClimb"));FullHeight=FParse::Param(FCommandLine::Get(),TEXT("PFFullHeightClimb"));}
    bool Update() override
    {
        const double Now=FPlatformTime::Seconds();if(Done){return Now-Done>(Client?8:15);}
        if(Now-Started>100){Test->AddError(FString::Printf(TEXT("[PrimalTraversal] Authored climb timeout ownerStage=%d %s"),OwnerStage,*Diagnostic));return true;}
        UWorld* W=nullptr;for(const auto& C:GEngine->GetWorldContexts()){if(C.World() && C.World()->IsGameWorld()){W=C.World();break;}}if(!W){return false;}
        if(!W->GetOutermost()->GetName().Contains(TEXT("L_PrimalFrontier_OpenWorld"))){Test->AddError(TEXT("Requires actual open-world map"));return true;}
        Client=W->GetNetMode()==NM_Client;
        if(!Validated)
        {
            for(int32 Lane=0;Lane<2;++Lane){for(int32 Kind=0;Kind<2;++Kind)
            {
                const FName Tag(*(FullHeight && Kind?FString::Printf(TEXT("PF.M14.Climb4_%c"),Lane?TEXT('B'):TEXT('A')):FString::Printf(TEXT("PF.M14.Ledge%d_%c"),Kind?110:80,Lane?TEXT('B'):TEXT('A'))));
                AStaticMeshActor* Found=nullptr;for(TActorIterator<AStaticMeshActor> It(W);It;++It){if(It->ActorHasTag(Tag)){if(Found){Test->AddError(TEXT("Duplicate authored ledge"));return true;}Found=*It;}}
                if(!Found){return false;}
                const FVector At(FullHeight && Kind?(Lane?2250:1850):(Lane?(Kind?1600:1000):(Kind?1250:650)),Lane?-3000:-1100,Kind?(FullHeight?200:55):40),Scale(3,3.6,Kind?(FullHeight?4:1.1):.8);
                if(!Found->GetActorLocation().Equals(At,.1f) || !Found->GetActorScale3D().Equals(Scale,.001f) ||
                   !Found->GetStaticMeshComponent()->GetStaticMesh() || Found->GetStaticMeshComponent()->GetCollisionProfileName()!=TEXT("BlockAll"))
                {Test->AddError(TEXT("Authored course geometry differs from saved contract"));return true;}
                Ledges[Lane][Kind]=Found;
            }}
            int32 Starts=0;
            for(TActorIterator<APlayerStart> It(W);It;++It)
            {
                const FVector Start=It->GetActorLocation(),Wall=Ledges[0][0]->GetActorLocation();
                const float FaceDistance=Wall.X-150-Start.X;
                if(FaceDistance<200 || FaceDistance>800 || FMath::Abs(Start.Y-Wall.Y)>10)
                {Test->AddError(TEXT("Climb wall must be beside both unobstructed camp starts (2-8m to west face)"));return true;}
                ++Starts;
            }
            if(Starts!=2){Test->AddError(TEXT("Expected both normal camp PlayerStarts"));return true;}
            Validated=true;Test->AddInfo(FString::Printf(TEXT("[PrimalTraversal] Loaded four real authored 80/%dcm walls; near-spawn wall verified; no runtime geometry created"),FullHeight?400:110));
        }
        TArray<APFSurvivalPlayerController*> Players;
        for(auto It=W->GetPlayerControllerIterator();It;++It){auto* PC=Cast<APFSurvivalPlayerController>(It->Get());if(PC && Cast<APFSurvivorCharacter>(PC->GetPawn()) && PC->GetInventory() && PC->GetInventory()->Catalog){Players.Add(PC);}}
        if(Players.Num()!=(Client?1:Expected)){return false;}
        if(!Client)
        {
            if(!Initialized)
            {
                for(int32 Lane=0;Lane<Players.Num();++Lane)
                {
                    auto* PC=Players[Lane];auto* P=CastChecked<APFSurvivorCharacter>(PC->GetPawn());
                    if(!PC->GetInventory()->GetStacks().IsEmpty()){Test->AddError(TEXT("Requires fresh disposable player bag"));return true;}
                    P->Survival->HungerDrainPerSecond=0;P->Survival->ThirstDrainPerSecond=0;P->Survival->StaminaRecoveryPerSecond=0;
                    // Shift is only a fixture readiness/landing acknowledgement.
                    // In-flight saved moves must not add unrelated sprint drain or
                    // boost to the isolated single-attempt climb cost transaction.
                    P->Traversal()->SprintStaminaPerSecond=0;P->Traversal()->SprintMultiplier=1;
                    FAuthoredClimbRecord R;R.Lane=Lane;R.Stamina=P->Survival->GetVitals().Stamina;Records.Add(PC,R);
                }
                Initialized=true;
            }
            bool All=true;
            for(auto* PC:Players)
            {
                auto& R=Records.FindChecked(PC);auto* P=CastChecked<APFSurvivorCharacter>(PC->GetPawn());auto* M=P->Traversal();
                if(R.Phase==0 && M->bWantsSprint){Place(PC,R,0);P->Survival->SetHealth(97);R.Phase=1;Test->AddInfo(FString::Printf(TEXT("[PrimalTraversal] Placed lane%d at %s"),R.Lane,*P->GetActorLocation().ToString()));}
                else if((R.Phase==1 || R.Phase==3) && Landed(P,R.Lane,R.Phase==3?1:0))
                {
                    M->StopMovementImmediately();Test->TestTrue(TEXT("Authored mantle spends exactly its manual or jump-contact cost"),
                        FMath::IsNearlyEqual(P->Survival->GetVitals().Stamina,R.Stamina-(Manual?0:P->JumpStaminaCost)-P->ClimbStaminaCost-FMath::Max(0.,Top(R.Lane,R.Phase==3?1:0)-2.15-P->MaxClimbHeight)*P->ClimbStaminaPerMetre/100.,.5f));
                    R.Landed=W->GetTimeSeconds();++R.Phase;
                }
                else if((R.Phase==2 || R.Phase==4) && M->bWantsSprint && W->GetTimeSeconds()-R.Landed>1.6)
                {
                    if(R.Phase==2){R.Stamina=P->Survival->GetVitals().Stamina;Place(PC,R,1);P->Survival->SetHealth(96);R.Phase=3;}
                    else
                    {
                        M->ClearTraversal();
                        if(!Directory.IsEmpty()){P->TeleportTo(FVector(-200,-850,100),FRotator::ZeroRotator,false,true);M->StopMovementImmediately();M->SetMovementMode(MOVE_Walking);}
                        P->Survival->SetHealth(95);R.Phase=5;
                    }
                }
                All &= R.Phase==5;
                if(R.Phase!=5){Diagnostic=FString::Printf(TEXT("lane%d phase%d at%s stamina%.1f sprint%d"),R.Lane,R.Phase,*P->GetActorLocation().ToString(),P->Survival->GetVitals().Stamina,M->bWantsSprint);}
            }
            // Health95 is each owner's completion signal. Wait for a distinct
            // replicated view-yaw acknowledgement before broadcasting health94;
            // otherwise adjacent snapshots can coalesce and hide95 from a client.
            bool OwnersSawCompletion=true;
            for(auto* PC:Players){OwnersSawCompletion &= FMath::Abs(FRotator::NormalizeAxis(PC->GetControlRotation().Yaw-123))<1;}
            if(All && (W->GetNetMode()==NM_Standalone?OwnerComplete:OwnersSawCompletion))
            {for(auto* PC:Players){CastChecked<APFSurvivorCharacter>(PC->GetPawn())->Survival->SetHealth(94);}Test->AddInfo(TEXT("[PrimalTraversal] Authored server real map contact landings and exact costs verified"));Done=Now;return false;}
        }
        if(Client || W->GetNetMode()==NM_Standalone)
        {
            auto* PC=Players[0];auto* P=CastChecked<APFSurvivorCharacter>(PC->GetPawn());auto* M=P->Traversal();const float H=P->Survival->GetVitals().Health;
            M->SprintStaminaPerSecond=0;M->SprintMultiplier=1;
            Diagnostic=FString::Printf(TEXT("ready%d approaching%d prepare%.3f health%.1f at%s yaw%.1f grounded%d W%d ignored%d"),Ready,Approaching,Prepare,H,*P->GetActorLocation().ToString(),PC->GetControlRotation().Yaw,M->IsMovingOnGround(),PC->IsInputKeyDown(EKeys::W),PC->IsMoveInputIgnored());
            if(TallPlaced && !OwnerComplete)
            {
                if(!TallPressed && M->IsMovingOnGround() && Now-TallAt>.5)
                {Key(PC,EKeys::F,IE_Pressed);TallPressed=true;TallAt=Now;}
                if(TallPressed && Now-TallAt>.6)
                {
                    Key(PC,EKeys::F,IE_Released);
                    Test->TestTrue(TEXT("Actual tall landmark stamina refusal reaches owner HUD feedback"),PC->RecentInteractionMessage().Contains(TEXT("stamina")));
                    Test->TestEqual(TEXT("Tall wall refusal grants no height or spends stamina"),P->Survival->GetVitals().Stamina,TallStamina);
                    Test->TestTrue(TEXT("Tall landmark remains solid; player stays grounded"),M->IsMovingOnGround() && P->GetActorLocation().Z<110);
                    Capture(TEXT("tall_wall_refusal"));OwnerComplete=true;
                }
                return false;
            }
            if(!Ready)
            {
                if(P->IsTraversalInputBlocked() || PC->GetPlayerSetupStatus()==EPFPlayerSetupStatus::Unconfirmed){return false;}
                if(!Directory.IsEmpty())
                {
                    if(SpawnViewAt==0){SpawnViewAt=Now;PC->SetControlRotation(FRotator(-10,0,0));}
                    if(Now-SpawnViewAt<.6){return false;}
                    Capture(TEXT("spawn_wall"));
                }
                Key(PC,EKeys::LeftShift,IE_Pressed);Ready=true;ReadyAt=Now;
            }
            // Startup can flush pressed keys or clear traversal intent before the
            // second client joins. Re-arm only the fixture's initial readiness cue;
            // actual W/F/Space climb inputs are never retried or replaced with calls.
            if(OwnerStage==0 && H==100 && !M->bWantsSprint && Now-ReadyAt>1 && !P->IsTraversalInputBlocked())
            {Key(PC,EKeys::LeftShift,IE_Released);Key(PC,EKeys::LeftShift,IE_Pressed);ReadyAt=Now;Test->AddInfo(TEXT("[PrimalTraversal] Re-armed initial raw-key readiness cue before any climb"));}
            const int32 Lane=FMath::Abs(P->GetActorLocation().Y+1100)<FMath::Abs(P->GetActorLocation().Y+3000)?0:1;
            if((OwnerStage==0 && H==97) || (OwnerStage==2 && H==96))
            {
                const int32 Kind=OwnerStage==2?1:0;const auto* A=Ledges[Lane][Kind].Get();const float X=A->GetActorLocation().X-150-P->GetCapsuleComponent()->GetScaledCapsuleRadius()-200;
                // Server reposition can be followed by replay of already-sent moves.
                // Accept the same clear approach corridor, then use actual W until
                // contact. Never retry or substitute the climb key itself.
                if(M->IsMovingOnGround() && P->GetActorLocation().X>=X-150 && P->GetActorLocation().X<=X+50)
                {
                    if(Prepare==0){Prepare=Now;Key(PC,EKeys::LeftShift,IE_Released);PC->SetControlRotation(FRotator(-15,0,0));}
                    if(!Directory.IsEmpty() && !Kind && Now-Prepare>.2 && !ApproachShot){Capture(TEXT("camp_approach"));ApproachShot=true;}
                    if(!Approaching && Now-Prepare>(Directory.IsEmpty()?.4:.5)){Key(PC,EKeys::W,IE_Pressed);Approaching=true;}
                }
                // Real keyboard routing through PlayerInput/Enhanced Input. Walk
                // into contact before F or Space rather than launch from an ideal gap.
                if(Approaching && M->IsMovingOnGround() && P->GetActorLocation().X>=X+199)
                {Key(PC,Manual?EKeys::F:EKeys::SpaceBar,IE_Pressed);ReleaseAt=Now+.15;Prepare=0;Approaching=false;++OwnerStage;}
            }
            if(OwnerStage==1 || OwnerStage==3)
            {
                if(Now>=ReleaseAt){Key(PC,Manual?EKeys::F:EKeys::SpaceBar,IE_Released);}
                const int32 Kind=OwnerStage==3?1:0;
                if(!Acknowledged && Landed(P,Lane,Kind))
                {
                    Key(PC,EKeys::W,IE_Released);M->StopMovementImmediately();PC->SetControlRotation(FRotator(-55,0,0));Capture(Kind?(FullHeight?TEXT("climb_400_top"):TEXT("climb_110_top")):TEXT("climb_80_top"));
                    Test->AddInfo(FString::Printf(TEXT("[PrimalTraversal] Owner real authored %.0fcm landing verified with raw W + %s input"),Top(Lane,Kind),Manual?TEXT("F"):TEXT("Space")));
                    Key(PC,EKeys::LeftShift,IE_Pressed);Acknowledged=true;
                }
                if(OwnerStage==1 && H==96){Test->TestTrue(TEXT("Owner saw80cm landing before next stage"),Acknowledged);Acknowledged=false;OwnerStage=2;}
            }
            if(OwnerStage==3 && H==95 && !OwnerComplete)
            {
                if(!Directory.IsEmpty())
                {
                    if(P->GetActorLocation().X>0){return false;}
                    if(OverviewAt==0){OverviewAt=Now;PC->SetControlRotation(FRotator(0,0,0));}
                    if(Now-OverviewAt<.5){return false;}
                    if(!OverviewCaptured){Capture(TEXT("course_sign"));OverviewCaptured=true;OverviewAt=Now;return false;}
                    if(Now-OverviewAt<.3){return false;} // Screenshot request is captured next frame.
                }
                Test->TestTrue(TEXT("Owner saw second wall landing before completion"),Acknowledged);Key(PC,EKeys::LeftShift,IE_Released);Key(PC,EKeys::W,IE_Released);Key(PC,EKeys::SpaceBar,IE_Released);Key(PC,EKeys::F,IE_Released);
                Test->TestTrue(TEXT("Authored traversal never grants inventory"),PC->GetInventory()->GetStacks().IsEmpty());
                if(Client){FString Reason;Test->TestFalse(TEXT("Client direct automatic mantle refused"),P->TryAutomaticClimb(Reason));}
                Test->AddInfo(TEXT("[PrimalTraversal] Authored owner both real map ledges, server acknowledgements and no item grants verified"));
                if(!Directory.IsEmpty())
                {
                    // Reproduce the reported ordinary attempt at the saved 4m landmark.
                    M->ClearTraversal();M->StopMovementImmediately();M->ClearAccumulatedForces();
                    P->TeleportTo(FVector(1500,4685,100),FRotator(0,90,0),false,true);M->SetMovementMode(MOVE_Walking);PC->SetControlRotation(FRotator(0,90,0));
                    P->Survival->ChangeStamina(15-P->Survival->GetVitals().Stamina);TallStamina=15;TallAt=Now;TallPlaced=true;OwnerStage=4;return false;
                }
                OwnerComplete=true;if(Client){PC->SetControlRotation(FRotator(0,123,0));}return false;
            }
            // Keep each client connected until every server lane completed.
            if(Client && OwnerComplete && H==94){Done=Now;}
        }
        return false;
    }
private:
    double Top(int32 Lane,int32 Kind) const {const auto* A=Ledges[Lane][Kind].Get();return A->GetActorLocation().Z+50*A->GetActorScale3D().Z;}
    bool Landed(APFSurvivorCharacter* P,int32 Lane,int32 Kind) const
    {
        const auto* A=Ledges[Lane][Kind].Get();const FVector At=P->GetActorLocation();
        const float Height=Top(Lane,Kind),Half=P->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
        return P->Traversal()->IsMovingOnGround() && At.X>A->GetActorLocation().X-150 && At.X<A->GetActorLocation().X+150 && FMath::Abs(At.Y-A->GetActorLocation().Y)<160 && FMath::Abs(At.Z-Half-Height)<5;
    }
    void Place(APFSurvivalPlayerController* PC,FAuthoredClimbRecord& R,int32 Kind)
    {
        auto* P=CastChecked<APFSurvivorCharacter>(PC->GetPawn());auto* M=P->Traversal();const auto* A=Ledges[R.Lane][Kind].Get();
        M->ClearTraversal();M->StopMovementImmediately();M->ClearAccumulatedForces();
        P->TeleportTo(FVector(A->GetActorLocation().X-150-P->GetCapsuleComponent()->GetScaledCapsuleRadius()-200,A->GetActorLocation().Y,100),FRotator::ZeroRotator,false,true);
        M->SetMovementMode(MOVE_Walking);PC->SetControlRotation(FRotator::ZeroRotator);
    }
    void Key(APFSurvivalPlayerController* PC,FKey K,EInputEvent Event)
    {
        if(!PC->PlayerInput){Test->AddError(TEXT("Missing real player input"));return;}
        PC->InputKey(FInputKeyEventArgs(nullptr,IPlatformInputDeviceMapper::Get().GetPrimaryInputDeviceForUser(PC->GetPlatformUserId()),K,Event,FPlatformTime::Cycles64()));
    }
    void Capture(const TCHAR* Name){if(!Directory.IsEmpty()){FScreenshotRequest::RequestScreenshot(Directory/(FString(Name)+TEXT(".png")),true,false);}}
    FAutomationTestBase* Test;FString Directory,Diagnostic;TWeakObjectPtr<AStaticMeshActor> Ledges[2][2];TMap<TWeakObjectPtr<APFSurvivalPlayerController>,FAuthoredClimbRecord> Records;
    double Started=FPlatformTime::Seconds(),Done=0,Prepare=0,ReleaseAt=0,OverviewAt=0,SpawnViewAt=0,TallAt=0,ReadyAt=0;float TallStamina=0;int32 Expected=1,OwnerStage=0;bool Client=false,Validated=false,Initialized=false,Ready=false,Acknowledged=false,OwnerComplete=false,ApproachShot=false,Manual=false,Approaching=false,TallPlaced=false,TallPressed=false,FullHeight=false,OverviewCaptured=false;
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFAuthoredClimbLiveTest,"PF.Traversal.AuthoredLive",EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)
bool FPFAuthoredClimbLiveTest::RunTest(const FString&)
{
    if(!FParse::Param(FCommandLine::Get(),TEXT("PFRunAuthoredClimbLiveTests"))){AddError(TEXT("Requires isolated authored-climb runner"));return false;}
    FString Directory,Label;if(!FParse::Param(FCommandLine::Get(),TEXT("nullrhi")))
    {
        FParse::Value(FCommandLine::Get(),TEXT("PFControlsEvidence="),Label);if(Label.IsEmpty() || Label.Len()>48){AddError(TEXT("Invalid evidence label"));return false;}
        for(TCHAR C:Label){if(!FChar::IsAlnum(C) && C!=TEXT('_')){AddError(TEXT("Invalid evidence label"));return false;}}
        Directory=FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()/TEXT("AutomationReports/ControlsUI")/Label);
        if(IFileManager::Get().DirectoryExists(*Directory)){AddError(TEXT("Refusing reused screenshot directory"));return false;}IFileManager::Get().MakeDirectory(*Directory,true);
    }
    ADD_LATENT_AUTOMATION_COMMAND(FAuthoredClimbExercise(this,Directory));return true;
}
#endif
