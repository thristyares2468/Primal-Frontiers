#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Inventory/PFHotbarHUD.h"
#include "Inventory/PFInventoryComponent.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "UI/PFItemPicture.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Components/InputComponent.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "HAL/PlatformTime.h"
#include "HAL/FileManager.h"
#include "UnrealClient.h"
#include "Settings/PFGameUserSettings.h"

namespace
{
class FHotbarExercise final:public IAutomationLatentCommand
{
public:
    FHotbarExercise(FAutomationTestBase* T,FString D):Test(T),Directory(MoveTemp(D)){}
    ~FHotbarExercise(){if(bChangedScale && UPFGameUserSettings::Get()){UPFGameUserSettings::Get()->Preferences.HUDScale=OriginalScale;}}
    bool Update() override
    {
        const double Now=FPlatformTime::Seconds();if(Now-Started>45){Test->AddError(TEXT("Hotbar live timeout"));return true;}if(Now<Until){return false;}
        if(Phase==0)
        {
            for(const auto& C:GEngine->GetWorldContexts()){if(C.World() && C.World()->IsGameWorld()){auto* Candidate=Cast<APFSurvivalPlayerController>(C.World()->GetFirstPlayerController());if(Candidate && Candidate->GetLocalPlayer() && Candidate->GetPawn() && Candidate->GetInventory()){PC=Candidate;break;}}}
            if(!PC.IsValid()){return false;}auto* I=PC->GetInventory();if(!PC->HasAuthority() || !I->GetStacks().IsEmpty()){Test->AddError(TEXT("Fresh isolated standalone required; refusing existing bag"));return true;}
            if(auto* Settings=UPFGameUserSettings::Get()){OriginalScale=Settings->Preferences.HUDScale;float Scale=1;FParse::Value(FCommandLine::Get(),TEXT("PFHUDScale="),Scale);Settings->Preferences.HUDScale=FMath::Clamp(Scale,.75f,1.5f);bChangedScale=true;}
            TArray<UUserWidget*> Widgets;UWidgetBlueprintLibrary::GetAllWidgetsOfClass(PC.Get(),Widgets,UPFHotbarHUD::StaticClass(),false);if(Widgets.Num()!=1){Test->AddError(TEXT("Actual hotbar widget missing"));return true;}HUD=Widgets[0];
            const FName Items[]={TEXT("Item_Wood"),TEXT("Item_Stone"),TEXT("Item_Food"),TEXT("Item_Water"),TEXT("Item_Fibre"),TEXT("Item_Cord"),TEXT("Item_CopperOre"),TEXT("Item_CookedFood")};
            Press(EKeys::Tab);for(int32 N=0;N<8;++N){Test->TestTrue(TEXT("Fixture actual batch"),I->Grant(Items[N],N+2));Ids.Add(I->GetStacks().Last().StackId);Test->TestTrue(TEXT("Select actual inventory stack"),PC->SelectInventoryStack(Ids.Last()));Test->TestTrue(TEXT("Actual modal key assigns"),PC->HandleInventoryKey(Keys[N]));}
            Press(EKeys::Tab);Press(EKeys::Three);PC->GetPawn()->FindComponentByClass<UPFPlayerSurvivalComponent>()->SetHunger(40);Until=Now+.5;Phase=1;return false;
        }
        if(!PC.IsValid() || !HUD.IsValid()){Test->AddError(TEXT("Lost hotbar fixture"));return true;}
        if(Phase==1)
        {
            Test->TestEqual(TEXT("All eight assigned"),PC->GetHotbar().Selected(),2);CheckLayout();Shot=Directory/TEXT("hotbar_assigned.png");FScreenshotRequest::RequestScreenshot(Shot,true,false);Until=Now+1;Phase=2;return false;
        }
        if(Phase==2)
        {
            if(IFileManager::Get().FileSize(*Shot)<=0){return false;}auto* I=PC->GetInventory();Press(EKeys::Q);Test->TestEqual(TEXT("Q consumes exactly one actual food"),I->Count(TEXT("Item_Food")),3);Until=Now+.3;Phase=3;return false;
        }
        if(Phase==3)
        {
            auto* I=PC->GetInventory();Test->TestTrue(TEXT("Sort real bag"),I->SortStacks());Test->TestEqual(TEXT("Sort preserves selected GUID"),PC->GetHotbar().Resolve(2,I,UPFInventoryComponent::ServerTime(PC->GetWorld()))->StackId,Ids[2]);
            Test->TestTrue(TEXT("Isolated max-stack readability fixture"),I->Grant(TEXT("Item_Wood"),999000-2));
            Press(EKeys::Gamepad_DPad_Right);Test->TestEqual(TEXT("Controller cycle slot4"),PC->GetHotbar().Selected(),3);Press(EKeys::Gamepad_DPad_Down);Test->TestEqual(TEXT("Controller same validated consume"),I->Count(TEXT("Item_Water")),4);
            Test->TestTrue(TEXT("Remove selected food"),I->Remove(Ids[2],3));Test->TestTrue(TEXT("Replacement same-type batch"),I->Grant(TEXT("Item_Food"),2));Press(EKeys::Three);Press(EKeys::Q);Test->TestEqual(TEXT("Missing bound batch refuses replacement"),I->Count(TEXT("Item_Food")),2);
            Until=Now+.5;Phase=4;return false;
        }
        if(Phase==4)
        {
            auto* Max=Cast<UTextBlock>(HUD->WidgetTree->FindWidget(TEXT("PF_HotbarCount1")));if(Max){Test->AddInfo(FString::Printf(TEXT("[PrimalUI] Max stack text=%s desired=%.2f allocated=%.2f"),*Max->GetText().ToString(),Max->GetDesiredSize().X,Max->GetCachedGeometry().GetLocalSize().X));}Test->TestTrue(TEXT("Maximum999000 quantity is actual and fits"),Max && Max->GetText().ToString()==TEXT("999000") && Max->GetDesiredSize().X<=Max->GetCachedGeometry().GetLocalSize().X+1);
            auto* Label=Cast<UTextBlock>(HUD->WidgetTree->FindWidget(TEXT("PF_HotbarCount3")));Test->TestTrue(TEXT("Removed slot explicitly lost"),Label && Label->GetText().ToString()==TEXT("lost"));Shot=Directory/TEXT("hotbar_missing.png");FScreenshotRequest::RequestScreenshot(Shot,true,false);Until=Now+.5;Phase=5;return false;
        }
        if(Phase==5)
        {if(IFileManager::Get().FileSize(*Shot)<=0){return false;}Press(EKeys::P);Until=Now+.5;Phase=6;return false;}
        if(Phase==6)
        {
            Test->TestEqual(TEXT("Pause suppresses shortcut panel"),HUD->WidgetTree->RootWidget->GetVisibility(),ESlateVisibility::Hidden);const int32 Before=PC->GetHotbar().Selected();Press(EKeys::Eight);Test->TestEqual(TEXT("Pause numeric shortcut cannot change selection"),PC->GetHotbar().Selected(),Before);Press(EKeys::P);Until=Now+.5;Phase=7;return false;
        }
        Test->TestEqual(TEXT("Panel restores after Pause"),HUD->WidgetTree->RootWidget->GetVisibility(),ESlateVisibility::HitTestInvisible);Test->TestTrue(TEXT("Actual screenshot saved"),IFileManager::Get().FileSize(*(Directory/TEXT("hotbar_assigned.png")))>0);Test->AddInfo(TEXT("[PrimalUI] Hotbar live bound-input/real sprite/conservation/reorder/controller route/missing/modal/layout proof: ")+Directory);return true;
    }
private:
    void Press(FKey K){for(const auto& B:PC->InputComponent->KeyBindings){if(B.Chord.Key==K && B.KeyEvent==IE_Pressed){B.KeyDelegate.Execute(K);return;}}Test->AddError(TEXT("Missing hotbar binding ")+K.ToString());}
    void CheckLayout()
    {
        const FGeometry Root=HUD->GetCachedGeometry();Test->TestTrue(TEXT("Read-only HUD never steals gameplay clicks"),HUD->GetVisibility()==ESlateVisibility::HitTestInvisible && !PC->bShowMouseCursor && !PC->IsMoveInputIgnored());
        for(int32 N=0;N<8;++N)
        {
            auto* Frame=HUD->WidgetTree->FindWidget(FName(*FString::Printf(TEXT("PF_HotbarSlot%d"),N+1)));const auto G=Frame->GetCachedGeometry();Test->TestTrue(TEXT("Every slot laid out inside hotbar"),G.GetLocalSize().X>40 && G.GetLocalSize().Y>70 && Root.AbsoluteToLocal(G.LocalToAbsolute(G.GetLocalSize())).X<=Root.GetLocalSize().X+1);
            auto* Label=Cast<UTextBlock>(HUD->WidgetTree->FindWidget(FName(*FString::Printf(TEXT("PF_HotbarCount%d"),N+1))));Test->TestEqual(TEXT("Actual batch quantity shown"),Label->GetText().ToString(),FString::FromInt(N+2));
        }
        TArray<UUserWidget*> Art;UWidgetBlueprintLibrary::GetAllWidgetsOfClass(PC.Get(),Art,UPFItemPicture::StaticClass(),false);int32 Loaded=0;for(auto* W:Art){auto* Pic=Cast<UPFItemPicture>(W);bool InHotbar=false;for(auto* Parent=Pic?Pic->GetParent():nullptr;Parent;Parent=Parent->GetParent()){if(Parent==HUD->WidgetTree->RootWidget){InHotbar=true;break;}}if(Pic && Pic->IsVisible() && InHotbar){auto* Image=Cast<UImage>(Pic->WidgetTree->RootWidget);if(Image && Image->GetBrush().GetResourceObject()){++Loaded;}}}Test->TestEqual(TEXT("Eight original loaded hotbar sprites"),Loaded,8);
    }
    FAutomationTestBase* Test;FString Directory,Shot;TWeakObjectPtr<APFSurvivalPlayerController> PC;TWeakObjectPtr<UUserWidget> HUD;TArray<FGuid> Ids;
    FKey Keys[8]={EKeys::One,EKeys::Two,EKeys::Three,EKeys::Four,EKeys::Five,EKeys::Six,EKeys::Seven,EKeys::Eight};int32 Phase=0;double Started=FPlatformTime::Seconds(),Until=0;float OriginalScale=1;bool bChangedScale=false;
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFHotbarLiveTest,"PF.UI.HotbarLive",EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)
bool FPFHotbarLiveTest::RunTest(const FString&)
{
    if(!FParse::Param(FCommandLine::Get(),TEXT("PFRunControlsUITest")) || FParse::Param(FCommandLine::Get(),TEXT("nullrhi"))){AddError(TEXT("Requires isolated rendered UI runner"));return false;}
    FString Label;FParse::Value(FCommandLine::Get(),TEXT("PFControlsEvidence="),Label);if(Label.IsEmpty() || Label.Len()>48){return false;}for(TCHAR C:Label){if(!FChar::IsAlnum(C) && C!=TEXT('_')){return false;}}
    const FString Directory=FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()/TEXT("AutomationReports/ControlsUI")/Label);if(IFileManager::Get().DirectoryExists(*Directory)){AddError(TEXT("Refusing reused hotbar evidence"));return false;}IFileManager::Get().MakeDirectory(*Directory,true);ADD_LATENT_AUTOMATION_COMMAND(FHotbarExercise(this,Directory));return true;
}
#endif
