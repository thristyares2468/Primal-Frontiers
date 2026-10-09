// Opt-in rendered action overlays, real authoritative requests and actual UMG bounds.
#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Crafting/PFCraftingHUD.h"
#include "Crafting/PFCraftingComponent.h"
#include "Building/PFBuildingHUD.h"
#include "Building/PFBuildingComponent.h"
#include "Building/PFBuildPiece.h"
#include "Inventory/PFInventoryComponent.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFSurvivalHUD.h"
#include "Settings/PFGameUserSettings.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/InputComponent.h"
#include "UI/PFItemPicture.h"
#include "Framework/Application/SlateApplication.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "UnrealClient.h"
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformTime.h"

namespace
{
class FActionOverlayExercise final : public IAutomationLatentCommand
{
public:
    FActionOverlayExercise(FAutomationTestBase* T,FString D):Test(T),Directory(MoveTemp(D)){}
    ~FActionOverlayExercise(){if(bChanged && UPFGameUserSettings::Get()){UPFGameUserSettings::Get()->Preferences.HUDScale=OriginalScale;}if(Storage.IsValid()){Storage->Destroy();}}
    bool Update() override
    {
        const double Now=FPlatformTime::Seconds();
        if(Now-Started>70){Test->AddError(TEXT("Action overlay UI timeout"));return true;}
        if(Now<Until){return false;}
        if(Phase==0)
        {
            for(const auto& Context:GEngine->GetWorldContexts())
            {if(Context.World() && Context.World()->IsGameWorld()){auto* Candidate=Cast<APFSurvivalPlayerController>(Context.World()->GetFirstPlayerController());if(Candidate && Candidate->GetLocalPlayer() && Candidate->GetPawn() && Candidate->GetInventory()){PC=Candidate;break;}}}
            if(!PC.IsValid()){return false;}
            if(!PC->HasAuthority() || !PC->GetInventory()->GetStacks().IsEmpty() || !PC->GetCrafting() || !PC->Building)
            {Test->AddError(TEXT("Refusing non-fresh standalone action overlay fixture"));return true;}
            if(auto* S=UPFGameUserSettings::Get()){OriginalScale=S->Preferences.HUDScale;float Scale=1;FParse::Value(FCommandLine::Get(),TEXT("PFHUDScale="),Scale);S->Preferences.HUDScale=FMath::Clamp(Scale,0.75f,1.5f);bChanged=true;}
            Craft=Find(UPFCraftingHUD::StaticClass());Build=Find(UPFBuildingHUD::StaticClass());
            if(!Craft.IsValid() || !Build.IsValid()){Test->AddError(TEXT("Missing native action widgets"));return true;}
            Press(EKeys::C);Press(EKeys::One);Wait(Now,0.4,1);return false;
        }
        if(!PC.IsValid() || !Craft.IsValid() || !Build.IsValid()){Test->AddError(TEXT("Lost action overlay fixture"));return true;}
        if(Phase==1)
        {
            Test->TestTrue(TEXT("Craft menu owns cursor/focus and movement/look but does not pause world"),PC->bShowMouseCursor && Craft->HasKeyboardFocus() && PC->IsMoveInputIgnored() && PC->IsLookInputIgnored() && !PC->IsPaused());
            Test->TestTrue(TEXT("Real missing ingredients refusal"),Label(Craft.Get(),TEXT("PF_CraftingPanel_Result")).Contains(TEXT("Refused: insufficient")));
            Test->TestEqual(TEXT("Rejected job stays idle"),PC->GetCrafting()->ActiveRecipe,NAME_None);
            Test->TestTrue(TEXT("Tool card has catalog output and missing fresh ingredients"),Label(Craft.Get(),TEXT("PF_CraftingPanel_Body")).Contains(TEXT("Makes 1 Stone gathering tool")) && Label(Craft.Get(),TEXT("PF_CraftingPanel_Body")).Contains(TEXT("Missing fresh ingredients")));
            Test->TestTrue(TEXT("Cook and dry have actual distinct output cards"),Label(Craft.Get(),TEXT("PF_Recipe1_Body")).Contains(TEXT("Makes 1 Cooked food")) && Label(Craft.Get(),TEXT("PF_Recipe2_Body")).Contains(TEXT("Makes 1 Dried food")));
            CheckLayout(Craft.Get(),TEXT("PF_CraftingPanel"));Capture(TEXT("craft_refused"));Wait(Now,0.5,2);return false;
        }
        if(Phase==2)
        {
            if(!CheckShot()){return false;}
            Test->TestTrue(TEXT("Authority fixture wood"),PC->GetInventory()->Grant(TEXT("Item_Wood"),3));
            Test->TestTrue(TEXT("Authority fixture stone"),PC->GetInventory()->Grant(TEXT("Item_Stone"),2));
            Press(EKeys::One);Wait(Now,0.4,3);return false;
        }
        if(Phase==3)
        {
            Test->TestEqual(TEXT("Bound input starts server-owned job"),PC->GetCrafting()->ActiveRecipe,FName(TEXT("Recipe_Tool")));
            Test->TestTrue(TEXT("Actual ingredient count rendered"),Label(Craft.Get(),TEXT("PF_CraftingPanel_Body")).Contains(TEXT("3/3")));
            Press(EKeys::One);Wait(Now,0.4,4);return false;
        }
        if(Phase==4)
        {
            Test->TestTrue(TEXT("Busy request refusal remains visible alongside running job"),Label(Craft.Get(),TEXT("PF_CraftingPanel_Result")).Contains(TEXT("Craft refused")));
            Test->TestEqual(TEXT("Busy request cannot grant output"),PC->GetInventory()->Count(TEXT("Item_Tool")),0);
            Test->TestTrue(TEXT("Running card never advertises immediate availability"),Label(Craft.Get(),TEXT("PF_CraftingPanel_Body")).Contains(TEXT("Job running - wait or cancel")));
            CheckLayout(Craft.Get(),TEXT("PF_CraftingPanel"));Capture(TEXT("craft_busy"));Wait(Now,0.5,5);return false;
        }
        if(Phase==5)
        {if(!CheckShot()){return false;}Press(EKeys::R);Wait(Now,0.4,6);return false;}
        if(Phase==6)
        {
            Test->TestTrue(TEXT("Server cancellation visible"),Label(Craft.Get(),TEXT("PF_CraftingPanel_Result")).Contains(TEXT("Cancelled")));
            Test->TestEqual(TEXT("Cancellation retains wood"),PC->GetInventory()->Count(TEXT("Item_Wood")),3);
            Test->TestTrue(TEXT("Cancellation returns ingredients-present advisory, not success"),Label(Craft.Get(),TEXT("PF_CraftingPanel_Body")).Contains(TEXT("Ingredients present; server validates")));
            Capture(TEXT("craft_cancelled"));Wait(Now,0.5,7);return false;
        }
        if(Phase==7)
        {
            if(!CheckShot()){return false;}
            SlatePress(EKeys::Down);Wait(Now,0.3,12);return false;
        }
        if(Phase==12)
        {
            auto* Menu=CastChecked<UPFCraftingHUD>(Craft.Get());
            Test->TestEqual(TEXT("Slate browse selects cook without crafting"),Menu->GetSelectedRecipe(),FName(TEXT("Recipe_Cook")));
            Test->TestEqual(TEXT("Browse cannot start a job"),PC->GetCrafting()->ActiveRecipe,NAME_None);
            auto* Icon=Cast<UPFItemPicture>(Craft->WidgetTree->FindWidget(TEXT("PF_SelectedItemPicture")));
            Test->TestTrue(TEXT("Selected picture matches cooked output"),Icon && Icon->GetItemId()==TEXT("Item_CookedFood") && Icon->GetCachedGeometry().GetLocalSize().X>0);
            Test->TestTrue(TEXT("Selected detail has real food benefit/shelf life"),Label(Craft.Get(),TEXT("PF_CraftingDetail_Stats")).Contains(TEXT("+55 food")) && Label(Craft.Get(),TEXT("PF_CraftingDetail_Stats")).Contains(TEXT("15 min")));
            CheckLayout(Craft.Get(),TEXT("PF_CraftingPanel"));Capture(TEXT("craft_food_selected"));Wait(Now,0.5,13);return false;
        }
        if(Phase==13)
        {
            if(!CheckShot()){return false;}
            SlatePress(EKeys::P);Wait(Now,0.3,14);return false;
        }
        if(Phase==14)
        {
            Test->TestTrue(TEXT("Craft to Pause transfers focus/input without stale menu"),PC->IsPauseMenuOpen() && !PC->IsCraftingOpen() && PC->IsPaused() && PC->IsMoveInputIgnored());
            SlatePress(EKeys::P);Wait(Now,0.3,15);return false;
        }
        if(Phase==15)
        {
            Test->TestFalse(TEXT("Resume balances movement/look ignore counters and cursor"),PC->IsPauseMenuOpen() || PC->IsMoveInputIgnored() || PC->IsLookInputIgnored() || PC->bShowMouseCursor);
            Press(EKeys::C);Wait(Now,0.3,16);return false;
        }
        if(Phase==16)
        {
            // Background-window synthetic hover did not deliver clicks reliably.
            // Exercise SButton activation with focus+Space, never broadcast OnClicked.
            Activate(Craft->WidgetTree->FindWidget(TEXT("PF_Recipe2_Button")));Wait(Now,0.3,17);return false;
        }
        if(Phase==17)
        {
            Test->TestEqual(TEXT("Button row selects dry without crafting"),CastChecked<UPFCraftingHUD>(Craft.Get())->GetSelectedRecipe(),FName(TEXT("Recipe_Dry")));
            Test->TestEqual(TEXT("Button selection cannot create output"),PC->GetInventory()->Count(TEXT("Item_DriedFood")),0);
            Activate(Craft->WidgetTree->FindWidget(TEXT("PF_CraftSelected")));Wait(Now,0.4,18);return false;
        }
        if(Phase==18)
        {
            Test->TestTrue(TEXT("Button craft request reaches server and refuses missing food"),Label(Craft.Get(),TEXT("PF_CraftingPanel_Result")).Contains(TEXT("Refused: insufficient")));
            Test->TestEqual(TEXT("Button refused job stays idle"),PC->GetCrafting()->ActiveRecipe,NAME_None);
            Test->TestEqual(TEXT("Button refusal preserves wood"),PC->GetInventory()->Count(TEXT("Item_Wood")),3);
            Activate(Craft->WidgetTree->FindWidget(TEXT("PF_CloseCraft")));Wait(Now,0.3,19);return false;
        }
        if(Phase==19)
        {
            Test->TestFalse(TEXT("Button Close restores gameplay input and hides cursor"),PC->IsCraftingOpen() || PC->IsMoveInputIgnored() || PC->IsLookInputIgnored() || PC->bShowMouseCursor);
            Press(EKeys::B);
            PC->Building->ServerPlace(NAME_None,0);Wait(Now,0.4,8);return false;
        }
        if(Phase==8)
        {
            auto* Panel=Craft->WidgetTree->FindWidget(TEXT("PF_CraftingPanel"));
            Test->TestTrue(TEXT("Opening build collapses crafting"),Panel && !Craft->IsVisible());
            Test->TestTrue(TEXT("Invalid placement shows actual server result"),Label(Build.Get(),TEXT("PF_BuildingPanel_Result")).Contains(TEXT("Invalid piece or survivor")));
            Test->TestTrue(TEXT("Preview explicitly advisory"),Label(Build.Get(),TEXT("PF_BuildingPanel_Body")).Contains(TEXT("Preview (advisory)")));
            Test->TestEqual(TEXT("Invalid placement retains cost"),PC->GetInventory()->Count(TEXT("Item_Wood")),3);
            CheckLayout(Build.Get(),TEXT("PF_BuildingPanel"));Capture(TEXT("build_refused"));Wait(Now,0.5,9);return false;
        }
        if(Phase==9)
        {
            if(!CheckShot()){return false;}
            // Transient authority fixture solely for a bounded, real storage-content view.
            const FTransform At(FRotator::ZeroRotator,PC->GetPawn()->GetActorLocation()+FVector(300,0,0));
            const FPFBuildingDefinition* Definition=nullptr;
            for(const auto& D:PC->Building->Catalog->Pieces){if(D.Kind==EPFBuildKind::Storage){Definition=&D;break;}}
            if(!Definition){Test->AddError(TEXT("Missing storage definition"));return true;}
            Storage=PC->GetWorld()->SpawnActorDeferred<APFBuildPiece>(APFBuildPiece::StaticClass(),At,nullptr,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
            if(!Storage.IsValid() || !Storage->Storage){Test->AddError(TEXT("Missing storage fixture"));return true;}
            Storage->Initialize(*Definition,PC->PlayerState,nullptr);Storage->FinishSpawning(At);
            for(int32 N=0;N<6;++N){Test->TestTrue(TEXT("Separate food batches"),Storage->Storage->AddExisting(TEXT("Item_Food"),1,UPFInventoryComponent::ServerTime(PC->GetWorld())+100+N));}
            PC->Building->OpenStorage=Storage.Get();Wait(Now,0.3,10);return false;
        }
        if(Phase==10)
        {
            const FString Body=Label(Build.Get(),TEXT("PF_BuildingPanel_Body"));
            Test->TestTrue(TEXT("Bounded storage view admits omitted rows"),Body.Contains(TEXT("+2 more stacks")));
            Test->TestTrue(TEXT("Storage keeps freshness visible"),Body.Contains(TEXT("[")));
            CheckLayout(Build.Get(),TEXT("PF_BuildingPanel"));Capture(TEXT("build_storage"));Wait(Now,0.5,11);return false;
        }
        if(!CheckShot()){return false;}
        PC->Building->OpenStorage=nullptr;Press(EKeys::B);
        Test->TestFalse(TEXT("Build closes via real binding"),PC->Building->bBuildMode);
        Test->AddInfo(TEXT("[PrimalUI] Actual crafting refusal/start/busy/cancel and invalid placement rendered; bounded storage UI fixture. No hardware/traversal/multiplayer acceptance claimed."));return true;
    }
private:
    UUserWidget* Find(UClass* Class){TArray<UUserWidget*> Widgets;UWidgetBlueprintLibrary::GetAllWidgetsOfClass(PC.Get(),Widgets,Class,false);return Widgets.Num()==1?Widgets[0]:nullptr;}
    FString Label(UUserWidget* Widget,FName Name){auto* T=Cast<UTextBlock>(Widget->WidgetTree->FindWidget(Name));if(!T){Test->AddError(TEXT("Missing action text ")+Name.ToString());return FString();}return T->GetText().ToString();}
    void CheckLayout(UUserWidget* Widget,const TCHAR* Name)
    {
        const FGeometry Root=Widget->GetCachedGeometry();auto* Panel=Widget->WidgetTree->FindWidget(Name);
        const FGeometry G=Panel->GetCachedGeometry();
        const FVector2D TL=Root.AbsoluteToLocal(G.LocalToAbsolute(FVector2D::ZeroVector)),BR=Root.AbsoluteToLocal(G.LocalToAbsolute(G.GetLocalSize())),Size=Root.GetLocalSize();
        const bool bRecipes=Widget->IsA<UPFCraftingHUD>();
        Test->TestTrue(TEXT("Panel fits viewport"),TL.X>=0 && TL.Y>=0 && BR.X<Size.X && BR.Y<Size.Y);
        if(bRecipes){Test->TestTrue(TEXT("Crafting is a centered modal menu"),FMath::Abs((TL.X+BR.X)*0.5-Size.X*0.5)<2 && FMath::Abs((TL.Y+BR.Y)*0.5-Size.Y*0.5)<2);}
        else
        {
        Test->TestTrue(TEXT("Build leaves centre aim/right margin"),TL.X>Size.X*0.55);
        auto* Vitals=PC->SurvivalHUD->WidgetTree->FindWidget(TEXT("PF_VitalsBounds"));const FGeometry V=Vitals->GetCachedGeometry();
        const FVector2D VBR=Root.AbsoluteToLocal(V.LocalToAbsolute(V.GetLocalSize()));
        Test->TestTrue(TEXT("Panel cannot cover left survival vitals"),VBR.X<TL.X);
        const FGeometry Prompt=PC->SurvivalHUD->WidgetTree->FindWidget(TEXT("PF_InteractionLabel"))->GetCachedGeometry();
        const FVector2D PromptBR=Root.AbsoluteToLocal(Prompt.LocalToAbsolute(Prompt.GetLocalSize()));
        Test->TestTrue(TEXT("Interaction prompt ends before action panel"),PromptBR.X<TL.X);
        }
        for(const TCHAR* Suffix:{TEXT("_Heading"),TEXT("_Body"),TEXT("_Result")})
        {auto* T=Cast<UTextBlock>(Widget->WidgetTree->FindWidget(FName(*(FString(Name)+Suffix))));Test->TestTrue(FString(TEXT("Text fits allocated row "))+Suffix,T && T->GetDesiredSize().Y<=T->GetCachedGeometry().GetLocalSize().Y+1);}
        auto* Body=Cast<UTextBlock>(Widget->WidgetTree->FindWidget(FName(*(FString(Name)+TEXT("_Body")))));
        Test->TestEqual(TEXT("Action text honors HUD scale"),Body->GetFont().Size,float(FMath::RoundToInt((bRecipes?16:18)*UPFGameUserSettings::Get()->Preferences.HUDScale)));
        {
            TArray<UWidget*> Widgets;Widget->WidgetTree->GetAllWidgets(Widgets);
            for(auto* Child:Widgets){if(auto* T=Cast<UTextBlock>(Child))
            {
                const auto Geometry=T->GetCachedGeometry();const auto Bottom=Root.AbsoluteToLocal(Geometry.LocalToAbsolute(Geometry.GetLocalSize()));
                Test->TestTrue(FString(TEXT("Action text fits screen: "))+T->GetName(),Bottom.Y<Size.Y && T->GetDesiredSize().Y<=Geometry.GetLocalSize().Y+1);
            }}
        }
    }
    void Press(FKey Key){for(const auto& B:PC->InputComponent->KeyBindings){if(B.Chord.Key==Key && B.KeyEvent==IE_Pressed){B.KeyDelegate.Execute(Key);return;}}Test->AddError(TEXT("Missing action binding ")+Key.ToString());}
    void SlatePress(FKey Key){auto& App=FSlateApplication::Get();App.ProcessKeyDownEvent(FKeyEvent(Key,FModifierKeysState(),0,false,0,0));App.ProcessKeyUpEvent(FKeyEvent(Key,FModifierKeysState(),0,false,0,0));}
    void Activate(UWidget* Widget)
    {
        auto* Button=Cast<UButton>(Widget);if(!Button){Test->AddError(TEXT("Missing button target"));return;}
        Button->SetKeyboardFocus();Test->TestTrue(TEXT("Actual button has Slate focus"),Button->HasKeyboardFocus());
        SlatePress(EKeys::SpaceBar);
    }
    void Wait(double Now,double Seconds,int32 Next){Until=Now+Seconds;Phase=Next;}
    void Capture(const TCHAR* Name){Shot=Directory/(FString(Name)+TEXT(".png"));FScreenshotRequest::RequestScreenshot(Shot,true,false);}
    bool CheckShot(){if(IFileManager::Get().FileSize(*Shot)<=0){return false;}Test->AddInfo(TEXT("[PrimalUI] Action overlay screenshot: ")+Shot);return true;}
    FAutomationTestBase* Test;FString Directory,Shot;TWeakObjectPtr<APFSurvivalPlayerController> PC;
    TWeakObjectPtr<UUserWidget> Craft,Build;TWeakObjectPtr<APFBuildPiece> Storage;
    int32 Phase=0;double Started=FPlatformTime::Seconds(),Until=0;float OriginalScale=1;bool bChanged=false;
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFActionOverlayLiveTest,"PF.UI.ActionOverlaysLive",EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)
bool FPFActionOverlayLiveTest::RunTest(const FString&)
{
    if(!FParse::Param(FCommandLine::Get(),TEXT("PFRunControlsUITest")) || FParse::Param(FCommandLine::Get(),TEXT("nullrhi")))
    {AddError(TEXT("Requires isolated rendered -game -PFRunControlsUITest"));return false;}
    FString Label;FParse::Value(FCommandLine::Get(),TEXT("PFControlsEvidence="),Label);
    if(Label.IsEmpty() || Label.Len()>48){AddError(TEXT("Supply unique bounded evidence label"));return false;}
    for(TCHAR C:Label){if(!FChar::IsAlnum(C) && C!=TEXT('_')){AddError(TEXT("Invalid evidence label"));return false;}}
    const FString Directory=FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()/TEXT("AutomationReports/ControlsUI")/Label);
    if(IFileManager::Get().DirectoryExists(*Directory)){AddError(TEXT("Refusing to overwrite UI evidence"));return false;}
    IFileManager::Get().MakeDirectory(*Directory,true);
    ADD_LATENT_AUTOMATION_COMMAND(FActionOverlayExercise(this,Directory));return true;
}
#endif
