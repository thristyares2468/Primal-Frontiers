#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Crafting/PFCraftingHUD.h"
#include "Crafting/PFCraftingComponent.h"
#include "Crafting/PFResourceNode.h"
#include "Inventory/PFInventoryComponent.h"
#include "Inventory/PFInventoryPlayerState.h"
#include "Progression/PFProgressionComponent.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFSurvivorCharacter.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Survival/PFInteraction.h"
#include "Creatures/PFCreature.h"
#include "Creatures/PFCreatureSpawner.h"
#include "World/PFWorldClock.h"
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
class FEryndorLive : public IAutomationLatentCommand
{
public:
    FEryndorLive(FAutomationTestBase* InTest,FString InDirectory):T(InTest),Directory(MoveTemp(InDirectory)){}
    bool Update() override
    {
        const double Now=FPlatformTime::Seconds();if(Now-Started>205){T->AddError(FString::Printf(TEXT("Eryndor timeout stage%d"),Step));return true;}
        if(Finished){if(Now-Changed<2){return false;}for(const auto& S:Shots){T->TestTrue(TEXT("Rendered screenshot exists"),IFileManager::Get().FileSize(*S)>0);}return true;}
        UWorld* W=nullptr;for(const auto& C:GEngine->GetWorldContexts()){if(C.World() && C.World()->IsGameWorld()){W=C.World();break;}}if(!W){return false;}
        APFSurvivalPlayerController* PC=nullptr;for(auto It=W->GetPlayerControllerIterator();It;++It){PC=Cast<APFSurvivalPlayerController>(It->Get());if(PC){break;}}
        auto* P=PC?Cast<APFSurvivorCharacter>(PC->GetPawn()):nullptr;auto* PS=PC?PC->GetPlayerState<APFInventoryPlayerState>():nullptr;
        if(!P || !PS || !PC->GetInventory() || !PC->GetCrafting()){return false;}auto* I=PC->GetInventory();auto* C=PC->GetCrafting();auto* G=PS->Progression.Get();
        if(!Ready)
        {
            if(W->GetNetMode()!=NM_Standalone || UWorld::RemovePIEPrefix(W->GetMapName())!=TEXT("L_PrimalFrontier_OpenWorld") || !I->GetStacks().IsEmpty()){T->AddError(TEXT("Fresh actual authored solo world required"));return true;}
            int32 Count=0;for(TActorIterator<APFResourceNode> It(W);It;++It){if(It->ActorHasTag(TEXT("PF.M12.EryndorTest"))){++Count;if(It->ResourceId==TEXT("Node_Diamond")){Diamond=*It;}if(It->ResourceId==TEXT("Node_Titanium")){Titanium=*It;}T->TestTrue(TEXT("Saved new resource configuration valid"),It->IsConfigurationValid());}}
            if(!T->TestTrue(TEXT("All nine saved fixtures"),Count==9 && Diamond && Titanium)){return true;}
            T->TestTrue(TEXT("Titanium exact reviewed location"),Titanium->GetActorLocation().Equals(FVector(-2600,2000,60),1));
            for(TActorIterator<APFCreatureSpawner> It(W);It;++It){It->bAutoSpawn=false;}for(TActorIterator<APFCreature> It(W);It;++It){It->Destroy();}
            for(TActorIterator<APFWorldClock> It(W);It;++It){It->SetActorTickEnabled(false);It->SetHour(10);}
            P->Survival->HungerDrainPerSecond=0;P->Survival->ThirstDrainPerSecond=0;P->GetCharacterMovement()->DisableMovement();
            FPFProgressionRecord Boundary;Boundary.Experience=700;FString Error;if(!G->Restore(Boundary,Error) || !I->Grant(TEXT("Item_CopperPick"),1)){T->AddError(TEXT("Boundary seed failed"));return true;}
            Ready=true;Changed=Now;return false;
        }
        if(Now-Changed<.7){return false;}
        if(Step==0)
        {
            Aim(PC,Diamond);const int32 Before=Diamond->HitsRemaining;PC->Interact();T->TestTrue(TEXT("Actual Copper cannot mine Diamond; clear first-person refusal"),Diamond->HitsRemaining==Before && PC->InteractionPrompt().Contains(TEXT("Requires Iron pick")));
            Shot(TEXT("diamond_refusal"));Next(Now);return false;
        }
        if(Step>=1 && Step<=4)
        {
            static const FName Recipes[]={TEXT("Recipe_BoundTool"),TEXT("Recipe_IronRefine"),TEXT("Recipe_DiamondPick"),TEXT("Recipe_TitaniumRefine")};
            static const FName Knowledge[]={TEXT("Tech_FieldTools"),TEXT("Tech_Metalwork"),TEXT("Tech_Gemwork"),TEXT("Tech_Titanium")};
            if(Sub==0)
            {if(!Select(PC,Recipes[Step-1],true)){return true;}Sub=1;Changed=Now;return false;}
            const int32 Stats=G->GetAvailableStatPoints();Press(EKeys::K);
            if(!T->TestTrue(TEXT("Actual Slate Learn purchases server-owned branch"),G->GetRecord().Knowledge.Contains(Knowledge[Step-1]))){return true;}
            T->TestEqual(TEXT("Separate stat budget unchanged"),G->GetAvailableStatPoints(),Stats);
            if(Step==4)
            {
                auto* Reward=Cast<UTextBlock>(Menu->WidgetTree->FindWidget(TEXT("PF_CraftingProgression_Reward")));
                T->TestTrue(TEXT("Learning description promises no free items/XP"),Reward && Reward->GetText().ToString().Contains(TEXT("no items or XP")));
                T->TestEqual(TEXT("Learning did not credit XP"),G->GetExperience(),700);
                Shot(TEXT("technology_tree"));Press(EKeys::Enter);T->TestTrue(TEXT("Tree Enter never manufactures representative recipe"),C->ActiveRecipe.IsNone());
            }
            Next(Now);return false;
        }
        if(Step==5 || Step==7 || Step==9 || Step==10 || Step==11 || Step==12)
        {
            const FName Recipe=Step==5?TEXT("Recipe_IronPick"):Step==7?TEXT("Recipe_DiamondPick"):Step==12?TEXT("Recipe_TitaniumPick"):TEXT("Recipe_TitaniumRefine");
            if(Sub==0)
            {
                PC->SetCraftingMenuOpen(false);
                if(Step==5){T->TestTrue(TEXT("Explicit craft boundary inputs"),I->Grant(TEXT("Item_IronIngot"),3) && I->Grant(TEXT("Item_Cord"),1));}
                if(Step==7){T->TestTrue(TEXT("Cord boundary"),I->Grant(TEXT("Item_Cord"),1));}
                if(Step==9){T->TestTrue(TEXT("Refining boundary fuel and cord"),I->Grant(TEXT("Item_Coal"),6) && I->Grant(TEXT("Item_Obsidian"),3) && I->Grant(TEXT("Item_Cord"),1));}
                if(!Select(PC,Recipe,false)){return true;}Sub=1;Changed=Now;return false;
            }
            if(Sub==1)
            {
                auto* Picture=Cast<UPFItemPicture>(Menu->WidgetTree->FindWidget(TEXT("PF_SelectedItemPicture")));const auto* D=C->Catalog->Recipe(Recipe,I->Catalog);
                T->TestTrue(TEXT("Correct visible item sprite selection"),Picture && D && Picture->GetItemId()==D->Output && Picture->GetCachedGeometry().GetLocalSize().X>0);
                if(Step==5 || Step==12){Shot(Step==5?TEXT("iron_pick_recipe"):TEXT("titanium_pick_recipe"));}
                Press(EKeys::Enter);if(!T->TestEqual(TEXT("Real Slate craft starts requested timed recipe"),C->ActiveRecipe,Recipe)){return true;}Sub=2;Changed=Now;return false;
            }
            if(!C->ActiveRecipe.IsNone()){return false;}
            if(!T->TestTrue(TEXT("Actual elapsed craft succeeded"),C->Feedback.StartsWith(TEXT("Completed")))){return true;}
            PC->SetCraftingMenuOpen(false);Next(Now);return false;
        }
        if(Step==6 || Step==8)
        {
            auto* Node=Step==6?Diamond: Titanium;Aim(PC,Node);const int32 Before=Node->HitsRemaining;PC->Interact();
            T->TestTrue(TEXT("Actual higher-tier first-person E gathers finite yield"),Node->HitsRemaining<Before);Next(Now);return false;
        }
        if(Step==13)
        {
            T->TestTrue(TEXT("One final pick, previous two consumed once"),I->Count(TEXT("Item_TitaniumPick"))==1 && I->Count(TEXT("Item_DiamondPick"))==0 && I->Count(TEXT("Item_IronPick"))==0 && I->Count(TEXT("Item_CopperPick"))==0);
            Aim(PC,Titanium);Shot(TEXT("titanium_complete"));
            T->AddInfo(TEXT("[PrimalAgentTools] EryndorLive: saved9fixtures, actual Slate tree unlocks/separate budget, rendered pictured recipes, timed upgrade/refine and first-person Iron/Diamond/Titanium interaction passed. Explicit level700XP/material boundary seeds and assisted travel; not an earned-pacing or FPS certificate."));Finished=true;Changed=Now;return false;
        }
        T->AddError(TEXT("Unexpected Eryndor step"));return true;
    }
private:
    void Next(double Now){++Step;Sub=0;Changed=Now;}
    void Press(FKey Key){auto& A=FSlateApplication::Get();A.ProcessKeyDownEvent(FKeyEvent(Key,FModifierKeysState(),0,false,0,0));A.ProcessKeyUpEvent(FKeyEvent(Key,FModifierKeysState(),0,false,0,0));}
    bool Select(APFSurvivalPlayerController* PC,FName Id,bool Tree)
    {
        PC->SetCraftingMenuOpen(true);TArray<UUserWidget*> Widgets;UWidgetBlueprintLibrary::GetAllWidgetsOfClass(PC,Widgets,UPFCraftingHUD::StaticClass(),false);
        if(Widgets.Num()!=1){T->AddError(TEXT("One crafting modal required"));return false;}Menu=CastChecked<UPFCraftingHUD>(Widgets[0]);
        Menu->SelectCategory(Tree?FGameplayTag::RequestGameplayTag(TEXT("UI.Crafting.Technology")):FGameplayTag());Menu->SelectRecipe(Id);Menu->SetKeyboardFocus();
        if(!T->TestEqual(TEXT("Real selected recipe/technology representative"),Menu->GetSelectedRecipe(),Id)){return false;}
        if(Tree){T->TestEqual(TEXT("Six visible knowledge branches"),Menu->GetVisibleRecipeIds().Num(),6);}return true;
    }
    void Aim(APFSurvivalPlayerController* PC,APFResourceNode* Node)
    {PC->SetCraftingMenuOpen(false);auto* Pawn=PC->GetPawn().Get();Pawn->SetActorLocation(Node->GetActorLocation()+FVector(-190,0,40));FVector Eye;FRotator Look;Pawn->GetActorEyesViewPoint(Eye,Look);PC->SetControlRotation((Node->GetActorLocation()-Eye).Rotation());T->TestTrue(TEXT("Actual authored eye trace"),PFInteraction::FindTarget(Pawn)==Node);}
    void Shot(const TCHAR* Name){const FString Path=Directory/(FString(Name)+TEXT(".png"));Shots.Add(Path);FScreenshotRequest::RequestScreenshot(Path,true,false);}
    FAutomationTestBase* T;FString Directory;TArray<FString> Shots;TWeakObjectPtr<UPFCraftingHUD> Menu;
    APFResourceNode* Diamond=nullptr;APFResourceNode* Titanium=nullptr;bool Ready=false,Finished=false;
    int32 Step=0,Sub=0;double Started=FPlatformTime::Seconds(),Changed=0;
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFEryndorLiveTest,"PF.Eryndor.Live",EAutomationTestFlags::ClientContext|EAutomationTestFlags::EngineFilter)
bool FPFEryndorLiveTest::RunTest(const FString&)
{
    FString Label;FParse::Value(FCommandLine::Get(),TEXT("PFControlsEvidence="),Label);
    if(!FParse::Param(FCommandLine::Get(),TEXT("PFRunEryndorUITest")) || !FParse::Param(FCommandLine::Get(),TEXT("PFRunControlsUITest")) || FParse::Param(FCommandLine::Get(),TEXT("nullrhi")) || Label.IsEmpty() || Label.Len()>64){AddError(TEXT("Requires isolated rendered Eryndor runner"));return false;}
    for(TCHAR C:Label){if(!FChar::IsAlnum(C) && C!=TEXT('_')){return false;}}
    const FString D=FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()/TEXT("AutomationReports/ControlsUI")/Label);
    if(IFileManager::Get().DirectoryExists(*D)){AddError(TEXT("Refusing reused evidence"));return false;}IFileManager::Get().MakeDirectory(*D,true);
    ADD_LATENT_AUTOMATION_COMMAND(FEryndorLive(this,D));return true;
}
#endif
