// PFInventoryHUD.h
//
// Inventory owns Items and Player upgrades tabs. Controller owns input state;
// item/stat mutations still use the existing server-authoritative requests.
//
// History: M3 (340c538); M7 (b5a3165) gamepad hints. Docs: Docs/INVENTORY_M3.md

#pragma once
#include "Blueprint/UserWidget.h"
#include "PFInventoryHUD.generated.h"
class UTextBlock;
class UBorder;
class UButton;
class UPFPlayerStatsMenu;
UCLASS()
class PRIMALFRONTIER_API UPFInventoryHUD : public UUserWidget
{
    GENERATED_BODY()
public:
    void SetPage(bool bStats);

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
    UFUNCTION() void ShowItems();
    UFUNCTION() void ShowStats();
    float Refresh=0;
    bool bWasOpen=false;
};
