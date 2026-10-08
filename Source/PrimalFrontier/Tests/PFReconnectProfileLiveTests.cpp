// Isolated rendered UI/file test. Never run in the user's game or Local profile.
#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFPauseMenu.h"
#include "Survival/PFControlsMenu.h"
#include "Persistence/PFLocalPlayer.h"
#include "Inventory/PFInventoryComponent.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "UnrealClient.h"
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformTime.h"
#include "Framework/Application/SlateApplication.h"

namespace
{
template<class T> T* FindReconnectWidget(UWorld* World)
{
    TArray<UUserWidget*> Widgets;
    UWidgetBlueprintLibrary::GetAllWidgetsOfClass(World,Widgets,T::StaticClass(),true);
    return Widgets.Num()==1?Cast<T>(Widgets[0]):nullptr;
}
void PressReconnectKey(FKey Key)
{FSlateApplication::Get().ProcessKeyDownEvent(FKeyEvent(Key,FModifierKeysState(),uint32(0),false,0,0));}

class FReconnectProfileExercise final : public IAutomationLatentCommand
{
public:
    FReconnectProfileExercise(FAutomationTestBase* T,FString D):Test(T),Directory(MoveTemp(D)){}
    ~FReconnectProfileExercise(){if(PC.IsValid()){PC->SetPauseMenuOpen(false);}}
    bool Update() override
    {
        const double Now=FPlatformTime::Seconds();
        if(Now-Started>45){Test->AddError(TEXT("Reconnect UI fixture timed out"));return true;}
        if(Now<Until){return false;}
        if(Phase==0)
        {
            for(const auto& C:GEngine->GetWorldContexts())
            {
                auto* W=C.World();
                if(!W || !W->IsGameWorld() || W->GetNetMode()!=NM_Standalone){continue;}
                auto* Candidate=Cast<APFSurvivalPlayerController>(W->GetFirstPlayerController());
                if(Candidate && Candidate->GetLocalPlayer() && Candidate->GetPawn() && Candidate->GetInventory())
                {PC=Candidate;break;}
            }
            if(!PC.IsValid()){return false;}
            auto* Local=Cast<UPFLocalPlayer>(PC->GetLocalPlayer());
            if(!Test->TestNotNull(TEXT("Project local player used"),Local)){return true;}
            LoginBefore=Local->GetGameLoginOptions();
            if(!Test->TestTrue(TEXT("Initial server-issued profile actually readable from disk"),
                LoginBefore.StartsWith(TEXT("PFReconnect=")) && FGuid::ParseExact(LoginBefore.Mid(12),EGuidFormats::Digits,Credential)))
            {return true;}
            Test->TestTrue(TEXT("Deferred server credential write reports Saved"),PC->GetLocalReconnectStatus()==EPFLocalReconnectStatus::Saved);
            Test->TestTrue(TEXT("Actual fresh server setup is confirmed separately"),PC->GetPlayerSetupStatus()==EPFPlayerSetupStatus::NewSurvivor);
            StackCount=PC->GetInventory()->GetStacks().Num();
            PC->SetPauseMenuOpen(true);Pause=FindReconnectWidget<UPFPauseMenu>(PC->GetWorld());
            if(!Test->TestNotNull(TEXT("Pause exists"),Pause.Get())){return true;}
            Phase=1;Until=Now+1;return false;
        }
        if(!PC.IsValid() || !Pause.IsValid()){Test->AddError(TEXT("Lost reconnect UI fixture"));return true;}
        if(Phase==1)
        {
            CheckFeedback(false);
            Shot=Directory/TEXT("saved.png");FScreenshotRequest::RequestScreenshot(Shot,true,false);
            Phase=2;Until=Now+2;return false;
        }
        if(Phase==2)
        {
            if(IFileManager::Get().FileSize(*Shot)<=0){return false;}
            PressReconnectKey(EKeys::Down);PressReconnectKey(EKeys::Down);PressReconnectKey(EKeys::Enter);
            Controls=FindReconnectWidget<UPFControlsMenu>(PC->GetWorld());
            if(!Test->TestNotNull(TEXT("Controls modal opens"),Controls.Get())){return true;}
            Phase=3;Until=Now+0.5;return false;
        }
        if(Phase==3)
        {
            Test->TestTrue(TEXT("Controls has focus before outcome update"),Controls->HasKeyboardFocus());
            // A deliberately invalid payload exercises the real rejection path, not a simulated enum.
            // Exactly this sanitized warning is expected; other warnings/errors remain failures.
            PC->ClientRememberReconnectCredential(FGuid());
            Test->TestTrue(TEXT("Rejected credential reports Failed"),PC->GetLocalReconnectStatus()==EPFLocalReconnectStatus::Failed);
            Test->TestTrue(TEXT("Rejected write preserves prior disk-backed login options"),PC->GetLocalPlayer()->GetGameLoginOptions()==LoginBefore);
            Test->TestTrue(TEXT("Outcome does not close or steal focus from Controls"),Controls->IsInViewport() && Controls->HasKeyboardFocus());
            PressReconnectKey(EKeys::Gamepad_FaceButton_Right);
            Phase=4;Until=Now+0.5;return false;
        }
        if(Phase==4)
        {
            CheckFeedback(true);
            Test->TestTrue(TEXT("Back returns focus to Pause"),Pause->HasKeyboardFocus());
            Shot=Directory/TEXT("failed.png");FScreenshotRequest::RequestScreenshot(Shot,true,false);
            Phase=5;Until=Now+2;return false;
        }
        if(Phase==5)
        {
            if(IFileManager::Get().FileSize(*Shot)<=0){return false;}
            PressReconnectKey(EKeys::Down);PressReconnectKey(EKeys::Enter); // Arm End session, never confirm.
            PC->ClientRememberReconnectCredential(Credential); // Repeat real valid write using server-issued value.
            Test->TestTrue(TEXT("Successful retry reports Saved"),PC->GetLocalReconnectStatus()==EPFLocalReconnectStatus::Saved);
            Test->TestTrue(TEXT("Successful retry retains credential identity"),PC->GetLocalPlayer()->GetGameLoginOptions()==LoginBefore);
            bool Armed=false;
            Pause->WidgetTree->ForEachWidget([&](UWidget* W){if(auto* T=Cast<UTextBlock>(W)){Armed|=T->GetText().ToString()==TEXT("Confirm end session");}});
            Test->TestTrue(TEXT("Outcome does not reset quit confirmation"),Armed);
            Phase=6;Until=Now+0.5;return false;
        }
        if(Phase==6)
        {
            CheckFeedback(false);Shot=Directory/TEXT("retry.png");FScreenshotRequest::RequestScreenshot(Shot,true,false);
            Phase=7;Until=Now+2;return false;
        }
        if(IFileManager::Get().FileSize(*Shot)<=0){return false;}
        PressReconnectKey(EKeys::P);
        Test->TestFalse(TEXT("Resume closes menu"),PC->IsPauseMenuOpen());
        Test->TestFalse(TEXT("Resume restores movement"),PC->IsMoveInputIgnored());
        Test->TestEqual(TEXT("Profile/UI operations never mutate inventory"),PC->GetInventory()->GetStacks().Num(),StackCount);
        Test->AddInfo(TEXT("[PrimalUI] Actual isolated profile write/rejection/retry and rendered feedback verified. Invalid-payload warning expected once; physical disk-full and network reconnect not simulated."));
        return true;
    }
private:
    void CheckFeedback(bool bFailed)
    {
        auto* Text=Cast<UTextBlock>(Pause->WidgetTree->FindWidget(TEXT("PF_ReconnectFeedback")));
        if(!Test->TestNotNull(TEXT("Reconnect feedback widget"),Text)){return;}
        const FString Value=Text->GetText().ToString();
        auto* Setup=Cast<UTextBlock>(Pause->WidgetTree->FindWidget(TEXT("PF_ServerSetupFeedback")));
        if(Test->TestNotNull(TEXT("Separate server setup label"),Setup))
        {Test->TestTrue(TEXT("Fresh setup never falsely says saved survivor restored"),Setup->GetText().ToString().Contains(TEXT("new survivor")));}
        Test->TestTrue(TEXT("Actual outcome visible"),Text->GetVisibility()==ESlateVisibility::HitTestInvisible &&
            Value.Contains(bFailed?TEXT("could not be saved"):TEXT("saved on this device")));
        Test->TestTrue(TEXT("Profile outcome does not claim world persistence"),Value.Contains(TEXT("does not save world state")));
        Test->TestFalse(TEXT("Feedback omits private credential"),Value.Contains(Credential.ToString(EGuidFormats::Digits)));
        Test->TestTrue(TEXT("Feedback has readable font"),Text->GetFont().Size>=22);
        Test->TestTrue(TEXT("Wrapped feedback fits allocation"),Text->GetDesiredSize().Y<=Text->GetCachedGeometry().GetLocalSize().Y+1);
        // Check the entire pause layout, including the new multiline status, fits the viewport.
        Pause->WidgetTree->ForEachWidget([&](UWidget* W)
        {
            if(auto* T=Cast<UTextBlock>(W);T && T->GetVisibility()!=ESlateVisibility::Collapsed)
            {
                const auto& G=T->GetCachedGeometry();const auto& Root=Pause->GetCachedGeometry();
                const FVector2D Min=Root.AbsoluteToLocal(G.LocalToAbsolute(FVector2D::ZeroVector));
                const FVector2D Max=Root.AbsoluteToLocal(G.LocalToAbsolute(G.GetLocalSize()));
                Test->TestTrue(TEXT("Pause text stays inside viewport"),Min.X>=-1 && Min.Y>=-1 && Max.X<=Root.GetLocalSize().X+1 && Max.Y<=Root.GetLocalSize().Y+1);
            }
        });
    }
    FAutomationTestBase* Test;FString Directory,Shot,LoginBefore;FGuid Credential;
    TWeakObjectPtr<APFSurvivalPlayerController> PC;TWeakObjectPtr<UPFPauseMenu> Pause;TWeakObjectPtr<UPFControlsMenu> Controls;
    int32 Phase=0,StackCount=0;double Started=FPlatformTime::Seconds(),Until=0;
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFReconnectProfileLiveTest,"PF.UI.ReconnectProfileLive",EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)
bool FPFReconnectProfileLiveTest::RunTest(const FString&)
{
    FString Profile,Label;FParse::Value(FCommandLine::Get(),TEXT("PFIdentityProfile="),Profile);
    FParse::Value(FCommandLine::Get(),TEXT("PFControlsEvidence="),Label);
    if(!FParse::Param(FCommandLine::Get(),TEXT("PFRunControlsUITest")) || FParse::Param(FCommandLine::Get(),TEXT("nullrhi")) ||
        !Profile.StartsWith(TEXT("UI")) || Profile.Len()!=14 || Label.IsEmpty() || Label.Len()>48)
    {AddError(TEXT("Requires isolated rendered game, opt-in and a unique UI-prefixed profile from RunControlsAutomation.ps1 -TestCase Reconnect"));return false;}
    for(TCHAR C:Profile+Label){if(!FChar::IsAlnum(C) && C!=TEXT('_')){AddError(TEXT("Invalid isolated profile/evidence label"));return false;}}
    const FString Directory=FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()/TEXT("AutomationReports/ControlsUI")/Label);
    if(IFileManager::Get().DirectoryExists(*Directory)){AddError(TEXT("Refusing reused evidence directory"));return false;}
    IFileManager::Get().MakeDirectory(*Directory,true);
    AddExpectedError(TEXT("Local reconnect profile could not be saved; credential omitted from log"),EAutomationExpectedErrorFlags::Contains,1);
    ADD_LATENT_AUTOMATION_COMMAND(FReconnectProfileExercise(this,Directory));return true;
}
#endif
