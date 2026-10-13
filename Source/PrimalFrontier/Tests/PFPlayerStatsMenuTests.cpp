#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Progression/PFPlayerStatsMenu.h"
#include "Progression/PFProgressionComponent.h"
#include "Inventory/PFInventoryPlayerState.h"
#include "Inventory/PFInventoryComponent.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFSurvivorCharacter.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Survival/PFTraversalMovement.h"
#include "Settings/PFGameUserSettings.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Components/ScrollBox.h"
#include "Framework/Application/SlateApplication.h"
#include "InputKeyEventArgs.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"
#include "Components/CapsuleComponent.h"
#include "UnrealClient.h"
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"

namespace
{
class FStatsMenuExercise : public IAutomationLatentCommand
{
public:
    FStatsMenuExercise(FAutomationTestBase* T,FString D):Test(T),Directory(MoveTemp(D)){}
    ~FStatsMenuExercise(){if(ChangedScale && UPFGameUserSettings::Get()){UPFGameUserSettings::Get()->Preferences.HUDScale=OldScale;}}
    bool Update() override
    {
        const double Now=FPlatformTime::Seconds();if(Now-Started>75){Test->AddError(FString::Printf(TEXT("Stats menu timeout%d"),Stage));return true;}if(Now<Until){return false;}
        if(Stage==0)
        {
            for(const auto& Context:GEngine->GetWorldContexts()){if(Context.World() && Context.World()->IsGameWorld()){auto* Candidate=Cast<APFSurvivalPlayerController>(Context.World()->GetFirstPlayerController());if(Candidate && Candidate->GetPawn() && Candidate->GetLocalPlayer() && Candidate->GetPlayerSetupStatus()!=EPFPlayerSetupStatus::Unconfirmed){PC=Candidate;break;}}}
            if(!PC.IsValid()){return false;}auto* PS=PC->GetPlayerState<APFInventoryPlayerState>();G=PS?PS->Progression:nullptr;
            if(!PC->HasAuthority() || PC->GetNetMode()!=NM_Standalone || !G.IsValid() || G->GetExperience()!=0 || !PC->GetInventory()->GetStacks().IsEmpty()){Test->AddError(TEXT("Requires fresh standalone stats UI fixture"));return true;}
            auto* V=PC->GetPawn()->FindComponentByClass<UPFPlayerSurvivalComponent>();V->HungerDrainPerSecond=0;V->ThirstDrainPerSecond=0;V->StaminaRecoveryPerSecond=0;
            if(auto* S=UPFGameUserSettings::Get()){OldScale=S->Preferences.HUDScale;float Scale=1;FParse::Value(FCommandLine::Get(),TEXT("PFHUDScale="),Scale);S->Preferences.HUDScale=Scale;ChangedScale=true;}
            Key(EKeys::U);Wait(Now,1);return false;
        }
        if(Stage==1)
        {
            TArray<UUserWidget*> Widgets;UWidgetBlueprintLibrary::GetAllWidgetsOfClass(PC.Get(),Widgets,UPFPlayerStatsMenu::StaticClass(),false);
            if(Widgets.Num()!=1){Test->AddError(TEXT("Real U input did not open stats widget"));return true;}Menu=CastChecked<UPFPlayerStatsMenu>(Widgets[0]);
            Test->TestTrue(TEXT("U opens real modal, movement/look blocked but simulation runs"),PC->IsStatMenuOpen() && Menu->HasKeyboardFocus() && PC->IsMoveInputIgnored() && PC->IsLookInputIgnored() && PC->bShowMouseCursor && !PC->IsPaused());
            for(int32 N=0;N<4;++N){Test->TestFalse(TEXT("Fresh no-points buttons disabled"),Button(N)->GetIsEnabled());}
            Test->TestTrue(TEXT("Separate budgets and honest ancient-tech status visible"),Text(TEXT("PF_StatsBudgets")).Contains(TEXT("STAT POINTS: 0")) && Text(TEXT("PF_StatsBudgets")).Contains(TEXT("Ancient technology: planned")));
            Shot(TEXT("stats_fresh"));Wait(Now,20);return false;
        }
        if(Stage==20){FPFProgressionRecord Seed;Seed.Experience=1000;FString Error;Test->TestTrue(TEXT("Explicit UI boundary seed is not earned pacing"),G->Restore(Seed,Error));Wait(Now,2);return false;}
        if(Stage==2)
        {Menu->RefreshMenu();Shot(TEXT("stats_unspent"));Wait(Now,21);return false;}
        if(Stage==21){Slate(EKeys::Down);Slate(EKeys::Enter);Slate(EKeys::Enter,true);Wait(Now,3);return false;}
        if(Stage==3)
        {
            Test->TestTrue(TEXT("Keyboard+held-repeat spends only1 stamina rank"),G->GetRecord().Stats.Stamina==1 && G->GetAvailableStatPoints()==4);
            Test->TestTrue(TEXT("Capacity changes without refill"),PC->GetPawn()->FindComponentByClass<UPFPlayerSurvivalComponent>()->GetVitals().MaxStamina==120 && PC->GetPawn()->FindComponentByClass<UPFPlayerSurvivalComponent>()->GetVitals().Stamina==100);
            Button(2)->OnClicked.Broadcast();Wait(Now,4);return false;
        }
        if(Stage==4)
        {Test->TestEqual(TEXT("Actual mouse-button delegate increases carry threshold"),PC->GetInventory()->WeightLimit,40.f);Slate(EKeys::Gamepad_DPad_Down);Slate(EKeys::Gamepad_FaceButton_Bottom);Wait(Now,5);return false;}
        if(Stage==5)
        {Test->TestEqual(TEXT("D-pad/A selects fourth crafting rank"),G->GetRecord().Stats.CraftingSpeed,1);Slate(EKeys::Down);Slate(EKeys::Enter);Wait(Now,6);return false;}
        if(Stage==6)
        {Test->TestEqual(TEXT("Wrap to attack upgrades correct stat"),G->GetRecord().Stats.Attack,1);Slate(EKeys::Down);Slate(EKeys::Enter);Wait(Now,7);return false;}
        if(Stage==7)
        {
            Menu->RefreshMenu();Test->TestTrue(TEXT("Exact spent budgets and no knowledge/XP change"),G->GetAvailableStatPoints()==0 && G->GetAvailablePoints()==15 && G->GetExperience()==1000 && G->GetRecord().Knowledge.IsEmpty());
            for(int32 N=0;N<4;++N){Test->TestFalse(TEXT("Spent buttons disabled"),Button(N)->GetIsEnabled());auto* Label=CastChecked<UTextBlock>(Menu->WidgetTree->FindWidget(FName(*FString::Printf(TEXT("PF_Stats%d_Label"),N))));Test->TestTrue(TEXT("Wrapped upgrade text fits button height"),Label->GetDesiredSize().Y<=Label->GetCachedGeometry().GetLocalSize().Y+1);}
            Shot(TEXT("stats_spent"));Slate(EKeys::Gamepad_DPad_Down);Slate(EKeys::Gamepad_DPad_Down);Wait(Now,8);return false;
        }
        if(Stage==8){Shot(TEXT("stats_crafting_scroll"));Wait(Now,14);return false;}
        if(Stage==14){Slate(EKeys::Escape);Wait(Now,9);return false;}
        if(Stage==9)
        {
            Test->TestFalse(TEXT("Esc closes stats, releases balanced input without pause"),PC->IsStatMenuOpen() || PC->IsPauseMenuOpen() || PC->IsMoveInputIgnored() || PC->IsLookInputIgnored() || PC->bShowMouseCursor);
            Key(EKeys::P);Wait(Now,10);return false;
        }
        if(Stage==10)
        {Test->TestTrue(TEXT("Pause still opens"),PC->IsPauseMenuOpen());Shot(TEXT("stats_pause"));Wait(Now,13);return false;}
        if(Stage==13){for(int32 N=0;N<5;++N){Slate(EKeys::Down);}Slate(EKeys::Enter);Wait(Now,11);return false;}
        if(Stage==11)
        {Test->TestTrue(TEXT("Pause Player upgrades entry opens running stat modal"),PC->IsStatMenuOpen() && !PC->IsPauseMenuOpen() && !PC->IsPaused());Slate(EKeys::U);Wait(Now,12);return false;}
        if(Stage==12)
        {
            Test->TestFalse(TEXT("U closes modal and restores input"),PC->IsStatMenuOpen() || PC->IsMoveInputIgnored() || PC->IsLookInputIgnored());
            auto* P=CastChecked<APFSurvivorCharacter>(PC->GetPawn());AStaticMeshActor* Wall=nullptr;
            for(TActorIterator<AStaticMeshActor> It(P->GetWorld());It;++It){if(It->ActorHasTag(TEXT("PF.M14.Climb10_A"))){if(Wall){Test->AddError(TEXT("Duplicate authored10m fixture"));return true;}Wall=*It;}}
            if(!Wall || !Wall->GetActorLocation().Equals(FVector(2450,-1100,500),.1) || !Wall->GetActorScale3D().Equals(FVector(3,3.6,10),.001)){Test->AddError(TEXT("Actual saved10m camp fixture unavailable/different"));return true;}
            P->Survival->StaminaRecoveryPerSecond=10;P->Traversal()->ClearTraversal();P->Traversal()->StopMovementImmediately();P->Traversal()->ClearAccumulatedForces();
            if(!P->TeleportTo(FVector(2066,-1100,100),FRotator::ZeroRotator,false,false)){Test->AddError(TEXT("Safe10m approach reposition refused"));return true;}
            P->Traversal()->SetMovementMode(MOVE_Walking);PC->SetControlRotation(FRotator::ZeroRotator);Wait(Now,24);return false;
        }
        if(Stage==24)
        {auto* P=CastChecked<APFSurvivorCharacter>(PC->GetPawn());if(!P->Traversal()->IsMovingOnGround() || P->Survival->GetVitals().Stamina<139.9f){return false;}Key(EKeys::W);HoldForward();Stage=25;return false;}
        if(Stage==25)
        {
            auto* P=CastChecked<APFSurvivorCharacter>(PC->GetPawn());if(!P->Traversal()->IsMovingOnGround() || P->GetActorLocation().X<2265){return false;}
            Key(EKeys::F);Stage=26;return false;
        }
        if(Stage==26)
        {
            auto* P=CastChecked<APFSurvivorCharacter>(PC->GetPawn());const float Feet=P->GetActorLocation().Z-P->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
            if(!P->Traversal()->IsMovingOnGround() || FMath::Abs(Feet-1000)>5 || P->GetActorLocation().X<2300 || !P->Traversal()->AutomaticClimbDrive.IsNearlyZero()){return false;}
            ReleaseForward();P->Traversal()->StopMovementImmediately();PC->SetControlRotation(FRotator(-55,0,0));
            Test->TestTrue(TEXT("Actual authored10m F landing and visible full-cost feedback"),PC->RecentInteractionMessage().Contains(TEXT("10.0 m")) && PC->RecentInteractionMessage().Contains(TEXT("102.")));
            Test->TestEqual(TEXT("Actual gameplay retains upgraded maximum"),P->Survival->GetVitals().MaxStamina,140.f);
            Shot(TEXT("stats_10m_climb"));Wait(Now,27);return false;
        }
        Test->TestFalse(TEXT("U closes modal and restores input"),PC->IsStatMenuOpen() || PC->IsMoveInputIgnored() || PC->IsLookInputIgnored());
        Test->TestTrue(TEXT("UI allocations never grant items"),PC->GetInventory()->GetStacks().IsEmpty());
        for(const TCHAR* F:{TEXT("stats_fresh"),TEXT("stats_unspent"),TEXT("stats_spent"),TEXT("stats_crafting_scroll"),TEXT("stats_pause"),TEXT("stats_10m_climb")}){Test->TestTrue(TEXT("Fresh screenshot artifact"),IFileManager::Get().FileSize(*(Directory/(FString(F)+TEXT(".png"))))>0);}
        Test->AddInfo(TEXT("[PrimalAgentTools] Stats U menu, disabled/earned allocations, real buttons, keyboard/gamepad routing, separate budgets, capacity, scroll, Pause entry, balanced Back and actual saved10m F landing verified"));return true;
    }
private:
    UButton* Button(int32 N){return CastChecked<UButton>(Menu->WidgetTree->FindWidget(FName(*FString::Printf(TEXT("PF_Stats%d_Button"),N))));}
    FString Text(FName Id){return CastChecked<UTextBlock>(Menu->WidgetTree->FindWidget(Id))->GetText().ToString();}
    void Key(FKey K){const auto Device=IPlatformInputDeviceMapper::Get().GetPrimaryInputDeviceForUser(PC->GetPlatformUserId());PC->InputKey(FInputKeyEventArgs(nullptr,Device,K,IE_Pressed,FPlatformTime::Cycles64()));PC->InputKey(FInputKeyEventArgs(nullptr,Device,K,IE_Released,FPlatformTime::Cycles64()));}
    void HoldForward(){const auto Device=IPlatformInputDeviceMapper::Get().GetPrimaryInputDeviceForUser(PC->GetPlatformUserId());PC->InputKey(FInputKeyEventArgs(nullptr,Device,EKeys::W,IE_Pressed,FPlatformTime::Cycles64()));}
    void ReleaseForward(){const auto Device=IPlatformInputDeviceMapper::Get().GetPrimaryInputDeviceForUser(PC->GetPlatformUserId());PC->InputKey(FInputKeyEventArgs(nullptr,Device,EKeys::W,IE_Released,FPlatformTime::Cycles64()));}
    void Slate(FKey K,bool Repeat=false){auto& A=FSlateApplication::Get();A.ProcessKeyDownEvent(FKeyEvent(K,FModifierKeysState(),0,Repeat,0,0));A.ProcessKeyUpEvent(FKeyEvent(K,FModifierKeysState(),0,false,0,0));}
    void Shot(const TCHAR* Name){FScreenshotRequest::RequestScreenshot(Directory/(FString(Name)+TEXT(".png")),true,false);}
    void Wait(double Now,int32 Next){Until=Now+.6;Stage=Next;}
    FAutomationTestBase* Test;FString Directory;TWeakObjectPtr<APFSurvivalPlayerController> PC;TWeakObjectPtr<UPFProgressionComponent> G;TWeakObjectPtr<UPFPlayerStatsMenu> Menu;
    double Started=FPlatformTime::Seconds(),Until=0;int32 Stage=0;float OldScale=1;bool ChangedScale=false;
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFStatsMenuLiveTest,"PF.UI.PlayerStatsLive",EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)
bool FPFStatsMenuLiveTest::RunTest(const FString&)
{
    FString Run;if(!FParse::Param(FCommandLine::Get(),TEXT("PFRunControlsUITest")) || FParse::Param(FCommandLine::Get(),TEXT("nullrhi")) || !FParse::Value(FCommandLine::Get(),TEXT("PFControlsEvidence="),Run) || !Run.StartsWith(TEXT("M12StatsUI")))
    {AddError(TEXT("Use isolated rendered PlayerStats controls runner"));return false;}
    const FString Dir=FPaths::ProjectSavedDir()/TEXT("AutomationReports/ControlsUI")/Run;IFileManager::Get().MakeDirectory(*Dir,true);ADD_LATENT_AUTOMATION_COMMAND(FStatsMenuExercise(this,Dir));return true;
}
#endif
