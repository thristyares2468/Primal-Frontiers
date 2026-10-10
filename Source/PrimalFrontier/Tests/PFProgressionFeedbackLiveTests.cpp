// Opt-in disposable rendered standalone feedback; not a human/controller acceptance test.
#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Crafting/PFCraftingHUD.h"
#include "Crafting/PFCraftingComponent.h"
#include "Crafting/PFResourceNode.h"
#include "Inventory/PFInventoryPlayerState.h"
#include "Inventory/PFItemPickup.h"
#include "Inventory/PFInventoryHUD.h"
#include "Creatures/PFCreature.h"
#include "Progression/PFProgressionComponent.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Survival/PFSurvivalHUD.h"
#include "Survival/PFInteraction.h"
#include "Settings/PFGameUserSettings.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "Components/InputComponent.h"
#include "Framework/Application/SlateApplication.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/DamageEvents.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
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
            Menu=CastChecked<UPFCraftingHUD>(Widgets[0]);Menu->SelectRecipe(TEXT("Recipe_BoundTool"));Wait(Now,.6,10);return false;
        }
        if(Stage==10)
        {
            Menu->RefreshMenu();Test->TestFalse(TEXT("Level one Learn button disabled"),Button(TEXT("PF_LearnKnowledge"))->GetIsEnabled());
            Test->TestFalse(TEXT("Locked optional Craft button disabled"),Button(TEXT("PF_CraftSelected"))->GetIsEnabled());
            Test->TestTrue(TEXT("Real level cost available requirement"),Text(TEXT("PF_KnowledgeRequirement")).Contains(TEXT("Level 2 | Cost 2 points | Available 0")));
            Test->TestTrue(TEXT("Real locked guide explains limited gathering and unique craft rewards"),Text(TEXT("PF_KnowledgeRequirement")).Contains(TEXT("Gather resources for limited XP")) && Text(TEXT("PF_KnowledgeRequirement")).Contains(TEXT("different recipes for first-craft XP")));
            Press(EKeys::K);Press(EKeys::Gamepad_DPad_Right);Press(EKeys::Enter);Test->TestEqual(TEXT("Locked UI never invents XP"),Progression->GetExperience(),0);
            Test->TestTrue(TEXT("Locked keys send no craft"),PC->GetCrafting()->ActiveRecipe.IsNone());Test->TestTrue(TEXT("Locked keys grant no knowledge"),Progression->GetRecord().Knowledge.IsEmpty());
            Bounds(true);Shot(TEXT("knowledge_level_locked"));Wait(Now,.6,16);return false;
        }
        if(Stage==16){Menu->SelectRecipe(TEXT("Recipe_Tool"));Wait(Now,.6,1);return false;}
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
            Test->TestTrue(TEXT("Cap recipe gives no XP"),Text(TEXT("PF_CraftingProgression_Reward")).Contains(TEXT("no extra XP")));Bounds();Shot(TEXT("level_cap"));Wait(Now,1,11);return false;
        }
        if(Stage==11)
        {
            // Explicit level boundary seed, not evidence that these two repeated crafts earned level two.
            auto Candidate=Progression->GetRecord();Candidate.Experience=100;FString Error;
            Test->TestTrue(TEXT("Trusted level two UI fixture"),Progression->Restore(Candidate,Error));
            Test->TestTrue(TEXT("Bound cord ingredient"),PC->GetInventory()->Grant(TEXT("Item_Cord"),1));Test->TestTrue(TEXT("Bound wood ingredients"),PC->GetInventory()->Grant(TEXT("Item_Wood"),2));
            Menu->SelectRecipe(TEXT("Recipe_BoundTool"));BringIntoView(TEXT("PF_KnowledgeRequirement"));Wait(Now,.6,12);return false;
        }
        if(Stage==12)
        {
            Menu->RefreshMenu();Test->TestTrue(TEXT("Learn button enabled at exact boundary"),Button(TEXT("PF_LearnKnowledge"))->GetIsEnabled());
            Test->TestFalse(TEXT("Craft stays locked until server grant"),Button(TEXT("PF_CraftSelected"))->GetIsEnabled());
            Test->TestTrue(TEXT("Real unspent points shown"),Text(TEXT("PF_KnowledgeRequirement")).Contains(TEXT("Cost 2 points | Available 3")));
            Bounds(true);Shot(TEXT("knowledge_available"));Wait(Now,.6,17);return false;
        }
        if(Stage==17){Button(TEXT("PF_LearnKnowledge"))->SetKeyboardFocus();Press(EKeys::SpaceBar);Wait(Now,.6,13);return false;}
        if(Stage==13)
        {
            if(!Progression->GetRecord().Knowledge.Contains(TEXT("Tech_FieldTools"))){return false;}
            Menu->RefreshMenu();Test->TestEqual(TEXT("Actual owned learn spends exactly two"),Progression->GetAvailablePoints(),1);
            Test->TestEqual(TEXT("Purchase earns no XP"),Progression->GetExperience(),100);Test->TestFalse(TEXT("Owned button disabled"),Button(TEXT("PF_LearnKnowledge"))->GetIsEnabled());
            Test->TestTrue(TEXT("Learned recipe Craft button enabled"),Button(TEXT("PF_CraftSelected"))->GetIsEnabled());
            Test->TestTrue(TEXT("Knowledge feedback truthful"),Text(TEXT("PF_CraftingPanel_Result")).Contains(TEXT("recipe access available")));
            Test->TestTrue(TEXT("Learned useful effect explicit"),Text(TEXT("PF_KnowledgeRequirement")).Contains(TEXT("learned. Recipe access available")));
            Bounds(true);Shot(TEXT("knowledge_learned"));Wait(Now,.6,18);return false;
        }
        if(Stage==18)
        {
            Menu->SetKeyboardFocus();Press(EKeys::K);Press(EKeys::Gamepad_DPad_Right);
            Test->TestEqual(TEXT("Repeated purchase controls don't spend again"),Progression->GetAvailablePoints(),1);Press(EKeys::Enter);Wait(Now,.6,14);return false;
        }
        if(Stage==14)
        {
            if(PC->GetInventory()->Count(TEXT("Item_BoundTool"))!=1 || !PC->GetCrafting()->ActiveRecipe.IsNone()){return false;}
            Test->TestEqual(TEXT("Learned timed completion earns first XP"),Progression->GetExperience(),120);
            Test->TestEqual(TEXT("Only one base tool converted"),PC->GetInventory()->Count(TEXT("Item_Tool")),1);
            Test->TestEqual(TEXT("Exact cord consumption"),PC->GetInventory()->Count(TEXT("Item_Cord")),0);Test->TestEqual(TEXT("Exact wood consumption"),PC->GetInventory()->Count(TEXT("Item_Wood")),0);
            Test->TestEqual(TEXT("No purchase or repeated-tool reward duplication"),Progression->GetRecord().CreditedCrafts.Num(),2);
            Menu->RefreshMenu();BringIntoView(TEXT("PF_CraftingProgression_Reward"));Wait(Now,.6,15);return false;
        }
        if(Stage==15)
        {BringIntoView(TEXT("PF_CraftingProgression_Reward"));Wait(Now,.6,19);return false;}
        if(Stage==19)
        {
            Test->TestTrue(TEXT("Bound first reward already earned"),Text(TEXT("PF_CraftingProgression_Reward")).Contains(TEXT("already earned")));
            Bounds();Shot(TEXT("learned_craft_complete"));Wait(Now,1,9);return false;
        }
        if(Stage==9)
        {
            Press(EKeys::C);Test->TestFalse(TEXT("Crafting closes through actual Slate key"),PC->IsCraftingOpen());PC->SetPauseMenuOpen(true);Test->TestTrue(TEXT("Pause opens afterward"),PC->IsPauseMenuOpen());PC->SetPauseMenuOpen(false);
            for(const auto& Path:Shots){Test->TestTrue(TEXT("Screenshot written"),IFileManager::Get().FileSize(*Path)>0);}
            Test->AddInfo(TEXT("[PrimalUI] Actual completed/cancelled/repeated craft feedback, locked/available/learned view, Slate Learn activation, exact server spend and learned timed conversion checked; level/cap XP are trusted fixture seeds, not earned pacing or physical-controller acceptance."));return true;
        }
        return false;
    }
