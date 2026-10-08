#include "UI/PFReadOnlyOverlay.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

void PFReadOnlyOverlay::Build(UWidgetTree* Tree,FName Name,UBorder*& Panel,UTextBlock*& Heading,UTextBlock*& Body,UTextBlock*& Result)
{
    auto* Canvas=Tree->ConstructWidget<UCanvasPanel>();Tree->RootWidget=Canvas;
    Panel=Tree->ConstructWidget<UBorder>(UBorder::StaticClass(),Name);
    auto* Placement=Canvas->AddChildToCanvas(Panel);
    // Stretch within the right-hand side. Leave centre aim and left vitals clear.
    Placement->SetAnchors(FAnchors(0.57f,0.03f,0.985f,0.95f));
    Placement->SetOffsets(FMargin(0));
    Panel->SetBrushColor(FLinearColor(0.02f,0.02f,0.02f,0.94f));
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
