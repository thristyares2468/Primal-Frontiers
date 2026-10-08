// Opt-in rendered local UI smoke, not a claim of human controller feel.
// -game L_PrimalFrontier_OpenWorld -PFRunControlsUITest -PFControlsEvidence=<safe label>
// -ExecCmds="Automation RunTests PF.UI.ControlsLive" -TestExit="Automation Test Queue Empty"
#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFPauseMenu.h"
#include "Survival/PFControlsMenu.h"
#include "Inventory/PFInventoryComponent.h"
#include "Building/PFBuildingComponent.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "UnrealClient.h"
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformTime.h"
#include "Tests/AutomationCommon.h"
#include "Framework/Application/SlateApplication.h"

namespace
{
struct FControlsState
{
    int32 Phase=0;
    double Started=FPlatformTime::Seconds(),Until=0;
    FString Directory,Shot;
    TWeakObjectPtr<APFSurvivalPlayerController> PC;
    TWeakObjectPtr<UPFPauseMenu> Pause;
    TWeakObjectPtr<UPFControlsMenu> Controls;
    int32 Wood=0;
};
template<class T> T* FindWidget(UWorld* World)
{
    TArray<UUserWidget*> Widgets;UWidgetBlueprintLibrary::GetAllWidgetsOfClass(World,Widgets,T::StaticClass(),true);
    return Widgets.Num()==1?Cast<T>(Widgets[0]):nullptr;
}
bool Press(FKey Key,bool bRepeat=false)
{
    // Deliver through Slate's real focus path, not directly to the widget handler.
    return FSlateApplication::Get().ProcessKeyDownEvent(FKeyEvent(Key,FModifierKeysState(),uint32(0),bRepeat,0,0));
}
class FControlsExercise final : public IAutomationLatentCommand
{
public:
    FControlsExercise(FAutomationTestBase* InTest,TSharedRef<FControlsState> InState):Test(InTest),S(InState){}
    bool Update() override
    {
        const double Now=FPlatformTime::Seconds();
        if(Now-S->Started>45){Test->AddError(TEXT("Controls UI timeout"));if(S->PC.IsValid()){S->PC->SetPauseMenuOpen(false);}return true;}
        if(Now<S->Until){return false;}
        if(S->Phase==0)
        {
            for(const auto& C:GEngine->GetWorldContexts())
            {
                if(C.World() && C.World()->IsGameWorld())
                {auto* PC=Cast<APFSurvivalPlayerController>(C.World()->GetFirstPlayerController());if(PC && PC->IsLocalController() && PC->GetLocalPlayer() && PC->GetPawn() && PC->GetInventory()){S->PC=PC;break;}}
            }
            if(!S->PC.IsValid()){return false;}
            auto* PC=S->PC.Get();S->Wood=PC->GetInventory()->Count(TEXT("Item_Wood"));
            PC->SetPauseMenuOpen(true);S->Pause=FindWidget<UPFPauseMenu>(PC->GetWorld());
            if(!Test->TestNotNull(TEXT("Pause widget added to local player"),S->Pause.Get())){PC->SetPauseMenuOpen(false);return true;}
            // SetInputMode queues focus through LocalPlayer Slate operations. Allow
            // a frame before sending user navigation, as real input does.
            S->Until=Now+1;S->Phase=10;return false;
        }
        if(S->Phase==10)
        {
            auto* PC=S->PC.Get();
            if(!PC || !S->Pause.IsValid()){Test->AddError(TEXT("Lost pause fixture"));return true;}
            Press(EKeys::Down);Press(EKeys::Down);Press(EKeys::Enter);
            S->Controls=FindWidget<UPFControlsMenu>(PC->GetWorld());
            if(!Test->TestNotNull(TEXT("Pause navigation opens controls"),S->Controls.Get())){PC->SetPauseMenuOpen(false);return true;}
            Test->TestTrue(TEXT("Help keeps controller paused and input blocked"),PC->IsPauseMenuOpen() && PC->IsMoveInputIgnored() && PC->IsLookInputIgnored());
            Test->TestTrue(TEXT("Help returns to owning pause widget"),S->Controls->ReturnFocus==S->Pause.Get());
            Test->TestFalse(TEXT("Help starts with keyboard page"),S->Controls->IsGamepadPage());
            Test->TestTrue(TEXT("Real Enhanced Input movement mappings shown"),PC->GetControlHints(false).ContainsByPredicate([](const FPFControlHint& H){return H.Context==TEXT("Movement / view");}));
            Test->TestTrue(TEXT("Gameplay attack key consumed by help"),Press(EKeys::LeftMouseButton));
            S->Until=Now+2;S->Phase=1;return false;
        }
        if(!S->PC.IsValid() || !S->Controls.IsValid()){Test->AddError(TEXT("Lost controls UI fixture"));return true;}
        if(S->Phase==1)
        {
            S->Shot=S->Directory/TEXT("keyboard.png");
            FScreenshotRequest::RequestScreenshot(S->Shot,true,false);S->Until=Now+2;S->Phase=2;return false;
        }
        if(S->Phase==2)
        {
            if(IFileManager::Get().FileSize(*S->Shot)<=0){return false;}
            Test->AddInfo(TEXT("[PrimalUI] Keyboard screenshot: ")+S->Shot);
            Press(EKeys::Gamepad_RightShoulder);
            Test->TestTrue(TEXT("Controller page reachable via RB"),S->Controls->IsGamepadPage());
            Press(EKeys::Gamepad_RightShoulder,true);
            Test->TestTrue(TEXT("Held tab key does not repeat toggle"),S->Controls->IsGamepadPage());
            S->Until=Now+1;S->Phase=3;return false;
        }
        if(S->Phase==3)
        {S->Shot=S->Directory/TEXT("controller.png");FScreenshotRequest::RequestScreenshot(S->Shot,true,false);S->Until=Now+2;S->Phase=4;return false;}
        if(S->Phase==4)
        {
            if(IFileManager::Get().FileSize(*S->Shot)<=0){return false;}
            Test->AddInfo(TEXT("[PrimalUI] Controller screenshot: ")+S->Shot);
            Press(EKeys::Gamepad_DPad_Down);
            Press(EKeys::Gamepad_FaceButton_Right);
            Test->TestFalse(TEXT("Back removes help"),S->Controls->IsInViewport());
            Test->TestTrue(TEXT("Back retains pause"),S->PC->IsPauseMenuOpen() && S->Pause->IsInViewport());
            Test->TestTrue(TEXT("Back restores keyboard focus to Pause"),S->Pause->HasKeyboardFocus());
            Test->TestEqual(TEXT("Help input does not consume or create inventory"),S->PC->GetInventory()->Count(TEXT("Item_Wood")),S->Wood);
            Test->TestFalse(TEXT("Help cannot open building"),S->PC->Building->bBuildMode);
            S->Shot=S->Directory/TEXT("pause.png");FScreenshotRequest::RequestScreenshot(S->Shot,true,false);S->Until=Now+2;S->Phase=5;return false;
        }
        if(IFileManager::Get().FileSize(*S->Shot)<=0){return false;}
        Press(EKeys::P);
        Test->TestFalse(TEXT("P resumes after returning from help"),S->PC->IsPauseMenuOpen());
        Test->TestFalse(TEXT("Resume restores movement input"),S->PC->IsMoveInputIgnored());
        Test->TestFalse(TEXT("Resume removes pause"),S->Pause->IsInViewport());
        // Programmatic closure/teardown must not leave a child modal stranded.
        S->PC->SetPauseMenuOpen(true);
        S->Pause=FindWidget<UPFPauseMenu>(S->PC->GetWorld());
        S->Pause->HandleNavigation(EKeys::Down);S->Pause->HandleNavigation(EKeys::Down);S->Pause->HandleNavigation(EKeys::Enter);
        auto* Reopened=FindWidget<UPFControlsMenu>(S->PC->GetWorld());
        Test->TestNotNull(TEXT("Controls can be reopened"),Reopened);
        S->PC->SetPauseMenuOpen(false);
        if(Reopened){Test->TestFalse(TEXT("Closing Pause removes child help"),Reopened->IsInViewport());}
        Test->AddInfo(TEXT("[PrimalUI] Rendered controls/navigation smoke complete. Physical hardware and human usability remain unverified."));
        return true;
    }
private:
    FAutomationTestBase* Test;TSharedRef<FControlsState> S;
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFControlsLiveTest,"PF.UI.ControlsLive",EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)
bool FPFControlsLiveTest::RunTest(const FString&)
{
    if(!FParse::Param(FCommandLine::Get(),TEXT("PFRunControlsUITest")) || FParse::Param(FCommandLine::Get(),TEXT("nullrhi")))
    {AddError(TEXT("Requires isolated rendered -game -PFRunControlsUITest; never run in a user session."));return false;}
    FString Label;
    FParse::Value(FCommandLine::Get(),TEXT("PFControlsEvidence="),Label);
    for(TCHAR C:Label){if(!FChar::IsAlnum(C) && C!=TEXT('_')){AddError(TEXT("Invalid evidence label"));return false;}}
    if(Label.IsEmpty() || Label.Len()>48){AddError(TEXT("Supply unique PFControlsEvidence label (1-48 alphanumeric/underscore)"));return false;}
    auto State=MakeShared<FControlsState>();State->Directory=FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()/TEXT("AutomationReports/ControlsUI")/Label);
    if(IFileManager::Get().DirectoryExists(*State->Directory)){AddError(TEXT("Evidence directory must be fresh; refusing overwrite"));return false;}
    IFileManager::Get().MakeDirectory(*State->Directory,true);
    ADD_LATENT_AUTOMATION_COMMAND(FControlsExercise(this,State));return true;
}
#endif
