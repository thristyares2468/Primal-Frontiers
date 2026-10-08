// PFCraftingHUD.h
//
// Placeholder crafting overlay (C / gamepad Y), beside the first-person view. Read-only:
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
    /** Refresh at 10 Hz while open; collapse immediately when closed. */
    virtual void NativeTick(const FGeometry& Geometry,float Delta) override;

private:
    UPROPERTY() TObjectPtr<UBorder> Panel;
    UPROPERTY() TObjectPtr<UTextBlock> Text;
    UPROPERTY() TObjectPtr<UTextBlock> Heading;
    UPROPERTY() TObjectPtr<UTextBlock> Result;
    float Refresh=0;
    bool bWasOpen=false;
};
