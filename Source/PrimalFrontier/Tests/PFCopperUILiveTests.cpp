// Disposable rendered earned route. Teleports shorten travel; no item/XP grants or time acceleration.
#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Crafting/PFCraftingHUD.h"
#include "Crafting/PFCraftingComponent.h"
#include "Crafting/PFCraftingCatalog.h"
#include "Crafting/PFResourceNode.h"
#include "Inventory/PFInventoryComponent.h"
#include "Inventory/PFInventoryPlayerState.h"
#include "Progression/PFProgressionComponent.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFSurvivorCharacter.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Survival/PFInteraction.h"
#include "Survival/PFSurvivalHUD.h"
#include "Creatures/PFCreature.h"
#include "Creatures/PFCreatureSpawner.h"
#include "World/PFWorldClock.h"
#include "Settings/PFGameUserSettings.h"
#include "UI/PFItemPicture.h"
#include "Blueprint/WidgetTree.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "UnrealClient.h"
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"

namespace
{
class FCopperUI : public IAutomationLatentCommand
{
public:
    FCopperUI(FAutomationTestBase* T,FString D):Test(T),Directory(MoveTemp(D)){}
    ~FCopperUI(){if(Scaled && UPFGameUserSettings::Get()){UPFGameUserSettings::Get()->Preferences.HUDScale=OldScale;}}
    bool Update() override
    {
        const double Now=FPlatformTime::Seconds();if(Now-Started>155){Test->AddError(FString::Printf(TEXT("Copper rendered timeout step%d sub%d"),Step,Sub));return true;}
        if(Finished){if(Now-Changed<2){return false;}for(const auto& S:Shots){Test->TestTrue(TEXT("Rendered evidence exists"),IFileManager::Get().FileSize(*S)>0);}return true;}
        UWorld* W=nullptr;for(const auto& X:GEngine->GetWorldContexts()){if(X.World() && X.World()->IsGameWorld()){W=X.World();break;}}if(!W){return false;}
        APFSurvivalPlayerController* PC=nullptr;for(auto It=W->GetPlayerControllerIterator();It;++It){PC=Cast<APFSurvivalPlayerController>(It->Get());if(PC){break;}}
        if(!PC || !PC->GetInventory() || !PC->GetCrafting() || !PC->GetPawn()){return false;}
        auto* I=PC->GetInventory();auto* C=PC->GetCrafting();auto* P=CastChecked<APFSurvivorCharacter>(PC->GetPawn());auto* G=PC->GetPlayerState<APFInventoryPlayerState>()->Progression.Get();
        if(!Ready)
        {
            if(W->GetNetMode()!=NM_Standalone || UWorld::RemovePIEPrefix(W->GetMapName())!=TEXT("L_PrimalFrontier_OpenWorld") || !I->GetStacks().IsEmpty() || G->GetExperience()!=0){Test->AddError(TEXT("Requires fresh actual authored standalone map"));return true;}
            for(TActorIterator<APFResourceNode> It(W);It;++It){if(It->ActorHasTag(TEXT("PF.M12.CopperTest"))){Copper=*It;}if(It->GetName()==TEXT("PFResourceNode_0")){Wood=*It;}if(It->GetName()==TEXT("PFResourceNode_1")){Stone=*It;}if(It->GetName()==TEXT("PFResourceNode_11")){Fibre=*It;}}
            if(!Copper || !Wood || !Stone || !Fibre){Test->AddError(TEXT("Missing authored reachable mining/resource fixtures"));return true;}
            Test->TestTrue(TEXT("Copper authored near camp"),Copper->GetActorLocation().Equals(FVector(0,-850,60),1));
            for(TActorIterator<APFCreatureSpawner> It(W);It;++It){It->bAutoSpawn=false;}for(TActorIterator<APFCreature> It(W);It;++It){It->Destroy();}
            for(TActorIterator<APFWorldClock> It(W);It;++It){It->SetActorTickEnabled(false);It->SetHour(10);}
            P->Survival->HungerDrainPerSecond=0;P->Survival->ThirstDrainPerSecond=0;P->GetCharacterMovement()->DisableMovement();
            if(auto* S=UPFGameUserSettings::Get()){OldScale=S->Preferences.HUDScale;float Scale=1;FParse::Value(FCommandLine::Get(),TEXT("PFHUDScale="),Scale);S->Preferences.HUDScale=Scale;Scaled=true;}
            Ready=true;Changed=Now;return false;
        }
        if(Now-Changed<.65){return false;}
        if(Step==0 || Step==3)
        {
            if(Sub==0)
            {
                Aim(PC,Copper);const int32 Hits=Copper->HitsRemaining,XP=G->GetExperience();PC->Interact();
                Test->TestTrue(TEXT("Readable required Stone pick in actual first-person prompt"),PC->InteractionPrompt().Contains(TEXT("Requires Stone pick")));
                Test->TestTrue(TEXT("Bare/Wood request conserves node and XP"),Copper->HitsRemaining==Hits && G->GetExperience()==XP && I->Count(TEXT("Item_CopperOre"))==0);
                Sub=1;Changed=Now;return false; // Let the actual HUD tick before capture, not only the provider.
            }
            TArray<UUserWidget*> HUDs;UWidgetBlueprintLibrary::GetAllWidgetsOfClass(PC,HUDs,UPFSurvivalHUD::StaticClass(),false);
            auto* Label=HUDs.Num()==1?Cast<UTextBlock>(HUDs[0]->WidgetTree->FindWidget(TEXT("PF_InteractionLabel"))):nullptr;
            Test->TestTrue(TEXT("Required grade actually rendered in HUD"),Label && Label->GetText().ToString().Contains(TEXT("Requires Stone pick")) && Label->GetCachedGeometry().GetLocalSize().X>0);
            Shot(Step==0?TEXT("bare_requirement"):TEXT("wood_requirement"));Next(Now);return false;
        }
        if(Step==1 || Step==4 || Step==6 || Step==7 || Step==11 || Step==15)
        {
            APFResourceNode* Node=Step==1 || Step==7?Wood:Step==4?Stone:Step==11?Fibre:Copper;
            const int32 Count=Step==1 || Step==6?3:Step==7?2:1;
            if(Sub==0 && (Step==7 || Step==15) && Node->HitsRemaining!= (Step==7?3:6)){return false;}
            Aim(PC,Node);const int32 Before=Node->HitsRemaining;PC->Interact();if(!Test->TestTrue(TEXT("Actual first-person E interaction spends compatible hits"),Node->HitsRemaining<Before)){return true;}
            ++Sub;Changed=Now;if(Sub==Count){Next(Now);}return false;
        }
        if(Step==13)
        {
            if(Sub==0){if(!OpenSelect(PC,TEXT("Recipe_CopperPick"))){return true;}Sub=1;Changed=Now;return false;}
            auto* Button=Cast<UButton>(Menu->WidgetTree->FindWidget(TEXT("PF_CraftSelected")));Test->TestTrue(TEXT("Earned ingredients but actual knowledge gate disables craft"),Button && !Button->GetIsEnabled());
            const int32 Stat=G->GetAvailableStatPoints();Press(EKeys::K);Test->TestTrue(TEXT("Actual learn input buys FieldTools with earned XP"),G->GetRecord().Knowledge.Contains(TEXT("Tech_FieldTools")));Test->TestEqual(TEXT("Learn leaves separate stat budget"),G->GetAvailableStatPoints(),Stat);Shot(TEXT("earned_fieldtools"));PC->SetCraftingMenuOpen(false);Menu.Reset();Next(Now);return false;
        }
        if(Step==16)
        {
            Test->TestTrue(TEXT("Entire rendered route earned exact outputs without grants"),I->Count(TEXT("Item_CopperPick"))==1 && I->Count(TEXT("Item_CopperIngot"))==0 && I->Count(TEXT("Item_Cord"))==0 && I->Count(TEXT("Item_CopperOre"))==14 && G->GetExperience()==135 && Copper->HitsRemaining==2);
            Shot(TEXT("earned_pick_gather"));Test->AddInfo(TEXT("[PrimalAgentTools] Copper rendered authored trace/refusals, earned Wood/Stone/ore/refining/cord/FieldTools/pick, actual Slate selection/craft/learn and centered pictures verified; assisted travel, manual pacing remains separate"));Finished=true;Changed=Now;return false;
        }
        const FName Recipe=Step==2?TEXT("Recipe_WoodTool"):Step==5?TEXT("Recipe_Tool"):Step>=8 && Step<=10?TEXT("Recipe_CopperRefine"):Step==12?TEXT("Recipe_Cord"):TEXT("Recipe_CopperPick");
        if(Sub==0){if(!OpenSelect(PC,Recipe)){return true;}Sub=1;Changed=Now;return false;}
        if(Sub==1)
        {
            Test->TestEqual(TEXT("Actual Slate recipe selection"),Menu->GetSelectedRecipe(),Recipe);
            const auto* D=C->Catalog->Recipe(Recipe,I->Catalog);auto* Picture=Cast<UPFItemPicture>(Menu->WidgetTree->FindWidget(TEXT("PF_SelectedItemPicture")));
            Test->TestTrue(TEXT("Visible correct placeholder item picture"),D && Picture && Picture->GetItemId()==D->Output && Picture->GetCachedGeometry().GetLocalSize().X>0);
            const auto Root=Menu->GetCachedGeometry();auto* Panel=Menu->WidgetTree->FindWidget(TEXT("PF_CraftingPanel"));const auto PG=Panel->GetCachedGeometry();const auto TL=Root.AbsoluteToLocal(PG.LocalToAbsolute(FVector2D::ZeroVector)),BR=Root.AbsoluteToLocal(PG.LocalToAbsolute(PG.GetLocalSize())),Size=Root.GetLocalSize();
            Test->TestTrue(TEXT("Recipe panel centered and fits"),TL.X>=0 && TL.Y>=0 && BR.X<Size.X && BR.Y<Size.Y && FMath::Abs(TL.X+BR.X-Size.X)<4);
            if(Step==5 || Step==14){auto* Stats=Cast<UTextBlock>(Menu->WidgetTree->FindWidget(TEXT("PF_CraftingDetail_Stats")));Test->TestTrue(TEXT("Actual readable harvesting capability"),Stats && Stats->GetText().ToString().Contains(Step==5?TEXT("Stone pick"):TEXT("Copper pick")));}
            if(Step==5 || Step==8 || Step==14){Shot(Step==5?TEXT("stone_capability"):Step==8?TEXT("refine_selected"):TEXT("copper_pick_selected"));}
            Press(EKeys::Enter);if(!Test->TestEqual(TEXT("Actual craft input starts requested recipe"),C->ActiveRecipe,Recipe)){return true;}Sub=2;Changed=Now;return false;
        }
        if(C->ActiveRecipe.IsNone())
        {if(!Test->TestTrue(TEXT("Real elapsed timed craft completes"),C->Feedback.StartsWith(TEXT("Completed")))){return true;}PC->SetCraftingMenuOpen(false);Menu.Reset();Next(Now);}return false;
    }
private:
    void Next(double Now){++Step;Sub=0;Changed=Now;UE_LOG(LogTemp,Display,TEXT("[PrimalAgentTools] Copper rendered step%d"),Step);}
    void Aim(APFSurvivalPlayerController* PC,APFResourceNode* N){PC->SetCraftingMenuOpen(false);auto* P=PC->GetPawn().Get();P->SetActorLocation(N->GetActorLocation()+FVector(-190,0,40));FVector Eye;FRotator Look;P->GetActorEyesViewPoint(Eye,Look);PC->SetControlRotation((N->GetActorLocation()-Eye).Rotation());Test->TestTrue(TEXT("Authored target reached by actual eye trace"),PFInteraction::FindTarget(P)==N);}
    void Press(FKey K){auto& A=FSlateApplication::Get();A.ProcessKeyDownEvent(FKeyEvent(K,FModifierKeysState(),0,false,0,0));A.ProcessKeyUpEvent(FKeyEvent(K,FModifierKeysState(),0,false,0,0));}
    bool OpenSelect(APFSurvivalPlayerController* PC,FName Recipe)
    {
        PC->SetCraftingMenuOpen(true);TArray<UUserWidget*> Widgets;UWidgetBlueprintLibrary::GetAllWidgetsOfClass(PC,Widgets,UPFCraftingHUD::StaticClass(),false);
        if(Widgets.Num()!=1){Test->AddError(TEXT("Expected actual crafting modal"));return false;}Menu=CastChecked<UPFCraftingHUD>(Widgets[0]);Menu->SelectCategory(FGameplayTag());
        const int32 Index=Menu->GetVisibleRecipeIds().IndexOfByKey(Recipe);auto* B=Cast<UButton>(Menu->WidgetTree->FindWidget(FName(*FString::Printf(TEXT("PF_Recipe%d_Button"),Index))));
        if(!B){Test->AddError(TEXT("Missing actual appended recipe choice"));return false;}B->SetKeyboardFocus();Press(EKeys::SpaceBar);Menu->SetKeyboardFocus();return true;
    }
    void Shot(const TCHAR* Name){const FString Path=Directory/(FString(Name)+TEXT(".png"));Shots.Add(Path);FScreenshotRequest::RequestScreenshot(Path,true,false);}
    FAutomationTestBase* Test;FString Directory;TArray<FString> Shots;TWeakObjectPtr<UPFCraftingHUD> Menu;
    APFResourceNode* Copper=nullptr;APFResourceNode* Wood=nullptr;APFResourceNode* Stone=nullptr;APFResourceNode* Fibre=nullptr;
    int32 Step=0,Sub=0;bool Ready=false,Finished=false,Scaled=false;float OldScale=1;double Started=FPlatformTime::Seconds(),Changed=0;
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFCopperUITest,"PF.UI.CopperLive",EAutomationTestFlags::ClientContext|EAutomationTestFlags::EngineFilter)
bool FPFCopperUITest::RunTest(const FString&)
{
    FString Label;FParse::Value(FCommandLine::Get(),TEXT("PFControlsEvidence="),Label);
    if(!FParse::Param(FCommandLine::Get(),TEXT("PFRunCopperUITest")) || !FParse::Param(FCommandLine::Get(),TEXT("PFRunControlsUITest")) || FParse::Param(FCommandLine::Get(),TEXT("nullrhi")) || Label.IsEmpty() || Label.Len()>48){AddError(TEXT("Requires isolated rendered Copper controls runner"));return false;}
    for(TCHAR C:Label){if(!FChar::IsAlnum(C) && C!=TEXT('_')){return false;}}
    const FString D=FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()/TEXT("AutomationReports/ControlsUI")/Label);if(IFileManager::Get().DirectoryExists(*D)){AddError(TEXT("Refusing reused evidence"));return false;}IFileManager::Get().MakeDirectory(*D,true);
    ADD_LATENT_AUTOMATION_COMMAND(FCopperUI(this,D));return true;
}
#endif
