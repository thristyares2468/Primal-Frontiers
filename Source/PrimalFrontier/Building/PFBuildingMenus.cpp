#include "Building/PFBuildingMenus.h"
#include "Building/PFBuildingComponent.h"
#include "Building/PFBuildPiece.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Inventory/PFInventoryComponent.h"
#include "Inventory/PFItemCatalog.h"
#include "Settings/PFGameUserSettings.h"
#include "UI/PFUITheme.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/UniformGridPanel.h"
#include "Components/ScrollBox.h"
#include "Components/WidgetSwitcher.h"
#include "Components/ButtonSlot.h"
#include "InputCoreTypes.h"

namespace
{
float MenuScale()
{
    const auto* S=UPFGameUserSettings::Get();
    const float Scale=S?S->Preferences.HUDScale:1;
    return FMath::IsFinite(Scale)?FMath::Clamp(Scale,0.75f,1.5f):1;
}
UTextBlock* Label(UWidgetTree* Tree,FName Name,const FString& Text,int32 Size=16)
{
    auto* T=Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(),Name);
    T->SetText(FText::FromString(Text));T->SetAutoWrapText(true);T->SetColorAndOpacity(PFUITheme::Text);
    auto Font=T->GetFont();Font.Size=FMath::RoundToInt(Size*MenuScale());T->SetFont(Font);return T;
}
UVerticalBox* Panel(UWidgetTree* Tree,FName Name)
{
    auto* Root=Tree->ConstructWidget<UCanvasPanel>();Tree->RootWidget=Root;
    auto* Frame=Tree->ConstructWidget<UBorder>(UBorder::StaticClass(),Name);
    Frame->SetBrush(FSlateRoundedBoxBrush(PFUITheme::Surface,12.f,PFUITheme::Accent,1.f));Frame->SetPadding(FMargin(20));
    auto* Slot=Root->AddChildToCanvas(Frame);Slot->SetAnchors(FAnchors(0.08f,0.06f,0.92f,0.94f));Slot->SetOffsets(FMargin(0));
    auto* Rows=Tree->ConstructWidget<UVerticalBox>();Frame->SetContent(Rows);return Rows;
}
FString BuildHint(EPFBuildKind K)
{
    if(K==EPFBuildKind::Foundation){return TEXT("Start on flat ground");}
    if(K==EPFBuildKind::Floor || K==EPFBuildKind::Ceiling){return TEXT("Aim at an owned wall/frame");}
    if(K==EPFBuildKind::Window){return TEXT("Window opening on a platform edge");}
    if(K==EPFBuildKind::Stairs){return TEXT("Climb to the next storey");}
    if(K==EPFBuildKind::Storage){return TEXT("Owned chest: E to open");}
    return TEXT("Aim at your foundation/floor");
}
}

void UPFBuildChoiceButton::InitializeChoice(UPFBuildingMenu* OwnerMenu,FName Id)
{Menu=OwnerMenu;Definition=Id;SetStyle(PFUITheme::NavigationButton(false));OnClicked.AddDynamic(this,&UPFBuildChoiceButton::Choose);}
void UPFBuildChoiceButton::Choose(){if(Menu){Menu->Choose(Definition);}}

