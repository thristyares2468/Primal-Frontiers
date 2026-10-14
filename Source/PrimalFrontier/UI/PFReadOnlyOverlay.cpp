#include "UI/PFReadOnlyOverlay.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "UI/PFUITheme.h"
#include "Components/Image.h"
#include "Brushes/SlateColorBrush.h"
#include "Engine/Texture2D.h"
#include "UI/PFUISprites.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/ButtonSlot.h"

UImage* PFUITheme::Icon(UWidgetTree* Tree,FName Id,float Size)
{
    auto* Image=Tree->ConstructWidget<UImage>();Image->SetVisibility(ESlateVisibility::HitTestInvisible);
    FSoftObjectPath Path;FBox2D Region;
    if(!IsRunningDedicatedServer() && PFUISprites::Resolve(Id,Path,Region))
    {
        TSoftObjectPtr<UTexture2D> Soft(Path);if(auto* Texture=Soft.LoadSynchronous())
        {FSlateBrush Brush;Brush.ImageSize=FVector2D(Size,Size);Brush.SetResourceObject(Texture);const bool Symbols=Path.ToString().Contains(TEXT("T_ExpeditionSymbols"));const bool Actions=Path.ToString().Contains(TEXT("T_ExpeditionActions"));const FVector2D Gutter(3.f/(Symbols?1271.f:Actions?1268.f:1254.f),3.f/(Symbols?1238.f:Actions?1241.f:1254.f));Brush.SetUVRegion(FBox2D(Region.Min+Gutter,Region.Max-Gutter));Image->SetBrush(Brush);}
    }
    return Image;
}
void PFUITheme::AddButtonIcon(UWidgetTree* Tree,UButton* Button,FName Id)
{
    auto* Caption=Button->GetContent();if(!Caption){return;}const FMargin Padding=CastChecked<UButtonSlot>(Caption->Slot)->GetPadding();Button->RemoveChild(Caption);
    auto* Row=Tree->ConstructWidget<UHorizontalBox>();Button->SetContent(Row);
    auto* Bounds=Tree->ConstructWidget<USizeBox>();Bounds->SetWidthOverride(36);Bounds->SetHeightOverride(36);Bounds->SetContent(Icon(Tree,Id));auto* ArtSlot=Row->AddChildToHorizontalBox(Bounds);ArtSlot->SetVerticalAlignment(VAlign_Center);ArtSlot->SetPadding(FMargin(0,0,8,0));
    auto* TextSlot=Row->AddChildToHorizontalBox(Caption);TextSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));TextSlot->SetVerticalAlignment(VAlign_Center);
    CastChecked<UButtonSlot>(Row->Slot)->SetHorizontalAlignment(HAlign_Fill);
    CastChecked<UButtonSlot>(Row->Slot)->SetPadding(Padding);
}

void PFUITheme::AddScrim(UWidgetTree* Tree,UCanvasPanel* Canvas)
{
    auto* Shade=Tree->ConstructWidget<UImage>();Shade->SetBrush(FSlateColorBrush(FLinearColor(.004f,.008f,.012f,.74f)));Shade->SetVisibility(ESlateVisibility::HitTestInvisible);
    auto* Placement=Canvas->AddChildToCanvas(Shade);Placement->SetAnchors(FAnchors(0,0,1,1));Placement->SetOffsets(FMargin(0));Placement->SetZOrder(-1);
}

void PFReadOnlyOverlay::Build(UWidgetTree* Tree,FName Name,UBorder*& Panel,UTextBlock*& Heading,UTextBlock*& Body,UTextBlock*& Result)
{
    auto* Canvas=Tree->ConstructWidget<UCanvasPanel>();Tree->RootWidget=Canvas;
    Panel=Tree->ConstructWidget<UBorder>(UBorder::StaticClass(),Name);
    auto* Placement=Canvas->AddChildToCanvas(Panel);
    // Stretch within the right-hand side. Leave centre aim and left vitals clear.
    Placement->SetAnchors(FAnchors(0.57f,0.03f,0.985f,0.95f));
    Placement->SetOffsets(FMargin(0));
    Panel->SetBrush(FSlateRoundedBoxBrush(PFUITheme::Surface,3.f,PFUITheme::Line,1.f));
    Panel->SetPadding(FMargin(16));Panel->SetClipping(EWidgetClipping::ClipToBounds);
    auto* Column=Tree->ConstructWidget<UVerticalBox>();Panel->SetContent(Column);
    auto Text=[&](const TCHAR* Suffix)
    {
        auto* Label=Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(),FName(*(Name.ToString()+Suffix)));
        Label->SetAutoWrapText(true);return Label;
    };
    Heading=Text(TEXT("_Heading"));Body=Text(TEXT("_Body"));Result=Text(TEXT("_Result"));
    Column->AddChildToVerticalBox(Heading)->SetPadding(FMargin(0,0,0,12));
    Column->AddChildToVerticalBox(Body)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    Column->AddChildToVerticalBox(Result)->SetPadding(FMargin(0,12,0,0));
    Result->SetColorAndOpacity(FLinearColor(1.f,0.85f,0.55f));
    SetScale(Heading,Body,Result,1);
}
void PFReadOnlyOverlay::SetScale(UTextBlock* Heading,UTextBlock* Body,UTextBlock* Result,float Scale)
{
    Scale=FMath::IsFinite(Scale)?FMath::Clamp(Scale,0.75f,1.5f):1;
    auto Font=[&](UTextBlock* Label,int32 Size){auto F=Label->GetFont();F.Size=FMath::RoundToInt(Size*Scale);if(Label->GetFont().Size!=F.Size){Label->SetFont(F);}};
    Font(Heading,18);Font(Body,22);Font(Result,22);
}
