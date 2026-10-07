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
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/World.h"
#include "Settings/PFSettingsMenu.h"

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

    // Centred 620x460 panel holding a vertical list of rows.
    auto* Panel=WidgetTree->ConstructWidget<UBorder>();
    Panel->SetBrushColor(FLinearColor(0.04f,0.04f,0.04f,1));
    Panel->SetPadding(FMargin(24));
    auto* Placement=Canvas->AddChildToCanvas(Panel);
    Placement->SetAnchors(FAnchors(0.5f,0.5f));
    Placement->SetAlignment(FVector2D(0.5f,0.5f));
    Placement->SetSize(FVector2D(620,460));
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
    ResumeButton=WidgetTree->ConstructWidget<UButton>();
    ResumeButton->SetContent(Text(TEXT("Resume"),30));
    Rows->AddChild(ResumeButton);
    ResumeButton->OnClicked.AddDynamic(this,&UPFPauseMenu::Resume);
    SettingsButton=WidgetTree->ConstructWidget<UButton>();
    SettingsButton->SetContent(Text(TEXT("Settings"),30));
    Rows->AddChild(SettingsButton);
    SettingsButton->OnClicked.AddDynamic(this,&UPFPauseMenu::Settings);
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
    bQuitSelected=false;
    SelectedButton=0;
    UpdateSelection();
    QuitLabel->SetText(FText::FromString(TEXT("End session")));
    Description->SetText(FText::FromString(GetWorld()->GetNetMode()==NM_Standalone?TEXT("Paused\nWorld changes are not saved yet.\n"):TEXT("Multiplayer continues while this menu is open.\nWorld changes are not saved yet.\n")));
}

void UPFPauseMenu::Resume(){if(auto* PC=Cast<APFSurvivalPlayerController>(GetOwningPlayer())){PC->SetPauseMenuOpen(false);}}

void UPFPauseMenu::Quit()
{
    // Two-step confirmation: there is no persistence yet, so quitting loses the world.
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
    QuitButton->SetBackgroundColor(SelectedButton==2?FLinearColor(0.2f,0.6f,0.9f):FLinearColor::Gray);
}

void UPFPauseMenu::Settings()
{
    SettingsMenu=CreateWidget<UPFSettingsMenu>(GetOwningPlayer());
    SettingsMenu->ReturnFocus=this;
    SettingsMenu->AddToPlayerScreen(110);
    SettingsMenu->SetKeyboardFocus();
}

FReply UPFPauseMenu::NativeOnPreviewKeyDown(const FGeometry& G,const FKeyEvent& E)
{
    const FKey K=E.GetKey();
    // Any "back/menu" key resumes.
    if(K==EKeys::P || K==EKeys::Escape || K==EKeys::Gamepad_Special_Right || K==EKeys::Gamepad_FaceButton_Right)
    {
        Resume();
        return FReply::Handled();
    }
    // D-pad moves the highlight between the two buttons.
    if(K==EKeys::Gamepad_DPad_Up || K==EKeys::Gamepad_DPad_Down || K==EKeys::Up || K==EKeys::Down)
    {
        SelectedButton=(SelectedButton+((K==EKeys::Gamepad_DPad_Down || K==EKeys::Down)?1:2))%3;
        UpdateSelection();
        return FReply::Handled();
    }
    // A activates the highlighted button (ignore key-repeat so a held A can't double-confirm).
    if(K==EKeys::Gamepad_FaceButton_Bottom || K==EKeys::Enter)
    {
        if(!E.IsRepeat()){if(SelectedButton==2){Quit();}else if(SelectedButton==1){Settings();}else{Resume();}}
        return FReply::Handled();
    }
    return Super::NativeOnPreviewKeyDown(G,E);
}
