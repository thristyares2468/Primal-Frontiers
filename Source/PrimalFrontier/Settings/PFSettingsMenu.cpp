#include "Settings/PFSettingsMenu.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "HAL/PlatformTime.h"
#include "Kismet/KismetSystemLibrary.h"

namespace
{
UTextBlock* Text(UWidgetTree* Tree,const FString& Value,int32 Size=22)
{
    auto* T=Tree->ConstructWidget<UTextBlock>();T->SetText(FText::FromString(Value));
    auto Font=T->GetFont();Font.Size=Size;T->SetFont(Font);return T;
}
FString Toggle(bool Value){return Value?TEXT("On"):TEXT("Off");}
FString Quality(int32 Value){const TCHAR* Names[]={TEXT("Low"),TEXT("Medium"),TEXT("High"),TEXT("Epic")};return Value>=0&&Value<4?Names[Value]:TEXT("Custom");}
int32 Cycle(int32 Value,int32 Direction,int32 Count){return (Value+Direction+Count)%Count;}
}

void UPFSettingsRow::InitializeRow(UPFSettingsMenu* InOwner,int32 InIndex)
{
    Menu=InOwner;Index=InIndex;
    auto* Box=WidgetTree->ConstructWidget<UHorizontalBox>();WidgetTree->RootWidget=Box;
    auto* Left=WidgetTree->ConstructWidget<UButton>();Left->SetContent(Text(WidgetTree,TEXT(" < ")));
    Box->AddChildToHorizontalBox(Left);Left->OnClicked.AddDynamic(this,&UPFSettingsRow::Previous);
    Label=Text(WidgetTree,TEXT(""));
    auto* LabelSlot=Box->AddChildToHorizontalBox(Label);LabelSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));LabelSlot->SetPadding(FMargin(12,6));
    auto* Right=WidgetTree->ConstructWidget<UButton>();Right->SetContent(Text(WidgetTree,TEXT(" > ")));
    Box->AddChildToHorizontalBox(Right);Right->OnClicked.AddDynamic(this,&UPFSettingsRow::Next);
}
void UPFSettingsRow::SetCaption(const FString& Caption,bool bSelected){Label->SetText(FText::FromString(Caption));Label->SetColorAndOpacity(bSelected?FLinearColor(0.3f,0.85f,1):FLinearColor::White);}
void UPFSettingsRow::Previous(){if(Menu){Menu->Change(Index,-1);}}
void UPFSettingsRow::Next(){if(Menu){Menu->Change(Index,1);}}

