#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Survival/PFSurvivorCharacter.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Survival/PFTraversalMovement.h"
#include "Inventory/PFInventoryComponent.h"
#include "Settings/PFGameUserSettings.h"
#include "GameFramework/PlayerState.h"
#include "Components/InputComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
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

namespace
{
struct FTraversalServerRecord {int32 Phase=0;double Changed=0,Landed=0;float StartStamina=100;float Walk=600;};
class FTraversalLiveExercise : public IAutomationLatentCommand
{
public:
    FTraversalLiveExercise(FAutomationTestBase* T,FString D,bool A=false):Test(T),Directory(MoveTemp(D)),Automatic(A)
    {FParse::Value(FCommandLine::Get(),TEXT("PFExpectedPlayers="),Expected);}
    bool Update() override
    {
        const double Now=FPlatformTime::Seconds();
        if(Expected>1)
        {
            for(const auto& Context:GEngine->GetWorldContexts())
            {
                auto* World=Context.World();if(!World || World->GetNetMode()!=NM_Client){continue;}
                for(TActorIterator<APFSurvivorCharacter> It(World);It;++It)
                {
                    if(It->GetLocalRole()==ROLE_SimulatedProxy && It->IsRemoteGliding() && It->Traversal()->IsFalling())
                    {
                        if(!SawRemoteGlide){Test->TestTrue(TEXT("Remote proxy uses server-confirmed glide gravity"),FMath::IsNearlyEqual(It->Traversal()->GetGravityZ(),World->GetGravityZ()*It->Traversal()->GravityScale*.15f,1.f));}
                        SawRemoteGlide=true;
                    }
                }
            }
        }
        if(Done)
        {const bool Finished=Now-Done>(ClientOnly?8:15);if(Finished && ClientOnly && Expected>1){Test->TestTrue(TEXT("Two-client peer glide state actually replicated"),SawRemoteGlide);}return Finished;}
        if(Now-Started>120){for(const auto& Pair:Records){if(Pair.Key.IsValid()){Test->AddInfo(FString::Printf(TEXT("[PrimalTraversal] Server phase=%d at %s"),Pair.Value.Phase,*Pair.Key->GetPawn()->GetActorLocation().ToString()));}}Test->AddError(FString::Printf(TEXT("Traversal live timeout clientStage=%d"),Stage));return true;}
        UWorld* W=nullptr;for(const auto& C:GEngine->GetWorldContexts()){if(C.World() && C.World()->IsGameWorld()){W=C.World();break;}}if(!W){return false;}
        ClientOnly=W->GetNetMode()==NM_Client;
        TArray<APFSurvivalPlayerController*> Players;
        for(auto It=W->GetPlayerControllerIterator();It;++It){auto* PC=Cast<APFSurvivalPlayerController>(It->Get());if(PC && Cast<APFSurvivorCharacter>(PC->GetPawn()) && PC->GetInventory() && PC->GetInventory()->Catalog){Players.Add(PC);}}
        if(Players.Num()!=(ClientOnly?1:Expected)){return false;}
        if(!ClientOnly)
        {
            if(!Initialized)
            {
                auto* Cube=LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube"));if(!Cube){Test->AddError(TEXT("Missing engine cube"));return true;}
                auto Make=[&](FVector At,FVector Scale)
                {auto* A=W->SpawnActor<AStaticMeshActor>();A->SetReplicates(true);A->SetReplicateMovement(true);auto* C=A->GetStaticMeshComponent();C->SetMobility(EComponentMobility::Movable);C->SetIsReplicated(true);C->SetStaticMesh(Cube);C->SetCollisionProfileName(TEXT("BlockAll"));A->SetActorScale3D(Scale);A->SetActorLocation(At);A->ForceNetUpdate();};
                Make(FVector(-1000,0,2975),FVector(160,40,.5));
                for(int32 N=0;N<Players.Num();++N)
                {
                    auto* PC=Players[N];auto* P=CastChecked<APFSurvivorCharacter>(PC->GetPawn());auto* I=PC->GetInventory();
                    if(!I->GetStacks().IsEmpty()){Test->AddError(TEXT("Requires disposable fresh empty bag"));return true;}
                    P->Survival->HungerDrainPerSecond=0;P->Survival->ThirstDrainPerSecond=0;P->Survival->SetExposure(0);
                    Make(FVector(-280,N*600,3040),FVector(3,3.6,.8));
                    FTraversalServerRecord R;R.Phase=-1;R.Changed=Now;R.StartStamina=P->Survival->GetVitals().Stamina;R.Walk=P->Traversal()->MaxWalkSpeed;Records.Add(PC,R);
                }
                Initialized=true;
            }
            bool All=true;
            for(auto* PC:Players)
            {
                auto& R=Records.FindChecked(PC);auto* P=CastChecked<APFSurvivorCharacter>(PC->GetPawn());auto* M=P->Traversal();auto* I=PC->GetInventory();
                const double Age=Now-R.Changed;
                if(R.Phase==-1 && M->bWantsSprint)
                {
                    // Existing saved-move intention acknowledges that this owner
                    // has received the replicated floor before we place it there.
                    const int32 Lane=Players.IndexOfByKey(PC);
                    P->TeleportTo(FVector(-1000,Lane*600,3100),FRotator::ZeroRotator,false,true);M->StopMovementImmediately();M->SetMovementMode(MOVE_Walking);PC->SetControlRotation(FRotator::ZeroRotator);Next(R,0,Now);
                }
                else if(R.Phase==0 && M->Velocity.Size2D()>R.Walk*1.25f && P->Survival->GetVitals().Stamina<R.StartStamina-3)
                {Test->TestTrue(TEXT("Server observed real predicted sprint and stamina loss"),M->IsSprinting());I->Grant(TEXT("Item_Wood"),61);P->Survival->SetHealth(99);Next(R,1,Now);}
                else if(R.Phase==1 && Age>1 && M->Velocity.Size2D()<=R.Walk*1.02f)
                {Test->TestFalse(TEXT("Server refuses encoded sprint intent overweight"),M->IsSprinting());Test->TestTrue(TEXT("Server slows excessive carried weight"),M->GetMaxSpeed()<R.Walk);I->Grant(TEXT("Item_Wood"),5);P->Survival->SetHealth(98);Next(R,2,Now);}
                else if(R.Phase==2 && Age>1.3)
                {Test->TestTrue(TEXT("Server overweight jump request remains grounded"),M->IsMovingOnGround());Test->TestFalse(TEXT("Server110percent disables jump"),P->CanJump());I->RemoveItem(TEXT("Item_Wood"),66);M->ClearTraversal();M->StopMovementImmediately();const float AtX=Automatic?-435-P->GetCapsuleComponent()->GetScaledCapsuleRadius():-500;P->TeleportTo(FVector(AtX,P->GetActorLocation().Y,3100),FRotator::ZeroRotator,false,true);M->SetMovementMode(MOVE_Walking);PC->SetControlRotation(FRotator::ZeroRotator);P->Survival->SetHealth(97);R.StartStamina=P->Survival->GetVitals().Stamina;Next(R,3,Now);}
                else if(R.Phase==3 && P->GetActorLocation().Z>3170 && P->GetActorLocation().X>-430 && M->IsMovingOnGround())
                {
                    if(R.Landed==0){R.Landed=Now;Test->TestTrue(TEXT("Server physically climbed with authoritative stamina cost"),P->Survival->GetVitals().Stamina<R.StartStamina-10);if(Automatic){Test->TestTrue(TEXT("Automatic contact spends jump plus climb, not repeated climb"),P->Survival->GetVitals().Stamina<=R.StartStamina-P->JumpStaminaCost-P->ClimbStaminaCost+1);}}
                    if(Now-R.Landed>1){M->ClearTraversal();M->StopMovementImmediately();P->TeleportTo(FVector(-800,P->GetActorLocation().Y,4300),FRotator::ZeroRotator,false,true);M->SetMovementMode(MOVE_Falling);M->Velocity=FVector(0,0,-600);R.StartStamina=P->Survival->GetVitals().Stamina;P->Survival->SetHealth(96);Next(R,4,Now);}
                }
                else if(R.Phase==4 && M->IsGliding() && M->Velocity.Z>=-M->GlideDescentSpeed-2 && P->Survival->GetVitals().Stamina<R.StartStamina-3)
                {Test->TestTrue(TEXT("Server limits real glide descent"),M->IsFalling());P->Survival->ChangeStamina(-100);P->Survival->SetHealth(95);Next(R,5,Now);}
                else if(R.Phase==5 && M->IsMovingOnGround())
                {Test->TestFalse(TEXT("Landing stops server glide"),M->IsGliding());P->Survival->ChangeStamina(100);P->Survival->SetHealth(94);Next(R,6,Now);}
                All &= R.Phase==6;
            }
            if(All && (W->GetNetMode()!=NM_Standalone || ClientComplete))
            {Test->AddInfo(TEXT("[PrimalTraversal] Server predicted movement, overweight sprint/jump refusals, real ledge climb and glide/landing verified"));if(Automatic){Test->AddInfo(TEXT("[PrimalTraversal] Server automatic real jump/wall contact landing verified"));}Done=Now;return false;}
        }
        if(ClientOnly || W->GetNetMode()==NM_Standalone)
        {
            auto* PC=Players[0];auto* P=CastChecked<APFSurvivorCharacter>(PC->GetPawn());auto* M=P->Traversal();auto* I=PC->GetInventory();const float Health=P->Survival->GetVitals().Health;
            if(Stage==0 && !FloorAcknowledged)
            {
                for(TActorIterator<AStaticMeshActor> It(W);It;++It)
                {
                    if(FMath::IsNearlyEqual(It->GetActorLocation().Z,2975.f,1.f) && It->GetStaticMeshComponent()->GetStaticMesh() && It->GetActorScale3D().X>100)
                    {Press(PC,EKeys::LeftShift);FloorAcknowledged=true;break;}
                }
            }
            if(Stage==0 && Health==100 && P->GetActorLocation().Z>3000 && M->IsMovingOnGround())
            {if(ClientOnly){GEngine->SetMaxFPS(120);}if(!Directory.IsEmpty()){float Scale=1;FParse::Value(FCommandLine::Get(),TEXT("PFHUDScale="),Scale);UPFGameUserSettings::Get()->Preferences.HUDScale=FMath::Clamp(Scale,.75f,1.5f);}Walk=M->MaxWalkSpeed;StartStamina=P->Survival->GetVitals().Stamina;Press(PC,EKeys::LeftShift);Stage=1;}
            if(Stage==1 || Stage==2){P->AddMovementInput(FVector(1,0,0));}
            if(Stage==1 && Health==99 && I->Count(TEXT("Item_Wood"))==61)
            {Test->TestFalse(TEXT("Owner immediately cannot sprint overweight"),M->IsSprinting());Test->TestTrue(TEXT("Owner speed slows from actual replicated weight"),M->GetMaxSpeed()<Walk);Test->TestTrue(TEXT("Owner sees server stamina loss"),P->Survival->GetVitals().Stamina<StartStamina);Stage=2;}
            if(Stage==2 && Health==99){CaptureOnce(TEXT("overweight"));}
            else if(Stage==2 && Health==98 && I->Count(TEXT("Item_Wood"))==66)
            {Test->TestFalse(TEXT("Owner cannot jump at110percent"),P->CanJump());P->Jump();JumpRelease=Now+.15;Release(PC,EKeys::LeftShift);Stage=3;}
            if(Stage==3 && Now>=JumpRelease){P->StopJumping();}
            if(Stage==3 && Health==97 && I->GetStacks().IsEmpty() && M->IsMovingOnGround() && P->GetActorLocation().X<-450)
            {P->StopJumping();PC->SetControlRotation(FRotator::ZeroRotator);if(ClimbRequest==0){ClimbRequest=Now+.35;}if(Now>=ClimbRequest){FString Reason;if(ClientOnly){Test->TestFalse(TEXT("Client direct climb authority refused"),P->TryClimb(Reason));}if(Automatic){P->Jump();JumpRelease=Now+.15;}else{Press(PC,EKeys::F);}Stage=4;}}
            else if(Stage==4)
            {
                if(Automatic && !SawClimb){P->AddMovementInput(FVector(1,0,0));if(Now>=JumpRelease){P->StopJumping();}}
                if(Health==97 && P->GetActorLocation().Z>3170 && M->IsMovingOnGround()){SawClimb=true;PC->SetControlRotation(FRotator(-20,0,0));CaptureOnce(TEXT("ledge_top"));}
                if(Health==96 && M->CanGlide()){Test->TestTrue(TEXT("Owner saw physical climb landing before falling fixture"),SawClimb);Press(PC,EKeys::V);Stage=5;}
            }
            else if(Stage==5)
            {
                if(M->IsGliding() && M->Velocity.Z>=-M->GlideDescentSpeed-2){SawGlide=true;CaptureOnce(TEXT("gliding"));}
                if(Health==95){Test->TestTrue(TEXT("Owner observed capped glide"),SawGlide);Test->TestFalse(TEXT("Server stamina depletion disables glide"),M->CanGlide());Press(PC,EKeys::V);Stage=6;}
            }
            if(!ClientComplete && Stage==6 && Health==94 && M->IsMovingOnGround())
            {
                Test->TestFalse(TEXT("Landed owner clears glide"),M->bWantsGlide);Test->TestTrue(TEXT("Traversing never creates items"),I->GetStacks().IsEmpty());
                if(ClientOnly){Test->TestFalse(TEXT("Client cannot grant a heavy item"),I->Grant(TEXT("Item_Stone"),999000));}
                Test->AddInfo(TEXT("[PrimalTraversal] Client saved-move sprint, replicated weight/stamina, jump refusal, acknowledged ledge climb and glide landing verified"));
                if(Automatic){Test->AddInfo(TEXT("[PrimalTraversal] Owner automatic jump/contact/acknowledged landing verified without F"));}
                ClientComplete=true;if(ClientOnly){Done=Now;}return false;
            }
        }
        return false;
    }
private:
    void Next(FTraversalServerRecord& R,int32 Phase,double Now){R.Phase=Phase;R.Changed=Now;}
    void Press(APFSurvivalPlayerController* PC,FKey K){Key(PC,K,IE_Pressed);}
    void Release(APFSurvivalPlayerController* PC,FKey K){Key(PC,K,IE_Released);}
    void Key(APFSurvivalPlayerController* PC,FKey K,EInputEvent Event)
    {if(PC->InputComponent){for(const auto& B:PC->InputComponent->KeyBindings){if(B.Chord.Key==K && B.KeyEvent==Event){B.KeyDelegate.Execute(K);return;}}}Test->AddError(TEXT("Missing actual traversal binding ")+K.ToString());}
    void Capture(const TCHAR* Name){if(!Directory.IsEmpty()){FScreenshotRequest::RequestScreenshot(Directory/(FString(Name)+TEXT(".png")),true,false);}}
    void CaptureOnce(const TCHAR* Name)
    {if(Directory.IsEmpty() || Shots.Contains(Name)){return;}const double Now=FPlatformTime::Seconds();auto* At=ShotTimes.Find(Name);if(!At){ShotTimes.Add(Name,Now);return;}if(Now-*At>=.2){Shots.Add(Name);Capture(Name);}}
    FAutomationTestBase* Test;FString Directory;TSet<FString> Shots;TMap<FString,double> ShotTimes;TMap<TWeakObjectPtr<APFSurvivalPlayerController>,FTraversalServerRecord> Records;
    double Started=FPlatformTime::Seconds(),Done=0,JumpRelease=0,ClimbRequest=0;int32 Stage=0,Expected=1;bool ClientOnly=false,Initialized=false,ClientComplete=false,SawClimb=false,SawGlide=false,SawRemoteGlide=false,Automatic=false,FloorAcknowledged=false;float Walk=600,StartStamina=100;
};
}
static bool QueueTraversalFixture(FAutomationTestBase& Test,bool Automatic)
{
    if(!FParse::Param(FCommandLine::Get(),Automatic?TEXT("PFRunAutomaticClimbLiveTests"):TEXT("PFRunTraversalLiveTests"))){Test.AddError(TEXT("Requires isolated opt-in traversal runner"));return false;}
    FString Directory,Label;if(!FParse::Param(FCommandLine::Get(),TEXT("nullrhi")))
    {
        FParse::Value(FCommandLine::Get(),TEXT("PFControlsEvidence="),Label);if(Label.IsEmpty() || Label.Len()>48){Test.AddError(TEXT("Invalid traversal evidence label"));return false;}
        for(TCHAR C:Label){if(!FChar::IsAlnum(C) && C!=TEXT('_')){Test.AddError(TEXT("Invalid evidence label"));return false;}}
        Directory=FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()/TEXT("AutomationReports/ControlsUI")/Label);
        if(IFileManager::Get().DirectoryExists(*Directory)){Test.AddError(TEXT("Refusing reused evidence directory"));return false;}IFileManager::Get().MakeDirectory(*Directory,true);
    }
    ADD_LATENT_AUTOMATION_COMMAND(FTraversalLiveExercise(&Test,Directory,Automatic));return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFTraversalLiveTest,"PF.Traversal.Live",EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)
bool FPFTraversalLiveTest::RunTest(const FString&){return QueueTraversalFixture(*this,false);}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFAutomaticClimbLiveTest,"PF.Traversal.AutomaticLive",EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)
bool FPFAutomaticClimbLiveTest::RunTest(const FString&){return QueueTraversalFixture(*this,true);}
#endif
