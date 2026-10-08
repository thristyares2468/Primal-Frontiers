// Opt-in rendered UI presentation test; same isolated launcher as ControlsLive.
#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Survival/PFSurvivalPlayerController.h"
#include "Inventory/PFInventoryComponent.h"
#include "Inventory/PFItemCatalog.h"
#include "Inventory/PFInventoryHUD.h"
#include "Settings/PFGameUserSettings.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
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
    ~FSelectionUIExercise(){if(bChanged && UPFGameUserSettings::Get()){UPFGameUserSettings::Get()->Preferences.HUDScale=OriginalScale;}}
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
            if(auto* S=UPFGameUserSettings::Get()){OriginalScale=S->Preferences.HUDScale;float Scale=1;FParse::Value(FCommandLine::Get(),TEXT("PFHUDScale="),Scale);S->Preferences.HUDScale=FMath::Clamp(Scale,0.75f,1.5f);bChanged=true;}
            TArray<UUserWidget*> Widgets;UWidgetBlueprintLibrary::GetAllWidgetsOfClass(PC.Get(),Widgets,UPFInventoryHUD::StaticClass(),false);
            if(Widgets.Num()!=1){Test->AddError(TEXT("Missing actual inventory widget"));return true;}HUD=Widgets[0];
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
        if(Phase==5)
        {
            if(IFileManager::Get().FileSize(*Shot)<=0){return false;}
            Test->AddInfo(TEXT("[PrimalUI] Reselection screenshot: ")+Shot);
            auto* I=PC->GetInventory();
            for(int32 N=0;N<7;++N){Test->TestTrue(TEXT("Fill bag with distinct real food batches"),I->AddExisting(TEXT("Item_Food"),1,UPFInventoryComponent::ServerTime(PC->GetWorld())+300+N));}
            Test->TestEqual(TEXT("Default bag filled"),I->GetStacks().Num(),8);
            Until=Now+0.4;Phase=6;return false;
        }
        if(Phase==6)
        {
            CheckReadability();Shot=Directory/TEXT("full_bag_first.png");FScreenshotRequest::RequestScreenshot(Shot,true,false);Until=Now+1;Phase=7;return false;
        }
        if(Phase==7)
        {
            if(IFileManager::Get().FileSize(*Shot)<=0){return false;}
            Test->AddInfo(TEXT("[PrimalUI] Full bag screenshot: ")+Shot);
            for(int32 N=0;N<7;++N){Press(EKeys::Gamepad_DPad_Down);}
            Test->TestEqual(TEXT("Bound navigation reaches final stack"),PC->GetSelectedInventoryIndex(),7);
            LastId=PC->GetInventory()->GetStacks().Last().StackId;
            Until=Now+0.4;Phase=8;return false;
        }
        if(Phase==8)
        {
            CheckReadability();CheckSelectedVisible();
            Shot=Directory/TEXT("full_bag_last.png");FScreenshotRequest::RequestScreenshot(Shot,true,false);Until=Now+1;Phase=9;return false;
        }
        if(Phase==9)
        {
            if(IFileManager::Get().FileSize(*Shot)<=0){return false;}
            Test->AddInfo(TEXT("[PrimalUI] Final row screenshot: ")+Shot);
            Test->TestTrue(TEXT("Remove earlier row through authority API"),PC->GetInventory()->Remove(PC->GetInventory()->GetStacks()[0].StackId,5));
            const int32 Selected=PC->GetSelectedInventoryIndex();Test->TestTrue(TEXT("Selection remains valid after reorder"),PC->GetInventory()->GetStacks().IsValidIndex(Selected));
            if(PC->GetInventory()->GetStacks().IsValidIndex(Selected)){Test->TestEqual(TEXT("Selection follows stable identity after reorder"),PC->GetInventory()->GetStacks()[Selected].StackId,LastId);}
            Until=Now+0.4;Phase=10;return false;
        }
        CheckReadability();CheckSelectedVisible();
        Press(EKeys::Tab);Test->TestFalse(TEXT("Inventory closes normally"),PC->IsInventoryOpen());
        Test->AddInfo(TEXT("[PrimalUI] Real expiry, full bag, stable selection and rendered bounds checked. No hardware/manual acceptance claimed."));return true;
    }
