#include "Inventory/PFItemCatalog.h"
#include "NativeGameplayTags.h"
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PF_Resource,"Item.Category.Resource");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PF_Food,"Item.Category.Food");
UPFItemCatalog::UPFItemCatalog()
{
    auto Add = [&](FName Id, const TCHAR* Name, float Weight, FGameplayTag Category)
    {
        FPFItemDefinition D; D.Id=Id; D.DisplayName=FText::FromString(Name); D.Weight=Weight; D.Category=Category;
        D.Icon=TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Engine/EngineResources/WhiteSquareTexture.WhiteSquareTexture")));
        Items.Add(D);
    };
    Add(TEXT("Item_Wood"),TEXT("Wood"),0.5f,TAG_PF_Resource);
    Add(TEXT("Item_Stone"),TEXT("Stone"),1.f,TAG_PF_Resource);
    Add(TEXT("Item_Food"),TEXT("Found food"),0.2f,TAG_PF_Food);
    Items.Last().StackLimit=10; Items.Last().ShelfLifeSeconds=300; Items.Last().FoodRecovery=35; Items.Last().WaterRecovery=10;
}
const FPFItemDefinition* UPFItemCatalog::Find(FName Id) const
{
    const FPFItemDefinition* Result=nullptr;
    for (const auto& D:Items)
    {
        if (D.Id!=Id) { continue; }
        if (Result || Id.IsNone() || !D.Category.IsValid() || D.StackLimit<1 || D.StackLimit>1000 ||
            !FMath::IsFinite(D.Weight) || D.Weight<=0 || !FMath::IsFinite(D.ShelfLifeSeconds) || D.ShelfLifeSeconds<0 || D.ShelfLifeSeconds>86400 ||
            !FMath::IsFinite(D.FoodRecovery) || D.FoodRecovery<0 || D.FoodRecovery>100 ||
            !FMath::IsFinite(D.WaterRecovery) || D.WaterRecovery<0 || D.WaterRecovery>100) { return nullptr; }
        Result=&D;
    }
    return Result;
}
