// PFInventoryHUD.cpp — see PFInventoryHUD.h.

#include "Inventory/PFInventoryHUD.h"
#include "Inventory/PFInventoryComponent.h"
#include "Inventory/PFItemCatalog.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "UI/PFReadOnlyOverlay.h"
#include "UI/PFInventoryDetails.h"
#include "UI/PFUITheme.h"
#include "Settings/PFGameUserSettings.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Button.h"
#include "Progression/PFPlayerStatsMenu.h"
#include "Components/UniformGridPanel.h"
#include "Components/ScrollBox.h"
#include "Components/SizeBox.h"
#include "Components/ButtonSlot.h"
#include "UI/PFItemPicture.h"
#include "PFRequestCodes.h"

void UPFInventorySlotButton::InitializeSlot(UPFInventoryHUD* InMenu,FGuid InId)
{Menu=InMenu;StackId=InId;OnClicked.AddDynamic(this,&UPFInventorySlotButton::Select);}
void UPFInventorySlotButton::Select(){if(Menu){Menu->SelectStack(StackId);}}
void UPFInventoryHUD::SelectStack(FGuid Id)
{if(auto* PC=Cast<APFSurvivalPlayerController>(GetOwningPlayer())){PC->SelectInventoryStack(Id);bWasOpen=false;SetKeyboardFocus();}}
UPFInventorySlotButton* UPFInventoryHUD::FindSlot(FGuid Id) const
{for(const auto& Entry:Slots){if(Entry->StackId==Id){return Entry.Get();}}return nullptr;}
void UPFInventoryHUD::Consume(){if(auto* PC=Cast<APFSurvivalPlayerController>(GetOwningPlayer())){PC->RequestInventoryAction(PFInventoryAction::Consume);}}
void UPFInventoryHUD::Split(){if(auto* PC=Cast<APFSurvivalPlayerController>(GetOwningPlayer())){PC->RequestInventoryAction(PFInventoryAction::Split);}}
void UPFInventoryHUD::Drop(){if(auto* PC=Cast<APFSurvivalPlayerController>(GetOwningPlayer())){PC->RequestInventoryAction(PFInventoryAction::Drop);}}

