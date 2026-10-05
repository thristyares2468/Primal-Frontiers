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
void UPFPauseMenu::NativeOnInitialized()
{
    Super::NativeOnInitialized();SetIsFocusable(true);
    auto* Canvas=WidgetTree->ConstructWidget<UCanvasPanel>();WidgetTree->RootWidget=Canvas;
    auto* Shade=WidgetTree->ConstructWidget<UBorder>();Shade->SetBrushColor(FLinearColor(0,0,0,0.8f));auto* Back=Canvas->AddChildToCanvas(Shade);Back->SetAnchors(FAnchors(0,0,1,1));Back->SetOffsets(FMargin(0));
    auto* Panel=WidgetTree->ConstructWidget<UBorder>();Panel->SetBrushColor(FLinearColor(0.04f,0.04f,0.04f,1));Panel->SetPadding(FMargin(24));
    auto* Placement=Canvas->AddChildToCanvas(Panel);Placement->SetAnchors(FAnchors(0.5f,0.5f));Placement->SetAlignment(FVector2D(0.5f,0.5f));Placement->SetSize(FVector2D(620,460));
    auto* Rows=WidgetTree->ConstructWidget<UVerticalBox>();Panel->SetContent(Rows);
    auto Text=[&](const TCHAR* Value,int32 Size){auto* T=WidgetTree->ConstructWidget<UTextBlock>();T->SetText(FText::FromString(Value));auto Font=T->GetFont();Font.Size=Size;T->SetFont(Font);T->SetAutoWrapText(true);return T;};
    Rows->AddChild(Text(TEXT("PRIMAL FRONTIER"),34));Description=Text(TEXT(""),24);Rows->AddChild(Description);
    ResumeButton=WidgetTree->ConstructWidget<UButton>();ResumeButton->SetContent(Text(TEXT("Resume"),30));Rows->AddChild(ResumeButton);ResumeButton->OnClicked.AddDynamic(this,&UPFPauseMenu::Resume);
    QuitButton=WidgetTree->ConstructWidget<UButton>();QuitLabel=Text(TEXT("End session"),30);QuitLabel->SetAutoWrapText(false);QuitButton->SetContent(QuitLabel);Rows->AddChild(QuitButton);QuitButton->OnClicked.AddDynamic(this,&UPFPauseMenu::Quit);
    Rows->AddChild(Text(TEXT("P / Esc / Menu / B: resume. D-pad: select. A: confirm. In Editor Play, Esc may stop PIE."),22));
}
void UPFPauseMenu::Refresh()
{
    bConfirmQuit=false;bQuitSelected=false;UpdateSelection();QuitLabel->SetText(FText::FromString(TEXT("End session")));
    Description->SetText(FText::FromString(GetWorld()->GetNetMode()==NM_Standalone?TEXT("Paused\nWorld changes are not saved yet.\n"):TEXT("Multiplayer continues while this menu is open.\nWorld changes are not saved yet.\n")));
}
void UPFPauseMenu::Resume(){if(auto* PC=Cast<APFSurvivalPlayerController>(GetOwningPlayer())){PC->SetPauseMenuOpen(false);}}
void UPFPauseMenu::Quit()
{
    if(!bConfirmQuit){bConfirmQuit=true;QuitLabel->SetText(FText::FromString(TEXT("Confirm end session")));return;}
    Resume();UKismetSystemLibrary::QuitGame(this,GetOwningPlayer(),EQuitPreference::Quit,false);
}
void UPFPauseMenu::UpdateSelection()
{ResumeButton->SetBackgroundColor(bQuitSelected?FLinearColor::Gray:FLinearColor(0.2f,0.6f,0.9f));QuitButton->SetBackgroundColor(bQuitSelected?FLinearColor(0.2f,0.6f,0.9f):FLinearColor::Gray);}
FReply UPFPauseMenu::NativeOnPreviewKeyDown(const FGeometry& G,const FKeyEvent& E)
{
    const FKey K=E.GetKey();
    if(K==EKeys::P || K==EKeys::Escape || K==EKeys::Gamepad_Special_Right || K==EKeys::Gamepad_FaceButton_Right){Resume();return FReply::Handled();}
    if(K==EKeys::Gamepad_DPad_Up || K==EKeys::Gamepad_DPad_Down){bQuitSelected=K==EKeys::Gamepad_DPad_Down;UpdateSelection();return FReply::Handled();}
    if(K==EKeys::Gamepad_FaceButton_Bottom){if(!E.IsRepeat()){if(bQuitSelected){Quit();}else{Resume();}}return FReply::Handled();}
    return Super::NativeOnPreviewKeyDown(G,E);
}
