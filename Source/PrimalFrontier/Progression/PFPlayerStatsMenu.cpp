#include "Progression/PFPlayerStatsMenu.h"
#include "Progression/PFProgressionComponent.h"
#include "Progression/PFProgressionCatalog.h"
#include "Inventory/PFInventoryPlayerState.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Settings/PFGameUserSettings.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "UI/PFUITheme.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/Image.h"
#include "GameFramework/Pawn.h"

void UPFStatUpgradeButton::InitializeChoice(UPFPlayerStatsMenu* Menu,EPFPlayerStat Stat)
{OwnerMenu=Menu;Choice=Stat;InitIsFocusable(true);OnClicked.AddDynamic(this,&UPFStatUpgradeButton::Choose);}
void UPFStatUpgradeButton::Choose(){if(OwnerMenu){OwnerMenu->RequestUpgrade(Choice);}}
void UPFPlayerStatsMenu::NativeOnInitialized()
{
    Super::NativeOnInitialized();SetIsFocusable(true);
    auto* Panel=WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(),TEXT("PF_StatsPanel"));
    WidgetTree->RootWidget=Panel;
    Panel->SetBrush(FSlateRoundedBoxBrush(PFUITheme::Surface,12.f,PFUITheme::Muted,1.f));Panel->SetPadding(FMargin(20));
    auto* Rows=WidgetTree->ConstructWidget<UVerticalBox>();Panel->SetContent(Rows);
    auto Label=[&](FName Name){auto* T=WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(),Name);T->SetAutoWrapText(true);T->SetColorAndOpacity(PFUITheme::Text);return T;};
    Heading=Label(TEXT("PF_StatsHeading"));Heading->SetColorAndOpacity(PFUITheme::Accent);Rows->AddChildToVerticalBox(Heading)->SetPadding(FMargin(0,0,0,8));
    Budgets=Label(TEXT("PF_StatsBudgets"));Rows->AddChildToVerticalBox(Budgets)->SetPadding(FMargin(0,0,0,12));
    auto* Scroll=WidgetTree->ConstructWidget<UScrollBox>();ChoicesScroll=Scroll;Rows->AddChildToVerticalBox(Scroll)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    auto* Choices=WidgetTree->ConstructWidget<UVerticalBox>();Scroll->AddChild(Choices);
    for(int32 N=0;N<4;++N)
    {
        auto* B=WidgetTree->ConstructWidget<UPFStatUpgradeButton>(UPFStatUpgradeButton::StaticClass(),FName(*FString::Printf(TEXT("PF_Stats%d_Button"),N)));
        B->InitializeChoice(this,static_cast<EPFPlayerStat>(N));auto* T=Label(FName(*FString::Printf(TEXT("PF_Stats%d_Label"),N)));B->SetContent(T);
        const FName Ids[]={TEXT("Attack"),TEXT("Stamina"),TEXT("Weight"),TEXT("Craft")};PFUITheme::AddButtonIcon(WidgetTree,B,Ids[N]);
        Choices->AddChildToVerticalBox(B)->SetPadding(FMargin(0,0,0,10));Buttons.Add(B);Labels.Add(T);
    }
    Feedback=Label(TEXT("PF_StatsFeedback"));Feedback->SetColorAndOpacity(PFUITheme::Accent);Rows->AddChildToVerticalBox(Feedback)->SetPadding(FMargin(0,12,0,8));
    Help=Label(TEXT("PF_StatsHelp"));Help->SetColorAndOpacity(PFUITheme::Muted);
    Help->SetText(FText::FromString(TEXT("Earn XP by gathering and first crafts. Stat and technology points are separate.\nUp-Down / D-pad + Enter-A: upgrade. Esc / U / B / LB: Items. Tab / View: close inventory. World keeps running.")));
    Rows->AddChildToVerticalBox(Help)->SetPadding(FMargin(0,0,0,8));
    auto* Close=WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(),TEXT("PF_StatsClose"));auto* Caption=Label(TEXT("PF_StatsCloseLabel"));Caption->SetText(FText::FromString(TEXT("Back to inventory")));Close->SetContent(Caption);Close->SetStyle(PFUITheme::NavigationButton(false));Close->OnClicked.AddDynamic(this,&UPFPlayerStatsMenu::CloseMenu);Rows->AddChild(Close);
    RefreshMenu();
}
void UPFPlayerStatsMenu::RefreshMenu()
{
    auto* PC=Cast<APFSurvivalPlayerController>(GetOwningPlayer());auto* PS=PC?PC->GetPlayerState<APFInventoryPlayerState>():nullptr;
    auto* G=PS?PS->Progression.Get():nullptr;auto* V=PC && PC->GetPawn()?PC->GetPawn()->FindComponentByClass<UPFPlayerSurvivalComponent>():nullptr;
    if(!G){Heading->SetText(FText::FromString(TEXT("PLAYER UPGRADES - waiting for player state")));for(auto B:Buttons){B->SetIsEnabled(false);}return;}
    const auto& R=G->GetRecord();const int32 Level=G->GetLevel(),Points=G->GetAvailableStatPoints();
    const FString Next=Level<10?FString::Printf(TEXT("Next level: %d XP"),FPFProgressionTransactions::ExperienceForLevel(Level+1)):TEXT("Greybox level cap reached");
    Heading->SetText(FText::FromString(FString::Printf(TEXT("PLAYER UPGRADES | Level %d | XP %d\n%s"),Level,G->GetExperience(),*Next)));
    Budgets->SetText(FText::FromString(FString::Printf(TEXT("STAT POINTS: %d   |   TECHNOLOGY POINTS: %d\nAncient technology: planned, not yet available. Point pools never mix."),Points,G->GetAvailablePoints())));
    const auto* Balance=GetDefault<UPFProgressionCatalog>();
    const FString Values[]={FString::Printf(TEXT("Attack | Rank %d | Damage %.0f%% (+%.0f%% per point)"),R.Stats.Attack,100*FPFProgressionTransactions::AttackMultiplier(R),100*Balance->AttackBonusPerPoint),
        FString::Printf(TEXT("Stamina | Rank %d | Maximum %.0f (+%.0f per point)\nUpgrade increases capacity; recover stamina before climbing."),R.Stats.Stamina,FPFProgressionTransactions::StaminaLimit(R),Balance->StaminaPerPoint),
        FString::Printf(TEXT("Carry weight | Rank %d | Threshold %.0f kg (+%.0f kg per point)"),R.Stats.Weight,FPFProgressionTransactions::WeightLimit(R),Balance->CarryKgPerPoint),
        FString::Printf(TEXT("Crafting speed | Rank %d | Speed %.0f%% (+%.0f%% per point)\nApplies when starting your next job."),R.Stats.CraftingSpeed,100*FPFProgressionTransactions::CraftingMultiplier(R),100*Balance->CraftingBonusPerPoint)};
    const float Scale=UPFGameUserSettings::Get()?UPFGameUserSettings::Get()->Preferences.HUDScale:1;
    for(int32 N=0;N<4;++N)
    {
        Labels[N]->SetText(FText::FromString(Values[N]+TEXT("\nSpend 1 stat point")));auto Font=Labels[N]->GetFont();Font.Size=FMath::RoundToInt(16*Scale);Labels[N]->SetFont(Font);
        Buttons[N]->SetIsEnabled(Points>0 && V && !V->IsDead());Buttons[N]->SetStyle(PFUITheme::NavigationButton(N==Selected));
    }
    for(auto* T:{Heading.Get(),Budgets.Get(),Feedback.Get(),Help.Get()}){auto F=T->GetFont();F.Size=FMath::RoundToInt((T==Help?12:16)*Scale);T->SetFont(F);}
    Feedback->SetText(FText::FromString(G->GetStatFeedback().IsEmpty()?TEXT("Choose where to spend your earned points. No free XP or items."):G->GetStatFeedback()));
}
void UPFPlayerStatsMenu::NativeTick(const FGeometry& Geo,float Delta)
{Super::NativeTick(Geo,Delta);UntilRefresh-=Delta;if(UntilRefresh<=0){UntilRefresh=.2f;RefreshMenu();}}
void UPFPlayerStatsMenu::RequestUpgrade(EPFPlayerStat Stat)
{
    const int32 Id=int32(Stat);if(!Buttons.IsValidIndex(Id) || !Buttons[Id]->GetIsEnabled()){return;}Selected=Id;
    if(auto* PC=Cast<APFSurvivalPlayerController>(GetOwningPlayer())){PC->ServerUpgradeStat(Stat);}RefreshMenu();
}
void UPFPlayerStatsMenu::CloseMenu(){if(auto* PC=Cast<APFSurvivalPlayerController>(GetOwningPlayer())){PC->SetStatMenuOpen(false);}}
FReply UPFPlayerStatsMenu::NativeOnPreviewKeyDown(const FGeometry&,const FKeyEvent& E)
{
    const FKey K=E.GetKey();if(E.IsRepeat()){return FReply::Handled();}
    if(K==EKeys::U || K==EKeys::Escape || K==EKeys::Gamepad_FaceButton_Right || K==EKeys::Gamepad_LeftShoulder){CloseMenu();return FReply::Handled();}
    if(K==EKeys::Tab || K==EKeys::Gamepad_Special_Left){if(auto* PC=Cast<APFSurvivalPlayerController>(GetOwningPlayer())){PC->SetInventoryOpen(false);}return FReply::Handled();}
    if(K==EKeys::P || K==EKeys::Gamepad_Special_Right){if(auto* PC=Cast<APFSurvivalPlayerController>(GetOwningPlayer())){PC->SetInventoryOpen(false);PC->SetPauseMenuOpen(true);}return FReply::Handled();}
    if(K==EKeys::Down || K==EKeys::Gamepad_DPad_Down){Selected=(Selected+1)%4;RefreshMenu();ChoicesScroll->ScrollWidgetIntoView(Buttons[Selected],true,EDescendantScrollDestination::IntoView);return FReply::Handled();}
    if(K==EKeys::Up || K==EKeys::Gamepad_DPad_Up){Selected=(Selected+3)%4;RefreshMenu();ChoicesScroll->ScrollWidgetIntoView(Buttons[Selected],true,EDescendantScrollDestination::IntoView);return FReply::Handled();}
    if(K==EKeys::Enter || K==EKeys::Gamepad_FaceButton_Bottom){RequestUpgrade(static_cast<EPFPlayerStat>(Selected));return FReply::Handled();}
    return FReply::Unhandled();
}
