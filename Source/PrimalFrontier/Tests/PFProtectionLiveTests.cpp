// Disposable, opt-in crafting, damage, replication and restart exercise.
#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Crafting/PFCraftingComponent.h"
#include "Crafting/PFCraftingHUD.h"
#include "Inventory/PFInventoryComponent.h"
#include "Inventory/PFInventoryHUD.h"
#include "Persistence/PFWorldPersistence.h"
#include "Persistence/PFSaveFileStore.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Survival/PFSurvivorCharacter.h"
#include "Creatures/PFCreature.h"
#include "Settings/PFGameUserSettings.h"
#include "UI/PFItemPicture.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/InputComponent.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/PlayerState.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/DamageEvents.h"
#include "EngineUtils.h"
#include "UnrealClient.h"
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
namespace
{
class FProtectionLiveExercise final : public IAutomationLatentCommand
{
public:
    FProtectionLiveExercise(FAutomationTestBase* T,FString D):Test(T),Directory(MoveTemp(D))
    {FParse::Value(FCommandLine::Get(),TEXT("PFExpectedPlayers="),Expected);FParse::Value(FCommandLine::Get(),TEXT("PFSaveSlot="),Slot);bRestore=FParse::Param(FCommandLine::Get(),TEXT("PFLoadSave"));}
    ~FProtectionLiveExercise(){if(bScaleChanged && UPFGameUserSettings::Get()){UPFGameUserSettings::Get()->Preferences.HUDScale=OldScale;}}
    bool Update() override
    {
        const double Now=FPlatformTime::Seconds();
        if(Finished){if(Now-Changed<(bClient?3:12)){return false;}for(const auto& S:Shots){Test->TestTrue(TEXT("Screenshot written"),IFileManager::Get().FileSize(*S)>0);}return true;}
        if(Now-Started>120){Test->AddError(FString::Printf(TEXT("[PrimalAgentTools] Protection timeout server=%d local=%d %s"),ServerStage,LocalStage,*LastObserved));return true;}
        UWorld* W=nullptr;for(const auto& X:GEngine->GetWorldContexts()){if(X.World() && X.World()->IsGameWorld()){W=X.World();break;}}
        if(!W){return false;}bClient=W->GetNetMode()==NM_Client;const bool Solo=W->GetNetMode()==NM_Standalone;
        TArray<APFSurvivalPlayerController*> Players;for(auto It=W->GetPlayerControllerIterator();It;++It){auto* PC=Cast<APFSurvivalPlayerController>(It->Get());if(PC && PC->GetPawn() && PC->GetInventory() && PC->GetCrafting()){Players.Add(PC);}}
        if(Players.Num()!=(bClient?1:Expected)){return false;}auto* PC=Players[0];auto* I=PC->GetInventory();auto* C=PC->GetCrafting();
        auto* Pawn=CastChecked<APFSurvivorCharacter>(PC->GetPawn());
        LastObserved=FString::Printf(TEXT("guard=%d fibre=%d active=%s health=%.1f"),I->Count(TEXT("Item_WovenGuard")),I->Count(TEXT("Item_Fibre")),*C->ActiveRecipe.ToString(),Pawn->Survival->GetVitals().Health);
        if(bRestore)
        {
            for(auto* P:Players){if(!Ready(P)){return false;}}
            if(bClient)
            {
                if(PC->GetPlayerSetupStatus()!=EPFPlayerSetupStatus::Restored || !Privacy(W,PC)){return false;}CheckBag(I);FString Error;
                Test->TestFalse(TEXT("Client cannot save restored world"),W->GetSubsystem<UPFWorldPersistence>()->Save(Slot,Error));
                Test->AddInfo(TEXT("[PrimalAgentTools] Protection restart client identity, health and privacy verified"));
            }
            else
            {
                FPFWorldSaveData Snapshot;FString Error;Test->TestTrue(TEXT("Actual restored capture"),W->GetSubsystem<UPFWorldPersistence>()->Capture(Snapshot,Error));
                Test->TestEqual(TEXT("Original player records"),Snapshot.Players.Num(),Expected);Test->TestTrue(TEXT("Restored capture validates"),W->GetSubsystem<UPFWorldPersistence>()->Validate(Snapshot,Error));
                for(auto* P:Players){CheckBag(P->GetInventory());}Test->AddInfo(TEXT("[PrimalAgentTools] Protection restart server restored every owner"));
            }
            Finished=true;Changed=Now;return false;
        }
        if(!bClient && ServerStage==0)
        {
            int32 Index=0;for(auto* P:Players)
            {
                if(!P->GetInventory()->GetStacks().IsEmpty()){Test->AddError(TEXT("Refusing non-fresh protection fixture"));return true;}
                auto* Needs=P->GetPawn()->FindComponentByClass<UPFPlayerSurvivalComponent>();Needs->HungerDrainPerSecond=0;Needs->ThirstDrainPerSecond=0;
                P->GetPawn()->SetActorLocation(FVector(-1200,Index++*400,100));P->SetControlRotation(FRotator::ZeroRotator);
                Test->TestTrue(TEXT("Fixture fibre"),P->GetInventory()->Grant(TEXT("Item_Fibre"),8));Test->TestTrue(TEXT("Fixture cord"),P->GetInventory()->Grant(TEXT("Item_Cord"),1));Test->TestTrue(TEXT("Fixture wood"),P->GetInventory()->Grant(TEXT("Item_Wood"),2));
            }
            ServerStage=1;
        }
        if(bClient || Solo)
        {
            if(LocalStage==0 && I->Count(TEXT("Item_Fibre"))==8 && I->Count(TEXT("Item_Cord"))==1 && I->Count(TEXT("Item_Wood"))==2)
            {
                if(bClient){Test->TestFalse(TEXT("Client direct craft refused"),C->Start(TEXT("Recipe_WovenGuard"),Pawn));Test->TestFalse(TEXT("Client direct gear creation refused"),I->Grant(TEXT("Item_WovenGuard"),1));Test->TestEqual(TEXT("Client direct damage refused"),Pawn->TakeDamage(20,FDamageEvent(),nullptr,nullptr),0.f);}
                if(!Directory.IsEmpty())
                {
                    if(auto* S=UPFGameUserSettings::Get()){OldScale=S->Preferences.HUDScale;float Scale=1;FParse::Value(FCommandLine::Get(),TEXT("PFHUDScale="),Scale);S->Preferences.HUDScale=Scale;bScaleChanged=true;}
                    PC->SetCraftingMenuOpen(true);TArray<UUserWidget*> Widgets;UWidgetBlueprintLibrary::GetAllWidgetsOfClass(PC,Widgets,UPFCraftingHUD::StaticClass(),false);
                    if(Widgets.Num()!=1){Test->AddError(TEXT("Missing crafting menu"));return true;}Menu=CastChecked<UPFCraftingHUD>(Widgets[0]);Next(20,Now);
                }
                else{Start(PC);Next(1,Now);}
            }
            else if(LocalStage==20 && Now-Changed>.5)
            {
                Test->TestEqual(TEXT("All eight recipes"),Menu->GetVisibleRecipeIds().Num(),8);auto* B=Cast<UButton>(Menu->WidgetTree->FindWidget(TEXT("PF_Category5_Button")));
                if(!B){Test->AddError(TEXT("Missing Protection category"));return true;}B->SetKeyboardFocus();Test->TestTrue(TEXT("Actual category Slate focus"),B->HasKeyboardFocus());Press(EKeys::SpaceBar);Next(21,Now);
            }
            else if(LocalStage==21 && Now-Changed>.4)
            {Test->TestEqual(TEXT("One Protection recipe"),Menu->GetVisibleRecipeIds().Num(),1);Test->TestEqual(TEXT("Clear category selection"),Menu->GetSelectedRecipe(),NAME_None);Press(EKeys::Down);Next(22,Now);}
            else if(LocalStage==22 && Now-Changed>.4)
            {
                Test->TestEqual(TEXT("Guard selected by real input"),Menu->GetSelectedRecipe(),FName(TEXT("Recipe_WovenGuard")));
                auto* Pic=Cast<UPFItemPicture>(Menu->WidgetTree->FindWidget(TEXT("PF_SelectedItemPicture")));Test->TestTrue(TEXT("Original guard picture"),Pic && Pic->GetItemId()==TEXT("Item_WovenGuard"));
                auto* Stats=Cast<UTextBlock>(Menu->WidgetTree->FindWidget(TEXT("PF_CraftingDetail_Stats")));Test->TestTrue(TEXT("Honest reduction and exclusion"),Stats && Stats->GetText().ToString().Contains(TEXT("25% creature hit reduction")) && Stats->GetText().ToString().Contains(TEXT("environmental damage unaffected")));
                Bounds(Menu.Get(),TEXT("PF_CraftingPanel"),true);Shot(TEXT("guard_selected"));Start(PC);Next(1,Now);
            }
            else if(LocalStage==1 && C->ActiveRecipe==TEXT("Recipe_WovenGuard") && Now-Changed>.6)
            {PC->ServerCraftAction(TEXT("Recipe_WovenGuard"),false);Next(2,Now);}
            else if(LocalStage==2 && Now-Changed>.6)
            {Test->TestEqual(TEXT("Duplicate keeps one job"),C->ActiveRecipe,FName(TEXT("Recipe_WovenGuard")));PC->ServerCraftAction(NAME_None,true);Next(3,Now);}
            else if(LocalStage==3 && C->ActiveRecipe.IsNone() && Now-Changed>.6)
            {Test->TestEqual(TEXT("Cancel preserves fibre"),I->Count(TEXT("Item_Fibre")),8);Test->TestEqual(TEXT("No canceled output"),I->Count(TEXT("Item_WovenGuard")),0);Start(PC);Next(4,Now);}
            else if(LocalStage==4 && Ready(PC))
            {CheckBag(I);if(!Privacy(W,PC)){return false;}if(Menu.IsValid()){Next(23,Now);}else{FinishLocal(Now);}}
            else if(LocalStage==23 && Now-Changed>.4)
            {Menu->RefreshMenu();auto* R=Cast<UTextBlock>(Menu->WidgetTree->FindWidget(TEXT("PF_CraftingPanel_Result")));Test->TestTrue(TEXT("Completed feedback visible"),R && R->GetText().ToString().Contains(TEXT("Completed")));Shot(TEXT("guard_complete"));Next(24,Now);}
            else if(LocalStage==24 && Now-Changed>.5)
            {
                Press(EKeys::C);Test->TestFalse(TEXT("Craft Close returns input"),PC->IsCraftingOpen());
                for(const auto& B:PC->InputComponent->KeyBindings){if(B.Chord.Key==EKeys::Tab && B.KeyEvent==IE_Pressed){B.KeyDelegate.Execute(EKeys::Tab);break;}}
                Test->TestTrue(TEXT("Real inventory binding opens bag"),PC->IsInventoryOpen());Next(25,Now);
            }
            else if(LocalStage==25 && Now-Changed>.5)
            {
                TArray<UUserWidget*> Widgets;UWidgetBlueprintLibrary::GetAllWidgetsOfClass(PC,Widgets,UPFInventoryHUD::StaticClass(),false);
                if(Widgets.Num()!=1){Test->AddError(TEXT("Missing actual bag HUD"));return true;}auto* HUD=Widgets[0];auto* Body=Cast<UTextBlock>(HUD->WidgetTree->FindWidget(TEXT("PF_InventoryDetailBody")));
                Test->TestTrue(TEXT("Actual bag shows carried protection"),Body && Body->GetText().ToString().Contains(TEXT("25% creature hit reduction")));Bounds(HUD,TEXT("PF_InventoryPanel"),false);Shot(TEXT("guard_inventory"));Next(26,Now);
            }
            else if(LocalStage==26 && Now-Changed>.5)
            {for(const auto& B:PC->InputComponent->KeyBindings){if(B.Chord.Key==EKeys::Tab && B.KeyEvent==IE_Pressed){B.KeyDelegate.Execute(EKeys::Tab);break;}}Test->TestFalse(TEXT("Bag closes normally"),PC->IsInventoryOpen());PC->SetPauseMenuOpen(true);Test->TestTrue(TEXT("Pause after bag works"),PC->IsPauseMenuOpen());PC->SetPauseMenuOpen(false);FinishLocal(Now);}
        }
        if(!bClient && ServerStage==1)
        {
            for(auto* P:Players){if(P->GetInventory()->Count(TEXT("Item_WovenGuard"))!=1 || !P->GetCrafting()->ActiveRecipe.IsNone()){return false;}}
            for(auto* P:Players)
            {
                FTransform At(FRotator::ZeroRotator,P->GetPawn()->GetActorLocation()+FVector(200,200,0));
                auto* Creature=W->SpawnActorDeferred<APFCreature>(APFCreature::StaticClass(),At,nullptr,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);Creature->FinishSpawning(At);Creature->SetActorTickEnabled(false);
                Test->TestEqual(TEXT("Actual server creature twenty damage reduces to fifteen"),P->GetPawn()->TakeDamage(20,FDamageEvent(),nullptr,Creature),15.f);Creature->Destroy();
            }
            ServerStage=2;ServerChanged=Now;
        }
        if(!bClient && ServerStage==2 && Now-ServerChanged>1 && (!Solo || LocalStage==99))
        {
            for(auto* P:Players){CheckBag(P->GetInventory());Test->TestTrue(TEXT("Actual server health retained"),Ready(P));}
            if(!Slot.IsEmpty()){FString Error;Test->TestTrue(TEXT("Actual isolated protection save"),W->GetSubsystem<UPFWorldPersistence>()->Save(Slot,Error));if(!Error.IsEmpty()){Test->AddError(Error);}}
            Test->AddInfo(TEXT("[PrimalAgentTools] Protection server damage, conservation and save verified"));Finished=true;Changed=Now;
        }
        return false;
    }
private:
    bool Ready(APFSurvivalPlayerController* P){return P->GetInventory()->Count(TEXT("Item_WovenGuard"))==1 && P->GetCrafting()->ActiveRecipe.IsNone() && FMath::IsNearlyEqual(CastChecked<APFSurvivorCharacter>(P->GetPawn())->Survival->GetVitals().Health,85.f);}
    void Next(int32 S,double Now){LocalStage=S;Changed=Now;const FString M=FString::Printf(TEXT("[PrimalAgentTools] Protection local stage=%d"),S);Test->AddInfo(M);UE_LOG(LogTemp,Display,TEXT("%s"),*M);}
    void FinishLocal(double Now){Test->AddInfo(TEXT("[PrimalAgentTools] Protection owned RPC loop, health and privacy verified"));LocalStage=99;if(bClient){Finished=true;Changed=Now;}}
    void CheckBag(UPFInventoryComponent* I){Test->TestEqual(TEXT("One guard/no duplication"),I->Count(TEXT("Item_WovenGuard")),1);for(const TCHAR* Id:{TEXT("Item_Fibre"),TEXT("Item_Cord"),TEXT("Item_Wood")}){Test->TestEqual(TEXT("Exact cost"),I->Count(Id),0);}Test->TestEqual(TEXT("Nonstacking carried protection"),I->CreatureHitReduction(),.25f);Test->TestEqual(TEXT("No gathering benefit"),I->GatheringHits(),1);Test->TestEqual(TEXT("No melee benefit"),I->MeleeDamage(),20.f);}
    bool Privacy(UWorld* W,APFSurvivalPlayerController* PC)
    {int32 Others=0,Healthy=0;for(TActorIterator<APlayerState> It(W);It;++It){if(*It!=PC->PlayerState){++Others;if(auto* Bag=It->FindComponentByClass<UPFInventoryComponent>()){Test->TestTrue(TEXT("Foreign gear inventory stays private"),Bag->GetStacks().IsEmpty());}}}for(TActorIterator<APFSurvivorCharacter> It(W);It;++It){if(*It!=PC->GetPawn() && FMath::IsNearlyEqual(It->Survival->GetVitals().Health,85.f)){++Healthy;}}return Others==Expected-1 && Healthy==Expected-1;}
    void Press(FKey K){auto& A=FSlateApplication::Get();A.ProcessKeyDownEvent(FKeyEvent(K,FModifierKeysState(),0,false,0,0));A.ProcessKeyUpEvent(FKeyEvent(K,FModifierKeysState(),0,false,0,0));}
    void Start(APFSurvivalPlayerController* P){if(Menu.IsValid()){Press(EKeys::Enter);}else{P->ServerCraftAction(TEXT("Recipe_WovenGuard"),false);}}
    void Shot(const TCHAR* N){const FString P=Directory/(FString(N)+TEXT(".png"));Shots.Add(P);FScreenshotRequest::RequestScreenshot(P,true,false);}
    void Bounds(UUserWidget* HUD,const TCHAR* Name,bool Center)
    {
        const auto Root=HUD->GetCachedGeometry();const auto Size=Root.GetLocalSize();auto* Panel=HUD->WidgetTree->FindWidget(Name);Test->TestNotNull(TEXT("Actual HUD panel"),Panel);if(!Panel){return;}
        const auto G=Panel->GetCachedGeometry();const auto TL=Root.AbsoluteToLocal(G.LocalToAbsolute(FVector2D::ZeroVector)),BR=Root.AbsoluteToLocal(G.LocalToAbsolute(G.GetLocalSize()));
        Test->TestTrue(TEXT("Protection HUD fits viewport"),TL.X>=0 && TL.Y>=0 && BR.X<Size.X && BR.Y<Size.Y && (Center?FMath::Abs(TL.X+BR.X-Size.X)<4:TL.X>Size.X*.55));
        TArray<UWidget*> Children;HUD->WidgetTree->GetAllWidgets(Children);for(auto* X:Children){if(auto* T=Cast<UTextBlock>(X)){Test->TestTrue(TEXT("Protection HUD text fits allocated height"),T->GetDesiredSize().Y<=T->GetCachedGeometry().GetLocalSize().Y+1);}}
    }
    FAutomationTestBase* Test;FString Directory,Slot,LastObserved;TArray<FString> Shots;TWeakObjectPtr<UPFCraftingHUD> Menu;
    double Started=FPlatformTime::Seconds(),Changed=0,ServerChanged=0;int32 Expected=1,ServerStage=0,LocalStage=0;bool bRestore=false,bClient=false,Finished=false,bScaleChanged=false;float OldScale=1;
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFProtectionLiveTest,"PF.Crafting.ProtectionLive",EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)
bool FPFProtectionLiveTest::RunTest(const FString&)
{
    FString Slot,Directory;int32 Expected=1;FParse::Value(FCommandLine::Get(),TEXT("PFExpectedPlayers="),Expected);
    if(!FParse::Param(FCommandLine::Get(),TEXT("PFRunProtectionTests")) || (Expected!=1 && Expected!=2)){AddError(TEXT("Requires isolated -PFRunProtectionTests -PFExpectedPlayers=1|2"));return false;}
    if(!FParse::Param(FCommandLine::Get(),TEXT("nullrhi")))
    {
        FString Label;FParse::Value(FCommandLine::Get(),TEXT("PFControlsEvidence="),Label);
        if(!FParse::Param(FCommandLine::Get(),TEXT("PFRunControlsUITest")) || Label.IsEmpty() || Label.Len()>48){AddError(TEXT("Requires disposable controls evidence"));return false;}
        for(TCHAR C:Label){if(!FChar::IsAlnum(C) && C!=TEXT('_')){return false;}}
        Directory=FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()/TEXT("AutomationReports/ControlsUI")/Label);
        if(IFileManager::Get().DirectoryExists(*Directory)){AddError(TEXT("Refusing reused evidence"));return false;}IFileManager::Get().MakeDirectory(*Directory,true);
    }
    else if(!FParse::Value(FCommandLine::Get(),TEXT("PFSaveSlot="),Slot) || !Slot.StartsWith(TEXT("AutomationM12")) || !FPFSaveFileStore::ValidSlot(Slot)){AddError(TEXT("Requires disposable AutomationM12 slot"));return false;}
    ADD_LATENT_AUTOMATION_COMMAND(FProtectionLiveExercise(this,Directory));return true;
}
#endif