void UPFInventoryHUD::NativeOnInitialized()
{
    Super::NativeOnInitialized();SetIsFocusable(true);
    UBorder* P;UTextBlock* H;UTextBlock* B;UTextBlock* R;
    PFReadOnlyOverlay::Build(WidgetTree,TEXT("PF_InventoryPanel"),P,H,B,R);
    Panel=P;Heading=H;Text=B;Result=R;
    Panel->SetBrush(FSlateRoundedBoxBrush(PFUITheme::Surface,10.f,PFUITheme::Inset,1.f));
    Panel->SetBrushColor(FLinearColor::White);
    Heading->SetColorAndOpacity(PFUITheme::Accent);
    Text->SetColorAndOpacity(PFUITheme::Text);
    Result->SetColorAndOpacity(PFUITheme::Muted);
    auto* Column=CastChecked<UVerticalBox>(Panel->GetContent());
    // Keep the action footer last. A separate inset prevents batch details and
    // old server feedback from looking like an optimistic action result.
    Column->RemoveChild(Result);Column->RemoveChild(Text);
    auto* Columns=WidgetTree->ConstructWidget<UHorizontalBox>();Column->AddChildToVerticalBox(Columns)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    auto* Bag=WidgetTree->ConstructWidget<UVerticalBox>();auto* BagSlot=Columns->AddChildToHorizontalBox(Bag);FSlateChildSize BagSize(ESlateSizeRule::Fill);BagSize.Value=.60f;BagSlot->SetSize(BagSize);BagSlot->SetPadding(FMargin(0,0,16,0));
    Bag->AddChildToVerticalBox(Text)->SetPadding(FMargin(0,0,0,8));
    GridScroll=WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(),TEXT("PF_InventoryGridScroll"));Bag->AddChildToVerticalBox(GridScroll)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    Grid=WidgetTree->ConstructWidget<UUniformGridPanel>(UUniformGridPanel::StaticClass(),TEXT("PF_InventoryGrid"));Grid->SetSlotPadding(FMargin(4));GridScroll->AddChild(Grid);
    auto* DetailPanel=WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(),TEXT("PF_InventoryDetails"));
    DetailPanel->SetBrush(FSlateRoundedBoxBrush(PFUITheme::Inset,8.f));
    DetailPanel->SetPadding(FMargin(12,8));
    auto* DetailSlot=Columns->AddChildToHorizontalBox(DetailPanel);FSlateChildSize DetailSize(ESlateSizeRule::Fill);DetailSize.Value=.40f;DetailSlot->SetSize(DetailSize);
    auto* DetailScroll=WidgetTree->ConstructWidget<UScrollBox>();DetailPanel->SetContent(DetailScroll);
    auto* Details=WidgetTree->ConstructWidget<UVerticalBox>();DetailScroll->AddChild(Details);
    auto* PictureBounds=WidgetTree->ConstructWidget<USizeBox>();PictureBounds->SetWidthOverride(150);PictureBounds->SetHeightOverride(150);
    SelectedPicture=CreateWidget<UPFItemPicture>(GetOwningPlayer());PictureBounds->SetContent(SelectedPicture);Details->AddChildToVerticalBox(PictureBounds)->SetHorizontalAlignment(HAlign_Center);
    DetailTitle=WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(),TEXT("PF_InventoryDetailTitle"));
    DetailTitle->SetColorAndOpacity(PFUITheme::Text);
    DetailTitle->SetTextOverflowPolicy(ETextOverflowPolicy::Ellipsis);
    Details->AddChildToVerticalBox(DetailTitle);
    DetailBody=WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(),TEXT("PF_InventoryDetailBody"));
    DetailBody->SetColorAndOpacity(PFUITheme::Muted);DetailBody->SetAutoWrapText(true);
    Details->AddChildToVerticalBox(DetailBody)->SetPadding(FMargin(0,4,0,0));
    auto* Actions=WidgetTree->ConstructWidget<UHorizontalBox>();Details->AddChildToVerticalBox(Actions)->SetPadding(FMargin(0,12,0,0));
    const auto Action=[&](const TCHAR* Name,const TCHAR* Caption)
    {auto* Button=WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(),Name);Button->SetStyle(PFUITheme::NavigationButton(false));auto* Label=WidgetTree->ConstructWidget<UTextBlock>();Label->SetText(FText::FromString(Caption));Label->SetColorAndOpacity(PFUITheme::Text);auto F=Label->GetFont();F.Size=14;Label->SetFont(F);Button->SetContent(Label);auto* S=Actions->AddChildToHorizontalBox(Button);S->SetSize(FSlateChildSize(ESlateSizeRule::Fill));S->SetPadding(FMargin(2));PFUITheme::AddButtonIcon(WidgetTree,Button,FString(Name).Contains(TEXT("Consume"))?TEXT("Use"):FString(Name).Contains(TEXT("Split"))?TEXT("Split"):TEXT("Drop"));return Button;};
    Action(TEXT("PF_InventoryConsume"),TEXT("Use [Q]"))->OnClicked.AddDynamic(this,&UPFInventoryHUD::Consume);
    Action(TEXT("PF_InventorySplit"),TEXT("Split [X]"))->OnClicked.AddDynamic(this,&UPFInventoryHUD::Split);
    Action(TEXT("PF_InventoryDrop"),TEXT("Drop [G]"))->OnClicked.AddDynamic(this,&UPFInventoryHUD::Drop);
    Column->AddChildToVerticalBox(Result)->SetPadding(FMargin(0,8,0,0));
    auto* Canvas=CastChecked<UCanvasPanel>(WidgetTree->RootWidget);
    PFUITheme::AddScrim(WidgetTree,Canvas);
    auto* ItemSlot=CastChecked<UCanvasPanelSlot>(Panel->Slot);ItemSlot->SetAnchors(FAnchors(.15f,.16f,.85f,.95f));
    auto* Tabs=WidgetTree->ConstructWidget<UHorizontalBox>();auto* TabSlot=Canvas->AddChildToCanvas(Tabs);TabSlot->SetAnchors(FAnchors(.15f,.05f,.85f,.14f));TabSlot->SetOffsets(FMargin(0));
    auto Tab=[&](FName Name,const TCHAR* Caption)
    {
        auto* B=WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(),Name);auto* T=WidgetTree->ConstructWidget<UTextBlock>();T->SetText(FText::FromString(Caption));T->SetColorAndOpacity(PFUITheme::Text);B->SetContent(T);
        auto* S=Tabs->AddChildToHorizontalBox(B);S->SetSize(FSlateChildSize(ESlateSizeRule::Fill));S->SetPadding(FMargin(4,0));return B;
    };
    ItemsTab=Tab(TEXT("PF_InventoryItemsTab"),TEXT("Items"));StatsTab=Tab(TEXT("PF_InventoryStatsTab"),TEXT("Player upgrades"));
    PFUITheme::AddButtonIcon(WidgetTree,ItemsTab,TEXT("Inventory"));PFUITheme::AddButtonIcon(WidgetTree,StatsTab,TEXT("Experience"));
    ItemsTab->OnClicked.AddDynamic(this,&UPFInventoryHUD::ShowItems);StatsTab->OnClicked.AddDynamic(this,&UPFInventoryHUD::ShowStats);
    StatsPage=CreateWidget<UPFPlayerStatsMenu>(GetOwningPlayer());auto* StatsSlot=Canvas->AddChildToCanvas(StatsPage);StatsSlot->SetAnchors(FAnchors(.15f,.16f,.85f,.95f));StatsSlot->SetOffsets(FMargin(0));
    SetPage(false);SetVisibility(ESlateVisibility::Collapsed);
}

