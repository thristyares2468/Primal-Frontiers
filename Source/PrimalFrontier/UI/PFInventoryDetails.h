#pragma once
#include "CoreMinimal.h"
struct FPFItemStack;
struct FPFItemDefinition;

/** Read-only catalog/batch presentation; availability never acknowledges a server action. */
namespace PFInventoryDetails
{
    struct FView
    {
        FText Title;
        FText Body;
    };
    FView Describe(const FPFItemStack* Stack,const FPFItemDefinition* Definition,double ServerTime);
}
