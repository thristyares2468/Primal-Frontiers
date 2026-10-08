#pragma once
#include "Blueprint/UserWidget.h"
#include "PFControlsMenu.generated.h"
class UTextBlock;
class UScrollBox;

/** Local read-only help. Uses the controller's registered bindings; never edits preferences. */
UCLASS()
class PRIMALFRONTIER_API UPFControlsMenu : public UUserWidget
{
    GENERATED_BODY()
public:
    UPROPERTY() TObjectPtr<UUserWidget> ReturnFocus;
    /** Shared Slate navigation path, also exercised by isolated automation. */
    FReply HandleNavigation(FKey Key,bool bRepeat=false);
    bool IsGamepadPage() const {return bGamepad;}
protected:
    virtual void NativeOnInitialized() override;
    virtual FReply NativeOnPreviewKeyDown(const FGeometry&,const FKeyEvent&) override;
private:
    UPROPERTY() TObjectPtr<UTextBlock> Heading;
    UPROPERTY() TObjectPtr<UTextBlock> Bindings;
    UPROPERTY() TObjectPtr<UScrollBox> Scroll;
    bool bGamepad=false;
    void Refresh();
    UFUNCTION() void Keyboard();
    UFUNCTION() void Controller();
    UFUNCTION() void Back();
};
