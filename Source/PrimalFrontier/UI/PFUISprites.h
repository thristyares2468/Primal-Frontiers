// Original UI symbols only. FNames identify pictures, never gameplay authority/state.
#pragma once
#include "CoreMinimal.h"
#include "Styling/SlateBrush.h"
namespace PFUISprites
{
    PRIMALFRONTIER_API bool Resolve(FName Id,FSoftObjectPath& Texture,FBox2D& Region);
}
