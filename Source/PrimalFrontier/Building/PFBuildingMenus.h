#pragma once
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "PFBuildingMenus.generated.h"
class UPFBuildingMenu;
class UPFStorageMenu;
class UWidgetSwitcher;
class UVerticalBox;
class UTextBlock;
class UPFInventoryComponent;
class APFSurvivalPlayerController;

UCLASS()
class PRIMALFRONTIER_API UPFBuildChoiceButton : public UButton
{
    GENERATED_BODY()
public:
    void InitializeChoice(UPFBuildingMenu* Menu,FName Id);
private:
    UPROPERTY() TObjectPtr<UPFBuildingMenu> Menu;
    FName Definition;
    UFUNCTION() void Choose();
};

/** Local selector only; actual placement still uses the authoritative building component. */
UCLASS()
class PRIMALFRONTIER_API UPFBuildingMenu : public UUserWidget
{
    GENERATED_BODY()
public:
    void RefreshMenu();
    void Choose(FName Id);
protected:
    virtual void NativeOnInitialized() override;
    virtual FReply NativeOnKeyDown(const FGeometry& Geometry,const FKeyEvent& Event) override;
private:
    UPROPERTY() TObjectPtr<UVerticalBox> Content;
    UFUNCTION() void CloseMenu();
};

UCLASS()
class PRIMALFRONTIER_API UPFStorageTransferButton : public UButton
{
    GENERATED_BODY()
public:
    void InitializeTransfer(APFSurvivalPlayerController* PC,bool bDeposit,FGuid Id,int32 Quantity);
private:
    TWeakObjectPtr<APFSurvivalPlayerController> Controller;
    FGuid Stack;
    bool bDeposit=false;
    int32 Quantity=0;
    UFUNCTION() void Transfer();
};

/** Owner-private bag/chest rows. No grants, predicted inventory edits or world pause. */
UCLASS()
class PRIMALFRONTIER_API UPFStorageFilterChoiceButton : public UButton
{
    GENERATED_BODY()
public:
    void InitializeChoice(UPFStorageMenu* Menu,FName Id);
private:
    UPROPERTY() TObjectPtr<UPFStorageMenu> OwnerMenu;
    FName ItemId;
    UFUNCTION() void Toggle();
};

UCLASS()
class PRIMALFRONTIER_API UPFStorageMenu : public UUserWidget
{
    GENERATED_BODY()
public:
    void RefreshMenu();
    void ToggleFilter(FName Id);
    void ResetView();
protected:
    virtual void NativeOnInitialized() override;
    virtual void NativeTick(const FGeometry& Geometry,float Delta) override;
    virtual FReply NativeOnKeyDown(const FGeometry& Geometry,const FKeyEvent& Event) override;
private:
    UPROPERTY() TObjectPtr<UVerticalBox> BagRows;
    UPROPERTY() TObjectPtr<UVerticalBox> ChestRows;
    UPROPERTY() TObjectPtr<UTextBlock> Status;
    UPROPERTY() TObjectPtr<UWidgetSwitcher> Pages;
    UPROPERTY() TObjectPtr<UVerticalBox> FilterRows;
    bool bDraftRestricted=false;
    TArray<FName> DraftAllowed;
    UPROPERTY() TArray<TObjectPtr<UTextBlock>> BagLabels;
    UPROPERTY() TArray<TObjectPtr<UTextBlock>> ChestLabels;
    FString Signature;
    float Refresh=0;
    void BuildRows(UPFInventoryComponent* Inventory,bool bDeposit,UVerticalBox* Rows,TArray<TObjectPtr<UTextBlock>>& Labels);
    void UpdateLabels(UPFInventoryComponent* Inventory,const TArray<TObjectPtr<UTextBlock>>& Labels);
    UFUNCTION() void CloseMenu();
    UFUNCTION() void SortChest();
    UFUNCTION() void QuickStore();
    UFUNCTION() void OpenSettings();
    UFUNCTION() void ToggleFilterMode();
    UFUNCTION() void ApplySettings();
    UFUNCTION() void CancelSettings();
    void RefreshFilters();
};
