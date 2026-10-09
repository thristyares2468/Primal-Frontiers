#include "Survival/PFControlsMenu.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Crafting/PFCraftingHUD.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

void UPFControlsMenu::NativeOnInitialized()
{
    Super::NativeOnInitialized();SetIsFocusable(true);
    auto* Canvas=WidgetTree->ConstructWidget<UCanvasPanel>();WidgetTree->RootWidget=Canvas;
    auto* Panel=WidgetTree->ConstructWidget<UBorder>();Panel->SetBrushColor(FLinearColor(0.025f,0.035f,0.045f,1));Panel->SetPadding(FMargin(20));
    auto* Placement=Canvas->AddChildToCanvas(Panel);Placement->SetAnchors(FAnchors(0.06f,0.05f,0.94f,0.95f));Placement->SetOffsets(FMargin(0));
    auto* Body=WidgetTree->ConstructWidget<UVerticalBox>();Panel->SetContent(Body);
    auto Text=[&](const FString& Value,int32 Size)
    {
        auto* T=WidgetTree->ConstructWidget<UTextBlock>();T->SetText(FText::FromString(Value));
        auto Font=T->GetFont();Font.Size=Size;T->SetFont(Font);T->SetAutoWrapText(true);return T;
    };
    Heading=Text(TEXT(""),26);Body->AddChild(Heading);
    auto* Tabs=WidgetTree->ConstructWidget<UHorizontalBox>();Body->AddChild(Tabs);
    auto Button=[&](const TCHAR* Caption){auto* B=WidgetTree->ConstructWidget<UButton>();B->SetContent(Text(Caption,20));Tabs->AddChild(B);return B;};
    Button(TEXT(" Keyboard & mouse "))->OnClicked.AddDynamic(this,&UPFControlsMenu::Keyboard);
    Button(TEXT(" Controller (Xbox names) "))->OnClicked.AddDynamic(this,&UPFControlsMenu::Controller);
    Button(TEXT(" Back to Pause "))->OnClicked.AddDynamic(this,&UPFControlsMenu::Back);
    Body->AddChild(Text(TEXT("Left/Right or LB/RB: page | Up/Down or D-pad: scroll | Esc/P/B/Menu: back to Pause"),16));
    Scroll=WidgetTree->ConstructWidget<UScrollBox>();Body->AddChildToVerticalBox(Scroll)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    Bindings=Text(TEXT(""),20);Scroll->AddChild(Bindings);
    Refresh();
}

void UPFControlsMenu::Refresh()
{
    const auto* PC=Cast<APFSurvivalPlayerController>(GetOwningPlayer());
    Heading->SetText(FText::FromString(bGamepad?TEXT("CONTROLS — CONTROLLER"):TEXT("CONTROLS — KEYBOARD & MOUSE")));
    FString Text=TEXT("Read-only bindings. Remapping is not implemented.\n\n");
    if(PC)
    {
        for(const auto& H:PC->GetControlHints(bGamepad))
        {Text+=FString::Printf(TEXT("[%s] %s — %s\n\n"),*H.Context,*H.Key.GetDisplayName().ToString(),*H.Action);}
    }
    Text+=FString(TEXT("CRAFTING MENU (C / Y)\n"))+UPFCraftingHUD::NavigationHelp()+TEXT("\nChanging category clears selection. Choose a recipe before crafting; browsing does not cancel a job or spend ingredients.\n\n");
    Text+=TEXT("SURVIVAL TIPS\nAim at a pickup or resource within 2.5 metres. Scenery is not automatically gatherable. Read the crosshair prompt and server refusal message.\n\nFood is finite: find/gather it, then cook/dry it when ingredients are available. Consume an edible inventory stack; expired food cannot be eaten. Freshness continues to age offline.\n\nOnly one inventory/crafting/building overlay is active. Closing help returns to Pause and cannot trigger a gameplay action. P is preferred in Editor Play because Esc may stop PIE.\n\n");
    Text+=PC && PC->GetNetMode()==NM_Standalone?TEXT("Standalone: the world is paused while this menu is open.\n"):TEXT("Multiplayer: the server/world continues while menus are open; you are not protected from danger.\n");
    Text+=TEXT("Save explicitly before ending a session. There is no timed autosave or automatic standalone/server-exit save. PF.SaveWorld / PF.LoadWorld are server developer commands.\n");
    Bindings->SetText(FText::FromString(Text));Scroll->ScrollToStart();
}
void UPFControlsMenu::Keyboard(){bGamepad=false;Refresh();SetKeyboardFocus();}
void UPFControlsMenu::Controller(){bGamepad=true;Refresh();SetKeyboardFocus();}
void UPFControlsMenu::Back(){RemoveFromParent();if(ReturnFocus){ReturnFocus->SetKeyboardFocus();}}
FReply UPFControlsMenu::HandleNavigation(FKey Key,bool bRepeat)
{
    if(Key==EKeys::Escape || Key==EKeys::P || Key==EKeys::Gamepad_FaceButton_Right || Key==EKeys::Gamepad_Special_Right)
    {if(!bRepeat){Back();}return FReply::Handled();}
    if(Key==EKeys::Left || Key==EKeys::Right || Key==EKeys::Gamepad_LeftShoulder || Key==EKeys::Gamepad_RightShoulder)
    {if(!bRepeat){if(bGamepad){Keyboard();}else{Controller();}}return FReply::Handled();}
    if(Key==EKeys::Up || Key==EKeys::Down || Key==EKeys::Gamepad_DPad_Up || Key==EKeys::Gamepad_DPad_Down)
    {const bool bUp=Key==EKeys::Up || Key==EKeys::Gamepad_DPad_Up;Scroll->SetScrollOffset(FMath::Max(0.f,Scroll->GetScrollOffset()+(bUp?-100.f:100.f)));return FReply::Handled();}
    // Consume keys even when focused on a child; help never forwards gameplay inputs.
    return FReply::Handled();
}
FReply UPFControlsMenu::NativeOnPreviewKeyDown(const FGeometry&,const FKeyEvent& E){return HandleNavigation(E.GetKey(),E.IsRepeat());}
