// PFPauseMenu.h
//
// Placeholder pause menu (P / Esc / gamepad Menu). Local UI only:
//  - Standalone: the controller pauses the world while it is open.
//  - Multiplayer: the world keeps running and the menu says so.
// "End session" needs two activations because nothing is saved yet (M8 adds saving).
// Fully navigable with keyboard or gamepad (D-pad to select, A to confirm).
//
// History: M7 (b5a3165). Docs: Docs/DECISIONS.md (2026-10-04 pause).

#pragma once
#include "Blueprint/UserWidget.h"
#include "PFPauseMenu.generated.h"
class UTextBlock;
class UButton;
class UPFSettingsMenu;
UCLASS()
class PRIMALFRONTIER_API UPFPauseMenu : public UUserWidget
{
    GENERATED_BODY()

public:
    /** Reset selection/confirmation and update the paused-vs-multiplayer description. Call when opening. */
    void Refresh();

protected:
    /** Build the placeholder layout (title, description, Resume, End session, help line). */
    virtual void NativeOnInitialized() override;
    /** Keyboard/gamepad navigation; handled before child buttons see the key. */
    virtual FReply NativeOnPreviewKeyDown(const FGeometry&,const FKeyEvent&) override;

private:
    UPROPERTY() TObjectPtr<UTextBlock> Description;
    UPROPERTY() TObjectPtr<UTextBlock> QuitLabel;
    UPROPERTY() TObjectPtr<UButton> ResumeButton;
    UPROPERTY() TObjectPtr<UButton> QuitButton;
    UPROPERTY() TObjectPtr<UButton> SettingsButton;
    UPROPERTY() TObjectPtr<UPFSettingsMenu> SettingsMenu;
    int32 SelectedButton=0;
    bool bQuitSelected=false;   // gamepad highlight: false = Resume, true = End session
    /** Recolour the two buttons to show the highlighted one. */
    void UpdateSelection();
    bool bConfirmQuit=false;    // first End-session press arms it, the second quits
    UFUNCTION() void Resume();
    UFUNCTION() void Quit();
    UFUNCTION() void Settings();
};