void UPFInventoryHUD::ShowItems(){if(auto* PC=Cast<APFSurvivalPlayerController>(GetOwningPlayer())){PC->SetStatMenuOpen(false);}}
void UPFInventoryHUD::ShowStats(){if(auto* PC=Cast<APFSurvivalPlayerController>(GetOwningPlayer())){PC->SetStatMenuOpen(true);}}
void UPFInventoryHUD::SetPage(bool Stats)
{
    Panel->SetVisibility(Stats?ESlateVisibility::Collapsed:ESlateVisibility::Visible);
    StatsPage->SetVisibility(Stats?ESlateVisibility::Visible:ESlateVisibility::Collapsed);
    ItemsTab->SetStyle(PFUITheme::NavigationButton(!Stats));StatsTab->SetStyle(PFUITheme::NavigationButton(Stats));bWasOpen=false;
    if(auto* PC=Cast<APFSurvivalPlayerController>(GetOwningPlayer());PC && PC->IsInventoryOpen())
    {if(Stats){StatsPage->RefreshMenu();StatsPage->SetKeyboardFocus();}else{SetKeyboardFocus();}}
}
FReply UPFInventoryHUD::NativeOnPreviewKeyDown(const FGeometry&,const FKeyEvent& E)
{
    if(auto* PC=Cast<APFSurvivalPlayerController>(GetOwningPlayer())){if(PC->HandleInventoryKey(E.GetKey(),E.IsRepeat())){return FReply::Handled();}}
    return FReply::Unhandled();
}

