// PFItemCatalog.cpp
//
// Default item definitions (greybox test values, not final balance) and the
// validating lookup. To add an item: add an Add(...) line here (or an entry in
// DA_ItemCatalog), give it a unique Item_ ID and a category tag, then reference
// that ID from recipes/resources/loot.

#include "Inventory/PFItemCatalog.h"
#include "NativeGameplayTags.h"
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PF_Resource,"Item.Category.Resource");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PF_Food,"Item.Category.Food");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PF_Tool,"Item.Category.Tool");

UPFItemCatalog::UPFItemCatalog()
{
    // Helper: append a nonperishable, inedible item; perishable/food fields are set after.
    auto Add = [&](FName Id, const TCHAR* Name, float Weight, FGameplayTag Category)
    {
        FPFItemDefinition D; D.Id=Id; D.DisplayName=FText::FromString(Name); D.Weight=Weight; D.Category=Category;
        D.Icon=TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Engine/EngineResources/WhiteSquareTexture.WhiteSquareTexture")));
        Items.Add(D);
    };

    // Resources (gathered from nodes; used by recipes and building costs).
    Add(TEXT("Item_Wood"),TEXT("Wood"),0.5f,TAG_PF_Resource);
    Add(TEXT("Item_Stone"),TEXT("Stone"),1.f,TAG_PF_Resource);

    // Found food: short 5-minute life so spoilage is visible in tests.
    Add(TEXT("Item_Food"),TEXT("Found food"),0.2f,TAG_PF_Food);
    Items.Last().StackLimit=10; Items.Last().ShelfLifeSeconds=300; Items.Last().FoodRecovery=35; Items.Last().WaterRecovery=10;

    // The primitive gathering tool: carrying one doubles gathering speed and raises melee damage.
    Add(TEXT("Item_Tool"),TEXT("Stone gathering tool"),2.f,TAG_PF_Tool);
    Items.Last().StackLimit=1;

    // Processed food (M4 recipes): more nutrition and longer shelf life than raw food.
    Add(TEXT("Item_CookedFood"),TEXT("Cooked food"),0.2f,TAG_PF_Food);
    Items.Last().StackLimit=10;Items.Last().ShelfLifeSeconds=900;Items.Last().FoodRecovery=55;Items.Last().WaterRecovery=10;
    Add(TEXT("Item_DriedFood"),TEXT("Dried food"),0.15f,TAG_PF_Food);
    Items.Last().StackLimit=10;Items.Last().ShelfLifeSeconds=1800;Items.Last().FoodRecovery=45;

    // M7 world resources: water is gathered and spent one portion at a time.
    Add(TEXT("Item_Fibre"),TEXT("Plant fibre"),0.1f,TAG_PF_Resource);
    Add(TEXT("Item_Water"),TEXT("Water portion (placeholder)"),0.5f,TAG_PF_Resource);
    Items.Last().StackLimit=10;Items.Last().WaterRecovery=35;
}

const FPFItemDefinition* UPFItemCatalog::Find(FName Id) const
{
    const FPFItemDefinition* Result=nullptr;
    for (const auto& D:Items)
    {
        if (D.Id!=Id) { continue; }
        // A second match (duplicate ID) or any out-of-range field invalidates the item entirely.
        if (Result || Id.IsNone() || !D.Category.IsValid() || D.StackLimit<1 || D.StackLimit>1000 ||
            !FMath::IsFinite(D.Weight) || D.Weight<=0 || !FMath::IsFinite(D.ShelfLifeSeconds) || D.ShelfLifeSeconds<0 || D.ShelfLifeSeconds>86400 ||
            !FMath::IsFinite(D.FoodRecovery) || D.FoodRecovery<0 || D.FoodRecovery>100 ||
            !FMath::IsFinite(D.WaterRecovery) || D.WaterRecovery<0 || D.WaterRecovery>100) { return nullptr; }
        Result=&D;
    }
    return Result;
}