void UPFSettingsMenu::CopyFromLive()
{
    if(auto* Live=UPFGameUserSettings::Get())
    {
        Draft=Live->CreateDraft(this);
    }
}
void UPFSettingsMenu::NativeOnInitialized()
{
    Super::NativeOnInitialized();SetIsFocusable(true);CopyFromLive();
    auto* Canvas=WidgetTree->ConstructWidget<UCanvasPanel>();WidgetTree->RootWidget=Canvas;
    auto* Panel=WidgetTree->ConstructWidget<UBorder>();Panel->SetBrushColor(FLinearColor(0.025f,0.035f,0.045f,1));Panel->SetPadding(FMargin(24));
    auto* Placement=Canvas->AddChildToCanvas(Panel);Placement->SetAnchors(FAnchors(0.08f,0.06f,0.92f,0.94f));Placement->SetOffsets(FMargin(0));
    auto* Body=WidgetTree->ConstructWidget<UVerticalBox>();Panel->SetContent(Body);
    Body->AddChild(Text(WidgetTree,TEXT("SETTINGS"),30));
    auto* Tabs=WidgetTree->ConstructWidget<UHorizontalBox>();Body->AddChild(Tabs);
    auto Tab=[&](const TCHAR* Name){auto* B=WidgetTree->ConstructWidget<UButton>();B->SetContent(Text(WidgetTree,Name));Tabs->AddChild(B);return B;};
    auto* G=Tab(TEXT(" Game "));G->OnClicked.AddDynamic(this,&UPFSettingsMenu::Game);
    auto* V=Tab(TEXT(" Graphics "));V->OnClicked.AddDynamic(this,&UPFSettingsMenu::Graphics);
    auto* A=Tab(TEXT(" Audio "));A->OnClicked.AddDynamic(this,&UPFSettingsMenu::Audio);
    auto* X=Tab(TEXT(" Accessibility "));X->OnClicked.AddDynamic(this,&UPFSettingsMenu::Accessibility);
    Scroll=WidgetTree->ConstructWidget<UScrollBox>();Body->AddChildToVerticalBox(Scroll)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    RowsPanel=WidgetTree->ConstructWidget<UVerticalBox>();Scroll->AddChild(RowsPanel);
    Status=Text(WidgetTree,TEXT("Changes apply only when confirmed. Display changes unavailable in PIE."),18);Status->SetAutoWrapText(true);Body->AddChild(Status);
    auto* Actions=WidgetTree->ConstructWidget<UHorizontalBox>();Body->AddChild(Actions);
    auto Action=[&](const TCHAR* Name){auto* B=WidgetTree->ConstructWidget<UButton>();B->SetContent(Text(WidgetTree,Name));Actions->AddChild(B);return B;};
    Action(TEXT(" Apply / Keep "))->OnClicked.AddDynamic(this,&UPFSettingsMenu::Apply);
    Action(TEXT(" Cancel / Back "))->OnClicked.AddDynamic(this,&UPFSettingsMenu::Cancel);
    Action(TEXT(" Defaults "))->OnClicked.AddDynamic(this,&UPFSettingsMenu::Defaults);
    Body->AddChild(Text(WidgetTree,TEXT("Arrows / D-pad: select & change | LB/RB: tab | Enter/A: apply | Esc/B: back | Y: defaults"),16));
    RefreshRows();
}
void UPFSettingsMenu::Game(){Category=0;Selected=0;RefreshRows();}
void UPFSettingsMenu::Graphics(){Category=1;Selected=0;RefreshRows();}
void UPFSettingsMenu::Audio(){Category=2;Selected=0;RefreshRows();}
void UPFSettingsMenu::Accessibility(){Category=3;Selected=0;RefreshRows();}
void UPFSettingsMenu::RefreshRows()
{
    RowsPanel->ClearChildren();Rows.Reset();if(!Draft){return;}
    const int32 Counts[]={4,17,5,4};
    for(int32 I=0;I<Counts[Category];++I){auto* Row=CreateWidget<UPFSettingsRow>(GetOwningPlayer());Row->InitializeRow(this,I);RowsPanel->AddChild(Row);Rows.Add(Row);Row->SetCaption(Caption(I),I==Selected);}
}
FString UPFSettingsMenu::Caption(int32 I) const
{
    const auto& P=Draft->Preferences;
    if(Category==0){switch(I){case 0:return FString::Printf(TEXT("Field of view: %.0f"),P.FieldOfView);case 1:return FString::Printf(TEXT("Look sensitivity: %.2f"),P.LookSensitivity);case 2:return TEXT("Invert vertical look: ")+Toggle(P.bInvertLook);default:return TEXT("Control hints: ")+Toggle(P.bControlHints);}}
    if(Category==2){const TCHAR* Names[]={TEXT("Master"),TEXT("Music"),TEXT("Effects"),TEXT("UI")};const float Values[]={P.MasterVolume,P.MusicVolume,P.EffectsVolume,P.UIVolume};return I<4?FString::Printf(TEXT("%s volume: %.0f%%"),Names[I],Values[I]*100):TEXT("Mute when unfocused: ")+Toggle(P.bMuteUnfocused);}
    if(Category==3){switch(I){case 0:return FString::Printf(TEXT("HUD text scale: %.0f%%"),P.HUDScale*100);case 1:return TEXT("Crosshair: ")+Toggle(P.bCrosshair);case 2:return FString::Printf(TEXT("Crosshair size: %.0f%%"),P.CrosshairScale*100);default:{const TCHAR* Modes[]={TEXT("Off"),TEXT("Deuteranope"),TEXT("Protanope"),TEXT("Tritanope")};return FString(TEXT("Colour vision correction: "))+Modes[P.ColorVisionMode];}}}
    const bool bPIE=GetWorld()->WorldType==EWorldType::PIE;
    switch(I)
    {
    case 0:return TEXT("Quality preset: ")+Quality(Draft->GetOverallScalabilityLevel());
    case 1:{const TCHAR* Modes[]={TEXT("Fullscreen"),TEXT("Borderless"),TEXT("Windowed")};return FString(TEXT("Display: "))+(bPIE?TEXT("Controlled by Editor"):Modes[Draft->GetFullscreenMode()]);}
    case 2:{auto R=Draft->GetScreenResolution();return bPIE?TEXT("Resolution: Controlled by Editor"):FString::Printf(TEXT("Resolution: %d x %d"),R.X,R.Y);}
    case 3:{float N,V,Min,Max;Draft->GetResolutionScaleInformationEx(N,V,Min,Max);return V<=0?TEXT("Render scale: Engine automatic"):FString::Printf(TEXT("Render scale: %.0f%%"),V);}
    case 4:return TEXT("VSync: ")+Toggle(Draft->IsVSyncEnabled());
    case 5:return Draft->GetFrameRateLimit()==0?TEXT("FPS limit: Unlimited"):FString::Printf(TEXT("FPS limit: %.0f"),Draft->GetFrameRateLimit());
    case 6:return TEXT("Motion blur: ")+Toggle(P.bMotionBlur);
    case 7:return TEXT("Depth of field: ")+Toggle(P.bDepthOfField);
    case 8:return TEXT("View distance: ")+Quality(Draft->GetViewDistanceQuality());
    case 9:return TEXT("Shadows: ")+Quality(Draft->GetShadowQuality());
    case 10:return TEXT("Global illumination: ")+Quality(Draft->GetGlobalIlluminationQuality());
    case 11:return TEXT("Reflections: ")+Quality(Draft->GetReflectionQuality());
    case 12:return TEXT("Textures: ")+Quality(Draft->GetTextureQuality());
    case 13:return TEXT("Anti-aliasing: ")+Quality(Draft->GetAntiAliasingQuality());
    case 14:return TEXT("Effects: ")+Quality(Draft->GetVisualEffectQuality());
    case 15:return TEXT("Foliage: ")+Quality(Draft->GetFoliageQuality());
    default:return TEXT("Post-processing: ")+Quality(Draft->GetPostProcessingQuality());
    }
}
void UPFSettingsMenu::Change(int32 I,int32 D)
{
    if(!Draft || ConfirmDeadline>0){return;}Selected=I;auto& P=Draft->Preferences;
    if(Category==0){switch(I){case 0:P.FieldOfView+=D*5;break;case 1:P.LookSensitivity+=D*0.25f;break;case 2:P.bInvertLook=!P.bInvertLook;break;default:P.bControlHints=!P.bControlHints;break;}}
    else if(Category==2){float* Values[]={&P.MasterVolume,&P.MusicVolume,&P.EffectsVolume,&P.UIVolume};if(I<4){*Values[I]+=D*0.1f;}else{P.bMuteUnfocused=!P.bMuteUnfocused;}}
    else if(Category==3){switch(I){case 0:P.HUDScale+=D*0.25f;break;case 1:P.bCrosshair=!P.bCrosshair;break;case 2:P.CrosshairScale+=D*0.25f;break;default:P.ColorVisionMode=Cycle(P.ColorVisionMode,D,4);break;}}
    else
    {
        switch(I)
        {
        case 0:Draft->SetOverallScalabilityLevel(Cycle(FMath::Max(0,Draft->GetOverallScalabilityLevel()),D,4));break;
        case 1:if(GetWorld()->WorldType!=EWorldType::PIE){Draft->SetFullscreenMode(static_cast<EWindowMode::Type>(Cycle(Draft->GetFullscreenMode(),D,3)));}break;
        case 2:if(GetWorld()->WorldType!=EWorldType::PIE){TArray<FIntPoint> Modes;UKismetSystemLibrary::GetSupportedFullscreenResolutions(Modes);const int32 Index=Modes.IndexOfByKey(Draft->GetScreenResolution());if(!Modes.IsEmpty()){Draft->SetScreenResolution(Modes[Cycle(FMath::Max(0,Index),D,Modes.Num())]);}}break;
        case 3:{float N,V,Min,Max;Draft->GetResolutionScaleInformationEx(N,V,Min,Max);Draft->SetResolutionScaleValueEx(FMath::Clamp(V+D*5,50.f,100.f));break;}
        case 4:Draft->SetVSyncEnabled(!Draft->IsVSyncEnabled());break;
        case 5:{const float Limits[]={0,30,60,90,120,144,165,240};int32 Index=0;for(int32 J=0;J<8;++J){if(Limits[J]==Draft->GetFrameRateLimit()){Index=J;}}Draft->SetFrameRateLimit(Limits[Cycle(Index,D,8)]);break;}
        case 6:P.bMotionBlur=!P.bMotionBlur;break;
        case 7:P.bDepthOfField=!P.bDepthOfField;break;
        case 8:Draft->SetViewDistanceQuality(Cycle(Draft->GetViewDistanceQuality(),D,4));break;
        case 9:Draft->SetShadowQuality(Cycle(Draft->GetShadowQuality(),D,4));break;
        case 10:Draft->SetGlobalIlluminationQuality(Cycle(Draft->GetGlobalIlluminationQuality(),D,4));break;
        case 11:Draft->SetReflectionQuality(Cycle(Draft->GetReflectionQuality(),D,4));break;
        case 12:Draft->SetTextureQuality(Cycle(Draft->GetTextureQuality(),D,4));break;
        case 13:Draft->SetAntiAliasingQuality(Cycle(Draft->GetAntiAliasingQuality(),D,4));break;
        case 14:Draft->SetVisualEffectQuality(Cycle(Draft->GetVisualEffectQuality(),D,4));break;
        case 15:Draft->SetFoliageQuality(Cycle(Draft->GetFoliageQuality(),D,4));break;
        case 16:Draft->SetPostProcessingQuality(Cycle(Draft->GetPostProcessingQuality(),D,4));break;
        }
    }
    P.Sanitize();for(int32 J=0;J<Rows.Num();++J){Rows[J]->SetCaption(Caption(J),J==Selected);}
}
void UPFSettingsMenu::Apply()
{
    auto* Live=UPFGameUserSettings::Get();if(!Live || !Draft){return;}
    if(ConfirmDeadline>0){ConfirmDeadline=0;Live->ConfirmVideoMode();Live->SaveSettings();Status->SetText(FText::FromString(TEXT("Display confirmed. Settings saved.")));return;}
    PreviousResolution=Live->GetScreenResolution();PreviousMode=Live->GetFullscreenMode();
    Live->Preferences=Draft->Preferences;Live->ScalabilityQuality=Draft->ScalabilityQuality;
    Live->SetVSyncEnabled(Draft->IsVSyncEnabled());Live->SetFrameRateLimit(Draft->GetFrameRateLimit());
    Live->ApplyNonResolutionSettings();Live->ApplyToWorld(GetWorld());
    const bool bDisplayChanged=GetWorld()->WorldType!=EWorldType::PIE && (PreviousResolution!=Draft->GetScreenResolution() || PreviousMode!=Draft->GetFullscreenMode());
    if(bDisplayChanged){Live->SetScreenResolution(Draft->GetScreenResolution());Live->SetFullscreenMode(Draft->GetFullscreenMode());Live->ApplyResolutionSettings(false);ConfirmDeadline=FPlatformTime::Seconds()+15;}
    else{Live->SaveSettings();Status->SetText(FText::FromString(TEXT("Settings applied and saved.")));}
}
void UPFSettingsMenu::RevertDisplay()
{
    if(ConfirmDeadline<=0){return;}ConfirmDeadline=0;
    if(auto* Live=UPFGameUserSettings::Get()){Live->SetScreenResolution(PreviousResolution);Live->SetFullscreenMode(PreviousMode);Live->ApplyResolutionSettings(false);Live->SaveSettings();CopyFromLive();}
    UE_LOG(LogTemp,Display,TEXT("[PrimalSettings] Unconfirmed display change reverted."));
}
void UPFSettingsMenu::NativeTick(const FGeometry& G,float D)
{
    Super::NativeTick(G,D);
    if(ConfirmDeadline>0){const double Remaining=ConfirmDeadline-FPlatformTime::Seconds();Status->SetText(FText::FromString(FString::Printf(TEXT("Keep display? Apply/A to confirm, Back/B to revert. %.0f seconds."),FMath::Max(0.,Remaining))));if(Remaining<=0){RevertDisplay();RefreshRows();Status->SetText(FText::FromString(TEXT("Display reverted after timeout.")));}}
}
void UPFSettingsMenu::Cancel(){RevertDisplay();RemoveFromParent();if(ReturnFocus){ReturnFocus->SetKeyboardFocus();}}
void UPFSettingsMenu::Defaults(){if(Draft && ConfirmDeadline<=0){const auto Resolution=Draft->GetScreenResolution();const auto Mode=Draft->GetFullscreenMode();Draft->SetToDefaults();Draft->SetScreenResolution(Resolution);Draft->SetFullscreenMode(Mode);RefreshRows();Status->SetText(FText::FromString(TEXT("Defaults selected. Apply to keep, Cancel to discard.")));}}
void UPFSettingsMenu::NativeDestruct(){RevertDisplay();Super::NativeDestruct();}
FReply UPFSettingsMenu::NativeOnPreviewKeyDown(const FGeometry& G,const FKeyEvent& E)
{
    const auto K=E.GetKey();
    if(K==EKeys::Escape || K==EKeys::Gamepad_FaceButton_Right){Cancel();return FReply::Handled();}
    if(K==EKeys::Enter || K==EKeys::Gamepad_FaceButton_Bottom){if(!E.IsRepeat()){Apply();}return FReply::Handled();}
    if(K==EKeys::Gamepad_FaceButton_Top){Defaults();return FReply::Handled();}
    if(K==EKeys::Gamepad_LeftShoulder || K==EKeys::Gamepad_RightShoulder){Category=Cycle(Category,K==EKeys::Gamepad_LeftShoulder?-1:1,4);Selected=0;RefreshRows();return FReply::Handled();}
    if(K==EKeys::Up || K==EKeys::Gamepad_DPad_Up || K==EKeys::Down || K==EKeys::Gamepad_DPad_Down){if(Rows.IsEmpty()){return FReply::Handled();}Selected=Cycle(Selected,(K==EKeys::Up || K==EKeys::Gamepad_DPad_Up)?-1:1,Rows.Num());for(int32 J=0;J<Rows.Num();++J){Rows[J]->SetCaption(Caption(J),J==Selected);}Scroll->ScrollWidgetIntoView(Rows[Selected],false);return FReply::Handled();}
    if(K==EKeys::Left || K==EKeys::Gamepad_DPad_Left || K==EKeys::Right || K==EKeys::Gamepad_DPad_Right){Change(Selected,(K==EKeys::Left || K==EKeys::Gamepad_DPad_Left)?-1:1);return FReply::Handled();}
    return Super::NativeOnPreviewKeyDown(G,E);
}
