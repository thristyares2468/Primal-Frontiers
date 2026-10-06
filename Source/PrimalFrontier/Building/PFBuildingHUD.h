// PFBuildingHUD.h
//
// Placeholder build overlay (B / gamepad RB), top-left of the screen. Read-only:
// controls, selected piece/cost/rotation, preview validity, the aimed-at piece's
// health, open storage contents (with freshness) and the latest server result.
//
// History: M5 (5702d4b); M7 (b5a3165) gamepad hints. Docs: Docs/BUILDING_M5.md

#pragma once
#include "Blueprint/UserWidget.h"
#include "PFBuildingHUD.generated.h"
class UBorder;
class UTextBlock;
UCLASS()
class PRIMALFRONTIER_API UPFBuildingHUD : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeOnInitialized() override;
    /** Rebuild the text every frame while build mode is on; collapse otherwise. */
    virtual void NativeTick(const FGeometry& Geometry,float Delta) override;

private:
    UPROPERTY() TObjectPtr<UBorder> Panel;
    UPROPERTY() TObjectPtr<UTextBlock> Text;
};
