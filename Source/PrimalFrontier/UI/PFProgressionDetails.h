#pragma once
#include "CoreMinimal.h"
struct FPFProgressionRecord;
struct FPFKnowledgeDefinition;
class UPFProgressionCatalog;
class UPFCraftingCatalog;
class UPFItemCatalog;
/** Read-only local-owner presentation. No grants, point spending or success claims. */
namespace PFProgressionDetails
{
    struct FView {FText Summary;FText RecipeReward;};
    FView Describe(const FPFProgressionRecord* Record,int32 AvailablePoints,FName ValidRecipe);
    struct FKnowledgeView {FText Requirement;FText Button;bool bCanLearn=false;bool bLearned=false;};
    /** Evaluates a copy through the shared transaction; never changes the owner's record. */
    FKnowledgeView DescribeKnowledge(const FPFProgressionRecord* Record,const FPFKnowledgeDefinition* Definition,
        const UPFProgressionCatalog* Catalog,const UPFCraftingCatalog* Crafting,const UPFItemCatalog* Items);
}
