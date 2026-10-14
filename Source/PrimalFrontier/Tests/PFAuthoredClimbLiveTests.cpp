#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Survival/PFSurvivorCharacter.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Survival/PFTraversalMovement.h"
#include "Inventory/PFInventoryComponent.h"
#include "Settings/PFGameUserSettings.h"
#include "Creatures/PFCreature.h"
#include "Creatures/PFCreatureSpawner.h"
#include "World/PFWorldClock.h"
#include "Components/CapsuleComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
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
struct FWallRecord {int32 Lane=0;bool Attached=false,Landed=false,Fell=false;float Walk=600;};
class FWallLive : public IAutomationLatentCommand
{
public:
    FWallLive(FAutomationTestBase* T,FString D):Test(T),Directory(MoveTemp(D)){FParse::Value(FCommandLine::Get(),TEXT("PFExpectedPlayers="),Expected);}
    ~FWallLive(){if(Scaled && UPFGameUserSettings::Get()){UPFGameUserSettings::Get()->Preferences.HUDScale=OldScale;}}
    bool Update() override
    {
        const double Now=FPlatformTime::Seconds();if(Now-Started>110){Test->AddError(FString::Printf(TEXT("Continuous wall timeout server%d owner%d %s"),ServerStage,OwnerStage,*Diagnostic));return true;}
        if(Done){if(Now-Done<(Client?8:15)){return false;}for(const auto& S:Shots){Test->TestTrue(TEXT("Wall screenshot written"),IFileManager::Get().FileSize(*S)>0);}return true;}
        UWorld* W=nullptr;for(const auto& X:GEngine->GetWorldContexts()){if(X.World() && X.World()->IsGameWorld()){W=X.World();break;}}if(!W){return false;}Client=W->GetNetMode()==NM_Client;
        if(UWorld::RemovePIEPrefix(W->GetMapName())!=TEXT("L_PrimalFrontier_OpenWorld")){Test->AddError(TEXT("Actual OpenWorld required"));return true;}
        if(!Validated)
        {
            for(TActorIterator<AStaticMeshActor> It(W);It;++It)
            {for(int32 Lane=0;Lane<2;++Lane){const FName Tag=Lane?FName(TEXT("PF.M14.Climb4_B")):FName(TEXT("PF.M14.Climb4_A"));if(It->ActorHasTag(Tag)){Walls[Lane]=*It;}}if(It->ActorHasTag(TEXT("PF.M14.Climb10_A"))){Tall=*It;}}
            if(!Walls[0].IsValid() || !Walls[1].IsValid() || !Tall.IsValid()){return false;}
            if(!Walls[0]->GetActorLocation().Equals({1850,-1100,200},.1) || !Walls[1]->GetActorLocation().Equals({2250,-3000,200},.1) || !Tall->GetActorLocation().Equals({2450,-1100,500},.1)){Test->AddError(TEXT("Saved fixture identities/layout differ"));return true;}
            Validated=true;
        }
        TArray<APFSurvivalPlayerController*> Players;for(auto It=W->GetPlayerControllerIterator();It;++It){auto* PC=Cast<APFSurvivalPlayerController>(It->Get());if(PC && Cast<APFSurvivorCharacter>(PC->GetPawn()) && PC->GetInventory()){Players.Add(PC);}}
        if(Players.Num()!=(Client?1:Expected)){return false;}
        if(!Client)
        {
            if(!Initialized)
            {
                for(TActorIterator<APFWorldClock> It(W);It;++It){It->SetHour(10);It->SetActorTickEnabled(false);}for(TActorIterator<APFCreatureSpawner> It(W);It;++It){It->bAutoSpawn=false;}for(TActorIterator<APFCreature> It(W);It;++It){It->Destroy();}
                int32 Lane=0;for(auto* PC:Players){auto* P=CastChecked<APFSurvivorCharacter>(PC->GetPawn());if(!PC->GetInventory()->GetStacks().IsEmpty()){Test->AddError(TEXT("Fresh isolated bag required"));return true;}P->Survival->HungerDrainPerSecond=0;P->Survival->ThirstDrainPerSecond=0;P->Survival->StaminaRecoveryPerSecond=0;P->Traversal()->SprintStaminaPerSecond=0;P->Traversal()->SprintMultiplier=1;FWallRecord R;R.Lane=Lane++;R.Walk=P->Traversal()->MaxWalkSpeed;Records.Add(PC,R);}Initialized=true;
            }
            if(ServerStage==0)
            {
                bool Ready=true;for(auto* PC:Players){Ready &= CastChecked<APFSurvivorCharacter>(PC->GetPawn())->Traversal()->bWantsSprint;}
                if(Ready){for(auto* PC:Players){Place(PC,Records.FindChecked(PC),false);CastChecked<APFSurvivorCharacter>(PC->GetPawn())->Survival->SetHealth(97);}ServerStage=1;}
            }
            else if(ServerStage==1)
            {
                bool All=true;for(auto* PC:Players)
                {
                    auto& R=Records.FindChecked(PC);auto* P=CastChecked<APFSurvivorCharacter>(PC->GetPawn());auto* M=P->Traversal();R.Attached |= M->IsWallClimbing();
                    if(!R.Landed && R.Attached && M->IsMovingOnGround() && Feet(P)>395 && Feet(P)<405)
                    {Test->TestTrue(TEXT("Server continuous4m landing spent elapsed stamina only"),P->Survival->GetVitals().Stamina>0 && P->Survival->GetVitals().Stamina<99);R.Landed=true;LandedAt=Now;M->MaxWalkSpeed=0;}
                    All &= R.Landed && FMath::Abs(PC->GetControlRotation().Yaw-30)<2;
                    Diagnostic=FString::Printf(TEXT("lane%d at%s mode%d stamina%.2f attached%d landed%d yaw%.1f"),R.Lane,*P->GetActorLocation().ToString(),int32(M->MovementMode),P->Survival->GetVitals().Stamina,R.Attached,R.Landed,PC->GetControlRotation().Yaw);
                }
                if(All && Now-LandedAt>.75){for(auto* PC:Players){Place(PC,Records.FindChecked(PC),true);CastChecked<APFSurvivorCharacter>(PC->GetPawn())->Survival->SetHealth(94);}ServerStage=3;}
            }
            else if(ServerStage==3)
            {
                bool All=true;for(auto* PC:Players)
                {
                    auto& R=Records.FindChecked(PC);auto* P=CastChecked<APFSurvivorCharacter>(PC->GetPawn());auto* M=P->Traversal();R.Attached |= M->IsWallClimbing();
                    if(!R.Fell && R.Attached && M->IsFalling() && P->Survival->GetVitals().Stamina==0)
                    {Test->TestTrue(TEXT("Server24stamina trial falls below10m top after partial climb"),Feet(P)>50 && Feet(P)<450);R.Fell=true;}
                    All &= R.Fell && FMath::Abs(PC->GetControlRotation().Yaw-60)<2;
                    Diagnostic=FString::Printf(TEXT("low lane%d at%s mode%d stamina%.2f attached%d fell%d yaw%.1f"),R.Lane,*P->GetActorLocation().ToString(),int32(M->MovementMode),P->Survival->GetVitals().Stamina,R.Attached,R.Fell,PC->GetControlRotation().Yaw);
                }
                if(All){for(auto* PC:Players){CastChecked<APFSurvivorCharacter>(PC->GetPawn())->Survival->SetHealth(93);}Test->AddInfo(TEXT("[PrimalAgentTools] Continuous wall server actual authored attach, timed drain, clear4m landing and zero-stamina10m fall verified"));ServerStage=9;if(W->GetNetMode()!=NM_Standalone){Done=Now;}}
            }
        }
        if(Client || W->GetNetMode()==NM_Standalone)
        {
            auto* PC=Players[0];auto* P=CastChecked<APFSurvivorCharacter>(PC->GetPawn());auto* M=P->Traversal();const float Health=P->Survival->GetVitals().Health;
            if(!Directory.IsEmpty() && !Scaled){auto* S=UPFGameUserSettings::Get();OldScale=S->Preferences.HUDScale;float Scale=1;FParse::Value(FCommandLine::Get(),TEXT("PFHUDScale="),Scale);S->Preferences.HUDScale=Scale;Scaled=true;}
            if(OwnerStage==0 && Health==100 && PC->PlayerInput && !P->IsTraversalInputBlocked()){Key(PC,EKeys::LeftShift,IE_Pressed);}
            if((OwnerStage==0 && Health==97) || (OwnerStage==2 && Health==94))
            {
                const bool Low=Health==94;const FVector At=P->GetActorLocation();const float Face=Low?2300:(FMath::Abs(At.Y+1100)<200?1700:2100);
                if(!M->IsMovingOnGround() || FMath::Abs(At.X-(Face-P->GetCapsuleComponent()->GetScaledCapsuleRadius()-4))>25 || (Low && FMath::Abs(P->Survival->GetVitals().Stamina-24)>.5)){return false;}
                Key(PC,EKeys::LeftShift,IE_Released);PC->SetControlRotation(FRotator::ZeroRotator);
                if(Client){FString Error;Test->TestFalse(TEXT("Client direct attach cannot authorize climbing"),P->TryClimb(Error));Test->TestFalse(TEXT("Client cannot grant server wall mode"),M->BeginWallClimb({-1,0,0}));}
                Key(PC,EKeys::F,IE_Pressed);Key(PC,EKeys::F,IE_Released);OwnerStage=Low?3:1;HoldingAt=0;UpHeld=false;
            }
            if(OwnerStage==1 || OwnerStage==3)
            {
                if(M->IsWallClimbing())
                {
                    if(!HoldingAt){HoldingAt=Now;}
                    if(!UpHeld && Now-HoldingAt>.6){Shot(OwnerStage==1?TEXT("wall_holding"):TEXT("wall_low_budget"));Key(PC,EKeys::W,IE_Pressed);UpHeld=true;}
                    if(!AscendingShot && OwnerStage==1 && Feet(P)>150 && Now-HoldingAt>1){Test->TestTrue(TEXT("Owning HUD provider confirms continuous climb controls"),PC->InteractionPrompt().Contains(TEXT("W/S")));Shot(TEXT("wall_ascending"));AscendingShot=true;}
                }
                if(Expected==2){for(TActorIterator<APFSurvivorCharacter> It(W);It;++It){if(*It!=P && It->Traversal()->IsWallClimbing() && Feet(*It)>30){RemoteSeen=true;}}}
                if(OwnerStage==1 && UpHeld && M->IsMovingOnGround() && Feet(P)>395 && Feet(P)<405)
                {Key(PC,EKeys::W,IE_Released);Test->TestTrue(TEXT("Owner saw wall state then real4m top"),HoldingAt>0 && P->Survival->GetVitals().Stamina<99);Shot(TEXT("wall_top"));PC->SetControlRotation({0,30,0});OwnerStage=2;}
                if(OwnerStage==3 && HoldingAt>0 && M->IsFalling() && P->Survival->GetVitals().Stamina==0 && Feet(P)>20)
                {Key(PC,EKeys::W,IE_Released);Test->TestTrue(TEXT("Owner sees exact zero and fall while below top"),Feet(P)<450);FallAt=Now;PC->SetControlRotation({0,60,0});OwnerStage=4;}
            }
            if(OwnerStage==4 && FallAt && Now-FallAt>.15 && !FallShot){Test->TestTrue(TEXT("Owner distinguishes exhausted fall from attach refusal"),PC->RecentInteractionMessage().Contains(TEXT("Climb ended: Stamina empty")));Shot(TEXT("wall_exhausted_fall"));FallShot=true;}
            if(OwnerStage==4 && Health==93 && (Directory.IsEmpty() || FallShot))
            {if(Expected==2){Test->TestTrue(TEXT("Remote full-body player exposes climbing movement"),RemoteSeen);}Test->TestTrue(TEXT("No traversal grants items"),PC->GetInventory()->GetStacks().IsEmpty());Test->AddInfo(TEXT("[PrimalAgentTools] Continuous wall owner actual F/W input, private authority refusal, timed stamina, top landing and exhaustion fall replicated"));OwnerStage=9;Done=Now;}
        }
        return false;
    }
private:
    float Feet(APFSurvivorCharacter* P) const {return P->GetActorLocation().Z-P->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();}
    void Place(APFSurvivalPlayerController* PC,FWallRecord& R,bool Low)
    {
        auto* P=CastChecked<APFSurvivorCharacter>(PC->GetPawn());auto* M=P->Traversal();M->ClearTraversal();M->StopMovementImmediately();M->ClearAccumulatedForces();M->MaxWalkSpeed=R.Walk;
        const auto* Wall=Low?Tall.Get():Walls[R.Lane].Get();FVector At=Wall->GetActorLocation();At.X-=150+P->GetCapsuleComponent()->GetScaledCapsuleRadius()+4;At.Z=P->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()+3;if(Low && Expected==2){At.Y+=R.Lane?100:-100;}
        P->SetActorLocation(At);P->SetActorRotation(FRotator::ZeroRotator);PC->SetControlRotation(FRotator::ZeroRotator);PC->ClientSetRotation(FRotator::ZeroRotator,false);M->SetMovementMode(MOVE_Walking);
        P->Survival->ChangeStamina(-P->Survival->GetVitals().Stamina);P->Survival->ChangeStamina(Low?24:100);R.Attached=false;
    }
    void Key(APFSurvivalPlayerController* PC,FKey K,EInputEvent Event){PC->InputKey(FInputKeyEventArgs(nullptr,IPlatformInputDeviceMapper::Get().GetPrimaryInputDeviceForUser(PC->GetPlatformUserId()),K,Event,FPlatformTime::Cycles64()));}
    void Shot(const TCHAR* Name){if(!Directory.IsEmpty()){const FString Path=Directory/(FString(Name)+TEXT(".png"));Shots.Add(Path);FScreenshotRequest::RequestScreenshot(Path,true,false);}}
    FAutomationTestBase* Test;FString Directory,Diagnostic;TArray<FString> Shots;TWeakObjectPtr<AStaticMeshActor> Walls[2],Tall;TMap<TWeakObjectPtr<APFSurvivalPlayerController>,FWallRecord> Records;
    int32 Expected=1,ServerStage=0,OwnerStage=0;bool Client=false,Initialized=false,Validated=false,UpHeld=false,RemoteSeen=false,AscendingShot=false,FallShot=false,Scaled=false;float OldScale=1;
    double Started=FPlatformTime::Seconds(),Done=0,HoldingAt=0,FallAt=0,LandedAt=0;
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFAuthoredClimbLiveTest,"PF.Traversal.AuthoredLive",EAutomationTestFlags::ClientContext|EAutomationTestFlags::ServerContext|EAutomationTestFlags::EngineFilter)
bool FPFAuthoredClimbLiveTest::RunTest(const FString&)
{
    int32 Expected=0;if(!FParse::Param(FCommandLine::Get(),TEXT("PFRunAuthoredClimbLiveTests")) || !FParse::Value(FCommandLine::Get(),TEXT("PFExpectedPlayers="),Expected) || (Expected!=1 && Expected!=2)){AddError(TEXT("Requires isolated authored continuous-wall runner1|2players"));return false;}
    FString Directory,Label;if(!FParse::Param(FCommandLine::Get(),TEXT("nullrhi")))
    {
        FParse::Value(FCommandLine::Get(),TEXT("PFControlsEvidence="),Label);if(!FParse::Param(FCommandLine::Get(),TEXT("PFRunControlsUITest")) || Label.IsEmpty() || Label.Len()>48){return false;}
        for(TCHAR C:Label){if(!FChar::IsAlnum(C) && C!=TEXT('_')){return false;}}Directory=FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()/TEXT("AutomationReports/ControlsUI")/Label);
        if(IFileManager::Get().DirectoryExists(*Directory)){AddError(TEXT("Refusing reused screenshots"));return false;}IFileManager::Get().MakeDirectory(*Directory,true);
    }
    ADD_LATENT_AUTOMATION_COMMAND(FWallLive(this,Directory));return true;
}
#endif
