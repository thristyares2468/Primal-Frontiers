// Opt-in disposable rendered standalone feedback; not a human/controller acceptance test.
#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Crafting/PFCraftingHUD.h"
#include "Crafting/PFCraftingComponent.h"
#include "Inventory/PFInventoryPlayerState.h"
#include "Progression/PFProgressionComponent.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Settings/PFGameUserSettings.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Framework/Application/SlateApplication.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "UnrealClient.h"
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
namespace
{
class FProgressionFeedbackExercise final : public IAutomationLatentCommand
{
public:
    FProgressionFeedbackExercise(FAutomationTestBase* T,FString D):Test(T),Directory(MoveTemp(D)){}
    ~FProgressionFeedbackExercise(){if(bChanged && UPFGameUserSettings::Get()){UPFGameUserSettings::Get()->Preferences.HUDScale=OldScale;}}
    bool Update() override
    {
        const double Now=FPlatformTime::Seconds();
        if(Now-Started>80){Test->AddError(FString::Printf(TEXT("Progression UI timeout stage%d"),Stage));return true;}
        if(Now<Until){return false;}
        if(Stage==0)
        {
            for(const auto& Context:GEngine->GetWorldContexts()){if(Context.World() && Context.World()->IsGameWorld()){auto* Candidate=Cast<APFSurvivalPlayerController>(Context.World()->GetFirstPlayerController());if(Candidate && Candidate->GetPawn() && Candidate->GetLocalPlayer() && Candidate->GetInventory() && Candidate->GetCrafting()){PC=Candidate;break;}}}
            if(!PC.IsValid()){return false;}
            auto* PS=PC->GetPlayerState<APFInventoryPlayerState>();
            if(!PC->HasAuthority() || PC->GetNetMode()!=NM_Standalone || !PC->GetInventory()->GetStacks().IsEmpty() || !PS || !PS->Progression || PS->Progression->GetExperience()!=0)
            {Test->AddError(TEXT("Refusing non-fresh standalone progression fixture"));return true;}
            Progression=PS->Progression;
            auto* Needs=PC->GetPawn()->FindComponentByClass<UPFPlayerSurvivalComponent>();Needs->HungerDrainPerSecond=0;Needs->ThirstDrainPerSecond=0;
            PC->GetPawn()->SetActorLocation(FVector(-1200,0,100));
            if(auto* S=UPFGameUserSettings::Get()){OldScale=S->Preferences.HUDScale;float Scale=1;FParse::Value(FCommandLine::Get(),TEXT("PFHUDScale="),Scale);S->Preferences.HUDScale=Scale;bChanged=true;}
            PC->SetCraftingMenuOpen(true);TArray<UUserWidget*> Widgets;UWidgetBlueprintLibrary::GetAllWidgetsOfClass(PC.Get(),Widgets,UPFCraftingHUD::StaticClass(),false);
            if(Widgets.Num()!=1){Test->AddError(TEXT("Missing real crafting HUD"));return true;}
            Menu=CastChecked<UPFCraftingHUD>(Widgets[0]);Menu->SelectRecipe(TEXT("Recipe_Tool"));Wait(Now,.6,1);return false;
        }
        if(Stage==1)
        {
            Check(0,TEXT("+20 XP once"));Shot(TEXT("first_craft_available"));
            Test->TestTrue(TEXT("Fixture wood"),PC->GetInventory()->Grant(TEXT("Item_Wood"),6));Test->TestTrue(TEXT("Fixture stone"),PC->GetInventory()->Grant(TEXT("Item_Stone"),4));
            Test->TestEqual(TEXT("Developer items never grant XP"),Progression->GetExperience(),0);
            Press(EKeys::Enter);Wait(Now,.6,2);return false;
        }
        if(Stage==2)
        {
            if(PC->GetCrafting()->ActiveRecipe.IsNone()){return false;}
            Press(EKeys::R);Wait(Now,.5,3);return false;
        }
        if(Stage==3)
        {
            Test->TestTrue(TEXT("Actual cancel idle"),PC->GetCrafting()->ActiveRecipe.IsNone());Check(0,TEXT("+20 XP once"));
            Test->TestEqual(TEXT("Cancelled output absent"),PC->GetInventory()->Count(TEXT("Item_Tool")),0);Press(EKeys::Enter);Wait(Now,.6,4);return false;
        }
        if(Stage==4)
        {
            if(PC->GetInventory()->Count(TEXT("Item_Tool"))!=1 || !PC->GetCrafting()->ActiveRecipe.IsNone()){return false;}
            Check(20,TEXT("already earned"));Shot(TEXT("first_craft_earned"));Wait(Now,1,5);return false;
        }
        if(Stage==5){Press(EKeys::Enter);Wait(Now,.6,6);return false;}
        if(Stage==6)
        {
            if(PC->GetInventory()->Count(TEXT("Item_Tool"))!=2 || !PC->GetCrafting()->ActiveRecipe.IsNone()){return false;}
            Check(20,TEXT("already earned"));Test->TestEqual(TEXT("Repeat retains one credited recipe"),Progression->GetRecord().CreditedCrafts.Num(),1);
            Menu->SelectRecipe(TEXT("Recipe_Cook"));Wait(Now,.6,7);return false;
        }
        if(Stage==7)
        {
            Check(20,TEXT("+20 XP once"));Shot(TEXT("different_recipe"));
            // Explicit trusted fixture seed to inspect long/cap UI, not an earned gameplay level claim.
            auto Candidate=Progression->GetRecord();Candidate.Experience=2700;FString Error;
            Test->TestTrue(TEXT("Trusted isolated cap fixture"),Progression->Restore(Candidate,Error));Wait(Now,.6,8);return false;
        }
        if(Stage==8)
        {
            Menu->RefreshMenu();Test->TestTrue(TEXT("Actual cap and available points"),Text(TEXT("PF_CraftingProgression_Summary")).Contains(TEXT("Level 10 | 2700 XP | Level cap | 27 knowledge points")));
            Test->TestTrue(TEXT("Cap recipe gives no XP"),Text(TEXT("PF_CraftingProgression_Reward")).Contains(TEXT("no extra XP")));Bounds();Shot(TEXT("level_cap"));Wait(Now,1,9);return false;
        }
        if(Stage==9)
        {
            Press(EKeys::C);Test->TestFalse(TEXT("Crafting closes through actual Slate key"),PC->IsCraftingOpen());PC->SetPauseMenuOpen(true);Test->TestTrue(TEXT("Pause opens afterward"),PC->IsPauseMenuOpen());PC->SetPauseMenuOpen(false);
            for(const auto& Path:Shots){Test->TestTrue(TEXT("Screenshot written"),IFileManager::Get().FileSize(*Path)>0);}
            Test->AddInfo(TEXT("[PrimalUI] Actual completed/cancelled/repeated craft feedback, private XP, unavailable spending, max-scale layout and close checked; level cap is a trusted fixture seed."));return true;
        }
        return false;
    }
private:
    FString Text(const TCHAR* Name){auto* T=Cast<UTextBlock>(Menu->WidgetTree->FindWidget(Name));Test->TestNotNull(TEXT("Actual progression label"),T);return T?T->GetText().ToString():FString();}
    void Check(int32 XP,const TCHAR* Reward)
    {
        Menu->RefreshMenu();Test->TestEqual(TEXT("Actual authoritative XP"),Progression->GetExperience(),XP);
        Test->TestTrue(TEXT("Actual XP shown"),Text(TEXT("PF_CraftingProgression_Summary")).Contains(FString::Printf(TEXT("%d / 100 XP"),XP)));
        Test->TestTrue(TEXT("Spending honestly unavailable"),Text(TEXT("PF_CraftingProgression_Summary")).Contains(TEXT("spending not available yet")));
        Test->TestTrue(TEXT("Actual selected reward state"),Text(TEXT("PF_CraftingProgression_Reward")).Contains(Reward));Bounds();
    }
    void Bounds()
    {
        const auto Root=Menu->GetCachedGeometry();const auto Size=Root.GetLocalSize();
        for(const TCHAR* Name:{TEXT("PF_CraftingPanel"),TEXT("PF_CraftingProgression_Summary"),TEXT("PF_CraftingProgression_Reward"),TEXT("PF_CraftSelected"),TEXT("PF_CraftingHints")})
        {
            auto* Widget=Menu->WidgetTree->FindWidget(Name);Test->TestNotNull(TEXT("Visible UI target"),Widget);if(!Widget){continue;}
            const auto G=Widget->GetCachedGeometry();const auto TL=Root.AbsoluteToLocal(G.LocalToAbsolute(FVector2D::ZeroVector)),BR=Root.AbsoluteToLocal(G.LocalToAbsolute(G.GetLocalSize()));
            Test->TestTrue(TEXT("Progression and controls inside viewport"),TL.X>=0 && TL.Y>=0 && BR.X<Size.X && BR.Y<Size.Y);
            if(auto* T=Cast<UTextBlock>(Widget)){Test->TestTrue(TEXT("Progression text fits row"),T->GetDesiredSize().Y<=G.GetLocalSize().Y+1);}
        }
    }
    void Press(FKey K){auto& A=FSlateApplication::Get();A.ProcessKeyDownEvent(FKeyEvent(K,FModifierKeysState(),0,false,0,0));A.ProcessKeyUpEvent(FKeyEvent(K,FModifierKeysState(),0,false,0,0));}
    void Shot(const TCHAR* Name){const FString Path=Directory/(FString(Name)+TEXT(".png"));Shots.Add(Path);FScreenshotRequest::RequestScreenshot(Path,true,false);}
    void Wait(double Now,double Seconds,int32 Next){Until=Now+Seconds;Stage=Next;}
    FAutomationTestBase* Test;FString Directory;TArray<FString> Shots;TWeakObjectPtr<APFSurvivalPlayerController> PC;TWeakObjectPtr<UPFCraftingHUD> Menu;TWeakObjectPtr<UPFProgressionComponent> Progression;
    double Started=FPlatformTime::Seconds(),Until=0;int32 Stage=0;float OldScale=1;bool bChanged=false;
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFProgressionFeedbackLiveTest,"PF.UI.ProgressionFeedbackLive",EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)
bool FPFProgressionFeedbackLiveTest::RunTest(const FString&)
{
    if(!FParse::Param(FCommandLine::Get(),TEXT("PFRunControlsUITest")) || FParse::Param(FCommandLine::Get(),TEXT("nullrhi"))){AddError(TEXT("Requires isolated rendered -game -PFRunControlsUITest"));return false;}
    FString Label;FParse::Value(FCommandLine::Get(),TEXT("PFControlsEvidence="),Label);
    if(Label.IsEmpty() || Label.Len()>48){AddError(TEXT("Supply unique bounded evidence label"));return false;}
    for(TCHAR C:Label){if(!FChar::IsAlnum(C) && C!=TEXT('_')){AddError(TEXT("Invalid evidence label"));return false;}}
    const FString Directory=FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()/TEXT("AutomationReports/ControlsUI")/Label);
    if(IFileManager::Get().DirectoryExists(*Directory)){AddError(TEXT("Refusing reused evidence"));return false;}IFileManager::Get().MakeDirectory(*Directory,true);
    ADD_LATENT_AUTOMATION_COMMAND(FProgressionFeedbackExercise(this,Directory));return true;
}
#endif
