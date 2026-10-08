// Opt-in rendered UI presentation test; same isolated launcher as ControlsLive.
#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Survival/PFSurvivalPlayerController.h"
#include "Inventory/PFInventoryComponent.h"
#include "Components/InputComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "UnrealClient.h"
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformTime.h"

namespace
{
class FSelectionUIExercise final : public IAutomationLatentCommand
{
public:
    FSelectionUIExercise(FAutomationTestBase* T,FString D):Test(T),Directory(MoveTemp(D)){}
    bool Update() override
    {
        const double Now=FPlatformTime::Seconds();if(Now-Started>45){Test->AddError(TEXT("Inventory UI timeout"));return true;}
        if(Now<Until){return false;}
        if(Phase==0)
        {
            for(const auto& C:GEngine->GetWorldContexts())
            {
                if(C.World() && C.World()->IsGameWorld())
                {auto* Candidate=Cast<APFSurvivalPlayerController>(C.World()->GetFirstPlayerController());if(Candidate && Candidate->GetLocalPlayer() && Candidate->GetPawn() && Candidate->GetInventory()){PC=Candidate;break;}}
            }
            if(!PC.IsValid()){return false;}
            if(!PC->HasAuthority() || !PC->GetInventory()->GetStacks().IsEmpty()){Test->AddError(TEXT("Requires isolated fresh standalone player; refusing existing inventory"));return true;}
            auto* I=PC->GetInventory();
            Test->TestTrue(TEXT("Create short-lived actual food"),I->AddExisting(TEXT("Item_Food"),1,UPFInventoryComponent::ServerTime(PC->GetWorld())+0.25));
            Test->TestTrue(TEXT("Create permanent actual wood"),I->Grant(TEXT("Item_Wood"),5));
            Press(EKeys::Tab);Test->TestTrue(TEXT("Bound key opens real inventory overlay"),PC->IsInventoryOpen());
            Test->TestEqual(TEXT("Initially selected food"),PC->GetSelectedInventoryIndex(),0);
            Until=Now+2;Phase=1;return false;
        }
        if(!PC.IsValid()){Test->AddError(TEXT("Lost inventory UI fixture"));return true;}
        if(Phase==1)
        {
            Test->TestEqual(TEXT("Expired food is pruned in playable world"),PC->GetInventory()->Count(TEXT("Item_Food")),0);
            Test->TestEqual(TEXT("No automatic replacement selection"),PC->GetSelectedInventoryIndex(),INDEX_NONE);
            Press(EKeys::G);Press(EKeys::Gamepad_DPad_Right);
            Test->TestEqual(TEXT("Both unselected drop routes preserve wood"),PC->GetInventory()->Count(TEXT("Item_Wood")),5);
            Test->TestTrue(TEXT("Explicit select feedback"),PC->GetInventoryMessage().Contains(TEXT("Select")));
            Until=Now+1;Phase=2;return false;
        }
        if(Phase==2){Shot=Directory/TEXT("expired_selection.png");FScreenshotRequest::RequestScreenshot(Shot,true,false);Until=Now+2;Phase=3;return false;}
        if(Phase==3)
        {
            if(IFileManager::Get().FileSize(*Shot)<=0){return false;}
            Test->AddInfo(TEXT("[PrimalUI] Expired-selection screenshot: ")+Shot);
            Press(EKeys::Gamepad_DPad_Down);Test->TestEqual(TEXT("Explicit controller reselection"),PC->GetSelectedInventoryIndex(),0);
            Test->TestTrue(TEXT("Reselection replaces stale missing-stack warning"),PC->GetInventoryMessage().StartsWith(TEXT("Selected ")));
            Until=Now+1;Phase=4;return false;
        }
        if(Phase==4){Shot=Directory/TEXT("reselected_stack.png");FScreenshotRequest::RequestScreenshot(Shot,true,false);Until=Now+2;Phase=5;return false;}
        if(IFileManager::Get().FileSize(*Shot)<=0){return false;}
        Test->AddInfo(TEXT("[PrimalUI] Reselection screenshot: ")+Shot);
        Press(EKeys::Tab);Test->TestFalse(TEXT("Inventory closes normally"),PC->IsInventoryOpen());
        Test->AddInfo(TEXT("[PrimalUI] Real world expiry and bound input paths rendered. No hardware/manual acceptance claimed."));return true;
    }
private:
    void Press(FKey Key)
    {
        for(const auto& B:PC->InputComponent->KeyBindings){if(B.Chord.Key==Key && B.KeyEvent==IE_Pressed){B.KeyDelegate.Execute(Key);return;}}
        Test->AddError(TEXT("Missing inventory binding ")+Key.ToString());
    }
    FAutomationTestBase* Test;FString Directory,Shot;TWeakObjectPtr<APFSurvivalPlayerController> PC;
    int32 Phase=0;double Started=FPlatformTime::Seconds(),Until=0;
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFInventorySelectionLiveTest,"PF.UI.InventorySelectionLive",EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)
bool FPFInventorySelectionLiveTest::RunTest(const FString&)
{
    if(!FParse::Param(FCommandLine::Get(),TEXT("PFRunControlsUITest")) || FParse::Param(FCommandLine::Get(),TEXT("nullrhi")))
    {AddError(TEXT("Requires isolated rendered -game -PFRunControlsUITest"));return false;}
    FString Label;FParse::Value(FCommandLine::Get(),TEXT("PFControlsEvidence="),Label);
    if(Label.IsEmpty() || Label.Len()>48){AddError(TEXT("Supply unique bounded evidence label"));return false;}
    for(TCHAR C:Label){if(!FChar::IsAlnum(C) && C!=TEXT('_')){AddError(TEXT("Invalid evidence label"));return false;}}
    const FString Directory=FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()/TEXT("AutomationReports/ControlsUI")/Label);
    if(IFileManager::Get().DirectoryExists(*Directory)){AddError(TEXT("Refusing to overwrite prior UI evidence"));return false;}
    IFileManager::Get().MakeDirectory(*Directory,true);
    ADD_LATENT_AUTOMATION_COMMAND(FSelectionUIExercise(this,Directory));return true;
}
#endif
