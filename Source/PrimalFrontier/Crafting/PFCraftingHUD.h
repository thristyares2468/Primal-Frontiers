// PFCraftingHUD.h
//
// Placeholder crafting overlay (C / gamepad Y), top-left of the screen. Read-only:
// lists the three greybox recipes with have/need counts per ingredient, the time
// left on the active job and the last crafting feedback. Hotkeys are handled by
// APFSurvivalPlayerController.
//
// History: M4 (e7ffd71); M7 (b5a3165) gamepad hints. Docs: Docs/GATHERING_CRAFTING_M4.md

#pragma once
#include "Blueprint/UserWidget.h"
#include "PFCraftingHUD.generated.h"
class UBorder;
class UTextBlock;
UCLASS()
class PRIMALFRONTIER_API UPFCraftingHUD : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeOnInitialized() override;
    /** Rebuild the text every frame while crafting is open; collapse otherwise. */
    virtual void NativeTick(const FGeometry& Geometry,float Delta) override;

private:
    UPROPERTY() TObjectPtr<UBorder> Panel;
    UPROPERTY() TObjectPtr<UTextBlock> Text;
};
