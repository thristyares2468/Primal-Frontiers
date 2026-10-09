// PFPauseMenu.cpp
//
// See PFPauseMenu.h. The owning APFSurvivalPlayerController opens/closes the menu,
// handles world pausing and switches input modes; this widget only draws and
// forwards Resume/Quit.

#include "Survival/PFPauseMenu.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/ButtonSlot.h"
#include "Components/SizeBox.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/World.h"
#include "Settings/PFSettingsMenu.h"
#include "Survival/PFControlsMenu.h"
#include "UI/PFUITheme.h"
#include "Persistence/PFSessionGameInstance.h"

void UPFPauseMenu::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    SetIsFocusable(true);
    auto* Canvas=WidgetTree->ConstructWidget<UCanvasPanel>();
    WidgetTree->RootWidget=Canvas;

    // Full-screen dark shade behind the panel.
    auto* Shade=WidgetTree->ConstructWidget<UBorder>();
    Shade->SetBrushColor(FLinearColor(0.005f,0.012f,0.01f,0.72f));
    auto* Back=Canvas->AddChildToCanvas(Shade);
    Back->SetAnchors(FAnchors(0,0,1,1));
    Back->SetOffsets(FMargin(0));

    // Two compact columns: actions stay separate from session/save information.
    // Rounded Slate brushes use no textures, imported fonts or expensive effects.
    auto* Panel=WidgetTree->ConstructWidget<UBorder>();
    Panel->SetBrush(FSlateRoundedBoxBrush(PFUITheme::Surface,12.f,FLinearColor(0.13f,0.22f,0.17f,1),1.f));
    Panel->SetPadding(FMargin(32));
    auto* Sizing=WidgetTree->ConstructWidget<USizeBox>();Sizing->SetWidthOverride(1000);Sizing->SetContent(Panel);
    auto* Placement=Canvas->AddChildToCanvas(Sizing);
    Placement->SetAnchors(FAnchors(0.5f,0.5f));
    Placement->SetAlignment(FVector2D(0.5f,0.5f));
    Placement->SetAutoSize(true);
    auto* Columns=WidgetTree->ConstructWidget<UHorizontalBox>();Panel->SetContent(Columns);
    auto* Rows=WidgetTree->ConstructWidget<UVerticalBox>();
    auto* Actions=Columns->AddChildToHorizontalBox(Rows);Actions->SetSize(FSlateChildSize(ESlateSizeRule::Fill));Actions->SetPadding(FMargin(0,0,28,0));
    auto* Information=WidgetTree->ConstructWidget<UBorder>();
    Information->SetBrush(FSlateRoundedBoxBrush(PFUITheme::Inset,8.f));Information->SetPadding(FMargin(24));
    auto* InfoSlot=Columns->AddChildToHorizontalBox(Information);InfoSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    auto* Details=WidgetTree->ConstructWidget<UVerticalBox>();Information->SetContent(Details);

    // Helper: wrapped text block at a given font size.
    auto Text=[&](const TCHAR* Value,int32 Size)
    {
        auto* T=WidgetTree->ConstructWidget<UTextBlock>();
        T->SetText(FText::FromString(Value));
        auto Font=T->GetFont();
        Font.Size=Size;
        T->SetFont(Font);
        T->SetAutoWrapText(true);
        T->SetColorAndOpacity(PFUITheme::Text);
        return T;
    };
    auto* Brand=Text(TEXT("PRIMAL FRONTIER"),18);Brand->SetColorAndOpacity(PFUITheme::Accent);
    Rows->AddChildToVerticalBox(Brand)->SetPadding(FMargin(0,0,0,8));
    Rows->AddChildToVerticalBox(Text(TEXT("Pause"),42))->SetPadding(FMargin(0,0,0,10));
    Description=Text(TEXT(""),22);Description->SetColorAndOpacity(PFUITheme::Muted);
    Rows->AddChildToVerticalBox(Description)->SetPadding(FMargin(0,0,0,20));
    auto* Session=Text(TEXT("SESSION"),18);Session->SetColorAndOpacity(PFUITheme::Accent);
    Details->AddChildToVerticalBox(Session)->SetPadding(FMargin(0,0,0,16));
    ServerSetupFeedback=WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(),TEXT("PF_ServerSetupFeedback"));
    auto SetupFont=ServerSetupFeedback->GetFont();SetupFont.Size=22;ServerSetupFeedback->SetFont(SetupFont);
    ServerSetupFeedback->SetAutoWrapText(true);ServerSetupFeedback->SetColorAndOpacity(PFUITheme::Text);
    Details->AddChildToVerticalBox(ServerSetupFeedback)->SetPadding(FMargin(0,0,0,20));
    ReconnectFeedback=WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(),TEXT("PF_ReconnectFeedback"));
    auto FeedbackFont=ReconnectFeedback->GetFont();FeedbackFont.Size=22;ReconnectFeedback->SetFont(FeedbackFont);
    ReconnectFeedback->SetAutoWrapText(true);
    Details->AddChildToVerticalBox(ReconnectFeedback)->SetPadding(FMargin(0,0,0,20));
    auto* SaveTitle=Text(TEXT("BEFORE YOU LEAVE"),18);SaveTitle->SetColorAndOpacity(PFUITheme::Warning);
    Details->AddChildToVerticalBox(SaveTitle)->SetPadding(FMargin(0,0,0,8));
    auto* SaveNote=Text(TEXT("Save explicitly before ending the session. Ending does not autosave."),22);
    SaveNote->SetColorAndOpacity(PFUITheme::Muted);Details->AddChild(SaveNote);
    auto Button=[&](const TCHAR* Caption)
    {
        auto* B=WidgetTree->ConstructWidget<UButton>();auto* Label=Text(Caption,26);Label->SetAutoWrapText(false);B->SetContent(Label);
        auto* ContentSlot=CastChecked<UButtonSlot>(B->GetContent()->Slot);
        ContentSlot->SetHorizontalAlignment(HAlign_Left);ContentSlot->SetPadding(FMargin(18,12));
        Rows->AddChildToVerticalBox(B)->SetPadding(FMargin(0,0,0,8));return B;
    };
    ResumeButton=Button(TEXT("Resume"));
    ResumeButton->OnClicked.AddDynamic(this,&UPFPauseMenu::Resume);
    SettingsButton=Button(TEXT("Settings"));
    SettingsButton->OnClicked.AddDynamic(this,&UPFPauseMenu::Settings);
    ControlsButton=Button(TEXT("Controls & help"));
    ControlsButton->OnClicked.AddDynamic(this,&UPFPauseMenu::Controls);
    QuitButton=Button(TEXT("End session"));
    QuitLabel=CastChecked<UTextBlock>(QuitButton->GetContent());
    QuitLabel->SetAutoWrapText(false);
    QuitButton->SetContent(QuitLabel);
    QuitButton->OnClicked.AddDynamic(this,&UPFPauseMenu::Quit);
    SaveButton=Button(TEXT("Save world"));SaveButton->OnClicked.AddDynamic(this,&UPFPauseMenu::Save);
    SaveFeedback=Text(TEXT(""),22);SaveFeedback->SetColorAndOpacity(PFUITheme::Accent);
    SaveFeedback->SetWrappingPolicy(ETextWrappingPolicy::AllowPerCharacterWrapping);
    Details->AddChildToVerticalBox(SaveFeedback)->SetPadding(FMargin(0,20,0,0));
    auto* Hint=Text(TEXT("P / Menu: resume\nArrows / D-pad: select | Enter / A: confirm\nIn Editor Play, Esc may end the session."),16);
    Hint->SetColorAndOpacity(PFUITheme::Muted);Rows->AddChildToVerticalBox(Hint)->SetPadding(FMargin(0,12,0,0));
}

