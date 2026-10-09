// PFCraftingHUD.cpp — see PFCraftingHUD.h.

#include "Crafting/PFCraftingHUD.h"
#include "Crafting/PFCraftingComponent.h"
#include "Inventory/PFItemCatalog.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "UI/PFReadOnlyOverlay.h"
#include "UI/PFUITheme.h"
#include "UI/PFRecipeDetails.h"
#include "Settings/PFGameUserSettings.h"

void UPFCraftingHUD::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    UBorder* P;UTextBlock* H;UTextBlock* B;UTextBlock* R;
    PFReadOnlyOverlay::Build(WidgetTree,TEXT("PF_CraftingPanel"),P,H,B,R);
    Panel=P;Heading=H;Result=R;
    Panel->SetBrush(FSlateRoundedBoxBrush(PFUITheme::Surface,10.f,PFUITheme::Inset,1.f));Panel->SetBrushColor(FLinearColor::White);
    Heading->SetColorAndOpacity(PFUITheme::Accent);Result->SetColorAndOpacity(PFUITheme::Muted);
    auto* Column=CastChecked<UVerticalBox>(Panel->GetContent());Column->RemoveChild(B);Column->RemoveChild(Result);
    auto* Cards=WidgetTree->ConstructWidget<UVerticalBox>();Column->AddChildToVerticalBox(Cards)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    for(int32 N=0;N<3;++N)
    {
        auto* Card=WidgetTree->ConstructWidget<UBorder>();Card->SetBrush(FSlateRoundedBoxBrush(PFUITheme::Inset,8.f));Card->SetPadding(FMargin(12,8));
        Cards->AddChildToVerticalBox(Card)->SetPadding(FMargin(0,0,0,8));
        auto* Detail=WidgetTree->ConstructWidget<UVerticalBox>();Card->SetContent(Detail);
        auto* Title=WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(),FName(*FString::Printf(TEXT("PF_Recipe%d_Title"),N)));
        Title->SetAutoWrapText(true);Title->SetColorAndOpacity(PFUITheme::Text);Detail->AddChildToVerticalBox(Title);
        // Retain the existing first recipe body locator for external UI fixtures.
        auto* Body=N==0?B:WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(),FName(*FString::Printf(TEXT("PF_Recipe%d_Body"),N)));
        Body->SetAutoWrapText(true);Body->SetColorAndOpacity(PFUITheme::Muted);Detail->AddChildToVerticalBox(Body)->SetPadding(FMargin(0,4,0,0));
        RecipeTitles.Add(Title);RecipeBodies.Add(Body);
    }
    Column->AddChildToVerticalBox(Result)->SetPadding(FMargin(0,8,0,0));
    SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UPFCraftingHUD::NativeTick(const FGeometry& Geometry,float Delta)
{
    Super::NativeTick(Geometry,Delta);
    const auto* PC=Cast<APFSurvivalPlayerController>(GetOwningPlayer());
    const bool bOpen=PC && PC->IsCraftingOpen() && !PC->IsPauseMenuOpen();
    Panel->SetVisibility(bOpen?ESlateVisibility::HitTestInvisible:ESlateVisibility::Collapsed);
    Refresh+=Delta;
    if(!bOpen){bWasOpen=false;return;}
    if(bWasOpen && Refresh<0.1f){return;}
    bWasOpen=true;Refresh=0;
    const auto* Settings=UPFGameUserSettings::Get();
    const float RequestedScale=Settings?Settings->Preferences.HUDScale:1;
    const float Scale=FMath::IsFinite(RequestedScale)?FMath::Clamp(RequestedScale,0.75f,1.5f):1;
    auto Font=[&](UTextBlock* Label,int32 Size){auto F=Label->GetFont();F.Size=FMath::RoundToInt(Size*Scale);if(Label->GetFont().Size!=F.Size){Label->SetFont(F);}};
    Font(Heading,18);Font(Result,16);
    for(int32 N=0;N<3;++N){Font(RecipeTitles[N],18);Font(RecipeBodies[N],16);}
    Heading->SetText(FText::FromString(!Settings || Settings->Preferences.bControlHints?
        TEXT("CRAFTING\nC / Y close | R / D-left cancel"):TEXT("CRAFTING")));
    const auto* C=PC->GetCrafting();
    const auto* I=PC->GetInventory();
    // Fixed list matching hotkeys 1/2/3 in the controller.
    const FName Ids[]={TEXT("Recipe_Tool"),TEXT("Recipe_Cook"),TEXT("Recipe_Dry")};
    const TCHAR* Keys[]={TEXT("1 / pad X"),TEXT("2 / D-up"),TEXT("3 / D-down")};
    for(int32 N=0;N<3;++N)
    {
        const auto* D=C && C->Catalog && I?C->Catalog->Recipe(Ids[N],I->Catalog):nullptr;
        const auto View=PFRecipeDetails::Describe(D,I,UPFInventoryComponent::ServerTime(GetWorld()),C && !C->ActiveRecipe.IsNone());
        const FString Prefix=!Settings || Settings->Preferences.bControlHints?FString(Keys[N])+TEXT(": "):FString();
        RecipeTitles[N]->SetText(FText::FromString(Prefix+View.Title.ToString()));RecipeBodies[N]->SetText(View.Body);
    }
    if(!C){Result->SetText(FText::FromString(TEXT("Waiting for server crafting state.")));return;}
    FString Status=C->ActiveRecipe.IsNone()?TEXT("Job: idle"):FString::Printf(TEXT("Job: %.1fs remaining"),FMath::Max(0.0,C->FinishAt-UPFInventoryComponent::ServerTime(GetWorld())));
    Status+=TEXT("\nLast craft status: ")+(C->Feedback.IsEmpty()?FString(TEXT("No server result yet")):C->Feedback);
    const FString Request=PC->GetInventoryMessage();
    if(Request.StartsWith(TEXT("Craft "))){Status+=TEXT("\nLast request: ")+Request;}
    Result->SetText(FText::FromString(Status));
}
