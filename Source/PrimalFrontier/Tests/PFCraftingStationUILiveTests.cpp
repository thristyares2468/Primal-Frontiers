#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Crafting/PFCraftingStation.h"
#include "Crafting/PFCraftingComponent.h"
#include "Crafting/PFCraftingHUD.h"
#include "Building/PFBuildingComponent.h"
#include "Building/PFBuildPiece.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFSurvivorCharacter.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Inventory/PFInventoryPlayerState.h"
#include "Progression/PFProgressionComponent.h"
#include "Creatures/PFCreatureSpawner.h"
#include "Creatures/PFCreature.h"
#include "World/PFWorldClock.h"
#include "Settings/PFGameUserSettings.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "UnrealClient.h"
#include "HAL/FileManager.h"
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
namespace
{
class FStationUI final : public IAutomationLatentCommand
{
public:
    FStationUI(FAutomationTestBase* T,FString D):Test(T),Directory(MoveTemp(D))
    {bOwned=FParse::Param(FCommandLine::Get(),TEXT("PFRunOwnedStationUITest"));bBatch=FParse::Param(FCommandLine::Get(),TEXT("PFRunBatchRefiningUITest"));}
    bool Update() override
    {
        const double Now=FPlatformTime::Seconds();
        if(Now-Started>120){Test->AddError(FString::Printf(TEXT("Station UI timeout stage%d"),Stage));return true;}
        if(Stage==99){if(Now-Changed<2){return false;}for(const auto& Shot:Shots){Test->TestTrue(TEXT("Screenshot emitted"),IFileManager::Get().FileSize(*Shot)>0);}return true;}
        UWorld* W=nullptr;for(const auto& X:GEngine->GetWorldContexts()){if(X.World() && X.World()->IsGameWorld()){W=X.World();break;}}if(!W){return false;}
        APFSurvivalPlayerController* PC=nullptr;for(auto It=W->GetPlayerControllerIterator();It;++It){PC=Cast<APFSurvivalPlayerController>(It->Get());if(PC){break;}}
        if(!PC || !PC->GetInventory() || !PC->GetCrafting() || !PC->GetPawn()){return false;}
        auto* P=CastChecked<APFSurvivorCharacter>(PC->GetPawn());auto* I=PC->GetInventory();auto* C=PC->GetCrafting();
        if(Stage==0)
        {
            if(W->GetNetMode()!=NM_Standalone || UWorld::RemovePIEPrefix(W->GetMapName())!=TEXT("L_PrimalFrontier_OpenWorld") || !I->GetStacks().IsEmpty()){Test->AddError(TEXT("Requires disposable authored standalone world"));return true;}
            for(TActorIterator<APFCraftingStation> It(W);It;++It){if(It->GetName()==TEXT("PF_Camp_Workbench")){Bench=*It;}if(It->GetName()==TEXT("PF_Camp_Refinery")){Refinery=*It;}}
            if(!Bench || !Refinery){Test->AddError(TEXT("Missing saved camp stations"));return true;}
            float Scale=1;FParse::Value(FCommandLine::Get(),TEXT("PFHUDScale="),Scale);
            if(!FMath::IsFinite(Scale) || Scale<.75f || Scale>1.5f || !UPFGameUserSettings::Get()){Test->AddError(TEXT("Invalid station UI scale fixture"));return true;}
            UPFGameUserSettings::Get()->Preferences.HUDScale=Scale;RequestedScale=Scale;
            Test->TestTrue(TEXT("Exact saved workbench position"),Bench->GetActorLocation().Equals(FVector(700,-500,0),1));
            Test->TestTrue(TEXT("Exact saved refinery position"),Refinery->GetActorLocation().Equals(FVector(1100,-500,0),1));
            for(TActorIterator<APFCreatureSpawner> It(W);It;++It){It->bAutoSpawn=false;}for(TActorIterator<APFCreature> It(W);It;++It){It->Destroy();}
            for(TActorIterator<APFWorldClock> It(W);It;++It){It->SetActorTickEnabled(false);It->SetHour(10);}
            P->Survival->HungerDrainPerSecond=0;P->Survival->ThirstDrainPerSecond=0;P->GetCharacterMovement()->DisableMovement();
            auto* G=PC->GetPlayerState<APFInventoryPlayerState>()->Progression.Get();FPFProgressionRecord Seed;Seed.Experience=700;FString Error;
            if(!G->Restore(Seed,Error) || !G->RequestKnowledge(TEXT("Tech_FieldTools"),P,Error) || !I->Grant(TEXT("Item_Wood"),6) || !I->Grant(TEXT("Item_Stone"),2) || !I->Grant(TEXT("Item_CopperOre"),2)){Test->AddError(TEXT("Station boundary fixture setup failed"));return true;}
            if(bBatch)
            {
                if(!I->Grant(TEXT("Item_CopperOre"),6)){Test->AddError(TEXT("Batch ore fixture refused"));return true;}
                P->SetActorLocation(FVector(-1400,-1100,100));Select(PC,TEXT("Recipe_CopperBatch"));Stage=80;Changed=Now;return false;
            }
            if(bOwned)
            {
                if(!I->Grant(TEXT("Item_Wood"),22)){Test->AddError(TEXT("Owned build material fixture refused"));return true;}
                P->SetActorLocation(FVector(-1200,-1100,100));
                auto Aim=[&](FVector At){FVector Eye;FRotator Look;P->GetActorEyesViewPoint(Eye,Look);PC->SetControlRotation((At-Eye).Rotation());};
                Aim(FVector(-800,-1200,0));auto* Base=PC->Building->Place(TEXT("Build_Foundation"),0);
                if(!Base){Test->AddError(TEXT("Owned fixture foundation placement failed"));return true;}
                Aim(Base->GetActorLocation()+FVector(-100,-100,10));auto* Work=PC->Building->Place(TEXT("Build_Workbench"),0);
                Aim(Base->GetActorLocation()+FVector(100,100,10));auto* Refine=PC->Building->Place(TEXT("Build_Refinery"),0);
                if(!Work || !Refine){Test->AddError(TEXT("Owned fixture station placement failed"));return true;}
                Bench=nullptr;Refinery=nullptr;
                for(TActorIterator<APFCraftingStation> It(W);It;++It){if(It->ProviderPiece==Work){Bench=*It;}if(It->ProviderPiece==Refine){Refinery=*It;}}
                if(!Bench || !Refinery){Test->AddError(TEXT("Owned fixture provider missing"));return true;}
                Test->TestEqual(TEXT("Actual foundation/bench/refinery charge22wood"),I->Count(TEXT("Item_Wood")),6);
                PC->SetControlRotation((Base->GetActorLocation()+FVector(0,0,50)-P->GetActorLocation()).Rotation());
                Stage=70;Changed=Now;return false;
            }
            Approach(PC,Bench);Select(PC,TEXT("Recipe_Tool"));Next(Now);return false;
        }
        if(Now-Changed<.75){return false;}
        if(Stage==80)
        {
            if(!BodyContains(TEXT("Requires refinery"))){return true;}
            auto* Button=Cast<UButton>(Menu->WidgetTree->FindWidget(TEXT("PF_CraftSelected")));
            Test->TestTrue(TEXT("Missing refinery disables craft button"),Button && !Button->GetIsEnabled());
            if(!WorkbenchCaptured){Shot(TEXT("batch_required_refinery"));WorkbenchCaptured=true;Changed=Now;return false;}
            Press(EKeys::Enter);Test->TestTrue(TEXT("Enter cannot start absent refinery job"),C->ActiveRecipe.IsNone());Stage=81;Changed=Now;return false;
        }
        if(Stage==81){Approach(PC,Refinery);Select(PC,TEXT("Recipe_CopperBatch"));Stage=82;Changed=Now;return false;}
        if(Stage==82)
        {
            if(!BodyContains(TEXT("Camp refinery: 150%")) || !BodyContains(TEXT("Makes 2")) || !BodyContains(TEXT("Wood 6/1"))){return true;}
            if(!BatchCaptured){Shot(TEXT("batch_refinery_ready"));BatchCaptured=true;Changed=Now;return false;}
            Press(EKeys::Enter);Test->TestEqual(TEXT("Slate starts actual refinery batch"),C->ActiveRecipe,FName(TEXT("Recipe_CopperBatch")));Stage=83;Changed=Now;return false;
        }
        if(Stage==83)
        {
            Test->TestEqual(TEXT("Fuel not charged before completion"),I->Count(TEXT("Item_Wood")),6);Press(EKeys::R);Stage=84;Changed=Now;return false;
        }
        if(Stage==84)
        {
            Test->TestTrue(TEXT("Slate cancel keeps exact batch inputs"),C->ActiveRecipe.IsNone() && I->Count(TEXT("Item_Wood"))==6 && I->Count(TEXT("Item_CopperOre"))==8 && I->Count(TEXT("Item_CopperIngot"))==0);
            Press(EKeys::Enter);Stage=85;Changed=Now;return false;
        }
        if(Stage==85)
        {
            if(!C->ActiveRecipe.IsNone()){return false;}
            Test->TestTrue(TEXT("Rendered batch two outputs and one fuel"),I->Count(TEXT("Item_CopperIngot"))==2 && I->Count(TEXT("Item_CopperOre"))==4 && I->Count(TEXT("Item_Wood"))==5);
            Stage=86;Changed=Now;return false;
        }
        if(Stage==86)
        {
            if(!BodyContains(TEXT("Wood 5/1"))){return true;}
            auto* Result=Cast<UTextBlock>(Menu->WidgetTree->FindWidget(TEXT("PF_CraftingPanel_Result")));
            Test->TestTrue(TEXT("Completion result refreshed before screenshot"),Result && Result->GetText().ToString().Contains(TEXT("Completed")));
            Shot(TEXT("batch_refinery_completed"));Test->AddInfo(TEXT("[PrimalCrafting] Rendered batch refinery: missing-station button, real Slate start/cancel/complete, two ingots and exact one fuel verified. Assisted positioning/seed, not earned pacing/FPS."));Stage=99;Changed=Now;return false;
        }
        if(Stage==70){Shot(TEXT("owned_stations_placed"));Stage=71;Changed=Now;return false;}
        if(Stage==71){Approach(PC,Bench);Select(PC,TEXT("Recipe_Tool"));Stage=1;Changed=Now;return false;}
        if(Stage==1)
        {
            if(!BodyContains(bOwned?TEXT("Owned workbench: 125%"):TEXT("Camp workbench: 125%"))){return true;}
            // A screenshot is emitted at frame end: retain the panel until a later update.
            if(!WorkbenchCaptured){Shot(TEXT("workbench_menu"));WorkbenchCaptured=true;Changed=Now;return false;}
            Press(EKeys::Enter);
            Test->TestEqual(TEXT("Slate Enter starts server workbench job"),C->ActiveStationMultiplier,1.25f);Test->TestEqual(TEXT("Real workbench recipe started"),C->ActiveRecipe,FName(TEXT("Recipe_Tool")));
            PC->SetCraftingMenuOpen(false);P->SetActorLocation(Bench->GetActorLocation()+FVector(-900,0,100));Next(Now);return false;
        }
        if(Stage==2)
        {
            Test->TestTrue(TEXT("Leaving actual camp cancels"),C->ActiveRecipe.IsNone() && C->Feedback.Contains(TEXT("ingredients kept")));
            Test->TestEqual(TEXT("Cancelled recipe preserves inputs"),I->Count(TEXT("Item_Wood")),6);Test->TestEqual(TEXT("Cancel has no free tool"),I->Count(TEXT("Item_Tool")),0);
            Approach(PC,Bench);Select(PC,TEXT("Recipe_Tool"));Next(Now);return false;
        }
        if(Stage==3){Press(EKeys::Enter);Next(Now);return false;}
        if(Stage==4)
        {
            if(!C->ActiveRecipe.IsNone()){return false;}
            if(!Test->TestEqual(TEXT("One completed station tool"),I->Count(TEXT("Item_Tool")),1)){return true;}
            Approach(PC,Refinery);Select(PC,TEXT("Recipe_CopperRefine"));Next(Now);return false;
        }
        if(Stage==5)
        {
            if(!BodyContains(bOwned?TEXT("Owned refinery: 150%"):TEXT("Camp refinery: 150%"))){return true;}Shot(TEXT("refinery_menu"));Press(EKeys::Enter);
            Test->TestEqual(TEXT("Slate refinery owns 150 percent speed"),C->ActiveStationMultiplier,1.5f);Next(Now);return false;
        }
        if(Stage==6)
        {
            if(!C->ActiveRecipe.IsNone()){return false;}
            Test->TestEqual(TEXT("One ingot"),I->Count(TEXT("Item_CopperIngot")),1);Test->TestEqual(TEXT("Metal input consumed once"),I->Count(TEXT("Item_CopperOre")),0);Test->TestEqual(TEXT("Tool and ingot fuel conserve wood"),I->Count(TEXT("Item_Wood")),2);
            PC->SetCraftingMenuOpen(false);P->SetActorLocation(FVector(450,-950,100));PC->SetControlRotation(FRotator(-8,45,0));Shot(TEXT("camp_first_person"));
            Test->AddInfo(bOwned?TEXT("[PrimalCrafting] Rendered owned stations: actual server placement22wood, owner workbench/refinery Slate craft, leave cancellation and exact tool/ingot verified. Explicit700XP/material seed and assisted travel; no earned pacing/FPS claim."):TEXT("[PrimalCrafting] Rendered camp stations: saved fixtures, actual Slate craft, leave cancellation, workbench tool and refinery ingot verified. Explicit700XP/material boundary seed and assisted travel; not earned pacing or FPS certification."));Stage=99;Changed=Now;return false;
        }
        return false;
    }
private:
    void Next(double Now){++Stage;Changed=Now;}
    void Press(FKey Key){auto& A=FSlateApplication::Get();A.ProcessKeyDownEvent(FKeyEvent(Key,FModifierKeysState(),0,false,0,0));A.ProcessKeyUpEvent(FKeyEvent(Key,FModifierKeysState(),0,false,0,0));}
    void Approach(APFSurvivalPlayerController* PC,APFCraftingStation* Station){PC->SetCraftingMenuOpen(false);PC->GetPawn()->SetActorLocation(Station->GetActorLocation()+FVector(-150,0,100));FVector Eye;FRotator Look;PC->GetPawn()->GetActorEyesViewPoint(Eye,Look);PC->SetControlRotation((Station->GetActorLocation()+FVector(0,0,35)-Eye).Rotation());}
    void Select(APFSurvivalPlayerController* PC,FName Recipe){PC->SetCraftingMenuOpen(true);TArray<UUserWidget*> Widgets;UWidgetBlueprintLibrary::GetAllWidgetsOfClass(PC,Widgets,UPFCraftingHUD::StaticClass(),false);if(Widgets.Num()!=1){Test->AddError(TEXT("One crafting panel required"));return;}Menu=CastChecked<UPFCraftingHUD>(Widgets[0]);Menu->SelectCategory(FGameplayTag());Menu->SelectRecipe(Recipe);Menu->SetKeyboardFocus();}
    bool BodyContains(const TCHAR* Text){auto* Body=Menu.IsValid()?Cast<UTextBlock>(Menu->WidgetTree->FindWidget(TEXT("PF_CraftingPanel_Body"))):nullptr;return Test->TestTrue(TEXT("Actual station UI description at requested font scale"),Body && Body->GetFont().Size==FMath::RoundToInt(16*RequestedScale) && Body->GetText().ToString().Contains(Text));}
    void Shot(const TCHAR* Name){const FString Path=Directory/(FString(Name)+TEXT(".png"));Shots.Add(Path);FScreenshotRequest::RequestScreenshot(Path,true,false);}
    FAutomationTestBase* Test;FString Directory;TArray<FString> Shots;TWeakObjectPtr<UPFCraftingHUD> Menu;APFCraftingStation* Bench=nullptr;APFCraftingStation* Refinery=nullptr;
    float RequestedScale=1;bool WorkbenchCaptured=false,BatchCaptured=false,bOwned=false,bBatch=false;int32 Stage=0;double Started=FPlatformTime::Seconds(),Changed=0;
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFStationUILiveTest,"PF.Crafting.StationsUI",EAutomationTestFlags::ClientContext|EAutomationTestFlags::EngineFilter)
bool FPFStationUILiveTest::RunTest(const FString&)
{
    FString Label;FParse::Value(FCommandLine::Get(),TEXT("PFControlsEvidence="),Label);
    if(!FParse::Param(FCommandLine::Get(),TEXT("PFRunStationUITest")) || !FParse::Param(FCommandLine::Get(),TEXT("PFRunControlsUITest")) || FParse::Param(FCommandLine::Get(),TEXT("nullrhi")) || Label.IsEmpty() || Label.Len()>64){AddError(TEXT("Requires isolated rendered station UI runner"));return false;}
    for(TCHAR Ch:Label){if(!FChar::IsAlnum(Ch) && Ch!=TEXT('_')){return false;}}
    const FString Directory=FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()/TEXT("AutomationReports/ControlsUI")/Label);
    if(IFileManager::Get().DirectoryExists(*Directory)){AddError(TEXT("Refusing reused evidence"));return false;}IFileManager::Get().MakeDirectory(*Directory,true);
    ADD_LATENT_AUTOMATION_COMMAND(FStationUI(this,Directory));return true;
}
#endif
