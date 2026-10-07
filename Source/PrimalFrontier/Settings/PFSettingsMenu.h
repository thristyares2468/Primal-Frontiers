#pragma once
#include "Blueprint/UserWidget.h"
#include "Settings/PFGameUserSettings.h"
#include "PFSettingsMenu.generated.h"
class UVerticalBox;
class UTextBlock;
class UScrollBox;
class UPFSettingsMenu;

/** A focusable value row. Buttons and keyboard/gamepad share the same mutation path. */
UCLASS()
class PRIMALFRONTIER_API UPFSettingsRow : public UUserWidget
{
    GENERATED_BODY()
public:
    void InitializeRow(UPFSettingsMenu* InOwner,int32 InIndex);
    void SetCaption(const FString& Caption,bool bSelected);
private:
    UPROPERTY() TObjectPtr<UPFSettingsMenu> Menu;
    UPROPERTY() TObjectPtr<UTextBlock> Label;
    int32 Index=0;
    UFUNCTION() void Previous();
    UFUNCTION() void Next();
};

UCLASS()
class PRIMALFRONTIER_API UPFSettingsMenu : public UUserWidget
{
    GENERATED_BODY()
public:
    void Change(int32 Index,int32 Direction);
    UPROPERTY() TObjectPtr<UUserWidget> ReturnFocus;
protected:
    virtual void NativeOnInitialized() override;
    virtual void NativeTick(const FGeometry&,float) override;
    virtual FReply NativeOnPreviewKeyDown(const FGeometry&,const FKeyEvent&) override;
    virtual void NativeDestruct() override;
private:
    UPROPERTY() TObjectPtr<UPFGameUserSettings> Draft;
    UPROPERTY() TObjectPtr<UVerticalBox> RowsPanel;
    UPROPERTY() TObjectPtr<UScrollBox> Scroll;
    UPROPERTY() TObjectPtr<UTextBlock> Status;
    UPROPERTY() TArray<TObjectPtr<UPFSettingsRow>> Rows;
    int32 Category=0, Selected=0;
    double ConfirmDeadline=0;
    FIntPoint PreviousResolution;
    EWindowMode::Type PreviousMode=EWindowMode::Windowed;
    void RefreshRows();
    void CopyFromLive();
    FString Caption(int32 Index) const;
    void RevertDisplay();
    UFUNCTION() void Game();
    UFUNCTION() void Graphics();
    UFUNCTION() void Audio();
    UFUNCTION() void Accessibility();
    UFUNCTION() void Apply();
    UFUNCTION() void Cancel();
    UFUNCTION() void Defaults();
};
