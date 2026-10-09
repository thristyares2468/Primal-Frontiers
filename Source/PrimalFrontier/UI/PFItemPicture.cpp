#include "UI/PFItemPicture.h"
#include "Inventory/PFItemCatalog.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Image.h"
#include "UI/PFUITheme.h"
#include "Rendering/DrawElements.h"

void UPFItemPicture::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    CatalogImage=WidgetTree->ConstructWidget<UImage>();WidgetTree->RootWidget=CatalogImage;
    SetVisibility(ESlateVisibility::HitTestInvisible);
}
void UPFItemPicture::SetItem(const FPFItemDefinition* Item)
{
    const FName NextId=Item?Item->Id:NAME_None;
    const FSoftObjectPath NextPath=Item?Item->Icon.ToSoftObjectPath():FSoftObjectPath();
    if(NextId==ItemId && NextPath==IconPath){return;}
    ItemId=NextId;IconPath=NextPath;
    bSketch=IconPath.IsNull() || IconPath.ToString()==TEXT("/Engine/EngineResources/WhiteSquareTexture.WhiteSquareTexture");
    CatalogImage->SetVisibility(bSketch?ESlateVisibility::Hidden:ESlateVisibility::HitTestInvisible);
    // Supported UImage asynchronous soft-texture path; only displayed rows load icons.
    if(!bSketch){CatalogImage->SetBrushFromSoftTexture(Item->Icon);}
    InvalidateLayoutAndVolatility();
}
int32 UPFItemPicture::NativePaint(const FPaintArgs& Args,const FGeometry& G,const FSlateRect& Clip,FSlateWindowElementList& Draw,int32 Layer,const FWidgetStyle& Style,bool bEnabled) const
{
    Layer=Super::NativePaint(Args,G,Clip,Draw,Layer,Style,bEnabled);
    if(!bSketch){return Layer;}
    ++Layer;
    const FVector2D Size=G.GetLocalSize();const float Side=FMath::Min(Size.X,Size.Y);
    const FVector2D Origin=(Size-FVector2D(Side,Side))*0.5;
    auto Box=[&](FVector2D At,FVector2D Extent,FLinearColor Color,float Radius)
    {
        const FSlateRoundedBoxBrush Brush(Color,Radius*Side);
        FSlateDrawElement::MakeBox(Draw,Layer,G.ToPaintGeometry(Extent*Side,FSlateLayoutTransform(Origin+At*Side)),&Brush,ESlateDrawEffect::None,Color*Style.GetColorAndOpacityTint());
    };
    auto Line=[&](TArray<FVector2D> Points,FLinearColor Color,float Width)
    {
        for(auto& Point:Points){Point=Origin+Point*Side;}
        FSlateDrawElement::MakeLines(Draw,Layer,G.ToPaintGeometry(),Points,ESlateDrawEffect::None,Color*Style.GetColorAndOpacityTint(),true,Width*Side);
    };
    Box({0,0},{1,1},PFUITheme::Inset,0.12f);
    if(ItemId==TEXT("Item_Tool") || ItemId==TEXT("Item_BoundTool"))
    {
        Line({{0.28,0.82},{0.68,0.21}},FLinearColor(0.53f,0.32f,0.14f,1),0.11f);
        Line({{0.23,0.25},{0.57,0.16},{0.81,0.37},{0.86,0.59}},FLinearColor(0.66f,0.72f,0.72f,1),0.16f);
        Line({{0.52,0.19},{0.69,0.31}},PFUITheme::Text,0.04f);
        if(ItemId==TEXT("Item_BoundTool")){Line({{0.52,0.27},{0.69,0.36},{0.49,0.32},{0.65,0.41}},PFUITheme::Accent,0.04f);}
    }
    else if(ItemId==TEXT("Item_Club") || ItemId==TEXT("Item_BoundClub"))
    {
        Line({{0.27,0.84},{0.64,0.28}},FLinearColor(0.53f,0.32f,0.14f,1),0.14f);
        Line({{0.54,0.39},{0.71,0.17}},FLinearColor(0.43f,0.25f,0.11f,1),0.25f);
        if(ItemId==TEXT("Item_BoundClub"))
        {Line({{0.50,0.35},{0.69,0.46}},PFUITheme::Accent,0.045f);Box({0.59,0.13},{0.25,0.23},PFUITheme::Muted,0.06f);}
    }
    else if(ItemId==TEXT("Item_WovenGuard"))
    {
        Box({0.20,0.15},{0.60,0.65},FLinearColor(0.42f,0.35f,0.20f,1),0.08f);
        for(float Y:{0.30f,0.45f,0.60f}){Line({{0.25,Y},{0.75,Y}},PFUITheme::Accent,0.045f);}
        for(float X:{0.35f,0.50f,0.65f}){Line({{X,0.20},{X,0.75}},PFUITheme::Muted,0.035f);}
    }
    else if(ItemId==TEXT("Item_Cord"))
    {
        Line({{0.74,0.76},{0.28,0.76},{0.18,0.54},{0.27,0.26},{0.68,0.23},{0.82,0.49},{0.72,0.66},{0.36,0.64},{0.32,0.43},{0.64,0.39}},FLinearColor(0.72f,0.60f,0.30f,1),0.07f);
    }
    else if(ItemId==TEXT("Item_CookedFood"))
    {
        Box({0.13,0.27},{0.74,0.52},FLinearColor(0.28f,0.13f,0.055f,1),0.25f);
        Box({0.19,0.24},{0.61,0.43},FLinearColor(0.78f,0.40f,0.18f,1),0.21f);
        Line({{0.33,0.33},{0.29,0.53}},FLinearColor(0.32f,0.16f,0.08f,1),0.035f);
        Line({{0.49,0.31},{0.43,0.56}},FLinearColor(0.32f,0.16f,0.08f,1),0.035f);
        Line({{0.65,0.33},{0.59,0.53}},FLinearColor(0.32f,0.16f,0.08f,1),0.035f);
    }
    else if(ItemId==TEXT("Item_DriedFood"))
    {
        for(int32 N=0;N<3;++N)
        {
            Box({0.18+0.22*N,0.18+0.06*(N%2)},{0.17,0.62},FLinearColor(0.54f,0.28f,0.12f,1),0.055f);
            Line({{0.25+0.22*N,0.26},{0.25+0.22*N,0.69}},FLinearColor(0.78f,0.50f,0.25f,1),0.022f);
        }
    }
    else
    {
        // Unknown/missing items have an honest neutral crate, never another item's picture.
        Box({0.2,0.25},{0.6,0.5},PFUITheme::Muted,0.04f);
        Line({{0.2,0.25},{0.8,0.75}},PFUITheme::Inset,0.045f);
        Line({{0.8,0.25},{0.2,0.75}},PFUITheme::Inset,0.045f);
    }
    return Layer;
}