private:
    void CheckSelectedVisible()
    {
        const auto* I=PC->GetInventory();const int32 Selected=PC->GetSelectedInventoryIndex();
        if(!I->GetStacks().IsValidIndex(Selected)){Test->AddError(TEXT("Missing selected stack"));return;}
        const auto& S=I->GetStacks()[Selected];const auto* D=I->Definition(S.ItemId);
        const FString Name=D?D->DisplayName.ToString():S.ItemId.ToString();
        const int32 Seconds=FMath::CeilToInt(S.ExpiresAt-UPFInventoryComponent::ServerTime(PC->GetWorld()));
        TArray<FString> Lines;AllText().ParseIntoArrayLines(Lines);bool bFound=false;
        for(const auto& Line:Lines)
        {if(Line.StartsWith(TEXT(">")) && Line.Contains(Name) && (Line.Contains(FString::Printf(TEXT("[%ds fresh]"),Seconds)) || Line.Contains(FString::Printf(TEXT("[%ds fresh]"),Seconds+1)))){bFound=true;}}
        Test->TestTrue(TEXT("Actual selected batch freshness row is visible"),bFound);
    }
    FString AllText() const
    {
        FString Out;TArray<UWidget*> Widgets;HUD->WidgetTree->GetAllWidgets(Widgets);
        for(auto* W:Widgets){if(auto* T=Cast<UTextBlock>(W)){Out+=T->GetText().ToString()+TEXT("\n");}}return Out;
    }
    void CheckReadability()
    {
        TArray<UWidget*> Widgets;HUD->WidgetTree->GetAllWidgets(Widgets);
        const FGeometry Root=HUD->GetCachedGeometry();bool bBody=false;
        for(auto* W:Widgets)
        {
            if(auto* T=Cast<UTextBlock>(W))
            {
                Test->TestTrue(TEXT("Inventory text fits allocated height"),T->GetDesiredSize().Y<=T->GetCachedGeometry().GetLocalSize().Y+1);
                if(T->GetText().ToString().Contains(TEXT("fresh")))
                {bBody=true;Test->TestEqual(TEXT("Inventory rows respect HUD scaling"),T->GetFont().Size,float(FMath::RoundToInt(22*UPFGameUserSettings::Get()->Preferences.HUDScale)));}
            }
            if(Cast<UBorder>(W))
            {
                const auto G=W->GetCachedGeometry();const auto TL=Root.AbsoluteToLocal(G.LocalToAbsolute(FVector2D::ZeroVector)),BR=Root.AbsoluteToLocal(G.LocalToAbsolute(G.GetLocalSize()));
                Test->TestTrue(TEXT("Inventory panel fits right side without covering aim"),TL.X>Root.GetLocalSize().X*0.55 && TL.Y>=0 && BR.X<Root.GetLocalSize().X && BR.Y<Root.GetLocalSize().Y);
            }
        }
        Test->TestTrue(TEXT("Actual food freshness rendered"),bBody);
    }
    void Press(FKey Key)
    {
        for(const auto& B:PC->InputComponent->KeyBindings){if(B.Chord.Key==Key && B.KeyEvent==IE_Pressed){B.KeyDelegate.Execute(Key);return;}}
        Test->AddError(TEXT("Missing inventory binding ")+Key.ToString());
    }
    FAutomationTestBase* Test;FString Directory,Shot;TWeakObjectPtr<APFSurvivalPlayerController> PC;
    TWeakObjectPtr<UUserWidget> HUD;FGuid LastId;float OriginalScale=1;bool bChanged=false;
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