void UPFPauseMenu::Refresh()
{
    bConfirmQuit=false;
    CloseChildMenus();
    SelectedButton=0;
    UpdateSelection();
    QuitLabel->SetText(FText::FromString(TEXT("End session")));
    Description->SetText(FText::FromString(GetWorld()->GetNetMode()==NM_Standalone?TEXT("Your world is paused."):TEXT("Multiplayer keeps running. Find safety before opening menus.")));
    UpdatePersistenceFeedback();
    if(const auto* PC=Cast<APFSurvivalPlayerController>(GetOwningPlayer()))
    {
        SaveButton->SetIsEnabled(PC->HasAuthority());
        SaveFeedback->SetText(FText::FromString(PC->HasAuthority()?PC->GetWorldSaveFeedback():TEXT("Only the host can save this world.")));
    }
}

void UPFPauseMenu::UpdatePersistenceFeedback()
{
    if(!ReconnectFeedback){return;}
    const auto* PC=Cast<APFSurvivalPlayerController>(GetOwningPlayer());
    if(ServerSetupFeedback){ServerSetupFeedback->SetText(PC?PC->GetPlayerSetupStatusText():FText::GetEmpty());}
    const FText Status=PC?PC->GetLocalReconnectStatusText():FText::GetEmpty();
    ReconnectFeedback->SetText(Status);
    ReconnectFeedback->SetVisibility(Status.IsEmpty()?ESlateVisibility::Collapsed:ESlateVisibility::HitTestInvisible);
    ReconnectFeedback->SetColorAndOpacity(PC && PC->GetLocalReconnectStatus()==EPFLocalReconnectStatus::Failed?PFUITheme::Warning:PFUITheme::Muted);
}