private:
    FString Text(const TCHAR* Name){auto* T=Cast<UTextBlock>(Menu->WidgetTree->FindWidget(Name));Test->TestNotNull(TEXT("Actual progression label"),T);return T?T->GetText().ToString():FString();}
    void Check(int32 XP,const TCHAR* Reward)
    {
        Menu->RefreshMenu();Test->TestEqual(TEXT("Actual authoritative XP"),Progression->GetExperience(),XP);
        Test->TestTrue(TEXT("Actual XP shown"),Text(TEXT("PF_CraftingProgression_Summary")).Contains(FString::Printf(TEXT("%d / 100 XP"),XP)));
        Test->TestFalse(TEXT("No obsolete unavailable spending claim"),Text(TEXT("PF_CraftingProgression_Summary")).Contains(TEXT("spending not available yet")));
        Test->TestTrue(TEXT("Actual selected reward state"),Text(TEXT("PF_CraftingProgression_Reward")).Contains(Reward));Bounds();
    }
    UButton* Button(const TCHAR* Name){auto* B=Cast<UButton>(Menu->WidgetTree->FindWidget(Name));Test->TestNotNull(TEXT("Actual transaction button"),B);return B;}
    void BringIntoView(const TCHAR* Name)
    {auto* W=Menu->WidgetTree->FindWidget(Name);for(auto* P=W?W->GetParent():nullptr;P;P=P->GetParent()){if(auto* Scroll=Cast<UScrollBox>(P)){if(FString(Name)==TEXT("PF_CraftingProgression_Reward")){Scroll->ScrollToEnd();}else{Scroll->ScrollWidgetIntoView(W,false);}break;}}}
    void Bounds(bool bKnowledge=false)
    {
        const auto Root=Menu->GetCachedGeometry();const auto Size=Root.GetLocalSize();
        TArray<const TCHAR*> Targets={TEXT("PF_CraftingPanel"),TEXT("PF_CraftingProgression_Summary"),TEXT("PF_CraftSelected"),TEXT("PF_CraftingHints")};
        if(bKnowledge){Targets.Append({TEXT("PF_KnowledgeRequirement"),TEXT("PF_LearnKnowledge"),TEXT("PF_LearnKnowledge_Label")});}else{Targets.Add(TEXT("PF_CraftingProgression_Reward"));}
        for(const TCHAR* Name:Targets)
        {
            auto* Widget=Menu->WidgetTree->FindWidget(Name);Test->TestNotNull(TEXT("Visible UI target"),Widget);if(!Widget){continue;}
            const auto G=Widget->GetCachedGeometry();const auto TL=Root.AbsoluteToLocal(G.LocalToAbsolute(FVector2D::ZeroVector)),BR=Root.AbsoluteToLocal(G.LocalToAbsolute(G.GetLocalSize()));
            Test->TestTrue(TEXT("Progression and controls inside viewport"),TL.X>=0 && TL.Y>=0 && BR.X<Size.X && BR.Y<Size.Y);
            for(auto* Parent=Widget->GetParent();Parent;Parent=Parent->GetParent())
            {
                if(auto* Scroll=Cast<UScrollBox>(Parent))
                {
                    const auto Clip=Scroll->GetCachedGeometry();const auto ClipTL=Clip.AbsoluteToLocal(G.LocalToAbsolute(FVector2D::ZeroVector)),ClipBR=Clip.AbsoluteToLocal(G.LocalToAbsolute(G.GetLocalSize()));
                    Test->TestTrue(*FString::Printf(TEXT("%s stage%d inside scroll: top%.1f bottom%.1f height%.1f offset%.1f end%.1f"),Name,Stage,ClipTL.Y,ClipBR.Y,Clip.GetLocalSize().Y,Scroll->GetScrollOffset(),Scroll->GetScrollOffsetOfEnd()),ClipTL.Y>=-1 && ClipBR.Y<=Clip.GetLocalSize().Y+1);break;
                }
            }
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
namespace
{
// Fresh disposable route: every item comes from the normal interaction trace and XP from actual bounded gathering or timed conversion.
class FEarnedUpgradeExercise final : public IAutomationLatentCommand
{
public:
    FEarnedUpgradeExercise(FAutomationTestBase* T,FString D,bool Combat=false):Test(T),Directory(MoveTemp(D)),bCombat(Combat){}
    ~FEarnedUpgradeExercise(){if(Node.IsValid()){Node->Destroy();}if(Creature.IsValid()){Creature->Destroy();}if(bScaleChanged && UPFGameUserSettings::Get()){UPFGameUserSettings::Get()->Preferences.HUDScale=OldScale;}}
    bool Update() override
    {
        const double Now=FPlatformTime::Seconds();if(Now-Started>(bCombat?130:100)){Test->AddError(FString::Printf(TEXT("[PrimalAgentTools] Earned upgrade timeout stage%d craft%d gather%d"),Stage,CraftIndex,GatherIndex));return true;}if(Now<Until){return false;}
        if(Stage==0)
        {
            for(const auto& X:GEngine->GetWorldContexts()){if(X.World() && X.World()->IsGameWorld()){auto* Candidate=Cast<APFSurvivalPlayerController>(X.World()->GetFirstPlayerController());if(Candidate && Candidate->GetLocalPlayer() && Candidate->GetPawn() && Candidate->GetCrafting() && Candidate->GetInventory()){PC=Candidate;break;}}}
            if(!PC.IsValid()){return false;}auto* PS=PC->GetPlayerState<APFInventoryPlayerState>();
            if(!PC->HasAuthority() || PC->GetNetMode()!=NM_Standalone || !PC->GetInventory()->GetStacks().IsEmpty() || !PS || !PS->Progression || PS->Progression->GetExperience()!=0 || !PS->Progression->GetRecord().Knowledge.IsEmpty()){Test->AddError(TEXT("Refusing non-fresh earned-upgrade fixture"));return true;}
            Progression=PS->Progression;auto* Needs=PC->GetPawn()->FindComponentByClass<UPFPlayerSurvivalComponent>();if(!Needs){Test->AddError(TEXT("Missing survivor needs"));return true;}Needs->HungerDrainPerSecond=0;Needs->ThirstDrainPerSecond=0;
            PC->GetPawn()->SetActorLocation(FVector(-1200,0,100));PC->SetControlRotation(FRotator::ZeroRotator);if(auto* Character=Cast<ACharacter>(PC->GetPawn())){Character->GetCharacterMovement()->DisableMovement();}
            if(auto* S=UPFGameUserSettings::Get()){OldScale=S->Preferences.HUDScale;float Scale=1;FParse::Value(FCommandLine::Get(),TEXT("PFHUDScale="),Scale);S->Preferences.HUDScale=Scale;bScaleChanged=true;}
            Wait(Now,.6,1);return false;
        }
        auto* I=PC->GetInventory();auto* C=PC->GetCrafting();auto* G=Progression.Get();
        if(Stage==1)
        {
            if(GatherIndex==UE_ARRAY_COUNT(Resources))
            {
                if(!Test->TestTrue(TEXT("Normal bare-hand stock and60bounded gatherXP"),I->Count(TEXT("Item_Wood"))==12 && I->Count(TEXT("Item_Stone"))==2 && I->Count(TEXT("Item_Food"))==4 && I->Count(TEXT("Item_Fibre"))==8 && G->GetExperience()==60)){return true;}
                PC->SetCraftingMenuOpen(true);TArray<UUserWidget*> Widgets;UWidgetBlueprintLibrary::GetAllWidgetsOfClass(PC.Get(),Widgets,UPFCraftingHUD::StaticClass(),false);if(Widgets.Num()!=1){Test->AddError(TEXT("Missing actual earned-route menu"));return true;}
                Menu=CastChecked<UPFCraftingHUD>(Widgets[0]);Wait(Now,.6,2);return false;
            }
            if(!Node.IsValid() && !SpawnNode(Resources[GatherIndex])){return true;}
            const auto* Definition=Node->Catalog->Resource(Node->ResourceId,I->Catalog);if(!Definition){Test->AddError(TEXT("Missing real resource definition"));return true;}
            const int32 Before=I->Count(Definition->YieldItem),Hits=Node->HitsRemaining;
            if(!Test->TestTrue(TEXT("Owned trace selects finite resource"),PFInteraction::FindTarget(PC->GetPawn())==Node.Get())){return true;}
            PC->Interact();if(!Test->TestTrue(TEXT("Real E interaction yields only one bare-hand hit"),I->Count(Definition->YieldItem)==Before+2 && Node->HitsRemaining==Hits-1)){return true;}
            if(++GatherAction==Actions[GatherIndex]){Node->Destroy();Node.Reset();GatherAction=0;++GatherIndex;}Wait(Now,.65,1);return false;
        }
        if(Stage==2)
        {
            Menu->SelectRecipe(Recipes[CraftIndex]);Menu->RefreshMenu();BeforeOutput=I->Count(Outputs[CraftIndex]);Menu->SetKeyboardFocus();Press(EKeys::Enter);
            if(!Test->TestEqual(TEXT("Real menu starts chosen job"),C->ActiveRecipe,FName(Recipes[CraftIndex]))){return true;}Wait(Now,.6,3);return false;
        }
        if(Stage==3)
        {
            if(!C->ActiveRecipe.IsNone()){return false;}
            if(!Test->TestTrue(TEXT("Actual timed conversion and earned XP"),C->Feedback==TEXT("Completed") && I->Count(Outputs[CraftIndex])==BeforeOutput+1 && G->GetExperience()==Experience[CraftIndex])){return true;}
            if(CraftIndex<5){++CraftIndex;Wait(Now,.4,2);return false;}
            if(CraftIndex==5){Menu->SelectRecipe(TEXT("Recipe_BoundTool"));Scroll(false);Wait(Now,.6,4);return false;}
            if(!Test->TestTrue(TEXT("Exact full earned-route conservation"),I->Count(TEXT("Item_BoundTool"))==1 && I->Count(TEXT("Item_Tool"))==0 && I->Count(TEXT("Item_Cord"))==0 && I->Count(TEXT("Item_Wood"))==1 && I->Count(TEXT("Item_Stone"))==0 && I->Count(TEXT("Item_Fibre"))==0 && I->Count(TEXT("Item_Food"))==1 && I->Count(TEXT("Item_CookedFood"))==1 && I->Count(TEXT("Item_DriedFood"))==1 && I->Count(TEXT("Item_Club"))==1 && G->GetRecord().CreditedCrafts.Num()==6 && G->GetAvailablePoints()==1)){return true;}
            Menu->RefreshMenu();Scroll(true);Wait(Now,.6,9);return false;
        }
        if(Stage==4)
        {
            Menu->RefreshMenu();if(!Test->TestTrue(TEXT("Five distinct completions and repeated cord earn first level only"),G->GetExperience()==160 && G->GetLevel()==2 && G->GetAvailablePoints()==3 && G->GetRecord().CreditedCrafts.Num()==5 && Button(TEXT("PF_LearnKnowledge"))->GetIsEnabled() && !Button(TEXT("PF_CraftSelected"))->GetIsEnabled())){return true;}
            Test->TestTrue(TEXT("Actual160XP summary visible"),Text(TEXT("PF_CraftingProgression_Summary")).Contains(TEXT("Level 2 | 160 / 250 XP")));Bounds(TEXT("PF_KnowledgeRequirement"));Shot(TEXT("earned_level_two"));Wait(Now,.6,5);return false;
        }
        if(Stage==5){Button(TEXT("PF_LearnKnowledge"))->SetKeyboardFocus();Press(EKeys::SpaceBar);Wait(Now,.6,6);return false;}
        if(Stage==6)
        {
            Menu->RefreshMenu();if(!Test->TestTrue(TEXT("Actual owned learned access and exact two earned points"),G->GetRecord().Knowledge.Contains(TEXT("Tech_FieldTools")) && G->GetAvailablePoints()==1 && G->GetExperience()==160 && !Button(TEXT("PF_LearnKnowledge"))->GetIsEnabled() && Button(TEXT("PF_CraftSelected"))->GetIsEnabled())){return true;}
            Bounds(TEXT("PF_KnowledgeRequirement"));Shot(TEXT("earned_knowledge"));Wait(Now,.6,7);return false;
        }
        if(Stage==7){Menu->SetKeyboardFocus();Press(EKeys::K);Test->TestEqual(TEXT("Learn repeat costs nothing"),G->GetAvailablePoints(),1);CraftIndex=6;Wait(Now,.4,2);return false;}
        if(Stage==9){Scroll(true);Wait(Now,.6,10);return false;}
        if(Stage==10){Bounds(TEXT("PF_CraftingProgression_Reward"));Shot(TEXT("earned_bound_tool"));Wait(Now,.6,11);return false;}
        if(Stage==11){Menu->SetKeyboardFocus();Press(EKeys::C);if(!Test->TestFalse(TEXT("Actual menu closes and releases interaction"),PC->IsCraftingOpen()) || !SpawnNode(TEXT("Node_Fibre"))){return true;}Wait(Now,.6,12);return false;}
        if(Stage==12)
        {
            PC->Interact();if(!Test->TestTrue(TEXT("Earned tool grants finite three-hit benefit with one fifth fibre credit"),I->GatheringHits()==3 && I->Count(TEXT("Item_Fibre"))==6 && Node->HitsRemaining==0 && G->GetExperience()==185)){return true;}Node->Destroy();Node.Reset();Wait(Now,.6,13);return false;
        }
        if(Stage==13)
        {
            if(bCombat)
            {
                const FTransform At(FRotator::ZeroRotator,PC->GetPawn()->GetActorLocation()+FVector(100,0,20));Creature=PC->GetWorld()->SpawnActorDeferred<APFCreature>(APFCreature::StaticClass(),At,PC->GetPawn(),nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
                if(!Creature.IsValid()){Test->AddError(TEXT("Missing controlled combat creature"));return true;}Creature->CreatureId=TEXT("Creature_Prowler");Creature->FinishSpawning(At);Creature->SetActorTickEnabled(false);Creature->GetCharacterMovement()->DisableMovement();
                if(!Test->TestTrue(TEXT("Loaded real Prowler defaults, no damage or health override"),Creature->Definition() && Creature->Definition()->Damage==8 && Creature->Health==100 && Creature->bHostile)){return true;}
                Creature->Think();if(!Test->TestEqual(TEXT("Real visible attack windup starts"),Creature->State.ToString(),FString(TEXT("Creature.State.Attack")))){return true;}Wait(Now,.7,20);return false;
            }
            for(const auto& Path:Shots){Test->TestTrue(TEXT("Earned-route screenshot written"),IFileManager::Get().FileSize(*Path)>0);}
            Test->AddInfo(TEXT("[PrimalAgentTools] Rendered earned upgrade:13real bare-hand interactions/60bounded gatherXP,7timed jobs/6unique craft credits,160XP/3points→owned2point Learn→180XP/one bound tool→185XP/1point after fifth fibre credit; no XP/item grants. Synthetic UI,not human route/controller/FPS acceptance."));return true;
        }
        auto* Needs=PC->GetPawn()?PC->GetPawn()->FindComponentByClass<UPFPlayerSurvivalComponent>():nullptr;
        if(Stage==20)
        {
            const float Before=Needs->GetVitals().Health;Creature->Think();if(!Test->TestEqual(TEXT("Actual unprotected default windup lands eight damage"),Before-Needs->GetVitals().Health,8.f)){return true;}
            Creature->SetActorLocation(PC->GetPawn()->GetActorLocation()+FVector(0,500,20));Wait(Now,.65,21);return false;
        }
        if(Stage==21)
        {
            if(!SpawnNode(TEXT("Node_Fibre"))){return true;}PC->Interact();if(!Test->TestTrue(TEXT("Earned tool supplies remaining guard fibre"),I->Count(TEXT("Item_Fibre"))==12 && Node->HitsRemaining==0)){return true;}Node->Destroy();Node.Reset();Wait(Now,.65,22);return false;
        }
        if(Stage==22)
        {
            if(!SpawnNode(TEXT("Node_Wood"))){return true;}PC->Interact();if(!Test->TestTrue(TEXT("Earned tool supplies guard wood without grants"),I->Count(TEXT("Item_Wood"))==7 && Node->HitsRemaining==0)){return true;}Node->Destroy();Node.Reset();
            PC->SetCraftingMenuOpen(true);TArray<UUserWidget*> Widgets;UWidgetBlueprintLibrary::GetAllWidgetsOfClass(PC.Get(),Widgets,UPFCraftingHUD::StaticClass(),false);if(Widgets.Num()!=1){Test->AddError(TEXT("Missing real guard crafting menu"));return true;}Menu=CastChecked<UPFCraftingHUD>(Widgets[0]);Wait(Now,.6,23);return false;
        }
        if(Stage==23){Menu->SelectRecipe(TEXT("Recipe_Cord"));Menu->RefreshMenu();Menu->SetKeyboardFocus();Press(EKeys::Enter);if(!Test->TestEqual(TEXT("Repeated real cord job starts"),C->ActiveRecipe,FName(TEXT("Recipe_Cord")))){return true;}Wait(Now,.6,24);return false;}
        if(Stage==24)
        {
            if(!C->ActiveRecipe.IsNone()){return false;}if(!Test->TestTrue(TEXT("Repeated cord consumes four fibre with noXP"),C->Feedback==TEXT("Completed") && I->Count(TEXT("Item_Fibre"))==8 && I->Count(TEXT("Item_Cord"))==1 && G->GetExperience()==185)){return true;}
            Menu->SelectRecipe(TEXT("Recipe_WovenGuard"));Menu->RefreshMenu();Menu->SetKeyboardFocus();Press(EKeys::Enter);if(!Test->TestEqual(TEXT("Real guard job starts"),C->ActiveRecipe,FName(TEXT("Recipe_WovenGuard")))){return true;}Wait(Now,.6,25);return false;
        }
        if(Stage==25)
        {
            if(!C->ActiveRecipe.IsNone()){return false;}if(!Test->TestTrue(TEXT("Earned guard exact conversion and once-only20XP"),C->Feedback==TEXT("Completed") && I->Count(TEXT("Item_WovenGuard"))==1 && I->Count(TEXT("Item_Fibre"))==0 && I->Count(TEXT("Item_Cord"))==0 && I->Count(TEXT("Item_Wood"))==5 && I->CreatureHitReduction()==.25f && G->GetExperience()==205 && G->GetAvailablePoints()==1 && G->GetRecord().CreditedCrafts.Num()==7)){return true;}
            Menu->RefreshMenu();Scroll(true);Wait(Now,.6,26);return false;
        }
        if(Stage==26){Scroll(true);Wait(Now,.6,27);return false;}
        if(Stage==27){Bounds(TEXT("PF_CraftingProgression_Reward"));Shot(TEXT("earned_guard"));Wait(Now,.6,28);return false;}
        if(Stage==28){Menu->SetKeyboardFocus();Press(EKeys::C);Creature->SetActorLocation(PC->GetPawn()->GetActorLocation()+FVector(100,0,20));Creature->Think();if(!Test->TestEqual(TEXT("Default cooldown elapsed and second windup starts"),Creature->State.ToString(),FString(TEXT("Creature.State.Attack")))){return true;}Wait(Now,.7,29);return false;}
        if(Stage==29)
        {
            const float Before=Needs->GetVitals().Health;Creature->Think();if(!Test->TestEqual(TEXT("Actual default8damage with earned25percent guard loses six"),Before-Needs->GetVitals().Health,6.f)){return true;}Wait(Now,.4,30);return false;
        }
        if(Stage==30){Shot(TEXT("guarded_creature_hit"));Wait(Now,.6,31);return false;}
        if(Stage==31)
        {
            const float Before=Creature->Health,Stamina=Needs->GetVitals().Stamina;Action(EKeys::LeftMouseButton);
            if(!Test->TestTrue(TEXT("Actual first-person melee uses earned45damage and five stamina"),Creature->Health==FMath::Max(0.f,Before-45) && Stamina-Needs->GetVitals().Stamina==5)){return true;}
            const float After=Creature->Health,AfterStamina=Needs->GetVitals().Stamina;Action(EKeys::LeftMouseButton);Test->TestTrue(TEXT("Immediate repeated swing refused by cooldown"),Creature->Health==After && Needs->GetVitals().Stamina==AfterStamina);
            if(++Swing<3){Wait(Now,.65,31);return false;}if(!Test->TestTrue(TEXT("Three accepted earned-tool hits kill default100HP Prowler"),Creature->IsDead() && G->GetExperience()==205)){return true;}
            int32 Count=0;for(TActorIterator<APFItemPickup> It(PC->GetWorld());It;++It){if(FVector::DistSquared(It->GetActorLocation(),Creature->GetActorLocation()-FVector(0,0,25))<1){Loot=*It;++Count;}}
            if(!Test->TestTrue(TEXT("One default perishable creature loot batch"),Count==1 && Loot.IsValid() && Loot->GetContents().ItemId==TEXT("Item_Food") && Loot->GetContents().Quantity==3)){return true;}LootExpiry=Loot->GetContents().ExpiresAt;
            FVector Eye;FRotator Look;PC->GetPawn()->GetActorEyesViewPoint(Eye,Look);PC->SetControlRotation((Loot->GetActorLocation()-Eye).Rotation());Wait(Now,.65,32);return false;
        }
        if(Stage==32)
        {
            if(!Test->TestTrue(TEXT("Normal view trace selects creature loot"),PFInteraction::FindTarget(PC->GetPawn())==Loot.Get())){return true;}PC->Interact();
            if(!Test->TestTrue(TEXT("Normal pickup adds exactly three and keeps original expiry"),I->Count(TEXT("Item_Food"))==4 && I->GetStacks().ContainsByPredicate([&](const auto& S){return S.ItemId==TEXT("Item_Food") && S.Quantity==3 && S.ExpiresAt==LootExpiry;}) && !Loot.IsValid())){return true;}
            Wait(Now,.65,33);return false;
        }
        if(Stage==33)
        {
            Action(EKeys::LeftMouseButton);PC->Interact();if(!Test->TestTrue(TEXT("Corpse attacks/repeated pickup create no loot orXP"),I->Count(TEXT("Item_Food"))==4 && G->GetExperience()==205 && Creature->IsDead())){return true;}
            Action(EKeys::Tab);Wait(Now,.6,34);return false;
        }
        if(Stage==34)
        {
            if(!Test->TestTrue(TEXT("Bag opens after actual earned combat"),PC->IsInventoryOpen())){return true;}
            const int32 LootIndex=I->GetStacks().IndexOfByPredicate([&](const auto& S){return S.ItemId==TEXT("Item_Food") && S.Quantity==3 && S.ExpiresAt==LootExpiry;});
            if(!Test->TestTrue(TEXT("Recovered loot retains its distinct batch"),LootIndex!=INDEX_NONE)){return true;}
            for(int32 N=0;N<I->GetStacks().Num() && PC->GetSelectedInventoryIndex()!=LootIndex;++N){Action(EKeys::Down);}
            if(!Test->TestEqual(TEXT("Real inventory navigation selects recovered batch"),PC->GetSelectedInventoryIndex(),LootIndex)){return true;}Wait(Now,.6,36);return false;
        }
        if(Stage==36)
        {
            TArray<UUserWidget*> Widgets;UWidgetBlueprintLibrary::GetAllWidgetsOfClass(PC.Get(),Widgets,UPFInventoryHUD::StaticClass(),false);
            if(!Test->TestEqual(TEXT("Actual loot inventory overlay"),Widgets.Num(),1)){return true;}auto* HUD=Widgets[0];
            const auto* Body=Cast<UTextBlock>(HUD->WidgetTree->FindWidget(TEXT("PF_InventoryPanel_Body")));const auto* Detail=Cast<UTextBlock>(HUD->WidgetTree->FindWidget(TEXT("PF_InventoryDetailBody")));
            if(!Test->TestTrue(TEXT("Recovered three-food freshness row and consumable detail visible"),Body && Body->GetText().ToString().Contains(TEXT("Found food x3 [")) && Detail && Detail->GetText().ToString().Contains(TEXT("+35 food / +10 water")))){return true;}
            for(const auto* Label:{Body,Detail}){Test->TestTrue(TEXT("Loot text fits allocated height"),Label->GetDesiredSize().Y<=Label->GetCachedGeometry().GetLocalSize().Y+1);}
            Shot(TEXT("earned_creature_loot"));Wait(Now,.6,35);return false;
        }
        if(Stage==35)
        {
            Action(EKeys::Tab);DeathAt=PC->GetWorld()->GetTimeSeconds();FString Error;
            if(!Test->TestTrue(TEXT("Capture actual205XP before ordinary death"),G->Capture(BeforeDeath,Error))){return true;}
            InventoryBeforeDeath=I->GetStacks();DeadPawn=PC->GetPawn();
            if(!Test->TestTrue(TEXT("Normal lethal server TakeDamage enters death"),PC->GetPawn()->TakeDamage(1000,FDamageEvent(),PC.Get(),PC.Get())>0 && Needs && Needs->IsDead())){return true;}
            Wait(Now,.4,37);return false;
        }
        if(Stage==37)
        {
            if(!Test->TestTrue(TEXT("Default respawn delay leaves real dead pawn for HUD"),DeadPawn.IsValid() && PC->GetPawn()==DeadPawn.Get() && Needs && Needs->IsDead() && Needs->GetVitals().Health==0)){return true;}
            CheckSurvivalHUD(true);Shot(TEXT("earned_death"));Wait(Now,.6,38);return false;
        }
        if(Stage==38)
        {
            if(!PC->GetPawn() || PC->GetPawn()==DeadPawn.Get()){return false;}
            auto* PS=PC->GetPlayerState<APFInventoryPlayerState>();FPFProgressionRecord AfterDeath;FString Error;
            if(!Test->TestTrue(TEXT("Default respawn keeps real owner PlayerState and progression"),PS && PS->Progression==G && Needs && !Needs->IsDead() && Needs->GetVitals().Health==100 && G->Capture(AfterDeath,Error))){return true;}
            bool SameInventory=I->GetStacks().Num()==InventoryBeforeDeath.Num();
            for(const auto& Before:InventoryBeforeDeath){SameInventory&=I->GetStacks().ContainsByPredicate([&](const auto& After){return After.StackId==Before.StackId && After.ItemId==Before.ItemId && After.Quantity==Before.Quantity && After.ExpiresAt==Before.ExpiresAt;});}
            Test->TestTrue(TEXT("Ordinary death preserves every earned stack ID quantity and original freshness"),SameInventory);
            Test->TestTrue(TEXT("Respawn preserves205XP exact learned ledger and one point"),AfterDeath.Experience==205 && AfterDeath.Knowledge==BeforeDeath.Knowledge && AfterDeath.CreditedCrafts==BeforeDeath.CreditedCrafts && G->GetAvailablePoints()==1 && G->CanCraftRecipe(TEXT("Recipe_BoundTool"),Error));
            Test->TestEqual(TEXT("Respawn category bucket cardinality unchanged"),AfterDeath.GatherWindows.Num(),BeforeDeath.GatherWindows.Num());
            for(const auto& Before:BeforeDeath.GatherWindows){const auto* After=AfterDeath.GatherWindows.FindByPredicate([&](const auto& X){return X.Category==Before.Category;});Test->TestTrue(TEXT("Respawn retains earned counts and ages active clock without renewal"),After && After->Rewards==Before.Rewards && FMath::IsNearlyEqual(After->RemainingSeconds,Before.RemainingSeconds-(PC->GetWorld()->GetTimeSeconds()-DeathAt),.00001));}
            Needs->HungerDrainPerSecond=0;Needs->ThirstDrainPerSecond=0;
            if(auto* Character=Cast<ACharacter>(PC->GetPawn())){Character->GetCharacterMovement()->DisableMovement();}
            Wait(Now,.4,39);return false;
        }
        if(Stage==39)
        {
            CheckSurvivalHUD(false);Shot(TEXT("earned_respawn"));Wait(Now,.6,42);return false;
        }
        if(Stage==42)
        {
            PC->SetCraftingMenuOpen(true);
            TArray<UUserWidget*> Widgets;UWidgetBlueprintLibrary::GetAllWidgetsOfClass(PC.Get(),Widgets,UPFCraftingHUD::StaticClass(),false);
            if(!Test->TestEqual(TEXT("Actual crafting menu reopens on new pawn"),Widgets.Num(),1)){return true;}
            Menu=CastChecked<UPFCraftingHUD>(Widgets[0]);Menu->SelectRecipe(TEXT("Recipe_BoundTool"));Wait(Now,.6,40);return false;
        }
        if(Stage==40)
        {
            Menu->RefreshMenu();Test->TestTrue(TEXT("Postrespawn actual205XP one-point menu visible"),Text(TEXT("PF_CraftingProgression_Summary")).Contains(TEXT("Level 2 | 205 / 250 XP | 45 to next level | 1 knowledge point")));
            Test->TestTrue(TEXT("Postrespawn earned knowledge stays learned"),Text(TEXT("PF_KnowledgeRequirement")).Contains(TEXT("learned. Recipe access available")) && !Button(TEXT("PF_LearnKnowledge"))->GetIsEnabled());
            Bounds(TEXT("PF_CraftingProgression_Summary"));Shot(TEXT("earned_respawn_knowledge"));Wait(Now,.6,41);return false;
        }
        if(Stage==41)
        {
            for(const auto& Path:Shots){Test->TestTrue(TEXT("Earned combat/death/respawn screenshot written"),IFileManager::Get().FileSize(*Path)>0);}
            Menu->SetKeyboardFocus();Press(EKeys::C);Test->TestFalse(TEXT("Reopened menu closes on new pawn"),PC->IsCraftingOpen());
            Test->AddInfo(TEXT("[PrimalAgentTools] Earned guard/default Prowler,205XP/one point,normal melee/loot and ordinary lethal TakeDamage/default respawn checked: complete inventory IDs/original freshness,knowledge/firstcraft ledger,category counts and active-clock aging conserved; actual death/100HP/205XP HUD inspected. No grants, forced respawn or shortened delay; synthetic standalone,not new multiplayer/navigation/human/controller/FPS acceptance."));return true;
        }
        return false;
    }
private:
    void CheckSurvivalHUD(bool Dead)
    {
        TArray<UUserWidget*> Widgets;UWidgetBlueprintLibrary::GetAllWidgetsOfClass(PC.Get(),Widgets,UPFSurvivalHUD::StaticClass(),false);
        if(!Test->TestEqual(TEXT("Actual survivor HUD survives possession lifecycle"),Widgets.Num(),1)){return;}
        const auto* Health=Cast<UTextBlock>(Widgets[0]->WidgetTree->FindWidget(TEXT("PF_HealthLabel")));const auto* State=Cast<UTextBlock>(Widgets[0]->WidgetTree->FindWidget(TEXT("PF_StateLabel")));
        Test->TestTrue(TEXT("Actual lifecycle health and state text truthful"),Health && State && Health->GetText().ToString()==(Dead?TEXT("HEALTH   0 / 100"):TEXT("HEALTH   100 / 100")) && State->GetText().ToString().Contains(TEXT("You died"))==Dead);
        for(const auto* Label:{Health,State}){if(Label && Label->GetVisibility()!=ESlateVisibility::Collapsed){const auto Size=Label->GetCachedGeometry().GetLocalSize();Test->TestTrue(TEXT("Lifecycle HUD text allocated visibly"),Size.X>0 && Size.Y>0 && Label->GetDesiredSize().Y<=Size.Y+1);}}
    }
    bool SpawnNode(FName Resource)
    {FVector Eye;FRotator Look;PC->GetPawn()->GetActorEyesViewPoint(Eye,Look);const FTransform At(FRotator::ZeroRotator,Eye+Look.Vector()*150);Node=PC->GetWorld()->SpawnActorDeferred<APFResourceNode>(APFResourceNode::StaticClass(),At,PC->GetPawn(),nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);if(!Node.IsValid()){Test->AddError(TEXT("Unable to spawn disposable node"));return false;}Node->ResourceId=Resource;Node->FinishSpawning(At);return true;}
    UButton* Button(const TCHAR* Name){return CastChecked<UButton>(Menu->WidgetTree->FindWidget(Name));}
    FString Text(const TCHAR* Name){return CastChecked<UTextBlock>(Menu->WidgetTree->FindWidget(Name))->GetText().ToString();}
    void Action(FKey Key){for(const auto& B:PC->InputComponent->KeyBindings){if(B.Chord.Key==Key && B.KeyEvent==IE_Pressed){B.KeyDelegate.Execute(Key);return;}}Test->AddError(TEXT("Missing actual action binding ")+Key.ToString());}
    void Press(FKey Key){auto& Slate=FSlateApplication::Get();Slate.ProcessKeyDownEvent(FKeyEvent(Key,FModifierKeysState(),0,false,0,0));Slate.ProcessKeyUpEvent(FKeyEvent(Key,FModifierKeysState(),0,false,0,0));}
    void Scroll(bool End){auto* W=Menu->WidgetTree->FindWidget(End?TEXT("PF_CraftingProgression_Reward"):TEXT("PF_KnowledgeRequirement"));for(auto* P=W->GetParent();P;P=P->GetParent()){if(auto* S=Cast<UScrollBox>(P)){if(End){S->ScrollToEnd();}else{S->ScrollToStart();}break;}}}
    void Bounds(const TCHAR* Name)
    {
        auto* W=Menu->WidgetTree->FindWidget(Name);const auto G=W->GetCachedGeometry();const auto Root=Menu->GetCachedGeometry();const auto TL=Root.AbsoluteToLocal(G.LocalToAbsolute(FVector2D::ZeroVector)),BR=Root.AbsoluteToLocal(G.LocalToAbsolute(G.GetLocalSize()));
        Test->TestTrue(TEXT("Earned selected feedback inside screen"),TL.X>=0 && TL.Y>=0 && BR.X<Root.GetLocalSize().X && BR.Y<Root.GetLocalSize().Y);Test->TestTrue(TEXT("Earned text fits allocated height"),W->GetDesiredSize().Y<=G.GetLocalSize().Y+1);
        for(auto* P=W->GetParent();P;P=P->GetParent()){if(auto* S=Cast<UScrollBox>(P)){const auto Clip=S->GetCachedGeometry();const auto T=Clip.AbsoluteToLocal(G.LocalToAbsolute(FVector2D::ZeroVector)),B=Clip.AbsoluteToLocal(G.LocalToAbsolute(G.GetLocalSize()));Test->TestTrue(TEXT("Earned feedback inside inner scroll"),T.Y>=-1 && B.Y<=Clip.GetLocalSize().Y+1);break;}}
    }
    void Shot(const TCHAR* Name){const FString Path=Directory/(FString(Name)+TEXT(".png"));Shots.Add(Path);FScreenshotRequest::RequestScreenshot(Path,true,false);}
    void Wait(double Now,double Seconds,int32 Next){Until=Now+Seconds;Stage=Next;}
    FAutomationTestBase* Test;FString Directory;TArray<FString> Shots;TWeakObjectPtr<APFSurvivalPlayerController> PC;TWeakObjectPtr<UPFCraftingHUD> Menu;TWeakObjectPtr<UPFProgressionComponent> Progression;TWeakObjectPtr<APFResourceNode> Node;TWeakObjectPtr<APFCreature> Creature;TWeakObjectPtr<APFItemPickup> Loot;
    bool bCombat=false;int32 Swing=0;double LootExpiry=0;
    FPFProgressionRecord BeforeDeath;TArray<FPFItemStack> InventoryBeforeDeath;TWeakObjectPtr<APawn> DeadPawn;double DeathAt=0;
    double Started=FPlatformTime::Seconds(),Until=0;int32 Stage=0,GatherIndex=0,GatherAction=0,CraftIndex=0,BeforeOutput=0;float OldScale=1;bool bScaleChanged=false;
    const FName Resources[6]={TEXT("Node_Wood"),TEXT("Node_Wood"),TEXT("Node_Stone"),TEXT("Node_Food"),TEXT("Node_Fibre"),TEXT("Node_Fibre")};const int32 Actions[6]={3,3,1,2,3,1};
    const TCHAR* Recipes[7]={TEXT("Recipe_Tool"),TEXT("Recipe_Cook"),TEXT("Recipe_Dry"),TEXT("Recipe_Cord"),TEXT("Recipe_Club"),TEXT("Recipe_Cord"),TEXT("Recipe_BoundTool")};
    const FName Outputs[7]={TEXT("Item_Tool"),TEXT("Item_CookedFood"),TEXT("Item_DriedFood"),TEXT("Item_Cord"),TEXT("Item_Club"),TEXT("Item_Cord"),TEXT("Item_BoundTool")};const int32 Experience[7]={80,100,120,140,160,160,180};
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFEarnedUpgradeLiveTest,"PF.Progression.EarnedUpgradeLive",EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)
bool FPFEarnedUpgradeLiveTest::RunTest(const FString&)
{
    if(!FParse::Param(FCommandLine::Get(),TEXT("PFRunControlsUITest")) || !FParse::Param(FCommandLine::Get(),TEXT("PFRunEarnedUpgradeTest")) || FParse::Param(FCommandLine::Get(),TEXT("nullrhi"))){AddError(TEXT("Requires isolated rendered -game -PFRunControlsUITest -PFRunEarnedUpgradeTest"));return false;}
    FString Label;FParse::Value(FCommandLine::Get(),TEXT("PFControlsEvidence="),Label);if(Label.IsEmpty() || Label.Len()>48){AddError(TEXT("Supply bounded earned evidence label"));return false;}
    for(TCHAR C:Label){if(!FChar::IsAlnum(C) && C!=TEXT('_')){AddError(TEXT("Invalid earned evidence label"));return false;}}
    const FString Directory=FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()/TEXT("AutomationReports/ControlsUI")/Label);if(IFileManager::Get().DirectoryExists(*Directory)){AddError(TEXT("Refusing reused earned evidence"));return false;}IFileManager::Get().MakeDirectory(*Directory,true);
    ADD_LATENT_AUTOMATION_COMMAND(FEarnedUpgradeExercise(this,Directory));return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFEarnedCombatLiveTest,"PF.Progression.EarnedCombatLive",EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)
bool FPFEarnedCombatLiveTest::RunTest(const FString&)
{
    if(!FParse::Param(FCommandLine::Get(),TEXT("PFRunControlsUITest")) || !FParse::Param(FCommandLine::Get(),TEXT("PFRunEarnedCombatTest")) || FParse::Param(FCommandLine::Get(),TEXT("nullrhi"))){AddError(TEXT("Requires isolated rendered -game -PFRunControlsUITest -PFRunEarnedCombatTest"));return false;}
    FString Label;FParse::Value(FCommandLine::Get(),TEXT("PFControlsEvidence="),Label);if(Label.IsEmpty() || Label.Len()>48){AddError(TEXT("Supply bounded earned combat evidence label"));return false;}for(TCHAR C:Label){if(!FChar::IsAlnum(C) && C!=TEXT('_')){AddError(TEXT("Invalid earned combat label"));return false;}}
    const FString Directory=FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()/TEXT("AutomationReports/ControlsUI")/Label);if(IFileManager::Get().DirectoryExists(*Directory)){AddError(TEXT("Refusing reused earned combat evidence"));return false;}IFileManager::Get().MakeDirectory(*Directory,true);
    ADD_LATENT_AUTOMATION_COMMAND(FEarnedUpgradeExercise(this,Directory,true));return true;
}
#endif
