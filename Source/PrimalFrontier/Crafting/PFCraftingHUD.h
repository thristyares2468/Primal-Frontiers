// Local, centered crafting browser. All transactions use the existing server RPC.
#pragma once
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "GameplayTagContainer.h"
#include "PFCraftingHUD.generated.h"
class UBorder;
class UTextBlock;
class UVerticalBox;
class UPFItemPicture;
class UPFCraftingHUD;

/** Local catalog category, never an authoritative crafting parameter. */
UCLASS()
class PRIMALFRONTIER_API UPFRecipeCategoryButton : public UButton
{
    GENERATED_BODY()
public:
    UPFRecipeCategoryButton();
    void InitializeCategory(UPFCraftingHUD* Menu,FGameplayTag Tag);
private:
    UPROPERTY() TObjectPtr<UPFCraftingHUD> OwnerMenu;
    FGameplayTag Category;
    UFUNCTION() void Choose();
};

/** Dynamic recipe rows carry a lookup ID, never ingredients or output authority. */
UCLASS()
class PRIMALFRONTIER_API UPFRecipeChoiceButton : public UButton
{
    GENERATED_BODY()
public:
    UPFRecipeChoiceButton();
    void InitializeChoice(UPFCraftingHUD* Menu,FName Id);
private:
    UPROPERTY() TObjectPtr<UPFCraftingHUD> OwnerMenu;
    FName RecipeId;
    UFUNCTION() void Choose();
};

UCLASS()
class PRIMALFRONTIER_API UPFCraftingHUD : public UUserWidget
{
    GENERATED_BODY()
public:
    /** Shared by the actual modal footer and Pause help; these are UI-only controls. */
    static const TCHAR* NavigationHelp()
    {return TEXT("PgUp/PgDn / LB/RB: category | Arrows / D-pad: recipe | Enter / A / X: craft\nR / D-left: cancel | C / Y / B / Esc: close | 1/2/3: visible quick craft | P / Menu: pause. World keeps running.");}
    void RefreshMenu();
    void SelectRecipe(FName Id);
    void MoveSelection(int32 Direction);
    void SelectCategory(FGameplayTag Tag);
    void MoveCategory(int32 Direction);
    FGameplayTag GetSelectedCategory() const {return CategoryFilter;}
    const TArray<FName>& GetVisibleRecipeIds() const {return RecipeIds;}
    FName GetSelectedRecipe() const {return SelectedRecipe;}
    UFUNCTION() void CraftSelected();
    UFUNCTION() void CancelCraft();
    UFUNCTION() void CloseMenu();
protected:
    virtual void NativeOnInitialized() override;
    virtual void NativeTick(const FGeometry& Geometry,float Delta) override;
    virtual FReply NativeOnPreviewKeyDown(const FGeometry&,const FKeyEvent&) override;
    virtual FReply NativeOnMouseButtonDown(const FGeometry&,const FPointerEvent&) override;
private:
    void RebuildRecipes(const TArray<FName>& Ids);
    void RebuildCategories(const TArray<FGameplayTag>& Tags);
    UPROPERTY() TObjectPtr<UBorder> Panel;
    UPROPERTY() TObjectPtr<UVerticalBox> RecipeList;
    UPROPERTY() TObjectPtr<class UHorizontalBox> Categories;
    UPROPERTY() TArray<TObjectPtr<UPFRecipeCategoryButton>> CategoryButtons;
    UPROPERTY() TArray<TObjectPtr<UTextBlock>> CategoryTitles;
    UPROPERTY() TArray<TObjectPtr<UPFRecipeChoiceButton>> RecipeButtons;
    UPROPERTY() TArray<TObjectPtr<UPFItemPicture>> RecipePictures;
    UPROPERTY() TArray<TObjectPtr<UTextBlock>> RecipeTitles;
    UPROPERTY() TArray<TObjectPtr<UTextBlock>> RecipeBodies;
    UPROPERTY() TObjectPtr<UTextBlock> Heading;
    UPROPERTY() TObjectPtr<UTextBlock> ProgressionSummary;
    UPROPERTY() TObjectPtr<UTextBlock> ProgressionReward;
    UPROPERTY() TObjectPtr<UTextBlock> DetailTitle;
    UPROPERTY() TObjectPtr<UTextBlock> DetailBody;
    UPROPERTY() TObjectPtr<UTextBlock> ItemStats;
    UPROPERTY() TObjectPtr<UTextBlock> Result;
    UPROPERTY() TObjectPtr<UTextBlock> Hints;
    UPROPERTY() TObjectPtr<UPFItemPicture> Picture;
    UPROPERTY() TObjectPtr<UButton> CraftButton;
    UPROPERTY() TObjectPtr<UButton> CancelButton;
    UPROPERTY() TArray<TObjectPtr<UTextBlock>> ButtonLabels;
    TArray<FName> RecipeIds;
    TArray<FGameplayTag> CategoryIds;
    FGameplayTag CategoryFilter;
    FName SelectedRecipe;
    bool bInitialChoiceMade=false;
    float Refresh=0;
};
