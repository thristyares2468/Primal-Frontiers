#pragma once
#include "CoreMinimal.h"
class UWidgetTree;
class UBorder;
class UTextBlock;

/** Native placeholder presentation only; no focus, input or gameplay authority. */
namespace PFReadOnlyOverlay
{
    void Build(UWidgetTree* Tree,FName Name,UBorder*& Panel,UTextBlock*& Heading,UTextBlock*& Body,UTextBlock*& Result);
    void SetScale(UTextBlock* Heading,UTextBlock* Body,UTextBlock* Result,float Scale);
}
