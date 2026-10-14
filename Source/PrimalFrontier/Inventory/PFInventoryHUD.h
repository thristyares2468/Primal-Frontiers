// PFInventoryHUD.h
//
// Inventory owns Items and Player upgrades tabs. Controller owns input state;
// item/stat mutations still use the existing server-authoritative requests.
//
// History: M3 (340c538); M7 (b5a3165) gamepad hints. Docs: Docs/INVENTORY_M3.md

#pragma once
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "PFInventoryHUD.generated.h"
class UTextBlock;
class UBorder;
class UButton;
class UPFPlayerStatsMenu;
class UUniformGridPanel;
class UPFItemPicture;
class UScrollBox;
class UPFInventoryHUD;
UCLASS()
class PRIMALFRONTIER_API UPFInventorySlotButton : public UButton
{
    GENERATED_BODY()
public:
    void InitializeSlot(UPFInventoryHUD* InMenu,FGuid InId);
    FGuid StackId;
private:
    UPROPERTY() TObjectPtr<UPFInventoryHUD> Menu;
    UFUNCTION() void Select();
};
UCLASS()
class PRIMALFRONTIER_API UPFInventoryHUD : public UUserWidget
{
    GENERATED_BODY()
public:
    void SetPage(bool bStats);
    void SelectStack(FGuid Id);
    UPFInventorySlotButton* FindSlot(FGuid Id) const;

protected:
    virtual void NativeOnInitialized() override;
    /** Refresh open presentation at 10 Hz; input/authority live in the controller. */
    virtual void NativeTick(const FGeometry& Geometry,float Delta) override;
    virtual FReply NativeOnPreviewKeyDown(const FGeometry&,const FKeyEvent&) override;

private:
    UPROPERTY() TObjectPtr<UTextBlock> Text;
    UPROPERTY() TObjectPtr<UTextBlock> Heading;
    UPROPERTY() TObjectPtr<UTextBlock> Result;
    UPROPERTY() TObjectPtr<UTextBlock> DetailTitle;
    UPROPERTY() TObjectPtr<UTextBlock> DetailBody;
    UPROPERTY() TObjectPtr<UBorder> Panel;
    UPROPERTY() TObjectPtr<UButton> ItemsTab;
    UPROPERTY() TObjectPtr<UButton> StatsTab;
    UPROPERTY() TObjectPtr<UPFPlayerStatsMenu> StatsPage;
    UPROPERTY() TObjectPtr<UUniformGridPanel> Grid;
    UPROPERTY() TObjectPtr<UScrollBox> GridScroll;
    UPROPERTY() TObjectPtr<UPFItemPicture> SelectedPicture;
    UPROPERTY() TArray<TObjectPtr<UPFInventorySlotButton>> Slots;
    UPROPERTY() TArray<TObjectPtr<UTextBlock>> SlotLabels;
    UPROPERTY() TArray<TObjectPtr<UPFItemPicture>> SlotPictures;
    TArray<FGuid> DisplayedIds;
    FGuid LastShownSelection;
    UFUNCTION() void Consume();
    UFUNCTION() void Split();
    UFUNCTION() void Drop();
    UFUNCTION() void ShowItems();
    UFUNCTION() void ShowStats();
    float Refresh=0;
    bool bWasOpen=false;
};
