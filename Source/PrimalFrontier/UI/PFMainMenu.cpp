#include "UI/PFMainMenu.h"
#include "UI/PFUITheme.h"
#include "Persistence/PFSessionGameInstance.h"
#include "Persistence/PFWorldMenuModel.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/ButtonSlot.h"
#include "Components/EditableTextBox.h"
#include "Components/ComboBoxString.h"
#include "Components/SizeBox.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Settings/PFSettingsMenu.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Misc/DateTime.h"
#include "Engine/World.h"

APFMainMenuGameMode::APFMainMenuGameMode()
{DefaultPawnClass=nullptr;PlayerControllerClass=APFMainMenuPlayerController::StaticClass();}
void APFMainMenuGameMode::HandleStartingNewPlayer_Implementation(APlayerController*)
{
    // A menu owns only its local controller/widget; never ask RestartPlayer to
    // spawn a null DefaultPawnClass or modify a gameplay world's player state.
}
void APFMainMenuPlayerController::BeginPlay()
{
    Super::BeginPlay();if(!IsLocalController() || !GetLocalPlayer()){return;}
    Menu=CreateWidget<UPFMainMenu>(this);if(!Menu || !Menu->AddToPlayerScreen()){return;}
    bShowMouseCursor=true;FInputModeUIOnly Mode;Mode.SetWidgetToFocus(Menu->TakeWidget());SetInputMode(Mode);Menu->SetKeyboardFocus();
}
void UPFMainMenu::NativeOnInitialized()
{
    Super::NativeOnInitialized();SetIsFocusable(true);
    auto* Canvas=WidgetTree->ConstructWidget<UCanvasPanel>();WidgetTree->RootWidget=Canvas;
    auto* Shade=WidgetTree->ConstructWidget<UBorder>();Shade->SetBrushColor(PFUITheme::Surface);
    auto* Back=Canvas->AddChildToCanvas(Shade);Back->SetAnchors(FAnchors(0,0,1,1));Back->SetOffsets(FMargin(0));
    auto* Panel=WidgetTree->ConstructWidget<UBorder>();Panel->SetBrush(FSlateRoundedBoxBrush(PFUITheme::Inset,12.f));Panel->SetPadding(FMargin(32));
    auto* Size=WidgetTree->ConstructWidget<USizeBox>();Size->SetWidthOverride(1100);Size->SetContent(Panel);
    auto* Placement=Canvas->AddChildToCanvas(Size);Placement->SetAnchors(FAnchors(0.5f,0.5f));Placement->SetAlignment(FVector2D(0.5f,0.5f));Placement->SetAutoSize(true);
    auto* Columns=WidgetTree->ConstructWidget<UHorizontalBox>();Panel->SetContent(Columns);
    auto* Rows=WidgetTree->ConstructWidget<UVerticalBox>();auto* Nav=Columns->AddChildToHorizontalBox(Rows);Nav->SetSize(FSlateChildSize(ESlateSizeRule::Fill));Nav->SetPadding(FMargin(0,0,32,0));
    auto Text=[&](const TCHAR* Caption,int32 FontSize)
    {
        auto* T=WidgetTree->ConstructWidget<UTextBlock>();T->SetText(FText::FromString(Caption));auto F=T->GetFont();F.Size=FontSize;T->SetFont(F);T->SetColorAndOpacity(PFUITheme::Text);T->SetAutoWrapText(true);
        Rows->AddChildToVerticalBox(T)->SetPadding(FMargin(0,0,0,12));return T;
    };
    Text(TEXT("PRIMAL\nFRONTIER"),48);Text(TEXT("SURVIVE. BUILD. ADAPT."),18)->SetColorAndOpacity(PFUITheme::Accent);
    Text(TEXT("A first-person survival frontier."),22)->SetColorAndOpacity(PFUITheme::Muted);
    auto Button=[&](const TCHAR* Caption,const TCHAR* Name)
    {
        auto* B=WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(),FName(Name));B->SetStyle(PFUITheme::NavigationButton(false));
        auto* T=WidgetTree->ConstructWidget<UTextBlock>();T->SetText(FText::FromString(Caption));auto F=T->GetFont();F.Size=24;T->SetFont(F);T->SetColorAndOpacity(PFUITheme::Text);B->SetContent(T);
        CastChecked<UButtonSlot>(T->Slot)->SetPadding(FMargin(18,12));Rows->AddChildToVerticalBox(B)->SetPadding(FMargin(0,0,0,12));return B;
    };
    Button(TEXT("Single player"),TEXT("PF_SinglePlayer"))->OnClicked.AddDynamic(this,&UPFMainMenu::SinglePlayer);
    Button(TEXT("Multiplayer"),TEXT("PF_Multiplayer"))->OnClicked.AddDynamic(this,&UPFMainMenu::Multiplayer);
    Button(TEXT("Settings"),TEXT("PF_MenuSettings"))->OnClicked.AddDynamic(this,&UPFMainMenu::Settings);
    Button(TEXT("Quit game"),TEXT("PF_QuitGame"))->OnClicked.AddDynamic(this,&UPFMainMenu::Quit);
    Text(TEXT("GREYBOX PROTOTYPE"),16)->SetColorAndOpacity(PFUITheme::Muted);
    auto* Right=WidgetTree->ConstructWidget<UVerticalBox>();Columns->AddChildToHorizontalBox(Right)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    WelcomePanel=WidgetTree->ConstructWidget<UVerticalBox>();Right->AddChild(WelcomePanel);Rows=WelcomePanel;
    Text(TEXT("Your next frontier"),32);Text(TEXT("Start a new single-player world or continue a saved survivor. Explore the open-world arena, gather supplies, craft tools and build shelter."),22);
    Text(TEXT("Worlds are saved explicitly from Pause. Your local reconnect profile is separate from your world save."),20)->SetColorAndOpacity(PFUITheme::Muted);
    SoloPanel=WidgetTree->ConstructWidget<UVerticalBox>();Right->AddChild(SoloPanel);Rows=SoloPanel;
    Text(TEXT("Single player"),32);Text(TEXT("Create a world"),22);
    WorldName=WidgetTree->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass(),TEXT("PF_NewWorldName"));
    auto FieldStyle=WorldName->GetWidgetStyle();auto FieldText=FieldStyle.TextStyle;FieldText.Font.Size=22;FieldText.SetColorAndOpacity(PFUITheme::Text);FieldStyle.SetTextStyle(FieldText);
    FieldStyle.SetBackgroundImageNormal(FSlateRoundedBoxBrush(PFUITheme::Surface,6.f));FieldStyle.SetBackgroundImageFocused(FSlateRoundedBoxBrush(PFUITheme::Surface,6.f,PFUITheme::Accent,1.f));WorldName->SetWidgetStyle(FieldStyle);
    WorldName->SetText(FText::FromString(TEXT("World_")+FDateTime::UtcNow().ToString(TEXT("%Y%m%d_%H%M%S"))));Rows->AddChildToVerticalBox(WorldName)->SetPadding(FMargin(0,0,0,8));
    Button(TEXT("Create new world"),TEXT("PF_NewWorld"))->OnClicked.AddDynamic(this,&UPFMainMenu::CreateWorld);
    Text(TEXT("Load a saved world"),24);
    Worlds=WidgetTree->ConstructWidget<UComboBoxString>(UComboBoxString::StaticClass(),TEXT("PF_SavedWorlds"));Worlds->OnGenerateWidgetEvent.BindDynamic(this,&UPFMainMenu::WorldOption);
    Worlds->SetClipping(EWidgetClipping::ClipToBounds);
    Rows->AddChildToVerticalBox(Worlds)->SetPadding(FMargin(0,0,0,8));Worlds->OnSelectionChanged.AddDynamic(this,&UPFMainMenu::SelectionChanged);
    Button(TEXT("Load selected world"),TEXT("PF_LoadWorld"))->OnClicked.AddDynamic(this,&UPFMainMenu::LoadWorld);
    Button(TEXT("Refresh worlds"),TEXT("PF_RefreshWorlds"))->OnClicked.AddDynamic(this,&UPFMainMenu::RefreshWorlds);
    RenameInput=WidgetTree->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass(),TEXT("PF_RenameWorldName"));RenameInput->SetWidgetStyle(FieldStyle);RenameInput->SetHintText(FText::FromString(TEXT("New display name")));
    Rows->AddChildToVerticalBox(RenameInput)->SetPadding(FMargin(0,0,0,8));
    Button(TEXT("Rename selected world"),TEXT("PF_RenameWorld"))->OnClicked.AddDynamic(this,&UPFMainMenu::RenameWorld);
    Status=Text(TEXT(""),20);Status->SetColorAndOpacity(PFUITheme::Muted);
    Button(TEXT("Back"),TEXT("PF_SoloBack"))->OnClicked.AddDynamic(this,&UPFMainMenu::Back);
    MultiplayerPanel=WidgetTree->ConstructWidget<UVerticalBox>();Right->AddChild(MultiplayerPanel);Rows=MultiplayerPanel;
    Text(TEXT("Multiplayer"),32);Text(TEXT("Dedicated-server development sessions"),24);
    Text(TEXT("Server/client gameplay and persistence have automated test support. Use the documented server launch and connection flow in Docs/PLAYTEST.md for current multiplayer sessions."),22);
    Text(TEXT("Menu hosting, server discovery and invite codes are not implemented yet. Multiplayer world saves remain controlled by the server. This screen does not start or join a session."),20)->SetColorAndOpacity(PFUITheme::Muted);
    Button(TEXT("Back"),TEXT("PF_MultiplayerBack"))->OnClicked.AddDynamic(this,&UPFMainMenu::Back);
    Text(TEXT("Tab / Shift+Tab: navigate  |  Enter / A: activate  |  Esc / B: back"),16)->SetColorAndOpacity(PFUITheme::Muted);
    ShowPanel(0);
    RefreshWorlds();
}
void UPFMainMenu::RefreshWorlds()
{
    const FString Before=Worlds->GetSelectedOption();Worlds->ClearOptions();const auto Entries=PFWorldMenuModel::List();
    for(const auto& E:Entries){Worlds->AddOption(E.Slot);}
    if(!Entries.IsEmpty()){Worlds->SetSelectedOption(Entries.ContainsByPredicate([&](const auto& E){return E.Slot==Before;})?Before:Entries[0].Slot);}
    else{Status->SetText(FText::FromString(TEXT("No saved worlds yet. Create a world, then Save in Pause.")));}
    if(auto* GI=GetGameInstance<UPFSessionGameInstance>();GI && !GI->MenuMessage.IsEmpty()){Status->SetText(FText::FromString(GI->MenuMessage));}
}
void UPFMainMenu::SelectionChanged(FString SelectedWorldName,ESelectInfo::Type)
{
    if(!Status){return;}FPFWorldMenuEntry E;PFWorldMenuModel::Inspect(SelectedWorldName,E);
    const FString MapLabel=E.Map==TEXT("L_PrimalFrontier_OpenWorld")?TEXT("Open-world frontier"):TEXT("Survival arena");
    Status->SetText(FText::FromString(E.bLoadable?MapLabel+TEXT("  |  saved ")+FDateTime::FromUnixTimestamp(E.SavedUtc).ToString()+TEXT(" UTC"):TEXT("Cannot load: ")+E.Issue));
}
void UPFMainMenu::CreateWorld()
{
    FString Error;auto* GI=GetGameInstance<UPFSessionGameInstance>();
    if(!GI || !GI->StartWorld(WorldName->GetText().ToString(),false,Error)){Status->SetText(FText::FromString(TEXT("New world refused: ")+Error));}
}
void UPFMainMenu::LoadWorld()
{
    FString Error;auto* GI=GetGameInstance<UPFSessionGameInstance>();
    if(!GI || !GI->StartWorld(Worlds->GetSelectedOption(),true,Error)){Status->SetText(FText::FromString(TEXT("Load refused: ")+Error));}
}
void UPFMainMenu::RenameWorld()
{
    FString Error;const FString Selected=Worlds->GetSelectedOption();
    if(GetWorld()->GetNetMode()!=NM_Standalone || !PFWorldMenuModel::Rename(Selected,RenameInput->GetText().ToString(),Error))
    {Status->SetText(FText::FromString(TEXT("Rename refused: ")+Error));return;}
    RefreshWorlds();Worlds->SetSelectedOption(Selected);
    Status->SetText(FText::FromString(TEXT("World renamed. Its save ID and gameplay records are unchanged.")));
}
void UPFMainMenu::Quit(){UKismetSystemLibrary::QuitGame(this,GetOwningPlayer(),EQuitPreference::Quit,false);}
UWidget* UPFMainMenu::WorldOption(FString WorldLabel)
{
    const FString Name=PFWorldMenuModel::NameFor(WorldLabel);
    auto* T=WidgetTree->ConstructWidget<UTextBlock>();T->SetText(FText::FromString(Name));T->SetToolTipText(FText::FromString(Name));T->SetTextOverflowPolicy(ETextOverflowPolicy::Ellipsis);auto F=T->GetFont();F.Size=22;T->SetFont(F);return T;
}
void UPFMainMenu::ShowPanel(int32 Index)
{
    WelcomePanel->SetVisibility(Index==0?ESlateVisibility::Visible:ESlateVisibility::Collapsed);
    SoloPanel->SetVisibility(Index==1?ESlateVisibility::Visible:ESlateVisibility::Collapsed);
    MultiplayerPanel->SetVisibility(Index==2?ESlateVisibility::Visible:ESlateVisibility::Collapsed);
}
void UPFMainMenu::SinglePlayer(){RefreshWorlds();ShowPanel(1);}
void UPFMainMenu::Multiplayer(){ShowPanel(2);}
void UPFMainMenu::Back(){ShowPanel(0);SetKeyboardFocus();}
void UPFMainMenu::Settings()
{
    if(SettingsMenu){SettingsMenu->RemoveFromParent();}
    SettingsMenu=CreateWidget<UPFSettingsMenu>(GetOwningPlayer());if(!SettingsMenu){return;}
    SettingsMenu->ReturnFocus=this;SettingsMenu->AddToPlayerScreen(110);SettingsMenu->SetKeyboardFocus();
}
FReply UPFMainMenu::NativeOnPreviewKeyDown(const FGeometry& Geometry,const FKeyEvent& Event)
{
    if(Event.GetKey()==EKeys::Escape || Event.GetKey()==EKeys::Gamepad_FaceButton_Right){if(!Event.IsRepeat()){Back();}return FReply::Handled();}
    return Super::NativeOnPreviewKeyDown(Geometry,Event);
}
void UPFMainMenu::NativeDestruct(){if(SettingsMenu){SettingsMenu->RemoveFromParent();SettingsMenu=nullptr;}Super::NativeDestruct();}
