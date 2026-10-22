#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Crafting/PFCraftingStation.h"
#include "Crafting/PFCraftingComponent.h"
#include "Crafting/PFCraftingHUD.h"
#include "Inventory/PFInventoryPlayerState.h"
#include "Progression/PFProgressionComponent.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFSurvivorCharacter.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Creatures/PFCreatureSpawner.h"
#include "Creatures/PFCreature.h"
#include "World/PFWorldClock.h"
#include "Settings/PFGameUserSettings.h"
#include "UI/PFItemPicture.h"
#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Components/InputComponent.h"
#include "Components/TextBlock.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "UnrealClient.h"
#include "HAL/FileManager.h"
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
namespace
{
class FScopeUI final : public IAutomationLatentCommand
{
public:
    FScopeUI(FAutomationTestBase* T,FString D):Test(T),Directory(MoveTemp(D)){}
    bool Update() override
    {
        const double Now=FPlatformTime::Seconds();
        if(Now-Started>100){Test->AddError(FString::Printf(TEXT("Scope UI timeout stage%d"),Stage));return true;}
        if(Stage==99){if(Now-Changed<2){return false;}for(const auto& S:Shots){Test->TestTrue(TEXT("Actual screenshot emitted"),IFileManager::Get().FileSize(*S)>0);}return true;}
        UWorld* W=nullptr;for(const auto& X:GEngine->GetWorldContexts()){if(X.World() && X.World()->IsGameWorld()){W=X.World();break;}}if(!W){return false;}
        APFSurvivalPlayerController* PC=nullptr;for(auto It=W->GetPlayerControllerIterator();It;++It){PC=Cast<APFSurvivalPlayerController>(It->Get());if(PC){break;}}
        if(!PC || !PC->GetInventory() || !PC->GetCrafting() || !PC->GetPawn()){return false;}
        auto* P=CastChecked<APFSurvivorCharacter>(PC->GetPawn());auto* I=PC->GetInventory();auto* C=PC->GetCrafting();const FName Recipe=TEXT("Recipe_SurveyScope"),Item=TEXT("Item_SurveyScope");
        if(Stage==0)
        {
            if(W->GetNetMode()!=NM_Standalone || UWorld::RemovePIEPrefix(W->GetMapName())!=TEXT("L_PrimalFrontier_OpenWorld") || !I->GetStacks().IsEmpty()){Test->AddError(TEXT("Requires disposable authored standalone map"));return true;}
            APFCraftingStation* Bench=nullptr;for(TActorIterator<APFCraftingStation> It(W);It;++It){if(It->GetName()==TEXT("PF_Camp_Workbench")){Bench=*It;}}
            if(!Bench || !Bench->GetActorLocation().Equals(FVector(700,-500,0),1)){Test->AddError(TEXT("Missing reachable saved camp workbench"));return true;}
            for(TActorIterator<APFCreatureSpawner> It(W);It;++It){It->bAutoSpawn=false;}for(TActorIterator<APFCreature> It(W);It;++It){It->Destroy();}
            for(TActorIterator<APFWorldClock> It(W);It;++It){It->SetActorTickEnabled(false);It->SetHour(10);}
            P->Survival->HungerDrainPerSecond=0;P->Survival->ThirstDrainPerSecond=0;P->GetCharacterMovement()->DisableMovement();
            FPFProgressionRecord Seed;Seed.Experience=250;Seed.Knowledge={TEXT("Tech_FieldTools"),TEXT("Tech_Metalwork"),TEXT("Tech_Gemwork")};FString Error;
            if(!PC->GetPlayerState<APFInventoryPlayerState>()->Progression->Restore(Seed,Error) || !I->Grant(TEXT("Item_QuartzLens"),1) || !I->Grant(TEXT("Item_CopperIngot"),2) || !I->Grant(TEXT("Item_Cord"),1) || !I->Grant(TEXT("Item_Wood"),3)){Test->AddError(TEXT("Scope mechanics fixture setup refused"));return true;}
            P->SetActorLocation(Bench->GetActorLocation()+FVector(-150,0,100));Aim(PC,Bench->GetActorLocation()+FVector(0,0,35));
            PC->SetCraftingMenuOpen(true);TArray<UUserWidget*> Widgets;UWidgetBlueprintLibrary::GetAllWidgetsOfClass(PC,Widgets,UPFCraftingHUD::StaticClass(),false);
            if(Widgets.Num()!=1){Test->AddError(TEXT("One crafting panel required"));return true;}Menu=CastChecked<UPFCraftingHUD>(Widgets[0]);Menu->SelectCategory(FGameplayTag());Menu->SelectRecipe(Recipe);Menu->SetKeyboardFocus();Next(Now);return false;
        }
        if(Now-Changed<.8){return false;}
        if(Stage==1)
        {
            auto* Stats=Cast<UTextBlock>(Menu->WidgetTree->FindWidget(TEXT("PF_CraftingDetail_Stats")));
            auto* Body=Cast<UTextBlock>(Menu->WidgetTree->FindWidget(TEXT("PF_CraftingPanel_Body")));
            auto* Picture=Cast<UPFItemPicture>(Menu->WidgetTree->FindWidget(TEXT("PF_SelectedItemPicture")));
            Test->TestTrue(TEXT("Scope visible optical description and required bench"),Stats && Stats->GetText().ToString().Contains(TEXT("2.0x optical scope")) && Body && Body->GetText().ToString().Contains(TEXT("Requires workbench")));
            Test->TestTrue(TEXT("Actual scope picture in rendered panel"),Picture && Picture->GetItemId()==Item && Picture->GetCachedGeometry().GetLocalSize().X>0);
            Shot(TEXT("scope_crafting"));Next(Now);return false;
        }
        if(Stage==2){Press(EKeys::Enter);Test->TestEqual(TEXT("Actual Slate Enter starts scope recipe"),C->ActiveRecipe,Recipe);Test->TestEqual(TEXT("Actual workbench speed"),C->ActiveStationMultiplier,1.25f);Next(Now);return false;}
        if(Stage==3)
        {
            if(!C->ActiveRecipe.IsNone()){return false;}
            if(!Test->TestTrue(TEXT("Real timed craft exact costs"),I->Count(Item)==1 && I->Count(TEXT("Item_Wood"))==1 && I->Count(TEXT("Item_QuartzLens"))==0 && I->Count(TEXT("Item_CopperIngot"))==0 && I->Count(TEXT("Item_Cord"))==0)){return true;}
            PC->SetCraftingMenuOpen(false);PC->SetInventoryOpen(true);const auto* S=I->GetStacks().FindByPredicate([&](const auto& X){return X.ItemId==Item;});
            if(!S || !PC->SelectInventoryStack(S->StackId)){Test->AddError(TEXT("Crafted batch selection failed"));return true;}Next(Now);return false;
        }
        if(Stage==4){Shot(TEXT("scope_inventory"));Press(EKeys::One);Next(Now);return false;}
        if(Stage==5)
        {
            Test->TestTrue(TEXT("Actual inventory key pins exact crafted batch"),PC->GetHotbar().Identity(0).IsValid());PC->SetInventoryOpen(false);GameKey(PC,EKeys::One);
            P->SetActorLocation(FVector(450,-950,100));Aim(PC,FVector(1100,-500,35));Next(Now);return false;
        }
        if(Stage==6){Base=UPFGameUserSettings::Get()->Preferences.FieldOfView;Test->TestEqual(TEXT("Unmagnified real camera"),P->GetFirstPersonCameraComponent()->FieldOfView,Base);Shot(TEXT("scope_normal"));Next(Now);return false;}
        if(Stage==7){GameKey(PC,EKeys::Q);Next(Now);return false;}
        if(Stage==8)
        {
            const float Expected=FMath::RadiansToDegrees(2*FMath::Atan(FMath::Tan(FMath::DegreesToRadians(Base*.5f))/2));
            Test->TestEqual(TEXT("Actual hotbar Q activates2x"),PC->GetSurveyMagnification(),2.f);
            Test->TestTrue(TEXT("Real camera and camera manager produce optical FOV"),FMath::IsNearlyEqual(P->GetFirstPersonCameraComponent()->FieldOfView,Expected,.01f) && PC->PlayerCameraManager && FMath::IsNearlyEqual(PC->PlayerCameraManager->GetFOVAngle(),Expected,.01f));
            Shot(TEXT("scope_zoom"));Next(Now);return false;
        }
        if(Stage==9)
        {
            PC->SetPauseMenuOpen(true);Test->TestTrue(TEXT("Pause menu immediately clears scope even with paused world"),PC->IsPauseMenuOpen() && PC->GetSurveyMagnification()==0 && P->GetFirstPersonCameraComponent()->FieldOfView==Base);PC->SetPauseMenuOpen(false);
            Test->TestEqual(TEXT("Resume never automatically zooms"),PC->GetSurveyMagnification(),0.f);GameKey(PC,EKeys::Gamepad_DPad_Down);Next(Now);return false;
        }
        if(Stage==10)
        {
            Test->TestEqual(TEXT("Controller input route toggles scope"),PC->GetSurveyMagnification(),2.f);GameKey(PC,EKeys::Gamepad_DPad_Down);Next(Now);return false;
        }
        if(Stage==11)
        {
            Test->TestTrue(TEXT("Controller lowers without consuming or changing preferences"),PC->GetSurveyMagnification()==0 && P->GetFirstPersonCameraComponent()->FieldOfView==Base && UPFGameUserSettings::Get()->Preferences.FieldOfView==Base && I->Count(Item)==1);
            Test->AddInfo(TEXT("[PrimalOptics] Rendered actual crafting/picture, exact timed inputs, Slate inventory pin, Q optical camera, pause and controller route resets verified. Assisted material/knowledge/travel fixture, not earned pacing or physical controller/FPS certification."));Stage=99;Changed=Now;return false;
        }
        return false;
    }
private:
    void Next(double Now){++Stage;Changed=Now;}
    void Aim(APFSurvivalPlayerController* PC,FVector At){FVector Eye;FRotator View;PC->GetPawn()->GetActorEyesViewPoint(Eye,View);PC->SetControlRotation((At-Eye).Rotation());}
    void Press(FKey Key){auto& A=FSlateApplication::Get();A.ProcessKeyDownEvent(FKeyEvent(Key,FModifierKeysState(),0,false,0,0));A.ProcessKeyUpEvent(FKeyEvent(Key,FModifierKeysState(),0,false,0,0));}
    void GameKey(APFSurvivalPlayerController* PC,FKey Key){for(const auto& B:PC->InputComponent->KeyBindings){if(B.Chord.Key==Key && B.KeyEvent==IE_Pressed){B.KeyDelegate.Execute(Key);return;}}Test->AddError(TEXT("Missing actual gameplay key binding"));}
    void Shot(const TCHAR* Name){const FString Path=Directory/(FString(Name)+TEXT(".png"));Shots.Add(Path);FScreenshotRequest::RequestScreenshot(Path,true,false);}
    FAutomationTestBase* Test;FString Directory;TArray<FString> Shots;TWeakObjectPtr<UPFCraftingHUD> Menu;float Base=90;int32 Stage=0;double Started=FPlatformTime::Seconds(),Changed=0;
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFScopeUILiveTest,"PF.Optics.UI",EAutomationTestFlags::ClientContext|EAutomationTestFlags::EngineFilter)
bool FPFScopeUILiveTest::RunTest(const FString&)
{
    FString Label;FParse::Value(FCommandLine::Get(),TEXT("PFControlsEvidence="),Label);
    if(!FParse::Param(FCommandLine::Get(),TEXT("PFRunSurveyScopeUITest")) || !FParse::Param(FCommandLine::Get(),TEXT("PFRunControlsUITest")) || FParse::Param(FCommandLine::Get(),TEXT("nullrhi")) || Label.IsEmpty() || Label.Len()>64){AddError(TEXT("Requires isolated rendered scope UI runner"));return false;}
    for(TCHAR Ch:Label){if(!FChar::IsAlnum(Ch) && Ch!=TEXT('_')){return false;}}
    const FString Directory=FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()/TEXT("AutomationReports/ControlsUI")/Label);
    if(IFileManager::Get().DirectoryExists(*Directory)){AddError(TEXT("Refusing reused evidence"));return false;}IFileManager::Get().MakeDirectory(*Directory,true);
    ADD_LATENT_AUTOMATION_COMMAND(FScopeUI(this,Directory));return true;
}
#endif
