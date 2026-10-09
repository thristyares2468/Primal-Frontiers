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
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/ScrollBox.h"
#include "Components/SizeBox.h"
#include "UI/PFUITheme.h"
#include "UI/PFRecipeDetails.h"
#include "UI/PFItemPicture.h"
#include "Settings/PFGameUserSettings.h"

UPFRecipeChoiceButton::UPFRecipeChoiceButton(){InitIsFocusable(true);}
void UPFRecipeChoiceButton::InitializeChoice(UPFCraftingHUD* Menu,FName Id)
{OwnerMenu=Menu;RecipeId=Id;OnClicked.AddDynamic(this,&UPFRecipeChoiceButton::Choose);}
void UPFRecipeChoiceButton::Choose(){if(OwnerMenu){OwnerMenu->SelectRecipe(RecipeId);}}

void UPFCraftingHUD::NativeOnInitialized()
{
    Super::NativeOnInitialized();SetIsFocusable(true);
    auto* Canvas=WidgetTree->ConstructWidget<UCanvasPanel>();WidgetTree->RootWidget=Canvas;
    auto* Shade=WidgetTree->ConstructWidget<UBorder>();Shade->SetBrushColor(FLinearColor(0.005f,0.012f,0.01f,0.78f));
    auto* Back=Canvas->AddChildToCanvas(Shade);Back->SetAnchors(FAnchors(0,0,1,1));Back->SetOffsets(FMargin(0));
    Panel=WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(),TEXT("PF_CraftingPanel"));
    Panel->SetBrush(FSlateRoundedBoxBrush(PFUITheme::Surface,12.f,PFUITheme::Muted,1.f));Panel->SetPadding(FMargin(20));Panel->SetClipping(EWidgetClipping::ClipToBounds);
    auto* Placement=Canvas->AddChildToCanvas(Panel);Placement->SetAnchors(FAnchors(0.1f,0.05f,0.9f,0.95f));Placement->SetOffsets(FMargin(0));
    auto* Rows=WidgetTree->ConstructWidget<UVerticalBox>();Panel->SetContent(Rows);
    auto Label=[&](FName Name,FLinearColor Color){auto* T=WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(),Name);T->SetAutoWrapText(true);T->SetColorAndOpacity(Color);return T;};
    Heading=Label(TEXT("PF_CraftingPanel_Heading"),PFUITheme::Accent);Heading->SetText(FText::FromString(TEXT("CRAFTING")));
    Rows->AddChildToVerticalBox(Heading)->SetPadding(FMargin(0,0,0,10));
    auto* Columns=WidgetTree->ConstructWidget<UHorizontalBox>();Rows->AddChildToVerticalBox(Columns)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    FSlateChildSize ListWidth(ESlateSizeRule::Fill);ListWidth.Value=0.42f;
    auto* Browser=WidgetTree->ConstructWidget<UScrollBox>();auto* BrowserSlot=Columns->AddChildToHorizontalBox(Browser);BrowserSlot->SetSize(ListWidth);BrowserSlot->SetPadding(FMargin(0,0,16,0));
    RecipeList=WidgetTree->ConstructWidget<UVerticalBox>();Browser->AddChild(RecipeList);
    auto* DetailPanel=WidgetTree->ConstructWidget<UBorder>();DetailPanel->SetBrush(FSlateRoundedBoxBrush(PFUITheme::Inset,8.f));DetailPanel->SetPadding(FMargin(14));
    FSlateChildSize DetailWidth(ESlateSizeRule::Fill);DetailWidth.Value=0.58f;
    Columns->AddChildToHorizontalBox(DetailPanel)->SetSize(DetailWidth);
    auto* DetailScroll=WidgetTree->ConstructWidget<UScrollBox>();DetailPanel->SetContent(DetailScroll);
    auto* Details=WidgetTree->ConstructWidget<UVerticalBox>();DetailScroll->AddChild(Details);
    auto* PictureSize=WidgetTree->ConstructWidget<USizeBox>();PictureSize->SetWidthOverride(108);PictureSize->SetHeightOverride(108);
    Picture=CreateWidget<UPFItemPicture>(GetOwningPlayer(),UPFItemPicture::StaticClass(),TEXT("PF_SelectedItemPicture"));PictureSize->SetContent(Picture);Details->AddChildToVerticalBox(PictureSize)->SetHorizontalAlignment(HAlign_Center);
    DetailTitle=Label(TEXT("PF_CraftingDetail_Title"),PFUITheme::Text);Details->AddChildToVerticalBox(DetailTitle)->SetPadding(FMargin(0,8,0,8));
    DetailBody=Label(TEXT("PF_CraftingPanel_Body"),PFUITheme::Text);Details->AddChildToVerticalBox(DetailBody)->SetPadding(FMargin(0,0,0,12));
    ItemStats=Label(TEXT("PF_CraftingDetail_Stats"),PFUITheme::Muted);Details->AddChildToVerticalBox(ItemStats);
    Result=Label(TEXT("PF_CraftingPanel_Result"),PFUITheme::Warning);Rows->AddChildToVerticalBox(Result)->SetPadding(FMargin(0,10,0,8));
    auto* Actions=WidgetTree->ConstructWidget<UHorizontalBox>();Rows->AddChild(Actions);
    auto Button=[&](FName Name,const TCHAR* Caption)
    {
        auto* B=WidgetTree->ConstructWidget<UPFRecipeChoiceButton>(UPFRecipeChoiceButton::StaticClass(),Name);B->SetStyle(PFUITheme::NavigationButton(false));
        auto* T=Label(NAME_None,PFUITheme::Text);T->SetAutoWrapText(false);T->SetText(FText::FromString(Caption));B->SetContent(T);ButtonLabels.Add(T);
        auto* ActionSlot=Actions->AddChildToHorizontalBox(B);ActionSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));ActionSlot->SetPadding(FMargin(0,0,8,0));return B;
    };
    CraftButton=Button(TEXT("PF_CraftSelected"),TEXT("Craft selected"));CraftButton->OnClicked.AddDynamic(this,&UPFCraftingHUD::CraftSelected);
    CancelButton=Button(TEXT("PF_CancelCraft"),TEXT("Cancel job"));CancelButton->OnClicked.AddDynamic(this,&UPFCraftingHUD::CancelCraft);
    auto* Close=Button(TEXT("PF_CloseCraft"),TEXT("Close"));Close->OnClicked.AddDynamic(this,&UPFCraftingHUD::CloseMenu);
    Hints=Label(TEXT("PF_CraftingHints"),PFUITheme::Muted);Rows->AddChildToVerticalBox(Hints)->SetPadding(FMargin(0,8,0,0));
    SetVisibility(ESlateVisibility::Collapsed);
}
void UPFCraftingHUD::RebuildRecipes(const TArray<FName>& Ids)
{
    RecipeList->ClearChildren();RecipeButtons.Reset();RecipeTitles.Reset();RecipeBodies.Reset();RecipePictures.Reset();RecipeIds=Ids;
    for(int32 N=0;N<Ids.Num();++N)
    {
        auto* Button=WidgetTree->ConstructWidget<UPFRecipeChoiceButton>(UPFRecipeChoiceButton::StaticClass(),FName(*FString::Printf(TEXT("PF_Recipe%d_Button"),N)));Button->InitializeChoice(this,Ids[N]);
        auto* Row=WidgetTree->ConstructWidget<UHorizontalBox>();Button->SetContent(Row);
        auto* Sizing=WidgetTree->ConstructWidget<USizeBox>();Sizing->SetWidthOverride(64);Sizing->SetHeightOverride(64);
        auto* Icon=CreateWidget<UPFItemPicture>(GetOwningPlayer());Sizing->SetContent(Icon);RecipePictures.Add(Icon);
        Row->AddChildToHorizontalBox(Sizing)->SetVerticalAlignment(VAlign_Center);
        auto* Info=WidgetTree->ConstructWidget<UVerticalBox>();auto* InfoSlot=Row->AddChildToHorizontalBox(Info);InfoSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));InfoSlot->SetPadding(FMargin(12,0,0,0));InfoSlot->SetVerticalAlignment(VAlign_Center);
        auto* Title=WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(),FName(*FString::Printf(TEXT("PF_Recipe%d_Title"),N)));Title->SetAutoWrapText(true);Title->SetColorAndOpacity(PFUITheme::Text);Info->AddChild(Title);
        auto* Body=WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(),FName(*FString::Printf(TEXT("PF_Recipe%d_Body"),N)));Body->SetAutoWrapText(true);Body->SetColorAndOpacity(PFUITheme::Muted);Info->AddChild(Body);
        RecipeTitles.Add(Title);RecipeBodies.Add(Body);RecipeButtons.Add(Button);RecipeList->AddChildToVerticalBox(Button)->SetPadding(FMargin(0,0,0,8));
    }
    if(!bInitialChoiceMade && !RecipeIds.IsEmpty()){SelectedRecipe=RecipeIds[0];bInitialChoiceMade=true;}
    else if(!RecipeIds.Contains(SelectedRecipe)){SelectedRecipe=NAME_None;}
}
void UPFCraftingHUD::NativeTick(const FGeometry& G,float Delta)
{Super::NativeTick(G,Delta);Refresh+=Delta;if(Refresh>=0.1f){RefreshMenu();}}
void UPFCraftingHUD::RefreshMenu()
{
    Refresh=0;
    const auto* PC=Cast<APFSurvivalPlayerController>(GetOwningPlayer());const auto* C=PC?PC->GetCrafting():nullptr;const auto* I=PC?PC->GetInventory():nullptr;
    TArray<FName> Ids;
    if(C && C->Catalog && I){for(const auto& D:C->Catalog->Recipes){if(C->Catalog->Recipe(D.Id,I->Catalog)){Ids.Add(D.Id);}}}
    if(Ids!=RecipeIds){RebuildRecipes(Ids);}
    const auto* Settings=UPFGameUserSettings::Get();const float Requested=Settings?Settings->Preferences.HUDScale:1;
    const float Scale=FMath::IsFinite(Requested)?FMath::Clamp(Requested,0.75f,1.5f):1;
    auto Font=[&](UTextBlock* T,int32 Size){auto F=T->GetFont();F.Size=FMath::RoundToInt(Size*Scale);if(T->GetFont().Size!=F.Size){T->SetFont(F);}};
    Font(Heading,20);Font(DetailTitle,18);Font(DetailBody,16);Font(ItemStats,14);Font(Result,14);Font(Hints,12);for(auto T:ButtonLabels){Font(T.Get(),16);}
    for(int32 N=0;N<RecipeIds.Num();++N)
    {
        const auto* D=C->Catalog->Recipe(RecipeIds[N],I->Catalog);const auto* Output=I->Definition(D->Output);
        RecipeTitles[N]->SetText(D->DisplayName);RecipeBodies[N]->SetText(FText::FromString(FString::Printf(TEXT("Makes %d %s | %.1fs"),D->OutputQuantity,*Output->DisplayName.ToString(),D->Duration)));
        Font(RecipeTitles[N],16);Font(RecipeBodies[N],12);RecipeButtons[N]->SetStyle(PFUITheme::NavigationButton(SelectedRecipe==D->Id));RecipePictures[N]->SetItem(Output);
    }
    const auto* D=C && C->Catalog && I?C->Catalog->Recipe(SelectedRecipe,I->Catalog):nullptr;
    const auto View=PFRecipeDetails::Describe(D,I,UPFInventoryComponent::ServerTime(GetWorld()),C && !C->ActiveRecipe.IsNone());
    DetailTitle->SetText(View.Title);DetailBody->SetText(View.Body);
    const auto* Output=D?I->Definition(D->Output):nullptr;Picture->SetItem(Output);
    FString Stats=TEXT("No valid item selected.");
    if(Output)
    {
        const FString Category=Output->Category.ToString().Replace(TEXT("Item.Category."),TEXT(""));
        Stats=FString::Printf(TEXT("%s | %.2f kg each | Stack %d\n"),*Category,Output->Weight,Output->StackLimit);
        if(Output->ShelfLifeSeconds>0){Stats+=FString::Printf(TEXT("Shelf life when made: %.0f min\n"),Output->ShelfLifeSeconds/60);}
        else{Stats+=TEXT("Does not expire\n");}
        if(Output->FoodRecovery>0 || Output->WaterRecovery>0){Stats+=FString::Printf(TEXT("Per portion: +%.0f food | +%.0f water\n"),Output->FoodRecovery,Output->WaterRecovery);}
        Stats+=TEXT("Greybox item preview");
    }
    ItemStats->SetText(FText::FromString(Stats));
    FString Status=C?(C->ActiveRecipe.IsNone()?TEXT("Job: idle"):FString::Printf(TEXT("Job: %.1fs remaining"),FMath::Max(0.0,C->FinishAt-UPFInventoryComponent::ServerTime(GetWorld())))):TEXT("Waiting for server crafting state");
    if(C){Status+=TEXT(" | ")+(C->Feedback.IsEmpty()?FString(TEXT("No server result yet")):C->Feedback);}
    if(PC && PC->GetInventoryMessage().StartsWith(TEXT("Craft "))){Status+=TEXT("\nLast request: ")+PC->GetInventoryMessage();}
    Result->SetText(FText::FromString(Status));CraftButton->SetIsEnabled(D!=nullptr);CancelButton->SetIsEnabled(C && !C->ActiveRecipe.IsNone());
    Hints->SetText(FText::FromString(!Settings || Settings->Preferences.bControlHints?TEXT("Arrows / D-pad: recipe | Enter / A / X: craft | R / D-left: cancel | C / Y / B / Esc: close\n1/2/3: quick craft | P / Menu: pause. The world keeps running."):TEXT("The world keeps running.")));
}
void UPFCraftingHUD::SelectRecipe(FName Id)
{RefreshMenu();if(RecipeIds.Contains(Id)){SelectedRecipe=Id;RefreshMenu();if(auto* Scroll=Cast<UScrollBox>(RecipeList->GetParent())){Scroll->ScrollWidgetIntoView(RecipeButtons[RecipeIds.IndexOfByKey(Id)],false);}}}
void UPFCraftingHUD::MoveSelection(int32 Direction)
{RefreshMenu();if(!RecipeIds.IsEmpty()){const int32 Index=RecipeIds.IndexOfByKey(SelectedRecipe);SelectRecipe(RecipeIds[(FMath::Max(0,Index)+Direction+RecipeIds.Num())%RecipeIds.Num()]);}}
void UPFCraftingHUD::CraftSelected()
{if(auto* PC=Cast<APFSurvivalPlayerController>(GetOwningPlayer())){if(PC->IsCraftingOpen() && !PC->IsPauseMenuOpen()){RefreshMenu();if(!SelectedRecipe.IsNone()){PC->ServerCraftAction(SelectedRecipe,false);}}}}
void UPFCraftingHUD::CancelCraft()
{if(auto* PC=Cast<APFSurvivalPlayerController>(GetOwningPlayer())){if(PC->IsCraftingOpen() && !PC->IsPauseMenuOpen()){PC->ServerCraftAction(NAME_None,true);}}}
void UPFCraftingHUD::CloseMenu(){if(auto* PC=Cast<APFSurvivalPlayerController>(GetOwningPlayer())){PC->SetCraftingMenuOpen(false);}}
FReply UPFCraftingHUD::NativeOnPreviewKeyDown(const FGeometry&,const FKeyEvent& E)
{
    const FKey K=E.GetKey();auto* PC=Cast<APFSurvivalPlayerController>(GetOwningPlayer());
    if(!PC || !PC->IsCraftingOpen()){return FReply::Unhandled();}
    // UI-only input cannot reach gameplay. Let Slate navigate/activate focused buttons.
    if(K==EKeys::Tab || K==EKeys::SpaceBar){return FReply::Unhandled();}
    if(K==EKeys::Down || K==EKeys::Up || K==EKeys::Gamepad_DPad_Down || K==EKeys::Gamepad_DPad_Up){MoveSelection(K==EKeys::Down || K==EKeys::Gamepad_DPad_Down?1:-1);}
    else if(!E.IsRepeat())
    {
        if(K==EKeys::C || K==EKeys::Escape || K==EKeys::Gamepad_FaceButton_Top || K==EKeys::Gamepad_FaceButton_Right){CloseMenu();}
        else if(K==EKeys::P || K==EKeys::Gamepad_Special_Right){PC->SetPauseMenuOpen(true);}
        else if(K==EKeys::R || K==EKeys::Gamepad_DPad_Left){CancelCraft();}
        else if(K==EKeys::Enter || K==EKeys::Gamepad_FaceButton_Bottom || K==EKeys::Gamepad_FaceButton_Left){CraftSelected();}
        else if(K==EKeys::One || K==EKeys::Two || K==EKeys::Three)
        {
            const FName Id=K==EKeys::One?TEXT("Recipe_Tool"):K==EKeys::Two?TEXT("Recipe_Cook"):TEXT("Recipe_Dry");
            RefreshMenu();if(RecipeIds.Contains(Id)){SelectRecipe(Id);CraftSelected();}
            else{Result->SetText(FText::FromString(TEXT("Quick recipe unavailable; no request sent.")));}
        }
    }
    // Consume unrelated gameplay keys while this modal menu owns focus.
    return FReply::Handled();
}
FReply UPFCraftingHUD::NativeOnMouseButtonDown(const FGeometry&,const FPointerEvent&)
{return FReply::Handled().SetUserFocus(TakeWidget());}
