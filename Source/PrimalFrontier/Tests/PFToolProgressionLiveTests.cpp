// Disposable M12 exercise only. Real owned RPCs; optional rendered Slate recipe selection.
#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Crafting/PFCraftingComponent.h"
#include "Crafting/PFCraftingHUD.h"
#include "Crafting/PFResourceNode.h"
#include "Inventory/PFInventoryComponent.h"
#include "Persistence/PFWorldPersistence.h"
#include "Persistence/PFSaveFileStore.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Survival/PFSurvivorCharacter.h"
#include "Settings/PFGameUserSettings.h"
#include "UI/PFItemPicture.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/PlayerState.h"
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
class FToolLiveExercise final : public IAutomationLatentCommand
{
public:
    FToolLiveExercise(FAutomationTestBase* T,FString D):Test(T),Directory(MoveTemp(D))
    {FParse::Value(FCommandLine::Get(),TEXT("PFExpectedPlayers="),Expected);FParse::Value(FCommandLine::Get(),TEXT("PFSaveSlot="),Slot);bRestore=FParse::Param(FCommandLine::Get(),TEXT("PFLoadSave"));}
    ~FToolLiveExercise(){if(bScaleChanged && UPFGameUserSettings::Get()){UPFGameUserSettings::Get()->Preferences.HUDScale=OldScale;}}
    bool Update() override
    {
        const double Now=FPlatformTime::Seconds();
        if(Finished){if(Now-Changed<(bClient?3:12)){return false;}for(const auto& Path:Shots){Test->TestTrue(TEXT("Rendered screenshot written"),IFileManager::Get().FileSize(*Path)>0);}return true;}
        // Leave enough time for Unreal to write a failed verdict before the runner's 180s cap.
        if(Now-Started>140){Test->AddError(FString::Printf(TEXT("[PrimalAgentTools] Tool live timeout server=%d local=%d %s"),ServerStage,LocalStage,*LastObserved));return true;}
        UWorld* W=nullptr;for(const auto& X:GEngine->GetWorldContexts()){if(X.World() && X.World()->IsGameWorld()){W=X.World();break;}}
        if(!W){return false;}bClient=W->GetNetMode()==NM_Client;const bool Solo=W->GetNetMode()==NM_Standalone;
        TArray<APFSurvivalPlayerController*> Players;
        for(auto It=W->GetPlayerControllerIterator();It;++It){auto* P=Cast<APFSurvivalPlayerController>(It->Get());if(P && P->GetPawn() && P->GetInventory() && P->GetCrafting()){Players.Add(P);}}
        if(Players.Num()!=(bClient?1:Expected)){return false;}
        auto* PC=Players[0];auto* I=PC->GetInventory();auto* C=PC->GetCrafting();
        if(!bClient && ServerStage==0)
        {
            if(bRestore)
            {
                // The UI acknowledgment is client-only. Authority observes actual restoration,
                // never invokes RestorePlayer itself or manufactures a presentation outcome.
                for(auto* P:Players){if(P->GetInventory()->Count(TEXT("Item_BoundTool"))!=1 || P->GetInventory()->Count(TEXT("Item_Fibre"))!=8){return false;}}
                FPFWorldSaveData Snapshot;FString Error;
                if(!W->GetSubsystem<UPFWorldPersistence>()->Capture(Snapshot,Error)){Test->AddError(Error);return true;}
                Test->TestEqual(TEXT("Restart retains exactly the original player records"),Snapshot.Players.Num(),Expected);
                Test->TestTrue(TEXT("Server restored snapshot validates"),W->GetSubsystem<UPFWorldPersistence>()->Validate(Snapshot,Error));
                for(auto* P:Players){CheckBag(P->GetInventory());}
                Test->AddInfo(TEXT("[PrimalAgentTools] Tool restart server restored every owner"));Finished=true;Changed=Now;return false;
            }
            else
            {
                int32 Index=0;for(auto* P:Players)
                {
                    if(!P->GetInventory()->GetStacks().IsEmpty()){Test->AddError(TEXT("Refusing non-fresh tool fixture"));return true;}
                    auto* Needs=P->GetPawn()->FindComponentByClass<UPFPlayerSurvivalComponent>();Needs->HungerDrainPerSecond=0;Needs->ThirstDrainPerSecond=0;
                    P->GetPawn()->SetActorLocation(FVector(-1200,Index++*400,100));P->SetControlRotation(FRotator::ZeroRotator);
                    FVector Eye;FRotator Look;P->GetPawn()->GetActorEyesViewPoint(Eye,Look);
                    FTransform At(FRotator::ZeroRotator,Eye+FVector(150,0,0));
                    auto* N=W->SpawnActorDeferred<APFResourceNode>(APFResourceNode::StaticClass(),At,P->GetPawn(),nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
                    N->ResourceId=TEXT("Node_Fibre");N->FinishSpawning(At);
                    Test->TestTrue(TEXT("Authority supplies existing tool only"),P->GetInventory()->Grant(TEXT("Item_Tool"),1));
                    Test->TestTrue(TEXT("Authority supplies two wood"),P->GetInventory()->Grant(TEXT("Item_Wood"),2));
                }
                ServerStage=1;
            }
        }
        if(bClient || Solo)
        {
            if(bRestore)
            {
                if(PC->GetPlayerSetupStatus()!=EPFPlayerSetupStatus::Restored || I->Count(TEXT("Item_BoundTool"))!=1 || I->Count(TEXT("Item_Fibre"))!=8){return false;}
                CheckBag(I);Privacy(W,PC);FString Error;
                if(bClient){Test->TestFalse(TEXT("Client cannot save restored world"),W->GetSubsystem<UPFWorldPersistence>()->Save(Slot,Error));}
                Test->AddInfo(TEXT("[PrimalAgentTools] Tool restart client identity, tier and privacy verified"));Finished=true;Changed=Now;return false;
            }
            APFResourceNode* Node=nullptr;for(TActorIterator<APFResourceNode> It(W);It;++It){if(It->GetOwner()==PC->GetPawn()){Node=*It;break;}}
            LastObserved=FString::Printf(TEXT("fibre=%d hits=%d deadline=%.3f pause=%d crafting=%d pawn=%s"),I->Count(TEXT("Item_Fibre")),Node?Node->HitsRemaining:-1,Node?Node->RespawnAt:-1,PC->IsPauseMenuOpen(),PC->IsCraftingOpen(),*GetNameSafe(PC->GetPawn()));
            if(LocalStage<10 && !Node){return false;}
            if(LocalStage==0)
            {
                if(I->Count(TEXT("Item_Tool"))!=1 || I->Count(TEXT("Item_Wood"))!=2){return false;}
                if(bClient){Test->TestFalse(TEXT("Client direct gathering refused"),Node->Gather(PC->GetPawn()));Test->TestFalse(TEXT("Client direct craft refused"),C->Start(TEXT("Recipe_BoundTool"),PC->GetPawn()));Test->TestFalse(TEXT("Client direct tool creation refused"),I->Grant(TEXT("Item_BoundTool"),1));}
                PC->SetControlRotation(FRotator::ZeroRotator);PC->ServerCraftAction(TEXT("Recipe_Unknown"),false);Next(1,Now);
            }
            else if(LocalStage==1 && Now-Changed>0.6)
            {Test->TestEqual(TEXT("Unknown recipe remains idle"),C->ActiveRecipe,NAME_None);PC->Interact();Next(2,Now);}
            else if(LocalStage==2 && I->Count(TEXT("Item_Fibre"))==4)
            {
                Test->TestEqual(TEXT("Baseline gather spends two hits"),Node->HitsRemaining,1);
                if(!Directory.IsEmpty())
                {
                    if(!Menu.IsValid())
                    {
                        if(auto* S=UPFGameUserSettings::Get()){OldScale=S->Preferences.HUDScale;float Scale=1;FParse::Value(FCommandLine::Get(),TEXT("PFHUDScale="),Scale);S->Preferences.HUDScale=Scale;bScaleChanged=true;}
                        PC->SetCraftingMenuOpen(true);TArray<UUserWidget*> Widgets;UWidgetBlueprintLibrary::GetAllWidgetsOfClass(PC,Widgets,UPFCraftingHUD::StaticClass(),false);
                        if(Widgets.Num()!=1){Test->AddError(TEXT("Missing centered recipe menu"));return true;}Menu=CastChecked<UPFCraftingHUD>(Widgets[0]);
                        for(int N=0;N<3;++N){Press(EKeys::Down);}Next(20,Now);return false;
                    }
                }
                Start(PC,TEXT("Recipe_Cord"));Next(3,Now);
            }
            else if(LocalStage==20 && Now-Changed>0.5)
            {
                CheckMenu(TEXT("Recipe_Cord"),TEXT("Item_Cord"));Test->TestEqual(TEXT("All eight recipes initially visible"),Menu->GetVisibleRecipeIds().Num(),8);
                auto* Button=Cast<UButton>(Menu->WidgetTree->FindWidget(TEXT("PF_Category3_Button")));if(!Button){Test->AddError(TEXT("Missing material category button"));return true;}Button->SetKeyboardFocus();Test->TestTrue(TEXT("Category button has real Slate focus"),Button->HasKeyboardFocus());Press(EKeys::SpaceBar);Next(24,Now);
            }
            else if(LocalStage==24 && Now-Changed>0.3)
            {
                Test->TestEqual(TEXT("Button selects material category"),Menu->GetSelectedCategory().ToString(),FString(TEXT("Recipe.Category.Material")));
                Test->TestEqual(TEXT("Material shows only cord"),Menu->GetVisibleRecipeIds().Num(),1);Test->TestTrue(TEXT("Material row is cord"),Menu->GetVisibleRecipeIds().Contains(TEXT("Recipe_Cord")));
                Test->TestEqual(TEXT("Filtering clears selection instead of silently replacing it"),Menu->GetSelectedRecipe(),NAME_None);Press(EKeys::Enter);Next(25,Now);
            }
            else if(LocalStage==25 && Now-Changed>0.3)
            {Test->TestEqual(TEXT("Empty selection sends no craft"),C->ActiveRecipe,NAME_None);Test->TestEqual(TEXT("Filtering preserves ingredients"),I->Count(TEXT("Item_Fibre")),4);Press(EKeys::PageUp);Next(26,Now);}
            else if(LocalStage==26 && Now-Changed>0.3)
            {
                Test->TestEqual(TEXT("Keyboard switches food category"),Menu->GetSelectedCategory().ToString(),FString(TEXT("Recipe.Category.Food")));Test->TestEqual(TEXT("Both food recipes visible"),Menu->GetVisibleRecipeIds().Num(),2);
                auto* Title=Cast<UTextBlock>(Menu->WidgetTree->FindWidget(TEXT("PF_CraftingDetail_Title")));Test->TestTrue(TEXT("Empty selection asks for a recipe instead of reporting invalid data"),Title && Title->GetText().ToString()==TEXT("CHOOSE A RECIPE"));
                Test->TestTrue(TEXT("Food IDs are cook and dry"),Menu->GetVisibleRecipeIds().Contains(TEXT("Recipe_Cook")) && Menu->GetVisibleRecipeIds().Contains(TEXT("Recipe_Dry")));Shot(TEXT("food_category"));Next(29,Now);
            }
            else if(LocalStage==29 && Now-Changed>0.3){Press(EKeys::Gamepad_LeftShoulder);Next(27,Now);}
            else if(LocalStage==27 && Now-Changed>0.3)
            {
                Test->TestEqual(TEXT("Controller shoulder switches tool category"),Menu->GetSelectedCategory().ToString(),FString(TEXT("Recipe.Category.Tool")));Test->TestEqual(TEXT("Both tools visible"),Menu->GetVisibleRecipeIds().Num(),2);
                const auto Prior=Menu->GetSelectedCategory();Menu->SelectCategory(FGameplayTag::RequestGameplayTag(TEXT("Item.Category.Resource")));Test->TestEqual(TEXT("Unknown recipe category safely refused"),Menu->GetSelectedCategory(),Prior);
                Press(EKeys::Gamepad_RightShoulder);Press(EKeys::Gamepad_RightShoulder);Press(EKeys::Down);Next(28,Now);
            }
            else if(LocalStage==28 && Now-Changed>0.3)
            {CheckMenu(TEXT("Recipe_Cord"),TEXT("Item_Cord"));Shot(TEXT("cord_selected"));Start(PC,TEXT("Recipe_Cord"));Next(3,Now);}
            else if(LocalStage==3 && C->ActiveRecipe==TEXT("Recipe_Cord") && Now-Changed>0.6)
            {PC->ServerCraftAction(TEXT("Recipe_Cord"),false);Next(4,Now);}
            else if(LocalStage==4 && Now-Changed>0.6)
            {if(Menu.IsValid()){const double Deadline=C->FinishAt;Press(EKeys::PageDown);Test->TestEqual(TEXT("Filter does not cancel active job"),C->ActiveRecipe,FName(TEXT("Recipe_Cord")));Test->TestEqual(TEXT("Filter does not restart or charge job"),C->FinishAt,Deadline);Press(EKeys::PageUp);Press(EKeys::Down);}PC->ServerCraftAction(NAME_None,true);Next(5,Now);}
            else if(LocalStage==5 && C->ActiveRecipe.IsNone() && Now-Changed>0.6)
            {Test->TestEqual(TEXT("Cancel preserves four fibre"),I->Count(TEXT("Item_Fibre")),4);Test->TestEqual(TEXT("Cancel creates no cord"),I->Count(TEXT("Item_Cord")),0);Start(PC,TEXT("Recipe_Cord"));Next(6,Now);}
            else if(LocalStage==6 && I->Count(TEXT("Item_Cord"))==1 && C->ActiveRecipe.IsNone())
            {
                Test->TestEqual(TEXT("Exact cord input cost"),I->Count(TEXT("Item_Fibre")),0);
                if(Menu.IsValid()){Press(EKeys::PageDown);Press(EKeys::PageDown);Press(EKeys::Down);Press(EKeys::Down);Next(21,Now);}else{Start(PC,TEXT("Recipe_BoundTool"));Next(7,Now);}
            }
            else if(LocalStage==21 && Now-Changed>0.5)
            {CheckMenu(TEXT("Recipe_BoundTool"),TEXT("Item_BoundTool"));Shot(TEXT("bound_selected"));Start(PC,TEXT("Recipe_BoundTool"));Next(7,Now);}
            else if(LocalStage==7 && I->Count(TEXT("Item_BoundTool"))==1 && C->ActiveRecipe.IsNone())
            {CheckCosts(I);if(Menu.IsValid()){Next(23,Now);}else{PC->Interact();Next(8,Now);}}
            else if(LocalStage==23 && Now-Changed>0.4)
            {Menu->RefreshMenu();auto* Result=Cast<UTextBlock>(Menu->WidgetTree->FindWidget(TEXT("PF_CraftingPanel_Result")));Test->TestTrue(TEXT("Completion visible after HUD refresh"),Result && Result->GetText().ToString().Contains(TEXT("Completed")));Shot(TEXT("bound_complete"));Next(22,Now);}
            else if(LocalStage==22 && Now-Changed>0.5)
            {PC->SetCraftingMenuOpen(false);PC->Interact();Next(8,Now);}
            else if(LocalStage==8 && I->Count(TEXT("Item_Fibre"))==2 && Node->HitsRemaining==0)
            {Test->TestTrue(TEXT("Depletion deadline replicated"),Node->RespawnAt>0);Next(9,Now);}
            else if(LocalStage==9 && Node->HitsRemaining==3 && Node->RespawnAt==0)
            {PC->Interact();Next(10,Now);}
            else if(LocalStage==10 && I->Count(TEXT("Item_Fibre"))==8)
            {CheckBag(I);Privacy(W,PC);Test->TestTrue(TEXT("Bound tool presentation flag replicates"),CastChecked<APFSurvivorCharacter>(PC->GetPawn())->HasGatheringTool());for(TActorIterator<APFSurvivorCharacter> It(W);It;++It){if(*It!=PC->GetPawn()){Test->TestTrue(TEXT("Other player's held tool is public"),It->HasGatheringTool());}}Test->AddInfo(TEXT("[PrimalAgentTools] Tool owned RPC loop, finite yield and privacy verified"));LocalStage=99;if(bClient){Finished=true;Changed=Now;}}
        }
        if(!bClient && ServerStage==1)
        {
            for(auto* P:Players){if(P->GetInventory()->Count(TEXT("Item_BoundTool"))!=1 || P->GetInventory()->Count(TEXT("Item_Fibre"))!=8){return false;}}
            if(Solo && LocalStage!=99){return false;}ServerStage=2;ServerChanged=Now;
        }
        if(!bClient && ServerStage==2 && Now-ServerChanged>1)
        {
            for(auto* P:Players){CheckBag(P->GetInventory());}
            // Map resources are name-matched on load; exclude only these transient fixture nodes.
            for(TActorIterator<APFResourceNode> It(W);It;++It){if(It->GetOwner() && Players.Contains(Cast<APFSurvivalPlayerController>(It->GetOwner()->GetInstigatorController()))){It->Destroy();}}
            if(!Slot.IsEmpty()){FString Error;Test->TestTrue(TEXT("Save real tool inventory to isolated world"),W->GetSubsystem<UPFWorldPersistence>()->Save(Slot,Error));if(!Error.IsEmpty()){Test->AddError(Error);}}
            Test->AddInfo(TEXT("[PrimalAgentTools] Tool server conservation and save verified"));Finished=true;Changed=Now;
        }
        if(!bClient && ServerStage==3 && Now-Changed>8){Finished=true;Changed=Now;}
        return false;
    }
private:
    void Next(int32 S,double Now){LocalStage=S;Changed=Now;const FString Message=FString::Printf(TEXT("[PrimalAgentTools] Tool local stage=%d"),S);Test->AddInfo(Message);UE_LOG(LogTemp,Display,TEXT("%s"),*Message);}
    void CheckCosts(UPFInventoryComponent* I){Test->TestEqual(TEXT("Old tool consumed once"),I->Count(TEXT("Item_Tool")),0);Test->TestEqual(TEXT("Cord consumed once"),I->Count(TEXT("Item_Cord")),0);Test->TestEqual(TEXT("Wood consumed once"),I->Count(TEXT("Item_Wood")),0);Test->TestEqual(TEXT("Owned gathering tier"),I->GatheringHits(),3);Test->TestEqual(TEXT("Owned melee tier"),I->MeleeDamage(),45.f);}
    void CheckBag(UPFInventoryComponent* I){CheckCosts(I);Test->TestEqual(TEXT("One bound tool"),I->Count(TEXT("Item_BoundTool")),1);Test->TestEqual(TEXT("Conserved finite fibre"),I->Count(TEXT("Item_Fibre")),8);}
    void Privacy(UWorld* W,APFSurvivalPlayerController* PC)
    {int32 Others=0;for(TActorIterator<APlayerState> It(W);It;++It){if(*It!=PC->PlayerState){++Others;if(auto* Bag=It->FindComponentByClass<UPFInventoryComponent>()){Test->TestTrue(TEXT("Foreign inventory stays private"),Bag->GetStacks().IsEmpty());}}}Test->TestEqual(TEXT("All remote player states visible"),Others,Expected-1);}
    void Press(FKey K){auto& A=FSlateApplication::Get();A.ProcessKeyDownEvent(FKeyEvent(K,FModifierKeysState(),0,false,0,0));A.ProcessKeyUpEvent(FKeyEvent(K,FModifierKeysState(),0,false,0,0));}
    void Start(APFSurvivalPlayerController* PC,FName Recipe){if(Menu.IsValid()){Test->TestEqual(TEXT("Selected recipe request"),Menu->GetSelectedRecipe(),Recipe);Press(EKeys::Enter);}else{PC->ServerCraftAction(Recipe,false);}}
    void Shot(const TCHAR* Name){const FString Path=Directory/(FString(Name)+TEXT(".png"));Shots.Add(Path);FScreenshotRequest::RequestScreenshot(Path,true,false);}
    void CheckMenu(FName Recipe,FName Item)
    {
        auto* M=Menu.Get();Test->TestEqual(TEXT("Slate selects appended recipe"),M->GetSelectedRecipe(),Recipe);
        auto* Picture=Cast<UPFItemPicture>(M->WidgetTree->FindWidget(TEXT("PF_SelectedItemPicture")));Test->TestTrue(TEXT("Correct original item picture"),Picture && Picture->GetItemId()==Item && Picture->GetCachedGeometry().GetLocalSize().X>0);
        const FGeometry Root=M->GetCachedGeometry();const auto Size=Root.GetLocalSize();
        auto* Panel=M->WidgetTree->FindWidget(TEXT("PF_CraftingPanel"));const auto G=Panel->GetCachedGeometry();const auto TL=Root.AbsoluteToLocal(G.LocalToAbsolute(FVector2D::ZeroVector)),BR=Root.AbsoluteToLocal(G.LocalToAbsolute(G.GetLocalSize()));
        Test->TestTrue(TEXT("Menu remains centered and fits viewport"),TL.X>=0 && TL.Y>=0 && BR.X<Size.X && BR.Y<Size.Y && FMath::Abs(TL.X+BR.X-Size.X)<4);
        TArray<UWidget*> Children;M->WidgetTree->GetAllWidgets(Children);for(auto* Child:Children){if(auto* Text=Cast<UTextBlock>(Child)){const auto Geometry=Text->GetCachedGeometry();Test->TestTrue(TEXT("Expanded menu text fits allocated row"),Text->GetDesiredSize().Y<=Geometry.GetLocalSize().Y+1);}}
        if(Item==TEXT("Item_BoundTool")){auto* Stats=Cast<UTextBlock>(M->WidgetTree->FindWidget(TEXT("PF_CraftingDetail_Stats")));Test->TestTrue(TEXT("Selected tier statistics visible"),Stats && Stats->GetText().ToString().Contains(TEXT("3 gather hits | 45 melee damage")));}
    }
    FAutomationTestBase* Test;FString Directory,Slot,LastObserved;TArray<FString> Shots;TWeakObjectPtr<UPFCraftingHUD> Menu;
    double Started=FPlatformTime::Seconds(),Changed=0,ServerChanged=0;int32 Expected=1,ServerStage=0,LocalStage=0;bool bRestore=false,bClient=false,Finished=false,bScaleChanged=false;float OldScale=1;
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFToolLiveTest,"PF.Crafting.ToolProgressionLive",EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)
bool FPFToolLiveTest::RunTest(const FString&)
{
    FString Slot,Directory;int32 Expected=1;FParse::Value(FCommandLine::Get(),TEXT("PFExpectedPlayers="),Expected);
    if(!FParse::Param(FCommandLine::Get(),TEXT("PFRunToolProgressionTests")) || (Expected!=1 && Expected!=2)){AddError(TEXT("Requires isolated -PFRunToolProgressionTests -PFExpectedPlayers=1|2"));return false;}
    if(!FParse::Param(FCommandLine::Get(),TEXT("nullrhi")))
    {
        FString Label;FParse::Value(FCommandLine::Get(),TEXT("PFControlsEvidence="),Label);
        if(!FParse::Param(FCommandLine::Get(),TEXT("PFRunControlsUITest")) || Label.IsEmpty() || Label.Len()>48){AddError(TEXT("Rendered tool test requires isolated controls runner"));return false;}
        for(TCHAR C:Label){if(!FChar::IsAlnum(C) && C!=TEXT('_')){return false;}}
        Directory=FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()/TEXT("AutomationReports/ControlsUI")/Label);
        if(IFileManager::Get().DirectoryExists(*Directory)){AddError(TEXT("Refusing reused evidence"));return false;}IFileManager::Get().MakeDirectory(*Directory,true);
    }
    else if(!FParse::Value(FCommandLine::Get(),TEXT("PFSaveSlot="),Slot) || !Slot.StartsWith(TEXT("AutomationM12")) || !FPFSaveFileStore::ValidSlot(Slot))
    {AddError(TEXT("Headless tool test requires disposable AutomationM12 save slot"));return false;}
    ADD_LATENT_AUTOMATION_COMMAND(FToolLiveExercise(this,Directory));return true;
}
#endif
