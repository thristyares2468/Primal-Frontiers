#pragma once
#include "Blueprint/UserWidget.h"
#include "PFPauseMenu.generated.h"
class UTextBlock;
class UButton;
UCLASS()
class PRIMALFRONTIER_API UPFPauseMenu : public UUserWidget
{
    GENERATED_BODY()
public:
    void Refresh();
protected:
    virtual void NativeOnInitialized() override;
    virtual FReply NativeOnPreviewKeyDown(const FGeometry&,const FKeyEvent&) override;
private:
    UPROPERTY() TObjectPtr<UTextBlock> Description;
    UPROPERTY() TObjectPtr<UTextBlock> QuitLabel;
    UPROPERTY() TObjectPtr<UButton> ResumeButton;
    UPROPERTY() TObjectPtr<UButton> QuitButton;
    bool bQuitSelected=false;
    void UpdateSelection();
    bool bConfirmQuit=false;
    UFUNCTION() void Resume();
    UFUNCTION() void Quit();
};
