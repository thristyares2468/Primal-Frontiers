#include "Inventory/PFHotbarHUD.h"
#include "Inventory/PFInventoryComponent.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Building/PFBuildingComponent.h"
#include "Settings/PFGameUserSettings.h"
#include "UI/PFItemPicture.h"
#include "UI/PFUITheme.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"

void UPFHotbarHUD::NativeOnInitialized()
{
    Super::NativeOnInitialized();auto* Column=WidgetTree->ConstructWidget<UVerticalBox>();WidgetTree->RootWidget=Column;
    Hint=WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(),TEXT("PF_HotbarHint"));Hint->SetJustification(ETextJustify::Center);Hint->SetColorAndOpacity(PFUITheme::Text);Column->AddChild(Hint);
    auto* Row=WidgetTree->ConstructWidget<UHorizontalBox>();Column->AddChild(Row);
    for(int32 N=0;N<FPFHotbar::SlotCount;++N)
    {
        auto* Frame=WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(),FName(*FString::Printf(TEXT("PF_HotbarSlot%d"),N+1)));Frame->SetPadding(FMargin(3));Frames.Add(Frame);Row->AddChildToHorizontalBox(Frame)->SetPadding(FMargin(2,4));
        auto* Bounds=WidgetTree->ConstructWidget<USizeBox>();Bounds->SetWidthOverride(88);Bounds->SetHeightOverride(108);Frame->SetContent(Bounds);
        auto* Cell=WidgetTree->ConstructWidget<UVerticalBox>();Bounds->SetContent(Cell);
        auto* Key=WidgetTree->ConstructWidget<UTextBlock>();Key->SetText(FText::AsNumber(N+1));Key->SetColorAndOpacity(PFUITheme::Muted);auto Font=Key->GetFont();Font.Size=11;Key->SetFont(Font);Cell->AddChild(Key);
        auto* ArtBounds=WidgetTree->ConstructWidget<USizeBox>();ArtBounds->SetWidthOverride(56);ArtBounds->SetHeightOverride(56);auto* Picture=CreateWidget<UPFItemPicture>(GetOwningPlayer());Pictures.Add(Picture);ArtBounds->SetContent(Picture);Cell->AddChildToVerticalBox(ArtBounds)->SetHorizontalAlignment(HAlign_Center);
        auto* Count=WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(),FName(*FString::Printf(TEXT("PF_HotbarCount%d"),N+1)));Count->SetJustification(ETextJustify::Center);Count->SetColorAndOpacity(PFUITheme::Text);Counts.Add(Count);Cell->AddChild(Count);
    }
    SetVisibility(ESlateVisibility::HitTestInvisible);
}
void UPFHotbarHUD::NativeTick(const FGeometry& Geometry,float Delta)
{
    Super::NativeTick(Geometry,Delta);Refresh+=Delta;if(Refresh<.1f){return;}Refresh=0;
    const auto* PC=Cast<APFSurvivalPlayerController>(GetOwningPlayer());const auto* I=PC?PC->GetInventory():nullptr;
    const auto* Survival=PC && PC->GetPawn()?PC->GetPawn()->FindComponentByClass<UPFPlayerSurvivalComponent>():nullptr;
    const bool Show=I && Survival && !Survival->IsDead() && !PC->IsPauseMenuOpen() && !PC->IsInventoryOpen() && !PC->IsCraftingOpen() && !PC->IsBuildingMenuOpen() && !PC->IsStorageMenuOpen() && (!PC->Building || !PC->Building->bBuildMode);
    // Hide contents instead of this ticking widget, so it can restore after modal close/respawn.
    WidgetTree->RootWidget->SetVisibility(Show?ESlateVisibility::HitTestInvisible:ESlateVisibility::Hidden);
    if(!Show){return;}
    const auto* Settings=UPFGameUserSettings::Get();const float Scale=Settings?Settings->Preferences.HUDScale:1.f;
    Hint->SetText(FText::FromString((!Settings || Settings->Preferences.bControlHints)?TEXT("1–8 select  ·  Q / D-down use  ·  Tab + 1–8 assign"):TEXT("")));auto HintFont=Hint->GetFont();HintFont.Size=FMath::RoundToInt(11*Scale);Hint->SetFont(HintFont);
    for(int32 N=0;N<FPFHotbar::SlotCount;++N)
    {
        const auto* Stack=PC->GetHotbar().Resolve(N,I,UPFInventoryComponent::ServerTime(GetWorld()));const bool Selected=N==PC->GetHotbar().Selected();
        Frames[N]->SetBrush(FSlateRoundedBoxBrush(PFUITheme::Surface,3.f,Selected?PFUITheme::Accent:PFUITheme::Line,Selected?2.f:1.f));
        Pictures[N]->SetItem(Stack?I->Definition(Stack->ItemId):nullptr);Pictures[N]->SetVisibility(Stack?ESlateVisibility::HitTestInvisible:ESlateVisibility::Hidden);
        Counts[N]->SetText(FText::FromString(Stack?FString::FromInt(Stack->Quantity):PC->GetHotbar().Identity(N).IsValid()?TEXT("lost"):TEXT("—")));
        auto Font=Counts[N]->GetFont();Font.Size=FMath::RoundToInt(12*Scale);Counts[N]->SetFont(Font);
    }
}
