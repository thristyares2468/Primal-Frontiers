// PFCraftingCatalog.cpp
//
// Default recipes/resources (greybox test values) and validating lookups.
// Cooking/drying are "portable" for now (no station needed); stations, technology
// unlocks and durability are future work.

#include "Crafting/PFCraftingCatalog.h"
#include "Inventory/PFItemCatalog.h"
#include "NativeGameplayTags.h"
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PF_CraftTool,"Recipe.Category.Tool");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PF_CraftFood,"Recipe.Category.Food");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PF_CraftMaterial,"Recipe.Category.Material");

UPFCraftingCatalog::UPFCraftingCatalog()
{
    auto AddRecipe=[&](FName Id,const TCHAR* Name,FName Output,float Seconds,FGameplayTag Category,TArray<FPFCraftIngredient> Inputs)
    {
        FPFRecipeDefinition D;
        D.Id=Id;
        D.DisplayName=FText::FromString(Name);
        D.Output=Output;
        D.Duration=Seconds;
        D.Category=Category;
        D.Ingredients=MoveTemp(Inputs);
        Recipes.Add(D);
    };
    // AddRecipe(ID, display name, output item, seconds, category, {ingredients}). Wood doubles as cooking fuel.
    AddRecipe(TEXT("Recipe_Tool"),TEXT("Stone gathering tool"),TEXT("Item_Tool"),5,TAG_PF_CraftTool,{{TEXT("Item_Wood"),3},{TEXT("Item_Stone"),2}});
    AddRecipe(TEXT("Recipe_Cook"),TEXT("Cook food"),TEXT("Item_CookedFood"),6,TAG_PF_CraftFood,{{TEXT("Item_Food"),1},{TEXT("Item_Wood"),1}});
    AddRecipe(TEXT("Recipe_Dry"),TEXT("Dry food"),TEXT("Item_DriedFood"),10,TAG_PF_CraftFood,{{TEXT("Item_Food"),2},{TEXT("Item_Wood"),2}});
    AddRecipe(TEXT("Recipe_Cord"),TEXT("Twist fibre cord"),TEXT("Item_Cord"),4,TAG_PF_CraftMaterial,{{TEXT("Item_Fibre"),4}});
    AddRecipe(TEXT("Recipe_BoundTool"),TEXT("Bind stone tool"),TEXT("Item_BoundTool"),8,TAG_PF_CraftTool,{{TEXT("Item_Tool"),1},{TEXT("Item_Cord"),1},{TEXT("Item_Wood"),2}});

    // Resource nodes use the struct defaults: 2 items per hit, 3 hits, 20 s regrowth.
    auto AddResource=[&](FName Id,const TCHAR* Name,FName Item)
    {
        FPFResourceDefinition D;
        D.Id=Id;
        D.DisplayName=FText::FromString(Name);
        D.YieldItem=Item;
        Resources.Add(D);
    };
    AddResource(TEXT("Node_Wood"),TEXT("Wood pile"),TEXT("Item_Wood"));
    AddResource(TEXT("Node_Stone"),TEXT("Stone outcrop"),TEXT("Item_Stone"));
    AddResource(TEXT("Node_Food"),TEXT("Forage patch"),TEXT("Item_Food"));
    AddResource(TEXT("Node_Fibre"),TEXT("Fibre patch"),TEXT("Item_Fibre"));
    AddResource(TEXT("Node_Water"),TEXT("Freshwater collection"),TEXT("Item_Water"));
}

const FPFRecipeDefinition* UPFCraftingCatalog::Recipe(FName Id,const UPFItemCatalog* Items) const
{
    const FPFRecipeDefinition* Found=nullptr;
    for(const auto& D:Recipes)
    {
        if(D.Id!=Id){continue;}
        if(Found || Id.IsNone() || !Items || !D.Category.IsValid() || !Items->Find(D.Output) || D.OutputQuantity<1 || D.OutputQuantity>100 ||
            !FMath::IsFinite(D.Duration) || D.Duration<0.1f || D.Duration>600 || D.Ingredients.IsEmpty() || D.Ingredients.Num()>8){return nullptr;}
        // Ingredients: known items, sane quantities, no duplicates, never the output itself.
        TSet<FName> Seen;
        for(const auto& I:D.Ingredients)
        {
            if(!Items->Find(I.ItemId) || I.Quantity<1 || I.Quantity>1000 || Seen.Contains(I.ItemId) || I.ItemId==D.Output){return nullptr;}
            Seen.Add(I.ItemId);
        }
        Found=&D;
    }
    return Found;
}

const FPFResourceDefinition* UPFCraftingCatalog::Resource(FName Id,const UPFItemCatalog* Items) const
{
    const FPFResourceDefinition* Found=nullptr;
    for(const auto& D:Resources)
    {
        if(D.Id!=Id){continue;}
        if(Found || Id.IsNone() || !Items || !Items->Find(D.YieldItem) || D.YieldPerHit<1 || D.YieldPerHit>100 || D.Hits<1 || D.Hits>100 ||
            !FMath::IsFinite(D.RespawnSeconds) || D.RespawnSeconds<1 || D.RespawnSeconds>3600){return nullptr;}
        Found=&D;
    }
    return Found;
}
