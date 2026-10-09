// Opt-in disposable game worlds only. Observe actual owned RPCs and replicated results.
#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Crafting/PFCraftingComponent.h"
#include "Crafting/PFCraftingHUD.h"
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
class FWeaponLiveExercise final : public IAutomationLatentCommand
{
public:
    FWeaponLiveExercise(FAutomationTestBase* T,FString D):Test(T),Directory(MoveTemp(D))
    {FParse::Value(FCommandLine::Get(),TEXT("PFExpectedPlayers="),Expected);FParse::Value(FCommandLine::Get(),TEXT("PFSaveSlot="),Slot);bRestore=FParse::Param(FCommandLine::Get(),TEXT("PFLoadSave"));}
    ~FWeaponLiveExercise(){if(bScaleChanged && UPFGameUserSettings::Get()){UPFGameUserSettings::Get()->Preferences.HUDScale=OldScale;}}
    bool Update() override
    {
        const double Now=FPlatformTime::Seconds();
        if(Finished){if(Now-Changed<(bClient?3:12)){return false;}for(const auto& Path:Shots){Test->TestTrue(TEXT("Screenshot written"),IFileManager::Get().FileSize(*Path)>0);}return true;}
        if(Now-Started>120){Test->AddError(FString::Printf(TEXT("[PrimalAgentTools] Weapon live timeout server=%d local=%d %s"),ServerStage,LocalStage,*LastObserved));return true;}
        UWorld* W=nullptr;for(const auto& X:GEngine->GetWorldContexts()){if(X.World() && X.World()->IsGameWorld()){W=X.World();break;}}
        if(!W){return false;}bClient=W->GetNetMode()==NM_Client;const bool Solo=W->GetNetMode()==NM_Standalone;
        TArray<APFSurvivalPlayerController*> Players;
        for(auto It=W->GetPlayerControllerIterator();It;++It){auto* P=Cast<APFSurvivalPlayerController>(It->Get());if(P && P->GetPawn() && P->GetInventory() && P->GetCrafting()){Players.Add(P);}}
        if(Players.Num()!=(bClient?1:Expected)){return false;}
        auto* PC=Players[0];auto* I=PC->GetInventory();auto* C=PC->GetCrafting();
        LastObserved=FString::Printf(TEXT("wood=%d cord=%d club=%d bound=%d active=%s held=%s"),I->Count(TEXT("Item_Wood")),I->Count(TEXT("Item_Cord")),I->Count(TEXT("Item_Club")),I->Count(TEXT("Item_BoundClub")),*C->ActiveRecipe.ToString(),*CastChecked<APFSurvivorCharacter>(PC->GetPawn())->GetHeldMeleeItem().ToString());
        if(bRestore)
        {
            for(auto* P:Players){if(!Ready(P)){return false;}}
            if(bClient)
            {
                if(PC->GetPlayerSetupStatus()!=EPFPlayerSetupStatus::Restored){return false;}
                CheckBag(I);if(!Privacy(W,PC)){return false;}FString Error;
                Test->TestFalse(TEXT("Client cannot save restored world"),W->GetSubsystem<UPFWorldPersistence>()->Save(Slot,Error));
                Test->AddInfo(TEXT("[PrimalAgentTools] Weapon restart client identity, tier and privacy verified"));
            }
            else
            {
                FPFWorldSaveData Snapshot;FString Error;
                Test->TestTrue(TEXT("Actual restored server capture"),W->GetSubsystem<UPFWorldPersistence>()->Capture(Snapshot,Error));
                Test->TestEqual(TEXT("Original player records retained"),Snapshot.Players.Num(),Expected);
                Test->TestTrue(TEXT("Actual restored capture validates"),W->GetSubsystem<UPFWorldPersistence>()->Validate(Snapshot,Error));
                for(auto* P:Players){CheckBag(P->GetInventory());}
                Test->AddInfo(TEXT("[PrimalAgentTools] Weapon restart server restored every owner"));
            }
            Finished=true;Changed=Now;return false;
        }
        if(!bClient && ServerStage==0)
        {
            int32 Index=0;for(auto* P:Players)
            {
                if(!P->GetInventory()->GetStacks().IsEmpty()){Test->AddError(TEXT("Refusing non-fresh weapon fixture"));return true;}
                auto* Needs=P->GetPawn()->FindComponentByClass<UPFPlayerSurvivalComponent>();Needs->HungerDrainPerSecond=0;Needs->ThirstDrainPerSecond=0;
                P->GetPawn()->SetActorLocation(FVector(-1200,Index++*400,100));P->SetControlRotation(FRotator::ZeroRotator);
                Test->TestTrue(TEXT("Fixture supplies wood only"),P->GetInventory()->Grant(TEXT("Item_Wood"),5));
                Test->TestTrue(TEXT("Fixture supplies cord only"),P->GetInventory()->Grant(TEXT("Item_Cord"),2));
                Test->TestTrue(TEXT("Fixture supplies stone only"),P->GetInventory()->Grant(TEXT("Item_Stone"),2));
            }
            ServerStage=1;
        }
        if(bClient || Solo)
        {
            if(LocalStage==0 && I->Count(TEXT("Item_Wood"))==5 && I->Count(TEXT("Item_Cord"))==2 && I->Count(TEXT("Item_Stone"))==2)
            {
                if(bClient){Test->TestFalse(TEXT("Client direct craft refused"),C->Start(TEXT("Recipe_Club"),PC->GetPawn()));Test->TestFalse(TEXT("Client direct weapon creation refused"),I->Grant(TEXT("Item_BoundClub"),1));}
                if(!Directory.IsEmpty())
                {
                    if(auto* S=UPFGameUserSettings::Get()){OldScale=S->Preferences.HUDScale;float Scale=1;FParse::Value(FCommandLine::Get(),TEXT("PFHUDScale="),Scale);S->Preferences.HUDScale=Scale;bScaleChanged=true;}
                    PC->SetCraftingMenuOpen(true);TArray<UUserWidget*> Widgets;UWidgetBlueprintLibrary::GetAllWidgetsOfClass(PC,Widgets,UPFCraftingHUD::StaticClass(),false);
                    if(Widgets.Num()!=1){Test->AddError(TEXT("Missing centered crafting menu"));return true;}Menu=CastChecked<UPFCraftingHUD>(Widgets[0]);Next(20,Now);
                }
                else{Start(PC,TEXT("Recipe_Club"));Next(1,Now);}
            }
            else if(LocalStage==20 && Now-Changed>.5)
            {
                Test->TestEqual(TEXT("All eight catalog recipes"),Menu->GetVisibleRecipeIds().Num(),8);
                auto* B=Cast<UButton>(Menu->WidgetTree->FindWidget(TEXT("PF_Category4_Button")));if(!B){Test->AddError(TEXT("Missing weapon category button"));return true;}
                B->SetKeyboardFocus();Test->TestTrue(TEXT("Category actual Slate focus"),B->HasKeyboardFocus());Press(EKeys::SpaceBar);Next(21,Now);
            }
            else if(LocalStage==21 && Now-Changed>.4)
            {Test->TestEqual(TEXT("Only two weapon recipes"),Menu->GetVisibleRecipeIds().Num(),2);Test->TestEqual(TEXT("Category clears selection"),Menu->GetSelectedRecipe(),NAME_None);Press(EKeys::Down);Next(22,Now);}
            else if(LocalStage==22 && Now-Changed>.4)
            {CheckMenu(TEXT("Recipe_Club"),TEXT("Item_Club"),40);Shot(TEXT("club_selected"));Start(PC,TEXT("Recipe_Club"));Next(1,Now);}
            else if(LocalStage==1 && C->ActiveRecipe==TEXT("Recipe_Club") && Now-Changed>.6)
            {PC->ServerCraftAction(TEXT("Recipe_Club"),false);Next(2,Now);}
            else if(LocalStage==2 && Now-Changed>.6)
            {Test->TestEqual(TEXT("Duplicate keeps single job"),C->ActiveRecipe,FName(TEXT("Recipe_Club")));PC->ServerCraftAction(NAME_None,true);Next(3,Now);}
            else if(LocalStage==3 && C->ActiveRecipe.IsNone() && Now-Changed>.6)
            {Test->TestEqual(TEXT("Cancel preserves inputs"),I->Count(TEXT("Item_Wood")),5);Test->TestEqual(TEXT("Cancel gives no weapon"),I->Count(TEXT("Item_Club")),0);Start(PC,TEXT("Recipe_Club"));Next(4,Now);}
            else if(LocalStage==4 && I->Count(TEXT("Item_Club"))==1 && C->ActiveRecipe.IsNone() && CastChecked<APFSurvivorCharacter>(PC->GetPawn())->GetHeldMeleeItem()==TEXT("Item_Club"))
            {
                Test->TestEqual(TEXT("Wood consumed once"),I->Count(TEXT("Item_Wood")),2);Test->TestEqual(TEXT("Cord consumed once"),I->Count(TEXT("Item_Cord")),1);
                Test->TestEqual(TEXT("Wood club damage"),I->MeleeDamage(),40.f);Test->TestEqual(TEXT("Wood club no gather benefit"),I->GatheringHits(),1);
                if(Menu.IsValid()){Press(EKeys::Down);Next(23,Now);}else{Start(PC,TEXT("Recipe_BoundClub"));Next(5,Now);}
            }
            else if(LocalStage==23 && Now-Changed>.4)
            {CheckMenu(TEXT("Recipe_BoundClub"),TEXT("Item_BoundClub"),60);Shot(TEXT("bound_club_selected"));Start(PC,TEXT("Recipe_BoundClub"));Next(5,Now);}
            else if(LocalStage==5 && Ready(PC))
            {CheckBag(I);if(!Privacy(W,PC)){return false;}if(Menu.IsValid()){Next(24,Now);}else{FinishLocal(Now);}}
            else if(LocalStage==24 && Now-Changed>.4)
            {
                Menu->RefreshMenu();auto* Result=Cast<UTextBlock>(Menu->WidgetTree->FindWidget(TEXT("PF_CraftingPanel_Result")));
                Test->TestTrue(TEXT("Completed result visible"),Result && Result->GetText().ToString().Contains(TEXT("Completed")));Shot(TEXT("bound_club_complete"));Next(25,Now);
            }
            else if(LocalStage==25 && Now-Changed>.5)
            {Press(EKeys::C);Test->TestFalse(TEXT("Close restores gameplay input"),PC->IsCraftingOpen());PC->SetPauseMenuOpen(true);Test->TestTrue(TEXT("Pause opens after crafting"),PC->IsPauseMenuOpen());PC->SetPauseMenuOpen(false);FinishLocal(Now);}
        }
        if(!bClient && ServerStage==1)
        {
            for(auto* P:Players){if(!Ready(P)){return false;}}
            if(Solo && LocalStage!=99){return false;}ServerStage=2;ServerChanged=Now;
        }
        if(!bClient && ServerStage==2 && Now-ServerChanged>1)
        {
            for(auto* P:Players){CheckBag(P->GetInventory());}
            if(!Slot.IsEmpty()){FString Error;Test->TestTrue(TEXT("Real isolated weapon world save"),W->GetSubsystem<UPFWorldPersistence>()->Save(Slot,Error));if(!Error.IsEmpty()){Test->AddError(Error);}}
            Test->AddInfo(TEXT("[PrimalAgentTools] Weapon server conservation and save verified"));Finished=true;Changed=Now;
        }
        return false;
    }
private:
    bool Ready(APFSurvivalPlayerController* PC){return PC->GetInventory()->Count(TEXT("Item_BoundClub"))==1 && PC->GetCrafting()->ActiveRecipe.IsNone() && CastChecked<APFSurvivorCharacter>(PC->GetPawn())->GetHeldMeleeItem()==TEXT("Item_BoundClub");}
    void Next(int32 Stage,double Now){LocalStage=Stage;Changed=Now;const FString M=FString::Printf(TEXT("[PrimalAgentTools] Weapon local stage=%d"),Stage);Test->AddInfo(M);UE_LOG(LogTemp,Display,TEXT("%s"),*M);}
    void FinishLocal(double Now){Test->AddInfo(TEXT("[PrimalAgentTools] Weapon owned RPC loop, tier and privacy verified"));LocalStage=99;if(bClient){Finished=true;Changed=Now;}}
    void CheckBag(UPFInventoryComponent* I)
    {Test->TestEqual(TEXT("One bound club"),I->Count(TEXT("Item_BoundClub")),1);for(const TCHAR* Id:{TEXT("Item_Club"),TEXT("Item_Wood"),TEXT("Item_Cord"),TEXT("Item_Stone")}){Test->TestEqual(TEXT("Exact upgrade costs/no duplicate"),I->Count(Id),0);}Test->TestEqual(TEXT("Weapon damage"),I->MeleeDamage(),60.f);Test->TestEqual(TEXT("No weapon gathering benefit"),I->GatheringHits(),1);}
    bool Privacy(UWorld* W,APFSurvivalPlayerController* PC)
    {
        int32 Others=0,ReadyOthers=0;for(TActorIterator<APlayerState> It(W);It;++It){if(*It!=PC->PlayerState){++Others;if(auto* Bag=It->FindComponentByClass<UPFInventoryComponent>()){Test->TestTrue(TEXT("Foreign inventory private"),Bag->GetStacks().IsEmpty());}}}
        for(TActorIterator<APFSurvivorCharacter> It(W);It;++It){if(*It!=PC->GetPawn() && It->GetHeldMeleeItem()==TEXT("Item_BoundClub")){++ReadyOthers;}}
        return Others==Expected-1 && ReadyOthers==Expected-1;
    }
    void Press(FKey K){auto& A=FSlateApplication::Get();A.ProcessKeyDownEvent(FKeyEvent(K,FModifierKeysState(),0,false,0,0));A.ProcessKeyUpEvent(FKeyEvent(K,FModifierKeysState(),0,false,0,0));}
    void Start(APFSurvivalPlayerController* PC,FName Id){if(Menu.IsValid()){Test->TestEqual(TEXT("Selected request matches"),Menu->GetSelectedRecipe(),Id);Press(EKeys::Enter);}else{PC->ServerCraftAction(Id,false);}}
    void Shot(const TCHAR* Name){const FString Path=Directory/(FString(Name)+TEXT(".png"));Shots.Add(Path);FScreenshotRequest::RequestScreenshot(Path,true,false);}
    void CheckMenu(FName Recipe,FName Item,int32 Damage)
    {
        Test->TestEqual(TEXT("Real modal recipe selection"),Menu->GetSelectedRecipe(),Recipe);
        auto* Pic=Cast<UPFItemPicture>(Menu->WidgetTree->FindWidget(TEXT("PF_SelectedItemPicture")));Test->TestTrue(TEXT("Correct original weapon picture"),Pic && Pic->GetItemId()==Item && Pic->GetCachedGeometry().GetLocalSize().X>0);
        auto* Stats=Cast<UTextBlock>(Menu->WidgetTree->FindWidget(TEXT("PF_CraftingDetail_Stats")));Test->TestTrue(TEXT("Honest weapon statistics visible"),Stats && Stats->GetText().ToString().Contains(FString::Printf(TEXT("%d melee damage | no gathering benefit"),Damage)));
        const auto Root=Menu->GetCachedGeometry();auto* Panel=Menu->WidgetTree->FindWidget(TEXT("PF_CraftingPanel"));const auto G=Panel->GetCachedGeometry();
        const auto TL=Root.AbsoluteToLocal(G.LocalToAbsolute(FVector2D::ZeroVector)),BR=Root.AbsoluteToLocal(G.LocalToAbsolute(G.GetLocalSize())),Size=Root.GetLocalSize();
        Test->TestTrue(TEXT("Centered menu fits viewport"),TL.X>=0 && TL.Y>=0 && BR.X<Size.X && BR.Y<Size.Y && FMath::Abs(TL.X+BR.X-Size.X)<4);
        TArray<UWidget*> Children;Menu->WidgetTree->GetAllWidgets(Children);for(auto* Child:Children){if(auto* Text=Cast<UTextBlock>(Child)){Test->TestTrue(TEXT("Menu text fits allocated row"),Text->GetDesiredSize().Y<=Text->GetCachedGeometry().GetLocalSize().Y+1);}}
    }
    FAutomationTestBase* Test;FString Directory,Slot,LastObserved;TArray<FString> Shots;TWeakObjectPtr<UPFCraftingHUD> Menu;
    double Started=FPlatformTime::Seconds(),Changed=0,ServerChanged=0;int32 Expected=1,ServerStage=0,LocalStage=0;bool bRestore=false,bClient=false,Finished=false,bScaleChanged=false;float OldScale=1;
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFWeaponLiveTest,"PF.Crafting.WeaponProgressionLive",EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)
bool FPFWeaponLiveTest::RunTest(const FString&)
{
    FString Slot,Directory;int32 Expected=1;FParse::Value(FCommandLine::Get(),TEXT("PFExpectedPlayers="),Expected);
    if(!FParse::Param(FCommandLine::Get(),TEXT("PFRunWeaponProgressionTests")) || (Expected!=1 && Expected!=2)){AddError(TEXT("Requires isolated -PFRunWeaponProgressionTests -PFExpectedPlayers=1|2"));return false;}
    if(!FParse::Param(FCommandLine::Get(),TEXT("nullrhi")))
    {
        FString Label;FParse::Value(FCommandLine::Get(),TEXT("PFControlsEvidence="),Label);
        if(!FParse::Param(FCommandLine::Get(),TEXT("PFRunControlsUITest")) || Label.IsEmpty() || Label.Len()>48){AddError(TEXT("Rendered weapon test requires isolated controls runner"));return false;}
        for(TCHAR C:Label){if(!FChar::IsAlnum(C) && C!=TEXT('_')){return false;}}
        Directory=FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()/TEXT("AutomationReports/ControlsUI")/Label);
        if(IFileManager::Get().DirectoryExists(*Directory)){AddError(TEXT("Refusing reused evidence"));return false;}IFileManager::Get().MakeDirectory(*Directory,true);
    }
    else if(!FParse::Value(FCommandLine::Get(),TEXT("PFSaveSlot="),Slot) || !Slot.StartsWith(TEXT("AutomationM12")) || !FPFSaveFileStore::ValidSlot(Slot))
    {AddError(TEXT("Headless weapon test requires disposable AutomationM12 save slot"));return false;}
    ADD_LATENT_AUTOMATION_COMMAND(FWeaponLiveExercise(this,Directory));return true;
}
#endif
