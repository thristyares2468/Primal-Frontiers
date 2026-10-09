// Original, texture-free survival UI styling. Presentation only; no gameplay state.
#pragma once
#include "CoreMinimal.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Styling/SlateTypes.h"

namespace PFUITheme
{
inline const FLinearColor Surface(0.025f,0.045f,0.043f,1);
inline const FLinearColor Inset(0.045f,0.075f,0.069f,1);
inline const FLinearColor Text(0.91f,0.94f,0.89f,1);
inline const FLinearColor Muted(0.63f,0.72f,0.68f,1);
inline const FLinearColor Accent(0.60f,0.80f,0.49f,1);
inline const FLinearColor Warning(1.f,0.77f,0.39f,1);

inline FButtonStyle NavigationButton(bool bSelected,bool bDestructive=false)
{
    const FLinearColor Idle=bDestructive?FLinearColor(0.14f,0.065f,0.052f,1):Inset;
    const FLinearColor Active(0.12f,0.23f,0.16f,1);
    FButtonStyle Style;
    Style.SetNormal(FSlateRoundedBoxBrush(bSelected?Active:Idle,8.f,bSelected?Accent:FLinearColor(0.12f,0.18f,0.15f,1),bSelected?2.f:1.f));
    Style.SetHovered(FSlateRoundedBoxBrush(Active,8.f,Accent,2.f));
    Style.SetPressed(FSlateRoundedBoxBrush(FLinearColor(0.18f,0.31f,0.20f,1),8.f,Accent,2.f));
    Style.SetNormalPadding(FMargin(18,12));
    Style.SetPressedPadding(FMargin(18,12));
    return Style;
}
}