void UPFBuildingMenu::NativeOnInitialized()
{Super::NativeOnInitialized();SetIsFocusable(true);Content=Panel(WidgetTree,TEXT("PF_BuildPickerPanel"));SetVisibility(ESlateVisibility::Collapsed);}
void UPFBuildingMenu::RefreshMenu()
{
    Content->ClearChildren();
    Content->AddChildToVerticalBox(Label(WidgetTree,NAME_None,TEXT("BUILD YOUR SHELTER"),24));
    Content->AddChildToVerticalBox(Label(WidgetTree,NAME_None,TEXT("Choose a piece, then aim to place it. Every placement is checked by the server.")))->SetPadding(FMargin(0,8,0,12));
    auto* Scroll=WidgetTree->ConstructWidget<UScrollBox>();Content->AddChildToVerticalBox(Scroll)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    auto* Grid=WidgetTree->ConstructWidget<UUniformGridPanel>();Grid->SetSlotPadding(FMargin(6));Scroll->AddChild(Grid);
    auto* PC=Cast<APFSurvivalPlayerController>(GetOwningPlayer());
    auto* Catalog=PC && PC->Building?PC->Building->Catalog.Get():nullptr;
    if(Catalog)
    {
        for(int32 N=0;N<Catalog->Pieces.Num();++N)
        {
            const auto& D=Catalog->Pieces[N];if(!Catalog->Find(D.Id)){continue;}
            auto* Choice=WidgetTree->ConstructWidget<UPFBuildChoiceButton>(UPFBuildChoiceButton::StaticClass(),FName(*(TEXT("PF_BuildChoice_")+D.Id.ToString())));
            Choice->InitializeChoice(this,D.Id);
            Choice->SetContent(Label(WidgetTree,NAME_None,FString::Printf(TEXT("%d. %s\n%d Wood\n%s"),N+1,*D.Name.ToString(),D.WoodCost,*BuildHint(D.Kind))));
            CastChecked<UButtonSlot>(Choice->GetContent()->Slot)->SetHorizontalAlignment(HAlign_Fill);
            Grid->AddChildToUniformGrid(Choice,N/2,N%2);
        }
    }
    auto* Close=WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(),TEXT("PF_CloseBuildPicker"));Close->SetStyle(PFUITheme::NavigationButton(false));
    Close->SetContent(Label(WidgetTree,NAME_None,TEXT("Close building  [B / Esc / Controller B]")));Close->OnClicked.AddDynamic(this,&UPFBuildingMenu::CloseMenu);
    Content->AddChildToVerticalBox(Close)->SetPadding(FMargin(0,12,0,0));
}
void UPFBuildingMenu::Choose(FName Id){if(auto* PC=Cast<APFSurvivalPlayerController>(GetOwningPlayer())){PC->SelectBuildingPiece(Id);}}
void UPFBuildingMenu::CloseMenu()
{if(auto* PC=Cast<APFSurvivalPlayerController>(GetOwningPlayer())){PC->Building->bBuildMode=false;PC->SetBuildingMenuOpen(false);}}
FReply UPFBuildingMenu::NativeOnKeyDown(const FGeometry& G,const FKeyEvent& E)
{
    const auto K=E.GetKey();
    if(K==EKeys::B || K==EKeys::Escape || K==EKeys::Gamepad_FaceButton_Right){if(!E.IsRepeat()){CloseMenu();}return FReply::Handled();}
    if(K==EKeys::P || K==EKeys::Gamepad_Special_Right){CloseMenu();CastChecked<APFSurvivalPlayerController>(GetOwningPlayer())->SetPauseMenuOpen(true);return FReply::Handled();}
    const FKey Keys[]={EKeys::One,EKeys::Two,EKeys::Three,EKeys::Four,EKeys::Five,EKeys::Six,EKeys::Seven,EKeys::Eight};
    auto* PC=Cast<APFSurvivalPlayerController>(GetOwningPlayer());
    for(int32 N=0;N<8;++N){if(K==Keys[N] && PC && PC->Building->Catalog && PC->Building->Catalog->Pieces.IsValidIndex(N)){Choose(PC->Building->Catalog->Pieces[N].Id);return FReply::Handled();}}
    return Super::NativeOnKeyDown(G,E);
}

void UPFStorageTransferButton::InitializeTransfer(APFSurvivalPlayerController* PC,bool Deposit,FGuid Id,int32 Amount)
{Controller=PC;bDeposit=Deposit;Stack=Id;Quantity=FMath::Clamp(Amount,1,PFItemLimits::MaxStackQuantity);SetStyle(PFUITheme::NavigationButton(false));OnClicked.AddDynamic(this,&UPFStorageTransferButton::Transfer);}
void UPFStorageTransferButton::Transfer()
{if(Controller.IsValid() && Controller->IsStorageMenuOpen()){Controller->Building->ServerTransfer(bDeposit,Stack,Quantity);}}

