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
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/SizeBox.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/World.h"
#include "Settings/PFSettingsMenu.h"
#include "Survival/PFControlsMenu.h"

void UPFPauseMenu::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    SetIsFocusable(true);
    auto* Canvas=WidgetTree->ConstructWidget<UCanvasPanel>();
    WidgetTree->RootWidget=Canvas;

    // Full-screen dark shade behind the panel.
    auto* Shade=WidgetTree->ConstructWidget<UBorder>();
    Shade->SetBrushColor(FLinearColor(0,0,0,0.8f));
    auto* Back=Canvas->AddChildToCanvas(Shade);
    Back->SetAnchors(FAnchors(0,0,1,1));
    Back->SetOffsets(FMargin(0));

    // Centred panel holding a vertical list of rows.
    auto* Panel=WidgetTree->ConstructWidget<UBorder>();
    Panel->SetBrushColor(FLinearColor(0.04f,0.04f,0.04f,1));
    Panel->SetPadding(FMargin(24));
    auto* Sizing=WidgetTree->ConstructWidget<USizeBox>();Sizing->SetWidthOverride(620);Sizing->SetContent(Panel);
    auto* Placement=Canvas->AddChildToCanvas(Sizing);
    Placement->SetAnchors(FAnchors(0.5f,0.5f));
    Placement->SetAlignment(FVector2D(0.5f,0.5f));
    Placement->SetAutoSize(true);
    auto* Rows=WidgetTree->ConstructWidget<UVerticalBox>();
    Panel->SetContent(Rows);

    // Helper: wrapped text block at a given font size.
    auto Text=[&](const TCHAR* Value,int32 Size)
    {
        auto* T=WidgetTree->ConstructWidget<UTextBlock>();
        T->SetText(FText::FromString(Value));
        auto Font=T->GetFont();
        Font.Size=Size;
        T->SetFont(Font);
        T->SetAutoWrapText(true);
        return T;
    };
    Rows->AddChild(Text(TEXT("PRIMAL FRONTIER"),34));
    Description=Text(TEXT(""),24);
    Rows->AddChild(Description);
    ReconnectFeedback=WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(),TEXT("PF_ReconnectFeedback"));
    auto FeedbackFont=ReconnectFeedback->GetFont();FeedbackFont.Size=22;ReconnectFeedback->SetFont(FeedbackFont);
    ReconnectFeedback->SetAutoWrapText(true);
    Rows->AddChild(ReconnectFeedback);
    ResumeButton=WidgetTree->ConstructWidget<UButton>();
    ResumeButton->SetContent(Text(TEXT("Resume"),30));
    Rows->AddChild(ResumeButton);
    ResumeButton->OnClicked.AddDynamic(this,&UPFPauseMenu::Resume);
    SettingsButton=WidgetTree->ConstructWidget<UButton>();
    SettingsButton->SetContent(Text(TEXT("Settings"),30));
    Rows->AddChild(SettingsButton);
    SettingsButton->OnClicked.AddDynamic(this,&UPFPauseMenu::Settings);
    ControlsButton=WidgetTree->ConstructWidget<UButton>();
    ControlsButton->SetContent(Text(TEXT("Controls & help"),30));
    Rows->AddChild(ControlsButton);
    ControlsButton->OnClicked.AddDynamic(this,&UPFPauseMenu::Controls);
    QuitButton=WidgetTree->ConstructWidget<UButton>();
    QuitLabel=Text(TEXT("End session"),30);
    QuitLabel->SetAutoWrapText(false);
    QuitButton->SetContent(QuitLabel);
    Rows->AddChild(QuitButton);
    QuitButton->OnClicked.AddDynamic(this,&UPFPauseMenu::Quit);
    Rows->AddChild(Text(TEXT("P / Esc / Menu / B: resume. D-pad: select. A: confirm. In Editor Play, Esc may stop PIE."),22));
}

void UPFPauseMenu::Refresh()
{
    bConfirmQuit=false;
    CloseChildMenus();
    SelectedButton=0;
    UpdateSelection();
    QuitLabel->SetText(FText::FromString(TEXT("End session")));
    Description->SetText(FText::FromString(GetWorld()->GetNetMode()==NM_Standalone?TEXT("Paused\nSave explicitly before ending the session.\n"):TEXT("Multiplayer continues while this menu is open.\nSave explicitly; ending does not autosave.\n")));
    UpdateReconnectFeedback();
}

void UPFPauseMenu::UpdateReconnectFeedback()
{
    if(!ReconnectFeedback){return;}
    const auto* PC=Cast<APFSurvivalPlayerController>(GetOwningPlayer());
    const FText Status=PC?PC->GetLocalReconnectStatusText():FText::GetEmpty();
    ReconnectFeedback->SetText(Status);
    ReconnectFeedback->SetVisibility(Status.IsEmpty()?ESlateVisibility::Collapsed:ESlateVisibility::HitTestInvisible);
    ReconnectFeedback->SetColorAndOpacity(PC && PC->GetLocalReconnectStatus()==EPFLocalReconnectStatus::Failed?FLinearColor(1,0.8f,0.3f):FLinearColor::White);
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
    UKismetSystemLibrary::QuitGame(this,GetOwningPlayer(),EQuitPreference::Quit,false);
}

void UPFPauseMenu::UpdateSelection()
{
    ResumeButton->SetBackgroundColor(SelectedButton==0?FLinearColor(0.2f,0.6f,0.9f):FLinearColor::Gray);
    SettingsButton->SetBackgroundColor(SelectedButton==1?FLinearColor(0.2f,0.6f,0.9f):FLinearColor::Gray);
    ControlsButton->SetBackgroundColor(SelectedButton==2?FLinearColor(0.2f,0.6f,0.9f):FLinearColor::Gray);
    QuitButton->SetBackgroundColor(SelectedButton==3?FLinearColor(0.2f,0.6f,0.9f):FLinearColor::Gray);
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
        SelectedButton=(SelectedButton+((K==EKeys::Gamepad_DPad_Down || K==EKeys::Down)?1:3))%4;
        UpdateSelection();
        return FReply::Handled();
    }
    // A activates the highlighted button (ignore key-repeat so a held A can't double-confirm).
    if(K==EKeys::Gamepad_FaceButton_Bottom || K==EKeys::Enter)
    {
        if(!bRepeat){if(SelectedButton==3){Quit();}else if(SelectedButton==2){Controls();}else if(SelectedButton==1){Settings();}else{Resume();}}
        return FReply::Handled();
    }
    return FReply::Handled();
}
FReply UPFPauseMenu::NativeOnPreviewKeyDown(const FGeometry&,const FKeyEvent& E){return HandleNavigation(E.GetKey(),E.IsRepeat());}
