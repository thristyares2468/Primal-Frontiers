#include "Crafting/PFEryndorProgression.h"
#include "Crafting/PFCraftingCatalog.h"
#include "Crafting/PFCraftingStation.h"
#include "Inventory/PFItemCatalog.h"
#include "Progression/PFProgressionCatalog.h"
#include "NativeGameplayTags.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_EryndorMaterial,"Progression.Knowledge.Material");
namespace PFEryndor
{
bool IsMineral(FName Item)
{
    static const TArray<FName> Ids={TEXT("Item_Coal"),TEXT("Item_Flint"),TEXT("Item_IronOre"),TEXT("Item_Diamond"),TEXT("Item_GoldOre"),TEXT("Item_SilverOre"),TEXT("Item_Quartz"),TEXT("Item_Obsidian"),TEXT("Item_TitaniumOre")};
    return Ids.Contains(Item);
}
void AddItems(UPFItemCatalog& C)
{
    const auto Resource=FGameplayTag::RequestGameplayTag(TEXT("Item.Category.Resource"));
    auto Add=[&](const TCHAR* Id,const TCHAR* Name,float Kg)
    {FPFItemDefinition D=*C.Find(TEXT("Item_Stone"));D.Id=Id;D.DisplayName=FText::FromString(Name);D.Category=Resource;D.Weight=Kg;C.Items.Add(D);};
    Add(TEXT("Item_Coal"),TEXT("Coal"),.3f);Add(TEXT("Item_Flint"),TEXT("Flint"),.25f);
    Add(TEXT("Item_IronOre"),TEXT("Iron ore"),.6f);Add(TEXT("Item_IronIngot"),TEXT("Iron ingot"),.8f);
    Add(TEXT("Item_Diamond"),TEXT("Diamond"),.1f);
    Add(TEXT("Item_GoldOre"),TEXT("Gold ore"),.7f);Add(TEXT("Item_GoldIngot"),TEXT("Gold ingot"),.9f);
    Add(TEXT("Item_SilverOre"),TEXT("Silver ore"),.6f);Add(TEXT("Item_SilverIngot"),TEXT("Silver ingot"),.8f);
    Add(TEXT("Item_Quartz"),TEXT("Quartz"),.3f);Add(TEXT("Item_Obsidian"),TEXT("Obsidian"),.4f);
    Add(TEXT("Item_TitaniumOre"),TEXT("Titanium ore"),.4f);Add(TEXT("Item_TitaniumIngot"),TEXT("Titanium ingot"),.5f);
    Add(TEXT("Item_ConductiveParts"),TEXT("Conductive parts"),.2f);Add(TEXT("Item_QuartzLens"),TEXT("Quartz lens"),.15f);
    auto Pick=[&](const TCHAR* Id,const TCHAR* Name,EPFHarvestGrade Grade,int32 Hits,float Kg)
    {FPFItemDefinition D=*C.Find(TEXT("Item_CopperPick"));D.Id=Id;D.DisplayName=FText::FromString(Name);D.HarvestGrade=Grade;D.GatheringHits=Hits;D.Weight=Kg;D.MeleeDamage=40+5*(Hits-4);C.Items.Add(D);};
    Pick(TEXT("Item_IronPick"),TEXT("Iron pick"),EPFHarvestGrade::Iron,5,3);
    Pick(TEXT("Item_DiamondPick"),TEXT("Diamond pick"),EPFHarvestGrade::Diamond,6,2.8f);
    Pick(TEXT("Item_TitaniumPick"),TEXT("Titanium pick"),EPFHarvestGrade::Titanium,7,2);
    FPFItemDefinition Blade=*C.Find(TEXT("Item_BoundClub"));Blade.Id=TEXT("Item_ObsidianBlade");Blade.DisplayName=FText::FromString(TEXT("Obsidian blade"));Blade.Weight=1.5f;Blade.MeleeDamage=70;C.Items.Add(Blade);
    FPFItemDefinition Scope=*C.Find(TEXT("Item_Cord"));Scope.Id=TEXT("Item_SurveyScope");Scope.DisplayName=FText::FromString(TEXT("Field survey scope"));
    Scope.Category=FGameplayTag::RequestGameplayTag(TEXT("Item.Category.Tool"));Scope.Weight=1.25f;Scope.StackLimit=1;Scope.OpticalMagnification=2;C.Items.Add(Scope);
}
void AddCrafting(UPFCraftingCatalog& C)
{
    const auto Material=FGameplayTag::RequestGameplayTag(TEXT("Recipe.Category.Material"));
    const auto Tool=FGameplayTag::RequestGameplayTag(TEXT("Recipe.Category.Tool"));
    const auto Weapon=FGameplayTag::RequestGameplayTag(TEXT("Recipe.Category.Weapon"));
    auto Recipe=[&](const TCHAR* Id,const TCHAR* Name,const TCHAR* Output,float Seconds,FGameplayTag Category,TArray<FPFCraftIngredient> Inputs)
    {FPFRecipeDefinition D;D.Id=Id;D.DisplayName=FText::FromString(Name);D.Output=Output;D.Duration=Seconds;D.Category=Category;D.Ingredients=MoveTemp(Inputs);C.Recipes.Add(D);};
    // Portable greybox metallurgy, explicitly labelled until physical stations are implemented.
    Recipe(TEXT("Recipe_IronRefine"),TEXT("Refine iron (portable greybox)"),TEXT("Item_IronIngot"),10,Material,{{TEXT("Item_IronOre"),2},{TEXT("Item_Coal"),1},{TEXT("Item_Flint"),1}});
    Recipe(TEXT("Recipe_IronPick"),TEXT("Iron pick"),TEXT("Item_IronPick"),12,Tool,{{TEXT("Item_IronIngot"),3},{TEXT("Item_CopperPick"),1},{TEXT("Item_Cord"),1}});
    Recipe(TEXT("Recipe_DiamondPick"),TEXT("Diamond pick"),TEXT("Item_DiamondPick"),14,Tool,{{TEXT("Item_Diamond"),3},{TEXT("Item_IronPick"),1},{TEXT("Item_Cord"),1}});
    Recipe(TEXT("Recipe_TitaniumRefine"),TEXT("Refine titanium (portable greybox)"),TEXT("Item_TitaniumIngot"),12,Material,{{TEXT("Item_TitaniumOre"),2},{TEXT("Item_Coal"),2},{TEXT("Item_Obsidian"),1}});
    Recipe(TEXT("Recipe_TitaniumPick"),TEXT("Titanium pick"),TEXT("Item_TitaniumPick"),16,Tool,{{TEXT("Item_TitaniumIngot"),3},{TEXT("Item_DiamondPick"),1},{TEXT("Item_Cord"),1}});
    Recipe(TEXT("Recipe_GoldRefine"),TEXT("Refine gold (portable greybox)"),TEXT("Item_GoldIngot"),8,Material,{{TEXT("Item_GoldOre"),2},{TEXT("Item_Coal"),1}});
    Recipe(TEXT("Recipe_SilverRefine"),TEXT("Refine silver (portable greybox)"),TEXT("Item_SilverIngot"),8,Material,{{TEXT("Item_SilverOre"),2},{TEXT("Item_Coal"),1}});
    Recipe(TEXT("Recipe_ConductiveParts"),TEXT("Make conductive parts"),TEXT("Item_ConductiveParts"),8,Material,{{TEXT("Item_GoldIngot"),1},{TEXT("Item_SilverIngot"),1},{TEXT("Item_CopperIngot"),1}});
    Recipe(TEXT("Recipe_QuartzLens"),TEXT("Polish quartz lens"),TEXT("Item_QuartzLens"),8,Material,{{TEXT("Item_Quartz"),3},{TEXT("Item_Flint"),1}});
    Recipe(TEXT("Recipe_ObsidianBlade"),TEXT("Bind obsidian blade"),TEXT("Item_ObsidianBlade"),10,Weapon,{{TEXT("Item_Obsidian"),3},{TEXT("Item_Wood"),2},{TEXT("Item_Cord"),1}});
    // Physical batch processing halves fuel per ingot. Fuel is an exact ordinary input,
    // consumed only with the successful output; no separate station fuel timer/state.
    Recipe(TEXT("Recipe_CopperBatch"),TEXT("Batch refine copper (wood fuel)"),TEXT("Item_CopperIngot"),12,Material,{{TEXT("Item_CopperOre"),4},{TEXT("Item_Wood"),1}});
    C.Recipes.Last().OutputQuantity=2;C.Recipes.Last().RequiredStation=APFCraftingStation::RefineryType();
    Recipe(TEXT("Recipe_IronBatch"),TEXT("Batch refine iron (coal fuel)"),TEXT("Item_IronIngot"),15,Material,{{TEXT("Item_IronOre"),4},{TEXT("Item_Coal"),1},{TEXT("Item_Flint"),2}});
    C.Recipes.Last().OutputQuantity=2;C.Recipes.Last().RequiredStation=APFCraftingStation::RefineryType();
    Recipe(TEXT("Recipe_SurveyScope"),TEXT("Assemble field survey scope"),TEXT("Item_SurveyScope"),12,Tool,{{TEXT("Item_QuartzLens"),1},{TEXT("Item_CopperIngot"),2},{TEXT("Item_Cord"),1},{TEXT("Item_Wood"),2}});
    C.Recipes.Last().RequiredStation=APFCraftingStation::WorkbenchType();
    auto Node=[&](const TCHAR* Id,const TCHAR* Name,const TCHAR* Item,EPFHarvestGrade Grade)
    {FPFResourceDefinition D;D.Id=Id;D.DisplayName=FText::FromString(Name);D.YieldItem=Item;D.Hits=8;D.RespawnSeconds=45;D.HarvestRequirement.Family=PFHarvest::Pick();D.HarvestRequirement.MinimumGrade=Grade;C.Resources.Add(D);};
    Node(TEXT("Node_Coal"),TEXT("Coal seam"),TEXT("Item_Coal"),EPFHarvestGrade::Stone);
    Node(TEXT("Node_Flint"),TEXT("Flint outcrop"),TEXT("Item_Flint"),EPFHarvestGrade::Stone);
    Node(TEXT("Node_Iron"),TEXT("Iron vein"),TEXT("Item_IronOre"),EPFHarvestGrade::Stone);
    Node(TEXT("Node_Diamond"),TEXT("Diamond deposit"),TEXT("Item_Diamond"),EPFHarvestGrade::Iron);
    Node(TEXT("Node_Gold"),TEXT("Gold vein"),TEXT("Item_GoldOre"),EPFHarvestGrade::Iron);
    Node(TEXT("Node_Silver"),TEXT("Silver vein"),TEXT("Item_SilverOre"),EPFHarvestGrade::Iron);
    Node(TEXT("Node_Quartz"),TEXT("Quartz outcrop"),TEXT("Item_Quartz"),EPFHarvestGrade::Copper);
    Node(TEXT("Node_Obsidian"),TEXT("Obsidian outcrop"),TEXT("Item_Obsidian"),EPFHarvestGrade::Iron);
    Node(TEXT("Node_Titanium"),TEXT("Titanium vein"),TEXT("Item_TitaniumOre"),EPFHarvestGrade::Diamond);
}
void AddKnowledge(UPFProgressionCatalog& C)
{
    auto Add=[&](const TCHAR* Id,const TCHAR* Name,int32 Level,int32 Cost,const TCHAR* Parent,TArray<FName> Recipes)
    {FPFKnowledgeDefinition D;D.Id=Id;D.DisplayName=FText::FromString(Name);D.Category=TAG_EryndorMaterial;D.MinimumLevel=Level;D.PointCost=Cost;D.Prerequisites.Add(Parent);D.Recipes=MoveTemp(Recipes);C.Knowledge.Add(D);};
    Add(TEXT("Tech_Metalwork"),TEXT("Iron metalworking"),3,2,TEXT("Tech_FieldTools"),{TEXT("Recipe_IronRefine"),TEXT("Recipe_IronPick"),TEXT("Recipe_SilverRefine"),TEXT("Recipe_GoldRefine"),TEXT("Recipe_IronBatch")});
    // Unique baseline + metalwork + gem crafts can reach level4 without waiting
    // for another 30-minute gather window. Optional branches still compete for points.
    Add(TEXT("Tech_Gemwork"),TEXT("Gem cutting"),3,2,TEXT("Tech_Metalwork"),{TEXT("Recipe_DiamondPick"),TEXT("Recipe_QuartzLens"),TEXT("Recipe_ObsidianBlade"),TEXT("Recipe_SurveyScope")});
    Add(TEXT("Tech_Titanium"),TEXT("Titanium metallurgy"),4,3,TEXT("Tech_Gemwork"),{TEXT("Recipe_TitaniumRefine"),TEXT("Recipe_TitaniumPick")});
    Add(TEXT("Tech_Conductors"),TEXT("Conductive fabrication"),5,2,TEXT("Tech_Metalwork"),{TEXT("Recipe_ConductiveParts")});
}
}
