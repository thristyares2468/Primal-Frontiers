#include "UI/PFItemPicture.h"
#include "Inventory/PFItemCatalog.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Image.h"
#include "UI/PFUITheme.h"
#include "Rendering/DrawElements.h"
#include "Engine/Texture2D.h"
#include "UI/PFUISprites.h"

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
    // This supported setter cancels old soft streaming, so a late callback from
    // a previously selected custom icon cannot replace the new atlas sprite.
    CatalogImage->SetBrushFromTexture(nullptr);
    ItemId=NextId;IconPath=NextPath;
    bSketch=IconPath.IsNull() || IconPath.ToString()==TEXT("/Engine/EngineResources/WhiteSquareTexture.WhiteSquareTexture");
    // One original atlas, shared by every visible UImage. Custom catalog art wins.
    static const TArray<FName> Cells={TEXT("Item_Wood"),TEXT("Item_Stone"),TEXT("Item_Food"),TEXT("Item_Fibre"),TEXT("Item_Water"),TEXT("Item_CookedFood"),TEXT("Item_DriedFood"),TEXT("Item_Cord"),TEXT("Item_WoodTool"),TEXT("Item_Tool"),TEXT("Item_BoundTool"),TEXT("Item_CopperPick"),TEXT("Item_Club"),TEXT("Item_BoundClub"),TEXT("Item_WovenGuard"),TEXT("Item_CopperOre")};
    FName SpriteId=ItemId;
    const bool EryndorPick=ItemId==TEXT("Item_IronPick") || ItemId==TEXT("Item_DiamondPick") || ItemId==TEXT("Item_TitaniumPick");
    const bool EryndorIngot=ItemId==TEXT("Item_IronIngot") || ItemId==TEXT("Item_TitaniumIngot") || ItemId==TEXT("Item_GoldIngot") || ItemId==TEXT("Item_SilverIngot");
    static const TArray<FName> Minerals={TEXT("Item_Coal"),TEXT("Item_Flint"),TEXT("Item_IronOre"),TEXT("Item_Diamond"),TEXT("Item_GoldOre"),TEXT("Item_SilverOre"),TEXT("Item_Quartz"),TEXT("Item_Obsidian"),TEXT("Item_TitaniumOre")};
    if(EryndorPick){SpriteId=TEXT("Item_CopperPick");}
    if(Minerals.Contains(ItemId)){SpriteId=TEXT("Item_CopperOre");}
    if(ItemId==TEXT("Item_ObsidianBlade")){SpriteId=TEXT("Item_BoundClub");}
    if(ItemId==TEXT("Item_ConductiveParts")){SpriteId=TEXT("Item_Cord");}
    if(ItemId==TEXT("Item_QuartzLens")){SpriteId=TEXT("Item_Stone");}
    const int32 Cell=Cells.IndexOfByKey(SpriteId);
    if(bSketch && Cell!=INDEX_NONE){bSketch=!ApplyAtlas(TEXT("/Game/PrimalFrontier/UI/Textures/T_ExpeditionItems.T_ExpeditionItems"),Cell,4);}
    if(bSketch && (ItemId==TEXT("Item_CopperIngot") || EryndorIngot)){bSketch=!ApplyAtlas(TEXT("/Game/PrimalFrontier/UI/Textures/T_ExpeditionStructures.T_ExpeditionStructures"),8,3);}
    // Original category sprites are placeholders; exact names and capabilities stay visible.
    CatalogImage->SetColorAndOpacity(FLinearColor::White);
    if(bSketch==false && (EryndorPick || EryndorIngot || Minerals.Contains(ItemId)) && IconPath.ToString()==TEXT("/Engine/EngineResources/WhiteSquareTexture.WhiteSquareTexture"))
    {
        const FString Name=ItemId.ToString();
        const FLinearColor Tint=Name.Contains(TEXT("Gold"))?FLinearColor(1,.8f,.25f):Name.Contains(TEXT("Diamond"))?FLinearColor(.4f,.95f,1):Name.Contains(TEXT("Obsidian")) || Name.Contains(TEXT("Coal"))?FLinearColor(.3f,.3f,.4f):FLinearColor(.7f,.85f,1);
        CatalogImage->SetColorAndOpacity(Tint);
    }
    CatalogImage->SetVisibility(bSketch?ESlateVisibility::Hidden:ESlateVisibility::HitTestInvisible);
    // Supported UImage asynchronous soft-texture path; only displayed rows load icons.
    if(!bSketch && !IconPath.IsNull() && IconPath.ToString()!=TEXT("/Engine/EngineResources/WhiteSquareTexture.WhiteSquareTexture"))
    {FSlateBrush Brush;CatalogImage->SetBrush(Brush);CatalogImage->SetBrushFromSoftTexture(Item->Icon);}
    InvalidateLayoutAndVolatility();
}
void UPFItemPicture::SetStructure(FName Id)
{
    ItemId=Id;IconPath.Reset();
    CatalogImage->SetBrushFromTexture(nullptr);
    static const TArray<FName> Cells={TEXT("Build_Foundation"),TEXT("Build_Wall"),TEXT("Build_Floor"),TEXT("Build_Ceiling"),TEXT("Build_Door"),TEXT("Build_Storage"),TEXT("Build_Window"),TEXT("Build_Stairs")};
    bSketch=!ApplyAtlas(TEXT("/Game/PrimalFrontier/UI/Textures/T_ExpeditionStructures.T_ExpeditionStructures"),Cells.IndexOfByKey(Id),3);
    CatalogImage->SetVisibility(bSketch?ESlateVisibility::Hidden:ESlateVisibility::HitTestInvisible);InvalidateLayoutAndVolatility();
}
bool UPFItemPicture::ApplyAtlas(const TCHAR* Path,int32 Cell,int32 Columns)
{
    if(Cell<0 || Columns<1 || IsRunningDedicatedServer()){return false;}
    TSoftObjectPtr<UTexture2D> Atlas{FSoftObjectPath(Path)};
    auto* Texture=Atlas.LoadSynchronous();if(!Texture){return false;}
    FSlateBrush Brush;Brush.SetResourceObject(Texture);Brush.ImageSize=FVector2D(128,128);
    const float Width=1.f/Columns;const FVector2D Min((Cell%Columns)*Width,(Cell/Columns)*Width);
    // Exclude three border texels per cell; generated alpha may contain isolated
    // edge specks and bilinear sampling must never borrow a neighboring sprite.
    // The source is validated at1254px. An asynchronously compiling texture can
    // temporarily expose a small proxy size; that must not change sprite UVs.
    const FVector2D Gutter(3.f/1254.f,3.f/1254.f);
    Brush.SetUVRegion(FBox2D(Min+Gutter,Min+FVector2D(Width,Width)-Gutter));CatalogImage->SetBrush(Brush);return true;
}
void UPFItemPicture::SetEncounter(FName Id)
{
    if(ItemId==Id){return;}ItemId=Id;IconPath.Reset();CatalogImage->SetBrushFromTexture(nullptr);bSketch=true;
    FSoftObjectPath Path;FBox2D Region;
    if(!IsRunningDedicatedServer() && PFUISprites::Resolve(Id,Path,Region))
    {
        TSoftObjectPtr<UTexture2D> Soft(Path);if(auto* Texture=Soft.LoadSynchronous())
        {FSlateBrush Brush;Brush.SetResourceObject(Texture);Brush.ImageSize=FVector2D(128,128);const FVector2D Gutter(3.f/1254.f,3.f/1254.f);Brush.SetUVRegion(FBox2D(Region.Min+Gutter,Region.Max-Gutter));CatalogImage->SetBrush(Brush);bSketch=false;}
    }
    CatalogImage->SetVisibility(bSketch?ESlateVisibility::Hidden:ESlateVisibility::HitTestInvisible);InvalidateLayoutAndVolatility();
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
    if(ItemId.ToString().StartsWith(TEXT("Build_")))
    {
        const auto Timber=PFUITheme::Accent;
        if(ItemId==TEXT("Build_Stairs"))
        {for(int32 N=0;N<4;++N){Box({.16f+.17f*N,.72f-.15f*N},{.20f,.12f},Timber,.02f);}}
        else if(ItemId==TEXT("Build_Ceiling"))
        {Line({{.12,.65},{.5,.22},{.88,.65}},Timber,.10f);Line({{.15,.65},{.85,.65}},PFUITheme::Muted,.04f);}
        else if(ItemId==TEXT("Build_Foundation") || ItemId==TEXT("Build_Floor"))
        {Box({.12,.50},{.76,.20},Timber,.02f);for(float X:{.25f,.45f,.65f}){Line({{X,.51},{X,.69}},PFUITheme::Inset,.025f);}}
        else if(ItemId==TEXT("Build_Storage"))
        {Box({.14,.28},{.72,.48},Timber,.03f);Line({{.14,.46},{.86,.46}},PFUITheme::Inset,.025f);Box({.46,.40},{.08,.18},PFUITheme::Text,.01f);}
        else
        {Box({.18,.15},{.64,.70},Timber,.02f);if(ItemId==TEXT("Build_Window")){Box({.32,.28},{.36,.28},PFUITheme::Inset,.02f);}else if(ItemId==TEXT("Build_Door")){Box({.37,.33},{.26,.52},PFUITheme::Inset,.02f);}else{for(float X:{.34f,.5f,.66f}){Line({{X,.2},{X,.8}},PFUITheme::Inset,.025f);}}}
    }
    else if(ItemId==TEXT("Item_Tool") || ItemId==TEXT("Item_BoundTool") || ItemId==TEXT("Item_WoodTool") || ItemId==TEXT("Item_CopperPick"))
    {
        const FLinearColor Head=ItemId==TEXT("Item_CopperPick")?FLinearColor(0.84f,0.43f,0.22f,1):ItemId==TEXT("Item_WoodTool")?FLinearColor(0.53f,0.32f,0.14f,1):FLinearColor(0.66f,0.72f,0.72f,1);
        Line({{0.28,0.82},{0.68,0.21}},FLinearColor(0.53f,0.32f,0.14f,1),0.11f);
        Line({{0.23,0.25},{0.57,0.16},{0.81,0.37},{0.86,0.59}},Head,0.16f);
        Line({{0.52,0.19},{0.69,0.31}},PFUITheme::Text,0.04f);
        if(ItemId==TEXT("Item_BoundTool")){Line({{0.52,0.27},{0.69,0.36},{0.49,0.32},{0.65,0.41}},PFUITheme::Accent,0.04f);}
    }
    else if(ItemId==TEXT("Item_CopperOre"))
    {
        Box({0.19,0.23},{0.62,0.56},PFUITheme::Muted,0.16f);
        for(const FVector2D At:{FVector2D(.26,.35),FVector2D(.53,.28),FVector2D(.46,.59)}){Box(At,{.20,.18},FLinearColor(.84f,.43f,.22f,1),.06f);}
    }
    else if(ItemId==TEXT("Item_CopperIngot"))
    {
        Box({.18,.31},{.64,.40},FLinearColor(.63f,.29f,.13f,1),.06f);
        Box({.22,.27},{.56,.25},FLinearColor(.91f,.52f,.27f,1),.05f);
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
