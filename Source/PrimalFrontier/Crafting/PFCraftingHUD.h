// Local, centered crafting browser. All transactions use the existing server RPC.
#pragma once
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "PFCraftingHUD.generated.h"
class UBorder;
class UTextBlock;
class UVerticalBox;
class UPFItemPicture;
class UPFCraftingHUD;

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
    void RefreshMenu();
    void SelectRecipe(FName Id);
    void MoveSelection(int32 Direction);
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
    UPROPERTY() TObjectPtr<UBorder> Panel;
    UPROPERTY() TObjectPtr<UVerticalBox> RecipeList;
    UPROPERTY() TArray<TObjectPtr<UPFRecipeChoiceButton>> RecipeButtons;
    UPROPERTY() TArray<TObjectPtr<UPFItemPicture>> RecipePictures;
    UPROPERTY() TArray<TObjectPtr<UTextBlock>> RecipeTitles;
    UPROPERTY() TArray<TObjectPtr<UTextBlock>> RecipeBodies;
    UPROPERTY() TObjectPtr<UTextBlock> Heading;
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
    FName SelectedRecipe;
    bool bInitialChoiceMade=false;
    float Refresh=0;
};
