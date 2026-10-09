#pragma once
#include "CoreMinimal.h"
struct FPFProgressionRecord;
/** Read-only local-owner presentation. No grants, point spending or success claims. */
namespace PFProgressionDetails
{
    struct FView {FText Summary;FText RecipeReward;};
    FView Describe(const FPFProgressionRecord* Record,int32 AvailablePoints,FName ValidRecipe);
}