void UPFPauseMenu::Resume(){if(auto* PC=Cast<APFSurvivalPlayerController>(GetOwningPlayer())){PC->SetPauseMenuOpen(false);}}

void UPFPauseMenu::Quit()
{
    // Explicit save/load exists; ending does not automatically publish a world save.
    if(!bConfirmQuit)
    {
        bConfirmQuit=true;
        QuitLabel->SetText(FText::FromString(TEXT("Confirm end session")));
        return;
    }
    Resume();
    if(auto* Session=GetGameInstance<UPFSessionGameInstance>()){Session->ReturnToWorldMenu();}
    else{UKismetSystemLibrary::QuitGame(this,GetOwningPlayer(),EQuitPreference::Quit,false);}
}

void UPFPauseMenu::Save()
{
    if(auto* PC=Cast<APFSurvivalPlayerController>(GetOwningPlayer()))
    {
        const bool bSaved=PC->SaveWorldFromMenu();SaveFeedback->SetText(FText::FromString(PC->GetWorldSaveFeedback()));
        SaveFeedback->SetColorAndOpacity(bSaved?PFUITheme::Accent:PFUITheme::Warning);
    }
}

void UPFPauseMenu::UpdateSelection()
{
    ResumeButton->SetStyle(PFUITheme::NavigationButton(SelectedButton==0));
    SettingsButton->SetStyle(PFUITheme::NavigationButton(SelectedButton==1));
    ControlsButton->SetStyle(PFUITheme::NavigationButton(SelectedButton==2));
    QuitButton->SetStyle(PFUITheme::NavigationButton(SelectedButton==3,true));
    SaveButton->SetStyle(PFUITheme::NavigationButton(SelectedButton==4));
}

void UPFPauseMenu::Settings()
{
    CloseChildMenus();
    SettingsMenu=CreateWidget<UPFSettingsMenu>(GetOwningPlayer());
    SettingsMenu->ReturnFocus=this;
    SettingsMenu->AddToPlayerScreen(110);
    SettingsMenu->SetKeyboardFocus();
}

void UPFPauseMenu::Controls()
{
    CloseChildMenus();
    ControlsMenu=CreateWidget<UPFControlsMenu>(GetOwningPlayer());
    if(!ControlsMenu){return;}
    ControlsMenu->ReturnFocus=this;
    ControlsMenu->AddToPlayerScreen(110);ControlsMenu->SetKeyboardFocus();
    UE_LOG(LogTemp,Display,TEXT("[PrimalUI] Read-only controls opened; bindings supplied by local controller."));
}
void UPFPauseMenu::CloseChildMenus()
{
    if(ControlsMenu){ControlsMenu->RemoveFromParent();ControlsMenu=nullptr;}
    if(SettingsMenu){SettingsMenu->RemoveFromParent();SettingsMenu=nullptr;}
}
void UPFPauseMenu::NativeDestruct(){CloseChildMenus();Super::NativeDestruct();}

FReply UPFPauseMenu::HandleNavigation(FKey K,bool bRepeat)
{
    // Any "back/menu" key resumes.
    if(K==EKeys::P || K==EKeys::Escape || K==EKeys::Gamepad_Special_Right || K==EKeys::Gamepad_FaceButton_Right)
    {
        if(!bRepeat){Resume();}
        return FReply::Handled();
    }
    // D-pad moves the highlight between all four buttons.
    if(K==EKeys::Gamepad_DPad_Up || K==EKeys::Gamepad_DPad_Down || K==EKeys::Up || K==EKeys::Down)
    {
        SelectedButton=(SelectedButton+((K==EKeys::Gamepad_DPad_Down || K==EKeys::Down)?1:4))%5;
        UpdateSelection();
        return FReply::Handled();
    }
    // A activates the highlighted button (ignore key-repeat so a held A can't double-confirm).
    if(K==EKeys::Gamepad_FaceButton_Bottom || K==EKeys::Enter)
    {
        if(!bRepeat){if(SelectedButton==4){Save();}else if(SelectedButton==3){Quit();}else if(SelectedButton==2){Controls();}else if(SelectedButton==1){Settings();}else{Resume();}}
        return FReply::Handled();
    }
    return FReply::Handled();
}
FReply UPFPauseMenu::NativeOnPreviewKeyDown(const FGeometry&,const FKeyEvent& E){return HandleNavigation(E.GetKey(),E.IsRepeat());}
