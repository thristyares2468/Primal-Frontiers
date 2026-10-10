// Opt-in disposable rendered layout checks; no human/controller acceptance claim.
#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Crafting/PFCraftingHUD.h"
#include "Crafting/PFCraftingComponent.h"
#include "Inventory/PFInventoryPlayerState.h"
#include "Progression/PFProgressionComponent.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Settings/PFGameUserSettings.h"
#include "UI/PFItemPicture.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Framework/Application/SlateApplication.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "UnrealClient.h"
namespace
{
class FCraftingScrollExercise final : public IAutomationLatentCommand
{
public:
    FCraftingScrollExercise(FAutomationTestBase* T,FString D):Test(T),Directory(MoveTemp(D)){}
    ~FCraftingScrollExercise(){if(bScaleChanged && UPFGameUserSettings::Get()){UPFGameUserSettings::Get()->Preferences.HUDScale=OldScale;}}
    bool Update() override
    {
        const double Now=FPlatformTime::Seconds();
        if(Now-Started>45){Test->AddError(FString::Printf(TEXT("Crafting scroll timeout stage%d"),Stage));return true;}
        if(Now<Until){return false;}
        if(Stage==0)
        {
            for(const auto& Context:GEngine->GetWorldContexts()){if(Context.World() && Context.World()->IsGameWorld()){auto* P=Cast<APFSurvivalPlayerController>(Context.World()->GetFirstPlayerController());if(P && P->GetLocalPlayer() && P->GetPawn() && P->GetInventory() && P->GetCrafting()){PC=P;break;}}}
            if(!PC.IsValid()){return false;}auto* PS=PC->GetPlayerState<APFInventoryPlayerState>();
            if(!PC->HasAuthority() || PC->GetNetMode()!=NM_Standalone || !PC->GetInventory()->GetStacks().IsEmpty() || !PS || !PS->Progression || PS->Progression->GetExperience()!=0){Test->AddError(TEXT("Refusing non-fresh standalone scroll fixture"));return true;}
            auto* V=PC->GetPawn()->FindComponentByClass<UPFPlayerSurvivalComponent>();V->HungerDrainPerSecond=0;V->ThirstDrainPerSecond=0;
            if(auto* S=UPFGameUserSettings::Get()){OldScale=S->Preferences.HUDScale;float Scale=1;FParse::Value(FCommandLine::Get(),TEXT("PFHUDScale="),Scale);S->Preferences.HUDScale=Scale;bScaleChanged=true;}
            PC->SetCraftingMenuOpen(true);TArray<UUserWidget*> Widgets;UWidgetBlueprintLibrary::GetAllWidgetsOfClass(PC.Get(),Widgets,UPFCraftingHUD::StaticClass(),false);
            if(Widgets.Num()!=1){Test->AddError(TEXT("Missing actual crafting menu"));return true;}Menu=CastChecked<UPFCraftingHUD>(Widgets[0]);Menu->RefreshMenu();
            if(Menu->GetVisibleRecipeIds().Num()<2){Test->AddError(TEXT("Expected existing recipe catalog"));return true;}Last=Menu->GetVisibleRecipeIds().Last();Menu->SelectRecipe(Last);Wait(Now,.7,1);return false;
        }
        if(!Menu.IsValid() || !PC.IsValid()){Test->AddError(TEXT("Lost scroll fixture"));return true;}
        if(Stage==1)
        {
            Browser=FindBrowser();if(!Browser.IsValid()){return true;}BeforeHeight=Browser->GetCachedGeometry().GetLocalSize().Y;Bounds();Shot(TEXT("last_row_before_feedback"));Wait(Now,.5,2);return false;
        }
        if(Stage==2)
        {
            // Real server refusals produce additional footer lines without selecting a different row.
            PC->ServerLearnKnowledge(TEXT("Tech_FieldTools"));PC->ServerCraftAction(Last,false);Wait(Now,.8,3);return false;
        }
        if(Stage==3)
        {
            Test->TestTrue(TEXT("Actual feedback shrinks browser"),Browser->GetCachedGeometry().GetLocalSize().Y<BeforeHeight-5);
            auto* T=Cast<UTextBlock>(Menu->WidgetTree->FindWidget(TEXT("PF_CraftingPanel_Result")));
            Test->TestTrue(TEXT("Real knowledge and craft refusal rendered"),T && T->GetText().ToString().Contains(TEXT("Knowledge:")) && T->GetText().ToString().Contains(TEXT("Last request:")));
            Test->TestEqual(TEXT("Footer preserves selected last recipe"),Menu->GetSelectedRecipe(),Last);Bounds();Shot(TEXT("last_row_after_feedback"));Wait(Now,.5,4);return false;
        }
        if(Stage==4){Browser->SetScrollOffset(0);Wait(Now,.7,5);return false;}
        if(Stage==5)
        {
            Test->TestTrue(TEXT("Stable layout permits deliberate manual scroll"),FMath::IsNearlyZero(Browser->GetScrollOffset(),0.1f));Test->TestEqual(TEXT("Manual scrolling never changes selection"),Menu->GetSelectedRecipe(),Last);
            Press(EKeys::Up);Wait(Now,.6,6);return false;
        }
        if(Stage==6){Bounds();Press(EKeys::Down);Wait(Now,.6,7);return false;}
        if(Stage==7){Test->TestEqual(TEXT("Keyboard returns to last recipe"),Menu->GetSelectedRecipe(),Last);Bounds();Press(EKeys::Down);Wait(Now,.6,8);return false;}
        if(Stage==8)
        {
            Test->TestEqual(TEXT("Keyboard wraps to first recipe"),Menu->GetSelectedRecipe(),Menu->GetVisibleRecipeIds()[0]);Bounds();Shot(TEXT("first_row_wrapped"));Wait(Now,.5,9);return false;
        }
        if(Stage==9){Press(EKeys::PageDown);Wait(Now,.6,10);return false;}
        if(Stage==10)
        {
            Test->TestTrue(TEXT("Category change clears selection"),Menu->GetSelectedRecipe().IsNone());Press(EKeys::Enter);Test->TestTrue(TEXT("Empty selection cannot craft"),PC->GetCrafting()->ActiveRecipe.IsNone());
            Press(EKeys::Down);Wait(Now,.6,11);return false;
        }
        if(Stage==11)
        {
            Bounds();Test->TestTrue(TEXT("Browsing/refusals preserve empty inventory and XP"),PC->GetInventory()->GetStacks().IsEmpty() && PC->GetPlayerState<APFInventoryPlayerState>()->Progression->GetExperience()==0);
            Press(EKeys::C);Test->TestFalse(TEXT("Close restores gameplay input and cursor"),PC->IsCraftingOpen() || PC->IsMoveInputIgnored() || PC->IsLookInputIgnored() || PC->bShowMouseCursor);
            for(const auto& Path:Shots){Test->TestTrue(TEXT("Screenshot written"),IFileManager::Get().FileSize(*Path)>0);}
            Test->AddInfo(TEXT("[PrimalAgentTools] CraftingScroll: full selected row/text/picture bounds after real footer growth, manual offset preserved in stable layout, keyboard wrap/category empty selection and close. Synthetic inputs, not physical mouse/controller or human acceptance."));return true;
        }
        return false;
    }
private:
    UScrollBox* FindBrowser()
    {auto* W=Menu->WidgetTree->FindWidget(TEXT("PF_Recipe0_Button"));for(auto* P=W?W->GetParent():nullptr;P;P=P->GetParent()){if(auto* S=Cast<UScrollBox>(P)){return S;}}Test->AddError(TEXT("Missing recipe scroll viewport"));return nullptr;}
    void Bounds()
    {
        const int32 Index=Menu->GetVisibleRecipeIds().IndexOfByKey(Menu->GetSelectedRecipe());if(Index==INDEX_NONE){Test->AddError(TEXT("Missing selected row"));return;}
        auto* Scroll=FindBrowser();if(!Scroll){return;}const auto Clip=Scroll->GetCachedGeometry();
        for(const TCHAR* Suffix:{TEXT("Button"),TEXT("Title"),TEXT("Body")})
        {
            const FName Name(*FString::Printf(TEXT("PF_Recipe%d_%s"),Index,Suffix));auto* Row=Menu->WidgetTree->FindWidget(Name);if(!Test->TestNotNull(TEXT("Actual selected widget"),Row)){continue;}
            const auto G=Row->GetCachedGeometry();const auto Top=Clip.AbsoluteToLocal(G.LocalToAbsolute(FVector2D::ZeroVector)),Bottom=Clip.AbsoluteToLocal(G.LocalToAbsolute(G.GetLocalSize()));
            Test->TestTrue(*FString::Printf(TEXT("Full selected %s inside browser stage%d: top%.1f bottom%.1f height%.1f"),Suffix,Stage,Top.Y,Bottom.Y,Clip.GetLocalSize().Y),Top.X>=-1 && Bottom.X<=Clip.GetLocalSize().X+1 && Top.Y>=-1 && Bottom.Y<=Clip.GetLocalSize().Y+1);
            if(auto* T=Cast<UTextBlock>(Row)){Test->TestTrue(TEXT("Selected text fits allocation"),T->GetDesiredSize().Y<=G.GetLocalSize().Y+1);}
        }
        auto* Button=Menu->WidgetTree->FindWidget(FName(*FString::Printf(TEXT("PF_Recipe%d_Button"),Index)));TArray<UWidget*> Widgets;Menu->WidgetTree->GetAllWidgets(Widgets);bool Found=false;
        for(auto* W:Widgets){if(Cast<UPFItemPicture>(W)){for(auto* Parent=W->GetParent();Parent;Parent=Parent->GetParent()){if(Parent==Button){const auto G=W->GetCachedGeometry();const auto Top=Clip.AbsoluteToLocal(G.LocalToAbsolute(FVector2D::ZeroVector)),Bottom=Clip.AbsoluteToLocal(G.LocalToAbsolute(G.GetLocalSize()));Test->TestTrue(TEXT("Selected picture fully inside browser"),Top.Y>=-1 && Bottom.Y<=Clip.GetLocalSize().Y+1 && G.GetLocalSize().X>0);Found=true;break;}}}}
        Test->TestTrue(TEXT("Actual selected row picture found"),Found);
    }
    void Press(FKey K){auto& A=FSlateApplication::Get();A.ProcessKeyDownEvent(FKeyEvent(K,FModifierKeysState(),0,false,0,0));A.ProcessKeyUpEvent(FKeyEvent(K,FModifierKeysState(),0,false,0,0));}
    void Shot(const TCHAR* Name){const FString Path=Directory/(FString(Name)+TEXT(".png"));Shots.Add(Path);FScreenshotRequest::RequestScreenshot(Path,true,false);}
    void Wait(double Now,double Seconds,int32 Next){Until=Now+Seconds;Stage=Next;}
    FAutomationTestBase* Test;FString Directory;TArray<FString> Shots;TWeakObjectPtr<APFSurvivalPlayerController> PC;TWeakObjectPtr<UPFCraftingHUD> Menu;TWeakObjectPtr<UScrollBox> Browser;
    FName Last;float OldScale=1;bool bScaleChanged=false;double BeforeHeight=0,Started=FPlatformTime::Seconds(),Until=0;int32 Stage=0;
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFCraftingScrollLiveTest,"PF.UI.CraftingScrollLive",EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)
bool FPFCraftingScrollLiveTest::RunTest(const FString&)
{
    if(!FParse::Param(FCommandLine::Get(),TEXT("PFRunControlsUITest")) || !FParse::Param(FCommandLine::Get(),TEXT("PFRunCraftingScrollTest")) || FParse::Param(FCommandLine::Get(),TEXT("nullrhi"))){AddError(TEXT("Requires isolated rendered -game -PFRunControlsUITest -PFRunCraftingScrollTest"));return false;}
    FString Label;FParse::Value(FCommandLine::Get(),TEXT("PFControlsEvidence="),Label);if(Label.IsEmpty() || Label.Len()>48){AddError(TEXT("Supply bounded scroll evidence label"));return false;}for(TCHAR C:Label){if(!FChar::IsAlnum(C) && C!=TEXT('_')){AddError(TEXT("Invalid scroll label"));return false;}}
    const FString Directory=FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()/TEXT("AutomationReports/ControlsUI")/Label);if(IFileManager::Get().DirectoryExists(*Directory)){AddError(TEXT("Refusing reused scroll evidence"));return false;}IFileManager::Get().MakeDirectory(*Directory,true);
    ADD_LATENT_AUTOMATION_COMMAND(FCraftingScrollExercise(this,Directory));return true;
}
#endif
