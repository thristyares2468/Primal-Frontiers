// Registered gameplay keys plus real Slate modal Back paths; no physical-device claim.
#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFPauseMenu.h"
#include "Survival/PFControlsMenu.h"
#include "Building/PFBuildingComponent.h"
#include "Building/PFBuildingMenus.h"
#include "Inventory/PFInventoryComponent.h"
#include "Crafting/PFCraftingComponent.h"
#include "Settings/PFSettingsMenu.h"
#include "Settings/PFGameUserSettings.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/InputComponent.h"
#include "Framework/Application/SlateApplication.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "UnrealClient.h"

namespace
{
class FMenuNavigationExercise final : public IAutomationLatentCommand
{
public:
    FMenuNavigationExercise(FAutomationTestBase* T,FString D):Test(T),Directory(MoveTemp(D)){}
    ~FMenuNavigationExercise(){if(PC.IsValid()){PC->SetPauseMenuOpen(false);}if(bChangedScale){if(auto* S=UPFGameUserSettings::Get()){S->Preferences.HUDScale=OriginalScale;}}}
    bool Update() override
    {
        const double Now=FPlatformTime::Seconds();
        if(Now-Started>70){Test->AddError(TEXT("Menu navigation timeout"));return true;}
        if(Now<Until){return false;}
        if(Stage==0)
        {
            for(const auto& C:GEngine->GetWorldContexts())
            {if(C.World() && C.World()->IsGameWorld()){PC=Cast<APFSurvivalPlayerController>(C.World()->GetFirstPlayerController());if(PC.IsValid() && PC->InputComponent && PC->GetInventory()){break;}}}
            if(!PC.IsValid() || !PC->GetInventory()){return false;}
            if(PC->GetNetMode()!=NM_Standalone || PC->GetWorld()->GetMapName()!=TEXT("L_PrimalFrontier_OpenWorld") || !PC->GetInventory()->GetStacks().IsEmpty())
            {Test->AddError(TEXT("Requires fresh standalone open-world menu fixture"));return true;}
            if(auto* S=UPFGameUserSettings::Get()){OriginalScale=S->Preferences.HUDScale;float Scale=1;FParse::Value(FCommandLine::Get(),TEXT("PFHUDScale="),Scale);S->Preferences.HUDScale=FMath::Clamp(Scale,0.75f,1.5f);bChangedScale=true;}
            Bound(EKeys::B);Wait(Now,1);return false;
        }
        switch(Stage)
        {
        case 1:
            Test->TestTrue(TEXT("First B opens build picker"),PC->IsBuildingMenuOpen());
            Slate(EKeys::One);Wait(Now,2);return false;
        case 2:
            Test->TestTrue(TEXT("Choosing a piece enters placement with input released"),PC->Building->bBuildMode && !PC->IsBuildingMenuOpen() && !PC->IsMoveInputIgnored() && !PC->bShowMouseCursor);
            PC->Building->Rotation=2;Bound(EKeys::B);Wait(Now,3);return false;
        case 3:
            Test->TestTrue(TEXT("Second B returns directly to visible picker"),PC->IsBuildingMenuOpen() && Find<UPFBuildingMenu>() && PC->bShowMouseCursor);
            Test->TestEqual(TEXT("Changing view preserves rotation"),PC->Building->Rotation,2);
            Capture(TEXT("build_picker_return"));Wait(Now,4);return false;
        case 4:
            if(!ShotExists()){return false;}Slate(EKeys::Escape,true);
            Test->TestTrue(TEXT("Held Back does not close picker"),PC->IsBuildingMenuOpen());
            Slate(EKeys::Escape);Wait(Now,5);return false;
        case 5:
            WorldInput(TEXT("Esc exits picker without opening Pause"));Bound(EKeys::Tab);Wait(Now,6);return false;
        case 6:
            Test->TestTrue(TEXT("Bag opened"),PC->IsInventoryOpen());Bound(EKeys::Escape);Wait(Now,7);return false;
        case 7:
            WorldInput(TEXT("Esc exits bag without Pause"));Bound(EKeys::C);Wait(Now,8);return false;
        case 8:
            Test->TestTrue(TEXT("Craft opened"),PC->IsCraftingOpen());Slate(EKeys::Escape);Wait(Now,9);return false;
        case 9:
            WorldInput(TEXT("Esc exits crafting without Pause"));
            Test->TestEqual(TEXT("Closing craft never starts/cancels a job"),PC->GetCrafting()->ActiveRecipe,NAME_None);
            Bound(EKeys::B);Wait(Now,10);return false;
        case 10: Slate(EKeys::One);Wait(Now,11);return false;
        case 11:
            Test->TestTrue(TEXT("Preview active"),PC->Building->bBuildMode);Bound(EKeys::Escape);Wait(Now,12);return false;
        case 12:
            WorldInput(TEXT("Esc exits placement without Pause"));Bound(EKeys::Escape);Wait(Now,13);return false;
        case 13:
            Test->TestTrue(TEXT("Esc with no menu opens real Pause and pauses solo world"),PC->IsPauseMenuOpen() && PC->IsPaused() && Find<UPFPauseMenu>());
            Capture(TEXT("escape_pause"));Wait(Now,14);return false;
        case 14:
            if(!ShotExists()){return false;}Slate(EKeys::Down);Slate(EKeys::Enter);Wait(Now,15);return false;
        case 15:
            Settings=Find<UPFSettingsMenu>();Test->TestTrue(TEXT("Settings child is visible"),Settings.IsValid() && Settings->IsInViewport());
            Slate(EKeys::Escape,true);Test->TestTrue(TEXT("Held Esc does not discard settings"),Settings.IsValid() && Settings->IsInViewport());
            Slate(EKeys::Right);Slate(EKeys::Escape);Wait(Now,16);return false;
        case 16:
            Test->TestTrue(TEXT("Settings Esc discards draft and returns to still-paused parent"),Settings.IsValid() && !Settings->IsInViewport() && PC->IsPauseMenuOpen() && PC->IsPaused() && Find<UPFPauseMenu>() && Find<UPFPauseMenu>()->HasKeyboardFocus());
            Slate(EKeys::Down);Slate(EKeys::Enter);Wait(Now,17);return false;
        case 17:
            Controls=Find<UPFControlsMenu>();Test->TestTrue(TEXT("Help child is visible"),Controls.IsValid() && Controls->IsInViewport());
            Slate(EKeys::Escape);Wait(Now,18);return false;
        case 18:
            Test->TestTrue(TEXT("Help Esc returns one layer to Pause"),Controls.IsValid() && !Controls->IsInViewport() && PC->IsPauseMenuOpen() && PC->IsPaused());
            Capture(TEXT("escape_child_back"));Wait(Now,19);return false;
        case 19:
            if(!ShotExists()){return false;}Slate(EKeys::Escape);Wait(Now,20);return false;
        case 20:
            WorldInput(TEXT("Esc on Pause resumes without ending simulation"));Bound(EKeys::B);Wait(Now,21);return false;
        case 21:
            Slate(EKeys::B);Wait(Now,22);return false;
        case 22:
            WorldInput(TEXT("B on picker closes it"));
            Test->TestTrue(TEXT("Navigation conserves empty inventory"),PC->GetInventory()->GetStacks().IsEmpty());
            Test->AddInfo(TEXT("[PrimalUI] B picker return and Esc back/solo Pause path passed; no settings save, world save or gameplay transaction."));return true;
        }
        return false;
    }
private:
    template<class T> T* Find()
    {TArray<UUserWidget*> Widgets;UWidgetBlueprintLibrary::GetAllWidgetsOfClass(PC.Get(),Widgets,T::StaticClass(),false);for(auto* W:Widgets){if(W->IsInViewport()){return Cast<T>(W);}}return nullptr;}
    void WorldInput(const TCHAR* Label)
    {Test->TestFalse(Label,PC->IsPauseMenuOpen() || PC->IsPaused() || PC->IsInventoryOpen() || PC->IsCraftingOpen() || PC->IsBuildingMenuOpen() || PC->IsStorageMenuOpen() || PC->Building->bBuildMode || PC->IsMoveInputIgnored() || PC->IsLookInputIgnored() || PC->bShowMouseCursor);}
    void Bound(FKey K)
    {for(const auto& B:PC->InputComponent->KeyBindings){if(B.Chord.Key==K && B.KeyEvent==IE_Pressed){B.KeyDelegate.Execute(K);return;}}Test->AddError(TEXT("Missing registered menu key"));}
    void Slate(FKey K,bool Repeat=false)
    {auto& App=FSlateApplication::Get();App.ProcessKeyDownEvent(FKeyEvent(K,FModifierKeysState(),0,Repeat,0,0));App.ProcessKeyUpEvent(FKeyEvent(K,FModifierKeysState(),0,false,0,0));}
    void Wait(double Now,int32 Next){Until=Now+0.35;Stage=Next;}
    void Capture(const TCHAR* Name){Shot=Directory/(FString(Name)+TEXT(".png"));FScreenshotRequest::RequestScreenshot(Shot,true,false);}
    bool ShotExists(){return IFileManager::Get().FileSize(*Shot)>0;}
    FAutomationTestBase* Test;FString Directory,Shot;TWeakObjectPtr<APFSurvivalPlayerController> PC;
    TWeakObjectPtr<UPFSettingsMenu> Settings;TWeakObjectPtr<UPFControlsMenu> Controls;
    double Started=FPlatformTime::Seconds(),Until=0;int32 Stage=0;
    float OriginalScale=1;bool bChangedScale=false;
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFMenuNavigationLiveTest,"PF.UI.MenuNavigationLive",EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)
bool FPFMenuNavigationLiveTest::RunTest(const FString&)
{
    if(!FParse::Param(FCommandLine::Get(),TEXT("PFRunControlsUITest")) || FParse::Param(FCommandLine::Get(),TEXT("nullrhi")))
    {AddError(TEXT("Requires isolated rendered menu navigation runner"));return false;}
    FString Label;FParse::Value(FCommandLine::Get(),TEXT("PFControlsEvidence="),Label);
    if(Label.IsEmpty() || Label.Len()>48){AddError(TEXT("Invalid evidence label"));return false;}
    for(TCHAR C:Label){if(!FChar::IsAlnum(C) && C!=TEXT('_')){AddError(TEXT("Invalid evidence label"));return false;}}
    const FString Directory=FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()/TEXT("AutomationReports/ControlsUI")/Label);
    if(IFileManager::Get().DirectoryExists(*Directory)){AddError(TEXT("Refusing reused screenshot directory"));return false;}
    IFileManager::Get().MakeDirectory(*Directory,true);
    ADD_LATENT_AUTOMATION_COMMAND(FMenuNavigationExercise(this,Directory));return true;
}
#endif
