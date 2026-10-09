#include "UI/PFRecipeDetails.h"
#include "Crafting/PFCraftingCatalog.h"
#include "Inventory/PFInventoryComponent.h"
#include "Inventory/PFItemCatalog.h"

PFRecipeDetails::FView PFRecipeDetails::Describe(const FPFRecipeDefinition* Recipe,const UPFInventoryComponent* Inventory,double ServerTime,bool bBusy)
{
    auto Unavailable=[](){return FView{FText::FromString(TEXT("RECIPE UNAVAILABLE")),FText::FromString(TEXT("Waiting for valid crafting/item data."))};};
    if(!Recipe || !Inventory || !FMath::IsFinite(ServerTime)){return Unavailable();}
    const auto* Output=Inventory->Definition(Recipe->Output);if(!Output){return Unavailable();}
    TArray<FString> Ingredients;bool bMissing=false;
    for(const auto& Need:Recipe->Ingredients)
    {
        const auto* Item=Inventory->Definition(Need.ItemId);if(!Item){return Unavailable();}
        int32 Have=0;
        for(const auto& Stack:Inventory->GetStacks())
        {
            if(Stack.ItemId==Need.ItemId && Stack.Quantity>0 && FMath::IsFinite(Stack.ExpiresAt) &&
                (Stack.ExpiresAt==0 || Stack.ExpiresAt>ServerTime)){Have+=Stack.Quantity;}
        }
        bMissing|=Have<Need.Quantity;
        Ingredients.Add(FString::Printf(TEXT("%s %d/%d"),*Item->DisplayName.ToString(),Have,Need.Quantity));
    }
    const FString State=bBusy?TEXT("Job running - wait or cancel"):
        bMissing?TEXT("Missing fresh ingredients"):TEXT("Ingredients present; server validates");
    return {FText::FromString(FString::Printf(TEXT("%s | %.1fs"),*Recipe->DisplayName.ToString(),Recipe->Duration)),
        FText::FromString(FString::Printf(TEXT("Makes %d %s\n%s\n%s"),Recipe->OutputQuantity,*Output->DisplayName.ToString(),*FString::Join(Ingredients,TEXT(" | ")),*State))};
}
