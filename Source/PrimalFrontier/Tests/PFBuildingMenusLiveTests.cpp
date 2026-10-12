#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Building/PFBuildingMenus.h"
#include "Building/PFBuildingComponent.h"
#include "Building/PFBuildPiece.h"
#include "Inventory/PFInventoryComponent.h"
#include "Inventory/PFInventoryPlayerState.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Settings/PFGameUserSettings.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Components/InputComponent.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "UnrealClient.h"

namespace
{
class FBuildingMenusExercise : public IAutomationLatentCommand
{
public:
    FBuildingMenusExercise(FAutomationTestBase* T,FString D):Test(T),Directory(MoveTemp(D)){}
    bool Update() override
    {
        const double Now=FPlatformTime::Seconds();if(Now-Started>100){Test->AddError(TEXT("Building/chest UI timeout"));return true;}
        if(Stage==22)
        {
            auto* Character=CastChecked<ACharacter>(PC->GetPawn());Character->AddMovementInput(FVector(-1,0,0),1);
            if(Character->GetActorLocation().X<Base->GetActorLocation().X+50 && Character->GetActorLocation().Z>390)
            {Test->AddInfo(TEXT("[PrimalBuilding] Character physically climbed all stair risers to the upper platform"));Wait(Now,0.4,23);return false;}
            if(Now-WalkStarted>12){Test->AddError(TEXT("Real stair traversal failed at ")+Character->GetActorLocation().ToString());return true;}
            return false;
        }
        if(Now<Until){return false;}
        if(Stage==0)
        {
            for(const auto& C:GEngine->GetWorldContexts()){if(C.World() && C.World()->IsGameWorld()){PC=Cast<APFSurvivalPlayerController>(C.World()->GetFirstPlayerController());if(PC.IsValid() && PC->GetPawn() && PC->GetInventory()){break;}}}
            if(!PC.IsValid() || !PC->GetPawn() || !PC->GetInventory()){return false;}
            if(PC->GetNetMode()!=NM_Standalone || !PC->GetInventory()->GetStacks().IsEmpty() || PC->GetWorld()->GetMapName()!=TEXT("L_PrimalFrontier_OpenWorld")){Test->AddError(TEXT("Requires fresh standalone open-world fixture"));return true;}
            float Scale=1;FParse::Value(FCommandLine::Get(),TEXT("PFHUDScale="),Scale);UPFGameUserSettings::Get()->Preferences.HUDScale=FMath::Clamp(Scale,0.75f,1.5f);
            auto* V=PC->GetPawn()->FindComponentByClass<UPFPlayerSurvivalComponent>();V->HungerDrainPerSecond=0;V->ThirstDrainPerSecond=0;
            PC->GetPawn()->SetActorLocation(FVector(-1200,-1100,100));
            Test->TestTrue(TEXT("Fixture wood grant"),PC->GetInventory()->Grant(TEXT("Item_Wood"),14));
            Test->TestTrue(TEXT("Fixture stone grant"),PC->GetInventory()->Grant(TEXT("Item_Stone"),3));
            Test->TestTrue(TEXT("Fixture fresh food"),PC->GetInventory()->AddExisting(TEXT("Item_Food"),2,UPFInventoryComponent::ServerTime(PC->GetWorld())+90));
            for(const auto& S:PC->GetInventory()->GetStacks()){if(S.ItemId==TEXT("Item_Food")){Deadline=S.ExpiresAt;}}
            Picker=Find(UPFBuildingMenu::StaticClass());ChestUI=Find(UPFStorageMenu::StaticClass());
            if(!Picker.IsValid() || !ChestUI.IsValid()){Test->AddError(TEXT("Missing real building/chest widgets"));return true;}
            Press(EKeys::B);Wait(Now,0.4,1);return false;
        }
        auto* B=PC->Building.Get();auto* I=PC->GetInventory();
        if(Stage==1)
        {
            Test->TestTrue(TEXT("B opens a visible picker and cursor"),PC->IsBuildingMenuOpen() && Picker->IsVisible() && PC->bShowMouseCursor);
            Test->TestEqual(TEXT("All eight actual saved catalog choices"),B->Catalog->Pieces.Num(),8);
            CheckBounds(Picker.Get(),TEXT("PF_BuildPickerPanel"));
            for(const auto& D:B->Catalog->Pieces){Test->TestNotNull(TEXT("Actual selectable piece ")+D.Id.ToString(),Picker->WidgetTree->FindWidget(FName(*(TEXT("PF_BuildChoice_")+D.Id.ToString()))));}
            Capture(TEXT("eight_piece_picker"));Wait(Now,0.5,2);return false;
        }
        if(Stage==2)
        {
            if(!ShotExists()){return false;}
            const auto Id=B->Catalog->Pieces[Choice].Id;Activate(Picker.Get(),FName(*(TEXT("PF_BuildChoice_")+Id.ToString())));
            Test->TestEqual(TEXT("UI selects requested piece"),B->SelectedId(),Id);
            Test->TestFalse(TEXT("Choice releases modal input"),PC->IsBuildingMenuOpen() || PC->bShowMouseCursor || PC->IsMoveInputIgnored() || PC->IsLookInputIgnored());
            if(++Choice<8){Press(EKeys::B);Test->TestTrue(TEXT("B returns directly to picker while placing"),PC->IsBuildingMenuOpen());Wait(Now,0.2,2);return false;}
            Test->TestEqual(TEXT("Selection alone consumes no wood"),I->Count(TEXT("Item_Wood")),14);
            PC->SelectBuildingPiece(TEXT("Build_Foundation"));Aim(FVector(-800,-1200,0));Press(EKeys::LeftMouseButton);Wait(Now,0.4,3);return false;
        }
        if(Stage==3)
        {
            for(TActorIterator<APFBuildPiece> It(PC->GetWorld());It;++It){if(It->Kind==EPFBuildKind::Foundation && It->IsOwnedBy(PC->PlayerState)){Base=*It;break;}}
            if(!Base.IsValid()){Test->AddError(TEXT("Bound placement did not create foundation: ")+B->Feedback);return true;}
            PC->GetPawn()->SetActorLocation(Base->GetActorLocation()+FVector(0,-350,90));
            PC->SelectBuildingPiece(TEXT("Build_Wall"));B->Rotation=0;Aim(Base->GetActorLocation()+FVector(0,50,10));
            Press(EKeys::R);Test->TestEqual(TEXT("Bound R rotates preview once"),B->Rotation,1);
            for(int32 N=0;N<3;++N){Press(EKeys::R);}Test->TestEqual(TEXT("Four R presses return to forward edge"),B->Rotation,0);
            Test->TestEqual(TEXT("Rotation alone spends no wood"),I->Count(TEXT("Item_Wood")),12);
            Wait(Now,0.3,300);return false;
        }
        if(Stage==300){Capture(TEXT("wall_ahead_preview"));Wait(Now,0.5,301);return false;}
        if(Stage==301){if(!ShotExists()){return false;}Press(EKeys::LeftMouseButton);Wait(Now,0.4,302);return false;}
        if(Stage==302)
        {
            APFBuildPiece* Ahead=nullptr;
            for(TActorIterator<APFBuildPiece> It(PC->GetWorld());It;++It){if(It->Kind==EPFBuildKind::Wall && It->IsOwnedBy(PC->PlayerState)){Ahead=*It;break;}}
            if(!Ahead){Test->AddError(TEXT("Actual forward wall request failed: ")+B->Feedback);return true;}
            Test->TestTrue(TEXT("Wall is ahead on +Y, not at sideways world +X"),Ahead->GetActorLocation().Equals(Base->GetActorLocation()+FVector(0,200,160),0.01));
            Test->TestEqual(TEXT("Real forward wall charges two wood"),I->Count(TEXT("Item_Wood")),10);
            PC->GetPawn()->SetActorLocation(Base->GetActorLocation()+FVector(0,-450,90));
            Aim(Ahead->GetActorLocation());Wait(Now,0.3,3025);return false;
        }
        if(Stage==3025){Capture(TEXT("wall_ahead_placed"));Wait(Now,0.5,303);return false;}
        if(Stage==303)
        {
            if(!ShotExists()){return false;}
            for(TActorIterator<APFBuildPiece> It(PC->GetWorld());It;++It){if(It->Kind==EPFBuildKind::Wall && It->IsOwnedBy(PC->PlayerState)){It->Destroy();break;}}
            PC->GetPawn()->SetActorLocation(FVector(-1200,-1100,100));
            Press(EKeys::B);Activate(Picker.Get(),TEXT("PF_BuildChoice_Build_Storage"));Aim(Base->GetActorLocation()+FVector(0,0,10));Press(EKeys::LeftMouseButton);Wait(Now,0.4,4);return false;
        }
        if(Stage==4)
        {
            for(TActorIterator<APFBuildPiece> It(PC->GetWorld());It;++It){if(It->Kind==EPFBuildKind::Storage && It->IsOwnedBy(PC->PlayerState)){Chest=*It;break;}}
            if(!Chest.IsValid()){Test->AddError(TEXT("Selected chest placement failed: ")+B->Feedback);return true;}
            Aim(Chest->GetActorLocation());Press(EKeys::E);WorldBefore=PC->GetWorld()->GetTimeSeconds();Wait(Now,0.5,5);return false;
        }
        if(Stage==5)
        {
            Test->TestTrue(TEXT("E opens actual chest window without build overlay"),PC->IsStorageMenuOpen() && ChestUI->IsVisible() && !B->bBuildMode);
            Test->TestTrue(TEXT("Chest holds cursor without pausing simulation"),PC->bShowMouseCursor && PC->IsMoveInputIgnored() && !PC->GetWorld()->IsPaused() && PC->GetWorld()->GetTimeSeconds()>WorldBefore);
            CheckBounds(ChestUI.Get(),TEXT("PF_StoragePanel"));Capture(TEXT("chest_open"));Wait(Now,0.5,6);return false;
        }
        if(Stage==6){if(!ShotExists()){return false;}Activate(ChestUI.Get(),TEXT("PF_Deposit_0_One"));Wait(Now,0.4,7);return false;}
        if(Stage==7)
        {
            Test->TestEqual(TEXT("Store one through actual UI"),Chest->Storage->Count(TEXT("Item_Wood")),1);
            Test->TestEqual(TEXT("Bag plus chest wood conserved"),I->Count(TEXT("Item_Wood"))+Chest->Storage->Count(TEXT("Item_Wood")),8);
            Activate(ChestUI.Get(),TEXT("PF_Withdraw_0_Stack"));Wait(Now,0.4,8);return false;
        }
        if(Stage==8){Test->TestEqual(TEXT("Take stack through UI"),I->Count(TEXT("Item_Wood")),8);Activate(ChestUI.Get(),TEXT("PF_ChestSettings"));Wait(Now,0.3,9);return false;}
        if(Stage==9)
        {
            Activate(ChestUI.Get(),TEXT("PF_Filter_Item_Wood"));
            Test->TestFalse(TEXT("Draft does not alter server filter"),Chest->bRestrictStorageItems);
            Capture(TEXT("chest_filter_draft"));Wait(Now,0.5,10);return false;
        }
        if(Stage==10){if(!ShotExists()){return false;}Activate(ChestUI.Get(),TEXT("PF_ApplyChestSettings"));Wait(Now,0.4,11);return false;}
        if(Stage==11)
        {
            Test->TestTrue(TEXT("Applied filter reaches server"),Chest->bRestrictStorageItems && Chest->AllowedStorageItems==TArray<FName>{FName(TEXT("Item_Wood"))});
            Activate(ChestUI.Get(),TEXT("PF_QuickStore"));Wait(Now,0.4,12);return false;
        }
        if(Stage==12)
        {
            Test->TestEqual(TEXT("Quick Store moves all allowed wood"),Chest->Storage->Count(TEXT("Item_Wood")),8);
            Test->TestEqual(TEXT("Quick Store leaves disallowed stone"),I->Count(TEXT("Item_Stone")),3);
            Test->TestEqual(TEXT("Quick Store leaves disallowed food"),I->Count(TEXT("Item_Food")),2);
            Activate(ChestUI.Get(),TEXT("PF_Withdraw_0_Stack"));Wait(Now,0.4,13);return false;
        }
        if(Stage==13){Activate(ChestUI.Get(),TEXT("PF_ChestSettings"));Activate(ChestUI.Get(),TEXT("PF_Filter_Item_Food"));Activate(ChestUI.Get(),TEXT("PF_CancelChestSettings"));Test->TestFalse(TEXT("Cancelled food draft stays refused"),Chest->AcceptsStoredItem(TEXT("Item_Food")));Activate(ChestUI.Get(),TEXT("PF_ChestSettings"));Activate(ChestUI.Get(),TEXT("PF_FilterMode"));Activate(ChestUI.Get(),TEXT("PF_ApplyChestSettings"));Wait(Now,0.4,14);return false;}
        if(Stage==14){Test->TestFalse(TEXT("Unrestricted mode applied"),Chest->bRestrictStorageItems);const int32 FoodIndex=I->GetStacks().IndexOfByPredicate([](const auto& S){return S.ItemId==TEXT("Item_Food");});Activate(ChestUI.Get(),FName(*FString::Printf(TEXT("PF_Deposit_%d_Stack"),FoodIndex)));Wait(Now,0.4,15);return false;}
        if(Stage==15)
        {
            Test->TestEqual(TEXT("Whole food batch stored"),Chest->Storage->Count(TEXT("Item_Food")),2);
            Test->TestEqual(TEXT("Freshness not reset by UI transfer"),Chest->Storage->GetStacks()[0].ExpiresAt,Deadline);
            Activate(ChestUI.Get(),TEXT("PF_SortChest"));Wait(Now,0.4,16);return false;
        }
        if(Stage==16)
        {
            Test->TestEqual(TEXT("Sort conserves stored food"),Chest->Storage->Count(TEXT("Item_Food")),2);
            Test->TestEqual(TEXT("Sort conserves freshness"),Chest->Storage->GetStacks()[0].ExpiresAt,Deadline);
            I->bEnforceWeightLimit=true;I->WeightLimit=0.1f;Activate(ChestUI.Get(),TEXT("PF_Withdraw_0_Stack"));Wait(Now,0.4,17);return false;
        }
        if(Stage==17)
        {
            Test->TestEqual(TEXT("Full bag refusal preserves chest"),Chest->Storage->Count(TEXT("Item_Food")),2);Test->TestEqual(TEXT("Refused withdrawal creates no bag food"),I->Count(TEXT("Item_Food")),0);
            Test->TestTrue(TEXT("Refusal visible"),B->Feedback.Contains(TEXT("refused")));I->WeightLimit=30;Capture(TEXT("chest_capacity_refusal"));Wait(Now,0.5,18);return false;
        }
        if(Stage==18){if(!ShotExists()){return false;}Activate(ChestUI.Get(),TEXT("PF_Withdraw_0_Stack"));Wait(Now,0.4,19);return false;}
        if(Stage==19)
        {
            Test->TestEqual(TEXT("Final food conservation"),I->Count(TEXT("Item_Food")),2);Test->TestTrue(TEXT("Chest empty after recovery"),Chest->Storage->GetStacks().IsEmpty());
            Activate(ChestUI.Get(),TEXT("PF_CloseStorage"));Test->TestFalse(TEXT("Close releases movement/look/cursor"),PC->IsStorageMenuOpen() || PC->IsMoveInputIgnored() || PC->IsLookInputIgnored() || PC->bShowMouseCursor);
            Wait(Now,0.3,190);return false;
        }
        if(Stage==190){Aim(Chest->GetActorLocation());Press(EKeys::E);Wait(Now,0.4,191);return false;}
        if(Stage==191)
        {
            Test->TestTrue(TEXT("Chest reopens for Escape Back"),PC->IsStorageMenuOpen());
            Activate(ChestUI.Get(),TEXT("PF_ChestSettings"));Activate(ChestUI.Get(),TEXT("PF_Filter_Item_Wood"));
            SlateBack(true);Test->TestEqual(TEXT("Held Escape retains chest settings page"),StoragePage(),1);
            SlateBack(false);Wait(Now,0.3,192);return false;
        }
        if(Stage==192)
        {
            Test->TestTrue(TEXT("Escape cancels filter draft and keeps chest open without Pause"),StoragePage()==0 && PC->IsStorageMenuOpen() && !PC->IsPauseMenuOpen() && !Chest->bRestrictStorageItems);
            SlateBack(false);Wait(Now,0.3,193);return false;
        }
        if(Stage==193)
        {
            Test->TestFalse(TEXT("Second Escape closes chest and restores gameplay without Pause"),PC->IsStorageMenuOpen() || PC->IsPauseMenuOpen() || PC->IsMoveInputIgnored() || PC->IsLookInputIgnored() || PC->bShowMouseCursor);
            Test->TestNull(TEXT("Esc close clears server open-storage pointer"),B->OpenStorage.Get());Wait(Now,0.3,20);return false;
        }
        if(Stage==20)
        {
            const auto OriginalId=Chest->PersistentOwnerId;auto* Other=PC->GetWorld()->SpawnActor<APFInventoryPlayerState>();Chest->PersistentOwnerId=Other->PersistentPlayerId;Aim(Chest->GetActorLocation());Press(EKeys::E);
            Test->TestFalse(TEXT("Another owner's chest cannot open"),PC->IsStorageMenuOpen());Chest->PersistentOwnerId=OriginalId;Other->Destroy();
            Aim(Base->GetActorLocation()+FVector(-100,-100,10));Window=B->Place(TEXT("Build_Window"),1);
            if(!Window.IsValid()){Test->AddError(TEXT("Actual window placement failed: ")+B->Feedback);return true;}
            Aim(Window->GetActorLocation()+FVector(140,0,0));Roof=B->Place(TEXT("Build_Ceiling"),0);
            if(!Roof.IsValid()){Test->AddError(TEXT("Actual window-supported roof failed: ")+B->Feedback);return true;}
            PC->GetPawn()->SetActorLocation(Base->GetActorLocation()+FVector(0,-400,90));
            Aim(Base->GetActorLocation()+FVector(-100,-100,10));Stairs=B->Place(TEXT("Build_Stairs"),0);
            if(!Stairs.IsValid()){Test->AddError(TEXT("Actual exterior stair placement failed: ")+B->Feedback);return true;}
            Capture(TEXT("window_stairs_scene"));Wait(Now,0.5,21);return false;
        }
        if(Stage==21)
        {
            if(!ShotExists()){return false;}
            auto* Character=CastChecked<ACharacter>(PC->GetPawn());Character->SetActorLocation(Base->GetActorLocation()+FVector(650,0,90));Character->GetCharacterMovement()->SetMovementMode(MOVE_Walking);PC->SetControlRotation(FRotator(0,180,0));
            WalkStarted=Now;Stage=22;return false;
        }
        Test->TestTrue(TEXT("Physical stair traversal reaches supported floor"),PC->GetPawn()->GetActorLocation().Z>390);
        Test->AddInfo(TEXT("[PrimalUI] Eight-piece picker, actual chest opening, transfers, Quick Store/filter Apply/Cancel, Sort/freshness/capacity, ownership and physical stair traversal verified"));return true;
    }
private:
    void SlateBack(bool Repeat){auto& App=FSlateApplication::Get();App.ProcessKeyDownEvent(FKeyEvent(EKeys::Escape,FModifierKeysState(),0,Repeat,0,0));App.ProcessKeyUpEvent(FKeyEvent(EKeys::Escape,FModifierKeysState(),0,false,0,0));}
    int32 StoragePage(){TArray<UWidget*> Widgets;ChestUI->WidgetTree->GetAllWidgets(Widgets);for(auto* W:Widgets){if(auto* Pages=Cast<UWidgetSwitcher>(W)){return Pages->GetActiveWidgetIndex();}}Test->AddError(TEXT("Missing chest page switcher"));return INDEX_NONE;}
    UUserWidget* Find(UClass* Type){TArray<UUserWidget*> Widgets;UWidgetBlueprintLibrary::GetAllWidgetsOfClass(PC.Get(),Widgets,Type,false);return Widgets.Num()==1?Widgets[0]:nullptr;}
    void Press(FKey K){for(const auto& Binding:PC->InputComponent->KeyBindings){if(Binding.Chord.Key==K && Binding.KeyEvent==IE_Pressed){Binding.KeyDelegate.Execute(K);return;}}Test->AddError(TEXT("Missing bound input"));}
    void Activate(UUserWidget* Menu,FName Name)
    {
        auto* Button=Cast<UButton>(Menu->WidgetTree->FindWidget(Name));if(!Button){Test->AddError(TEXT("Missing button ")+Name.ToString());return;}
        Button->SetKeyboardFocus();Test->TestTrue(TEXT("Real button gets Slate focus"),Button->HasKeyboardFocus());
        auto& App=FSlateApplication::Get();App.ProcessKeyDownEvent(FKeyEvent(EKeys::SpaceBar,FModifierKeysState(),0,false,0,0));App.ProcessKeyUpEvent(FKeyEvent(EKeys::SpaceBar,FModifierKeysState(),0,false,0,0));
    }
    void Aim(FVector At){FVector Eye;FRotator Look;PC->GetPawn()->GetActorEyesViewPoint(Eye,Look);PC->SetControlRotation((At-Eye).Rotation());}
    void Wait(double Now,double Seconds,int32 Next){Until=Now+Seconds;Stage=Next;}
    void Capture(const TCHAR* Name){Shot=Directory/(FString(Name)+TEXT(".png"));FScreenshotRequest::RequestScreenshot(Shot,true,false);}
    bool ShotExists(){return IFileManager::Get().FileSize(*Shot)>0;}
    void CheckBounds(UUserWidget* Widget,FName Name)
    {auto* P=Widget->WidgetTree->FindWidget(Name);const auto R=Widget->GetCachedGeometry(),G=P->GetCachedGeometry();const auto TL=R.AbsoluteToLocal(G.LocalToAbsolute(FVector2D::ZeroVector)),BR=R.AbsoluteToLocal(G.LocalToAbsolute(G.GetLocalSize()));Test->TestTrue(TEXT("Modal panel fits viewport"),TL.X>=0 && TL.Y>=0 && BR.X<=R.GetLocalSize().X && BR.Y<=R.GetLocalSize().Y);}
    FAutomationTestBase* Test;FString Directory,Shot;TWeakObjectPtr<APFSurvivalPlayerController> PC;
    TWeakObjectPtr<UUserWidget> Picker,ChestUI;TWeakObjectPtr<APFBuildPiece> Base,Chest,Window,Roof,Stairs;
    double Started=FPlatformTime::Seconds(),Until=0,WorldBefore=0,Deadline=0,WalkStarted=0;int32 Stage=0,Choice=0;
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFBuildingMenusLiveTest,"PF.UI.BuildingMenusLive",EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)
bool FPFBuildingMenusLiveTest::RunTest(const FString&)
{
    if(!FParse::Param(FCommandLine::Get(),TEXT("PFRunBuildingMenusTest")) || FParse::Param(FCommandLine::Get(),TEXT("nullrhi"))){AddError(TEXT("Requires isolated rendered BuildingMenus runner"));return false;}
    FString Label;FParse::Value(FCommandLine::Get(),TEXT("PFControlsEvidence="),Label);
    if(Label.IsEmpty() || Label.Len()>48){AddError(TEXT("Invalid evidence label"));return false;}for(TCHAR C:Label){if(!FChar::IsAlnum(C) && C!=TEXT('_')){AddError(TEXT("Invalid evidence label"));return false;}}
    const FString Directory=FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()/TEXT("AutomationReports/ControlsUI")/Label);
    if(IFileManager::Get().DirectoryExists(*Directory)){AddError(TEXT("Refusing reused screenshot directory"));return false;}IFileManager::Get().MakeDirectory(*Directory,true);
    ADD_LATENT_AUTOMATION_COMMAND(FBuildingMenusExercise(this,Directory));return true;
}
#endif
