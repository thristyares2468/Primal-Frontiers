// PFInventoryHUD.h
//
// Themed inventory overlay (Tab / gamepad View), top-right of the screen.
// Read-only: lists the bag's stacks, slot/weight totals, selected batch details,
// the selected row and the latest server feedback. Selection and actions are
// handled by APFSurvivalPlayerController.
//
// History: M3 (340c538); M7 (b5a3165) gamepad hints. Docs: Docs/INVENTORY_M3.md

#pragma once
#include "Blueprint/UserWidget.h"
#include "PFInventoryHUD.generated.h"
class UTextBlock;
class UBorder;
UCLASS()
class PRIMALFRONTIER_API UPFInventoryHUD : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeOnInitialized() override;
    /** Refresh open presentation at 10 Hz; input/authority live in the controller. */
    virtual void NativeTick(const FGeometry& Geometry,float Delta) override;

private:
    UPROPERTY() TObjectPtr<UTextBlock> Text;
    UPROPERTY() TObjectPtr<UTextBlock> Heading;
    UPROPERTY() TObjectPtr<UTextBlock> Result;
    UPROPERTY() TObjectPtr<UTextBlock> DetailTitle;
    UPROPERTY() TObjectPtr<UTextBlock> DetailBody;
    UPROPERTY() TObjectPtr<UBorder> Panel;
    float Refresh=0;
    bool bWasOpen=false;
};
