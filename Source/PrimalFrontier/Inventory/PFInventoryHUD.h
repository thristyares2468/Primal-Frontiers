// PFInventoryHUD.h
//
// Placeholder inventory overlay (Tab / gamepad View), top-right of the screen.
// Read-only: lists the bag's stacks, slot/weight totals, freshness countdowns,
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
    /** Rebuild the text every frame while the bag is open; collapse otherwise. */
    virtual void NativeTick(const FGeometry& Geometry,float Delta) override;

private:
    UPROPERTY() TObjectPtr<UTextBlock> Text;
    UPROPERTY() TObjectPtr<UBorder> Panel;
};
