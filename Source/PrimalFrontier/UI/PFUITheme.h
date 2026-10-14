// Original expedition UI palette. Presentation only; no gameplay state.
#pragma once
#include "CoreMinimal.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Styling/SlateTypes.h"
class UWidgetTree;
class UCanvasPanel;
class UImage;
class UButton;

namespace PFUITheme
{
PRIMALFRONTIER_API void AddScrim(UWidgetTree* Tree,UCanvasPanel* Canvas);
PRIMALFRONTIER_API UImage* Icon(UWidgetTree* Tree,FName Id,float Size=36);
PRIMALFRONTIER_API void AddButtonIcon(UWidgetTree* Tree,UButton* Button,FName Id);
inline const FLinearColor Surface(0.018f,0.025f,0.033f,0.98f);
inline const FLinearColor Inset(0.037f,0.049f,0.057f,1);
inline const FLinearColor Text(0.93f,0.91f,0.85f,1);
inline const FLinearColor Muted(0.61f,0.67f,0.68f,1);
inline const FLinearColor Accent(0.87f,0.59f,0.30f,1);
inline const FLinearColor Warning(1.f,0.69f,0.33f,1);
inline const FLinearColor Line(0.16f,0.20f,0.22f,1);

inline FButtonStyle NavigationButton(bool bSelected,bool bDestructive=false)
{
    const FLinearColor Idle=bDestructive?FLinearColor(0.14f,0.065f,0.052f,1):Inset;
    const FLinearColor Active(0.12f,0.105f,0.077f,1);
    FButtonStyle Style;
    Style.SetNormal(FSlateRoundedBoxBrush(bSelected?Active:Idle,3.f,bSelected?Accent:Line,bSelected?2.f:1.f));
    Style.SetHovered(FSlateRoundedBoxBrush(Active,3.f,Accent,2.f));
    Style.SetPressed(FSlateRoundedBoxBrush(FLinearColor(0.22f,0.15f,0.09f,1),3.f,Accent,2.f));
    Style.SetDisabled(FSlateRoundedBoxBrush(Surface,3.f,Line,1.f));
    Style.SetNormalPadding(FMargin(18,12));
    Style.SetPressedPadding(FMargin(18,12));
    return Style;
}
}
