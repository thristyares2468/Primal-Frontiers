#include "UI/PFInventoryDetails.h"
#include "Inventory/PFInventoryComponent.h"
#include "Inventory/PFItemCatalog.h"

PFInventoryDetails::FView PFInventoryDetails::Describe(const FPFItemStack* Stack,const FPFItemDefinition* Definition,double ServerTime)
{
    if(!Stack){return {FText::FromString(TEXT("SELECT AN ITEM")),FText::FromString(TEXT("Up/Down or D-pad selects a stack.\nRemoved stacks never choose a replacement."))};}
    if(!Definition || Definition->Id!=Stack->ItemId || Stack->Quantity<=0 || !FMath::IsFinite(ServerTime) || !FMath::IsFinite(Stack->ExpiresAt))
    {return {FText::FromString(TEXT("DETAILS UNAVAILABLE")),FText::FromString(TEXT("Item data is not ready.\nActions are checked by the server."))};}

    // Category comes from the validated catalog, including designer-defined child tags.
    FString Category=Definition->Category.ToString();
    Category.RemoveFromStart(TEXT("Item.Category."));
    FString Body=FString::Printf(TEXT("%s | %d / %d per stack\n%.2f kg each | %.2f kg this batch\n"),*Category,Stack->Quantity,Definition->StackLimit,Definition->Weight,Definition->Weight*Stack->Quantity);
    const bool bExpired=Stack->ExpiresAt>0 && Stack->ExpiresAt<=ServerTime;
    if(bExpired){Body+=TEXT("Expired - waiting for server removal\nCannot consume an expired batch.");}
    else
    {
        Body+=Stack->ExpiresAt>0?FString::Printf(TEXT("Fresh for %ds (this batch)\n"),FMath::Max(1,FMath::CeilToInt(Stack->ExpiresAt-ServerTime))):TEXT("Does not expire\n");
        Body+=(Definition->FoodRecovery>0 || Definition->WaterRecovery>0)?FString::Printf(TEXT("One portion: +%.0f food / +%.0f water"),Definition->FoodRecovery,Definition->WaterRecovery):TEXT("Not consumable");
        if(Definition->CreatureHitReduction>0){Body+=FString::Printf(TEXT("\nCarried: %.0f%% creature hit reduction"),Definition->CreatureHitReduction*100);}
    }
    return {Definition->DisplayName,FText::FromString(Body)};
}