void UPFInventoryHUD::NativeTick(const FGeometry& Geometry,float Delta)
{
    Super::NativeTick(Geometry,Delta);
    const auto* PC=Cast<APFSurvivalPlayerController>(GetOwningPlayer());
    const bool bOpen=PC && PC->IsInventoryOpen() && !PC->IsPauseMenuOpen();
    Refresh+=Delta;
    if(!bOpen || PC->IsStatMenuOpen()){bWasOpen=false;return;}
    if(bWasOpen && Refresh<0.1f){return;}
    bWasOpen=true;Refresh=0;
    const auto* Settings=UPFGameUserSettings::Get();
    const float RequestedScale=Settings?Settings->Preferences.HUDScale:1;
    const float Scale=FMath::IsFinite(RequestedScale)?FMath::Clamp(RequestedScale,0.75f,1.5f):1;
    PFReadOnlyOverlay::SetScale(Heading,Text,Result,Scale);
    auto Font=[&](UTextBlock* Label,int32 Size){auto F=Label->GetFont();F.Size=FMath::RoundToInt(Size*Scale);F.TypefaceFontName=Label==Heading || Label==DetailTitle?TEXT("Bold"):TEXT("Regular");if(Label->GetFont()!=F){Label->SetFont(F);}};
    Font(Heading,22);Font(Text,14);Font(DetailTitle,20);Font(DetailBody,14);Font(Result,14);
    // The inventory replicates with the PlayerState; it can be briefly missing after joining.
    const auto* I=PC->GetInventory();
    if(!I){Heading->SetText(FText::FromString(TEXT("INVENTORY")));Text->SetText(FText::FromString(TEXT("Waiting for inventory...")));DetailTitle->SetText(FText::GetEmpty());DetailBody->SetText(FText::GetEmpty());Result->SetText(FText::GetEmpty());return;}

    // Header with capacity and controls.
    FString Header=FString::Printf(TEXT("INVENTORY %d/%d slots | %.1f/%.1f kg"),I->GetStacks().Num(),I->SlotLimit,I->GetWeight(),I->WeightLimit);
    Heading->SetText(FText::FromString(Header));

    // One row per stack: ">" marks the selection; perishables show seconds remaining.
    const int32 Selected=PC->GetSelectedInventoryIndex();
    const auto& Stacks=I->GetStacks();
    // Window follows the resolved GUID selection; it never picks a replacement stack.
    TArray<FGuid> Ids;for(const auto& S:Stacks){Ids.Add(S.StackId);}
    if(Ids!=DisplayedIds)
    {
        Grid->ClearChildren();Slots.Reset();SlotLabels.Reset();SlotPictures.Reset();DisplayedIds=Ids;LastShownSelection.Invalidate();
        for(int32 N=0;N<Stacks.Num();++N)
        {
            auto* Button=WidgetTree->ConstructWidget<UPFInventorySlotButton>(UPFInventorySlotButton::StaticClass(),FName(*FString::Printf(TEXT("PF_InventorySlot%d"),N)));Button->InitializeSlot(this,Stacks[N].StackId);Slots.Add(Button);
            auto* Bounds=WidgetTree->ConstructWidget<USizeBox>();Bounds->SetWidthOverride(108);Bounds->SetMinDesiredHeight(126);Button->SetContent(Bounds);CastChecked<UButtonSlot>(Bounds->Slot)->SetPadding(FMargin(5));
            auto* Cell=WidgetTree->ConstructWidget<UVerticalBox>();Bounds->SetContent(Cell);
            auto* ArtSize=WidgetTree->ConstructWidget<USizeBox>();ArtSize->SetWidthOverride(74);ArtSize->SetHeightOverride(74);auto* Art=CreateWidget<UPFItemPicture>(GetOwningPlayer());ArtSize->SetContent(Art);SlotPictures.Add(Art);Cell->AddChildToVerticalBox(ArtSize)->SetHorizontalAlignment(HAlign_Center);
            auto* Label=WidgetTree->ConstructWidget<UTextBlock>();Label->SetColorAndOpacity(PFUITheme::Text);Label->SetAutoWrapText(true);Label->SetJustification(ETextJustify::Center);Cell->AddChild(Label);SlotLabels.Add(Label);
            Grid->AddChildToUniformGrid(Button,N/4,N%4);
        }
    }
    Text->SetText(FText::FromString(Stacks.IsEmpty()?TEXT("Empty - find world pickups (E)."):TEXT("SUPPLIES  /  click a slot to inspect")));
    for(int32 N=0;N<Stacks.Num();++N)
    {
        const auto& S=Stacks[N];const auto* D=I->Definition(S.ItemId);const FString Name=D?D->DisplayName.ToString():S.ItemId.ToString();
        const FString Fresh=S.ExpiresAt>0?FString::Printf(TEXT("\n[%ds fresh]"),FMath::Max(0,FMath::CeilToInt(S.ExpiresAt-UPFInventoryComponent::ServerTime(GetWorld())))):TEXT("");
        SlotLabels[N]->SetText(FText::FromString(FString::Printf(TEXT("%s%s\nx%d%s"),N==Selected?TEXT("> "):TEXT(""),*Name,S.Quantity,*Fresh)));Font(SlotLabels[N],12);SlotPictures[N]->SetItem(D);Slots[N]->SetStyle(PFUITheme::NavigationButton(N==Selected));Slots[N]->SetToolTipText(FText::FromString(Name));
    }
    if(Stacks.IsValidIndex(Selected) && LastShownSelection!=Stacks[Selected].StackId){GridScroll->ScrollWidgetIntoView(Slots[Selected],false,EDescendantScrollDestination::IntoView);LastShownSelection=Stacks[Selected].StackId;}
    const FPFItemStack* SelectedStack=Stacks.IsValidIndex(Selected)?&Stacks[Selected]:nullptr;
    const auto Detail=PFInventoryDetails::Describe(SelectedStack,SelectedStack?I->Definition(SelectedStack->ItemId):nullptr,UPFInventoryComponent::ServerTime(GetWorld()));
    SelectedPicture->SetItem(SelectedStack?I->Definition(SelectedStack->ItemId):nullptr);
    DetailTitle->SetText(Detail.Title);DetailBody->SetText(Detail.Body);
    FString Feedback=PC->GetInventoryMessage();
    if(Selected==INDEX_NONE && !Stacks.IsEmpty()){Feedback=TEXT("No selected stack.");}
    else if(!Feedback.IsEmpty()){Feedback=TEXT("Last: ")+Feedback;}
    if(!Settings || Settings->Preferences.bControlHints){Feedback+=TEXT("\nTab/View close | U/RB upgrades | Click/Up/Down select\n1–8 pin hotbar | 0 clear | X split | G drop | Q use");}
    Result->SetText(FText::FromString(Feedback));
}
