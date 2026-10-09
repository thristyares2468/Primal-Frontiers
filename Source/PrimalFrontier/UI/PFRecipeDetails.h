#pragma once
#include "CoreMinimal.h"
class UPFInventoryComponent;
struct FPFRecipeDefinition;

/** Read-only view of validated recipes and still-fresh replicated batches. */
namespace PFRecipeDetails
{
    struct FView {FText Title;FText Body;};
    FView Describe(const FPFRecipeDefinition* Recipe,const UPFInventoryComponent* Inventory,double ServerTime,bool bBusy);
}
