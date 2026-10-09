// Opt-in disposable rendered standalone feedback; not a human/controller acceptance test.
#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Crafting/PFCraftingHUD.h"
#include "Crafting/PFCraftingComponent.h"
#include "Crafting/PFResourceNode.h"
#include "Inventory/PFInventoryPlayerState.h"
#include "Progression/PFProgressionComponent.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Survival/PFInteraction.h"
#include "Settings/PFGameUserSettings.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "Framework/Application/SlateApplication.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
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
// Fresh disposable route: every item comes from the normal interaction trace and every XP from a timed conversion.
class FEarnedUpgradeExercise final : public IAutomationLatentCommand
{
public:
    FEarnedUpgradeExercise(FAutomationTestBase* T,FString D):Test(T),Directory(MoveTemp(D)){}
    ~FEarnedUpgradeExercise(){if(Node.IsValid()){Node->Destroy();}if(bScaleChanged && UPFGameUserSettings::Get()){UPFGameUserSettings::Get()->Preferences.HUDScale=OldScale;}}
    bool Update() override
    {
        const double Now=FPlatformTime::Seconds();if(Now-Started>100){Test->AddError(FString::Printf(TEXT("[PrimalAgentTools] Earned upgrade timeout stage%d craft%d gather%d"),Stage,CraftIndex,GatherIndex));return true;}if(Now<Until){return false;}
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
                if(!Test->TestTrue(TEXT("Normal bare-hand input stock and zero reward"),I->Count(TEXT("Item_Wood"))==12 && I->Count(TEXT("Item_Stone"))==2 && I->Count(TEXT("Item_Food"))==4 && I->Count(TEXT("Item_Fibre"))==8 && G->GetExperience()==0)){return true;}
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
            Menu->RefreshMenu();if(!Test->TestTrue(TEXT("Five distinct completions and repeated cord earn first level only"),G->GetExperience()==100 && G->GetLevel()==2 && G->GetAvailablePoints()==3 && G->GetRecord().CreditedCrafts.Num()==5 && Button(TEXT("PF_LearnKnowledge"))->GetIsEnabled() && !Button(TEXT("PF_CraftSelected"))->GetIsEnabled())){return true;}
            Test->TestTrue(TEXT("Actual100XP summary visible"),Text(TEXT("PF_CraftingProgression_Summary")).Contains(TEXT("Level 2 | 100 / 250 XP")));Bounds(TEXT("PF_KnowledgeRequirement"));Shot(TEXT("earned_level_two"));Wait(Now,.6,5);return false;
        }
        if(Stage==5){Button(TEXT("PF_LearnKnowledge"))->SetKeyboardFocus();Press(EKeys::SpaceBar);Wait(Now,.6,6);return false;}
        if(Stage==6)
        {
            Menu->RefreshMenu();if(!Test->TestTrue(TEXT("Actual owned learned access and exact two earned points"),G->GetRecord().Knowledge.Contains(TEXT("Tech_FieldTools")) && G->GetAvailablePoints()==1 && G->GetExperience()==100 && !Button(TEXT("PF_LearnKnowledge"))->GetIsEnabled() && Button(TEXT("PF_CraftSelected"))->GetIsEnabled())){return true;}
            Bounds(TEXT("PF_KnowledgeRequirement"));Shot(TEXT("earned_knowledge"));Wait(Now,.6,7);return false;
        }
        if(Stage==7){Menu->SetKeyboardFocus();Press(EKeys::K);Test->TestEqual(TEXT("Learn repeat costs nothing"),G->GetAvailablePoints(),1);CraftIndex=6;Wait(Now,.4,2);return false;}
        if(Stage==9){Scroll(true);Wait(Now,.6,10);return false;}
        if(Stage==10){Bounds(TEXT("PF_CraftingProgression_Reward"));Shot(TEXT("earned_bound_tool"));Wait(Now,.6,11);return false;}
        if(Stage==11){Menu->SetKeyboardFocus();Press(EKeys::C);if(!Test->TestFalse(TEXT("Actual menu closes and releases interaction"),PC->IsCraftingOpen()) || !SpawnNode(TEXT("Node_Fibre"))){return true;}Wait(Now,.6,12);return false;}
        if(Stage==12)
        {
            PC->Interact();if(!Test->TestTrue(TEXT("Earned tool grants finite three-hit benefit without extra XP"),I->GatheringHits()==3 && I->Count(TEXT("Item_Fibre"))==6 && Node->HitsRemaining==0 && G->GetExperience()==120)){return true;}Node->Destroy();Node.Reset();Wait(Now,.6,13);return false;
        }
        if(Stage==13)
        {
            for(const auto& Path:Shots){Test->TestTrue(TEXT("Earned-route screenshot written"),IFileManager::Get().FileSize(*Path)>0);}
            Test->AddInfo(TEXT("[PrimalAgentTools] Rendered earned upgrade:13real bare-hand interactions,7timed jobs/6unique credits,100XP/3points→owned2point Learn→120XP/1point/one bound tool,actual three-hit gathering; no XP/item grants. Synthetic UI,not human route/controller/FPS acceptance."));return true;
        }
        return false;
    }
private:
    bool SpawnNode(FName Resource)
    {FVector Eye;FRotator Look;PC->GetPawn()->GetActorEyesViewPoint(Eye,Look);const FTransform At(FRotator::ZeroRotator,Eye+Look.Vector()*150);Node=PC->GetWorld()->SpawnActorDeferred<APFResourceNode>(APFResourceNode::StaticClass(),At,PC->GetPawn(),nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);if(!Node.IsValid()){Test->AddError(TEXT("Unable to spawn disposable node"));return false;}Node->ResourceId=Resource;Node->FinishSpawning(At);return true;}
    UButton* Button(const TCHAR* Name){return CastChecked<UButton>(Menu->WidgetTree->FindWidget(Name));}
    FString Text(const TCHAR* Name){return CastChecked<UTextBlock>(Menu->WidgetTree->FindWidget(Name))->GetText().ToString();}
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
    FAutomationTestBase* Test;FString Directory;TArray<FString> Shots;TWeakObjectPtr<APFSurvivalPlayerController> PC;TWeakObjectPtr<UPFCraftingHUD> Menu;TWeakObjectPtr<UPFProgressionComponent> Progression;TWeakObjectPtr<APFResourceNode> Node;
    double Started=FPlatformTime::Seconds(),Until=0;int32 Stage=0,GatherIndex=0,GatherAction=0,CraftIndex=0,BeforeOutput=0;float OldScale=1;bool bScaleChanged=false;
    const FName Resources[6]={TEXT("Node_Wood"),TEXT("Node_Wood"),TEXT("Node_Stone"),TEXT("Node_Food"),TEXT("Node_Fibre"),TEXT("Node_Fibre")};const int32 Actions[6]={3,3,1,2,3,1};
    const TCHAR* Recipes[7]={TEXT("Recipe_Tool"),TEXT("Recipe_Cook"),TEXT("Recipe_Dry"),TEXT("Recipe_Cord"),TEXT("Recipe_Club"),TEXT("Recipe_Cord"),TEXT("Recipe_BoundTool")};
    const FName Outputs[7]={TEXT("Item_Tool"),TEXT("Item_CookedFood"),TEXT("Item_DriedFood"),TEXT("Item_Cord"),TEXT("Item_Club"),TEXT("Item_Cord"),TEXT("Item_BoundTool")};const int32 Experience[7]={20,40,60,80,100,100,120};
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
#endif