void UPFStorageFilterChoiceButton::InitializeChoice(UPFStorageMenu* Menu,FName Id)
{OwnerMenu=Menu;ItemId=Id;SetStyle(PFUITheme::NavigationButton(false));OnClicked.AddDynamic(this,&UPFStorageFilterChoiceButton::Toggle);}
void UPFStorageFilterChoiceButton::Toggle(){if(OwnerMenu){OwnerMenu->ToggleFilter(ItemId);}}

void UPFStorageMenu::NativeOnInitialized()
{
    Super::NativeOnInitialized();SetIsFocusable(true);
    auto* Rows=Panel(WidgetTree,TEXT("PF_StoragePanel"));
    Rows->AddChildToVerticalBox(Label(WidgetTree,TEXT("PF_StorageTitle"),TEXT("CHEST"),24));
    Rows->AddChildToVerticalBox(Label(WidgetTree,TEXT("PF_StorageHint"),TEXT("Store or take a batch. Food keeps ageing here; the world continues running.")))->SetPadding(FMargin(0,8,0,12));
    auto* Toolbar=WidgetTree->ConstructWidget<UHorizontalBox>();Rows->AddChildToVerticalBox(Toolbar)->SetPadding(FMargin(0,0,0,8));
    const auto Action=[&](const TCHAR* Name,const TCHAR* Caption,auto Handler)
    {
        auto* Button=WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(),Name);Button->SetStyle(PFUITheme::NavigationButton(false));Button->SetContent(Label(WidgetTree,NAME_None,Caption));
        if(Handler==0){Button->OnClicked.AddDynamic(this,&UPFStorageMenu::SortChest);}
        else if(Handler==1){Button->OnClicked.AddDynamic(this,&UPFStorageMenu::QuickStore);}
        else{Button->OnClicked.AddDynamic(this,&UPFStorageMenu::OpenSettings);}
        auto* Slot=Toolbar->AddChildToHorizontalBox(Button);Slot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));Slot->SetPadding(FMargin(4));
    };
    Action(TEXT("PF_SortChest"),TEXT("Sort chest"),0);Action(TEXT("PF_QuickStore"),TEXT("Quick Store"),1);Action(TEXT("PF_ChestSettings"),TEXT("Chest settings"),2);
    Pages=WidgetTree->ConstructWidget<UWidgetSwitcher>();Rows->AddChildToVerticalBox(Pages)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    auto* Columns=WidgetTree->ConstructWidget<UHorizontalBox>();Pages->AddChild(Columns);
    const auto Column=[&](const TCHAR* Title,const TCHAR* Name)
    {
        auto* Frame=WidgetTree->ConstructWidget<UBorder>();Frame->SetBrush(FSlateRoundedBoxBrush(PFUITheme::Inset,8.f));Frame->SetPadding(FMargin(12));
        auto* Slot=Columns->AddChildToHorizontalBox(Frame);Slot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));Slot->SetPadding(FMargin(4));
        auto* Box=WidgetTree->ConstructWidget<UVerticalBox>();Frame->SetContent(Box);Box->AddChildToVerticalBox(Label(WidgetTree,FName(*(FString(Name)+TEXT("_Title"))),Title,20));
        auto* Scroll=WidgetTree->ConstructWidget<UScrollBox>();Box->AddChildToVerticalBox(Scroll)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
        auto* List=WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(),Name);Scroll->AddChild(List);return List;
    };
    BagRows=Column(TEXT("YOUR BAG"),TEXT("PF_StorageBagRows"));ChestRows=Column(TEXT("CHEST CONTENTS"),TEXT("PF_StorageChestRows"));
    auto* Settings=WidgetTree->ConstructWidget<UVerticalBox>();Pages->AddChild(Settings);
    Settings->AddChildToVerticalBox(Label(WidgetTree,NAME_None,TEXT("Allowed deposits — existing contents can always be taken out.")));
    auto* Mode=WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(),TEXT("PF_FilterMode"));Mode->SetStyle(PFUITheme::NavigationButton(false));Mode->SetContent(Label(WidgetTree,TEXT("PF_FilterModeText"),TEXT("")));Mode->OnClicked.AddDynamic(this,&UPFStorageMenu::ToggleFilterMode);Settings->AddChildToVerticalBox(Mode);
    CastChecked<UButtonSlot>(Mode->GetContent()->Slot)->SetHorizontalAlignment(HAlign_Fill);
    auto* FilterScroll=WidgetTree->ConstructWidget<UScrollBox>();Settings->AddChildToVerticalBox(FilterScroll)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));FilterRows=WidgetTree->ConstructWidget<UVerticalBox>();FilterScroll->AddChild(FilterRows);
    auto* Apply=WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(),TEXT("PF_ApplyChestSettings"));Apply->SetStyle(PFUITheme::NavigationButton(false));Apply->SetContent(Label(WidgetTree,NAME_None,TEXT("Apply filter")));Apply->OnClicked.AddDynamic(this,&UPFStorageMenu::ApplySettings);Settings->AddChildToVerticalBox(Apply);
    auto* Cancel=WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(),TEXT("PF_CancelChestSettings"));Cancel->SetStyle(PFUITheme::NavigationButton(false));Cancel->SetContent(Label(WidgetTree,NAME_None,TEXT("Cancel settings")));Cancel->OnClicked.AddDynamic(this,&UPFStorageMenu::CancelSettings);Settings->AddChildToVerticalBox(Cancel);
    Status=Label(WidgetTree,TEXT("PF_StorageResult"),TEXT(""));Rows->AddChildToVerticalBox(Status)->SetPadding(FMargin(0,10,0,10));
    auto* Close=WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(),TEXT("PF_CloseStorage"));Close->SetStyle(PFUITheme::NavigationButton(false));
    Close->SetContent(Label(WidgetTree,TEXT("PF_StorageCloseLabel"),TEXT("Close chest  [E / Tab / Esc / Controller B]")));Close->OnClicked.AddDynamic(this,&UPFStorageMenu::CloseMenu);Rows->AddChildToVerticalBox(Close);
    SetVisibility(ESlateVisibility::Collapsed);
}
void UPFStorageMenu::BuildRows(UPFInventoryComponent* I,bool Deposit,UVerticalBox* Rows,TArray<TObjectPtr<UTextBlock>>& Labels)
{
    Rows->ClearChildren();Labels.Reset();auto* PC=CastChecked<APFSurvivalPlayerController>(GetOwningPlayer());
    if(!I || I->GetStacks().IsEmpty()){Rows->AddChildToVerticalBox(Label(WidgetTree,NAME_None,TEXT("Empty")));return;}
    int32 N=0;
    for(const auto& Stack:I->GetStacks())
    {
        auto* Row=WidgetTree->ConstructWidget<UHorizontalBox>();Rows->AddChildToVerticalBox(Row)->SetPadding(FMargin(0,6));
        auto* T=Label(WidgetTree,NAME_None,TEXT(""));Labels.Add(T);Row->AddChildToHorizontalBox(T)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
        for(int32 All=0;All<2;++All)
        {
            const FString Name=FString::Printf(TEXT("PF_%s_%d_%s"),Deposit?TEXT("Deposit"):TEXT("Withdraw"),N,All?TEXT("Stack"):TEXT("One"));
            auto* Button=WidgetTree->ConstructWidget<UPFStorageTransferButton>(UPFStorageTransferButton::StaticClass(),FName(*Name));Button->InitializeTransfer(PC,Deposit,Stack.StackId,All?Stack.Quantity:1);
            Button->SetContent(Label(WidgetTree,NAME_None,FString(Deposit?TEXT("Store"):TEXT("Take"))+(All?TEXT(" stack"):TEXT(" 1")),14));Row->AddChildToHorizontalBox(Button)->SetPadding(FMargin(4,0));
        }
        ++N;
    }
}
void UPFStorageMenu::UpdateLabels(UPFInventoryComponent* I,const TArray<TObjectPtr<UTextBlock>>& Labels)
{
    if(!I){return;}
    for(int32 N=0;N<I->GetStacks().Num() && Labels.IsValidIndex(N);++N)
    {
        const auto& S=I->GetStacks()[N];const auto* D=I->Definition(S.ItemId);
        FString Text=FString::Printf(TEXT("%s x%d"),D?*D->DisplayName.ToString():*S.ItemId.ToString(),S.Quantity);
        Text+=S.ExpiresAt>0?FString::Printf(TEXT("\nExpires in %.0fs"),FMath::Max(0.0,S.ExpiresAt-UPFInventoryComponent::ServerTime(GetWorld()))):TEXT("\nDoes not expire");
        Labels[N]->SetText(FText::FromString(Text));
    }
}
void UPFStorageMenu::RefreshMenu()
{
    auto* PC=Cast<APFSurvivalPlayerController>(GetOwningPlayer());auto* Chest=PC?PC->Building->OpenStorage.Get():nullptr;
    if(!PC || !IsValid(Chest)){return;}
    auto* Bag=PC->GetInventory();auto* Store=Chest->Storage.Get();
    FString New=Chest->PersistentId.ToString()+FString::SanitizeFloat(MenuScale());
    New+=Chest->bRestrictStorageItems?TEXT("restricted"):TEXT("all");for(FName Id:Chest->AllowedStorageItems){New+=Id.ToString();}
    for(auto* I:{Bag,Store}){if(I){for(const auto& S:I->GetStacks()){New+=S.StackId.ToString()+TEXT(":")+FString::FromInt(S.Quantity);}}New+=TEXT("|");}
    if(New!=Signature)
    {
        Signature=New;BuildRows(Bag,true,BagRows,BagLabels);BuildRows(Store,false,ChestRows,ChestLabels);
        if(IsVisible()){SetKeyboardFocus();} // Retain close/pause navigation after a transferred row disappears.
    }
    const TPair<const TCHAR*,int32> Sizes[]={{TEXT("PF_StorageTitle"),24},{TEXT("PF_StorageHint"),16},{TEXT("PF_StorageBagRows_Title"),20},{TEXT("PF_StorageChestRows_Title"),20},{TEXT("PF_StorageCloseLabel"),16},{TEXT("PF_StorageResult"),16}};
    for(const auto& Size:Sizes){if(auto* T=Cast<UTextBlock>(WidgetTree->FindWidget(Size.Key))){auto Font=T->GetFont();Font.Size=FMath::RoundToInt(Size.Value*MenuScale());T->SetFont(Font);}}
    UpdateLabels(Bag,BagLabels);UpdateLabels(Store,ChestLabels);
    const FString Capacity=FString::Printf(TEXT("Chest %d/%d slots, %.1f/%.1f kg | %s\n"),Store->GetStacks().Num(),Store->SlotLimit,Store->GetWeight(),Store->WeightLimit,Chest->bRestrictStorageItems?TEXT("Filtered deposits"):TEXT("All items accepted"));
    Status->SetText(FText::FromString(Capacity+(PC->Building->Feedback.IsEmpty()?TEXT("Select Store or Take. The server confirms each transfer."):PC->Building->Feedback)));
}
void UPFStorageMenu::NativeTick(const FGeometry& G,float Delta)
{Super::NativeTick(G,Delta);if(IsVisible()){Refresh+=Delta;if(Refresh>=0.1f){Refresh=0;RefreshMenu();}}}
void UPFStorageMenu::CloseMenu()
{Pages->SetActiveWidgetIndex(0);if(auto* PC=Cast<APFSurvivalPlayerController>(GetOwningPlayer())){PC->Building->ServerCloseStorage();PC->SetStorageMenuOpen(false);}}
void UPFStorageMenu::SortChest(){CastChecked<APFSurvivalPlayerController>(GetOwningPlayer())->Building->ServerSortStorage();}
void UPFStorageMenu::ResetView(){Pages->SetActiveWidgetIndex(0);Signature.Reset();DraftAllowed.Reset();bDraftRestricted=false;}
void UPFStorageMenu::QuickStore(){CastChecked<APFSurvivalPlayerController>(GetOwningPlayer())->Building->ServerQuickStore();}
void UPFStorageMenu::OpenSettings()
{
    auto* PC=CastChecked<APFSurvivalPlayerController>(GetOwningPlayer());auto* Chest=PC->Building->OpenStorage.Get();if(!IsValid(Chest)){return;}
    bDraftRestricted=Chest->bRestrictStorageItems;DraftAllowed=Chest->AllowedStorageItems;FilterRows->ClearChildren();
    if(auto* I=PC->GetInventory();I && I->Catalog)
    {
        for(const auto& D:I->Catalog->Items)
        {
            if(!I->Definition(D.Id)){continue;}
            auto* Button=WidgetTree->ConstructWidget<UPFStorageFilterChoiceButton>(UPFStorageFilterChoiceButton::StaticClass(),FName(*(TEXT("PF_Filter_")+D.Id.ToString())));Button->InitializeChoice(this,D.Id);
            Button->SetContent(Label(WidgetTree,FName(*(TEXT("PF_FilterLabel_")+D.Id.ToString())),D.DisplayName.ToString()));FilterRows->AddChildToVerticalBox(Button)->SetPadding(FMargin(0,3));
            CastChecked<UButtonSlot>(Button->GetContent()->Slot)->SetHorizontalAlignment(HAlign_Fill);
        }
    }
    RefreshFilters();Pages->SetActiveWidgetIndex(1);SetKeyboardFocus();
}
void UPFStorageMenu::ToggleFilter(FName Id)
{bDraftRestricted=true;if(DraftAllowed.Contains(Id)){DraftAllowed.Remove(Id);}else{DraftAllowed.Add(Id);}RefreshFilters();}
void UPFStorageMenu::ToggleFilterMode(){bDraftRestricted=!bDraftRestricted;RefreshFilters();}
void UPFStorageMenu::RefreshFilters()
{
    CastChecked<UTextBlock>(WidgetTree->FindWidget(TEXT("PF_FilterModeText")))->SetText(FText::FromString(bDraftRestricted?TEXT("Selected items only (empty selection blocks every new deposit)"):TEXT("All items allowed")));
    auto* PC=CastChecked<APFSurvivalPlayerController>(GetOwningPlayer());auto* I=PC->GetInventory();
    if(I && I->Catalog){for(const auto& D:I->Catalog->Items){if(auto* T=Cast<UTextBlock>(WidgetTree->FindWidget(FName(*(TEXT("PF_FilterLabel_")+D.Id.ToString()))))){T->SetText(FText::FromString(FString(DraftAllowed.Contains(D.Id)?TEXT("[x] "):TEXT("[ ] "))+D.DisplayName.ToString()));}}}
}
void UPFStorageMenu::ApplySettings()
{CastChecked<APFSurvivalPlayerController>(GetOwningPlayer())->Building->ServerSetStorageFilter(bDraftRestricted,DraftAllowed);CancelSettings();}
void UPFStorageMenu::CancelSettings(){Pages->SetActiveWidgetIndex(0);SetKeyboardFocus();}
FReply UPFStorageMenu::NativeOnKeyDown(const FGeometry& G,const FKeyEvent& E)
{
    const auto K=E.GetKey();
    if(K==EKeys::E || K==EKeys::Tab || K==EKeys::Escape || K==EKeys::Gamepad_FaceButton_Right){if(!E.IsRepeat()){if(Pages->GetActiveWidgetIndex()==1){CancelSettings();}else{CloseMenu();}}return FReply::Handled();}
    if(K==EKeys::P || K==EKeys::Gamepad_Special_Right){CloseMenu();CastChecked<APFSurvivalPlayerController>(GetOwningPlayer())->SetPauseMenuOpen(true);return FReply::Handled();}
    return Super::NativeOnKeyDown(G,E);
}
