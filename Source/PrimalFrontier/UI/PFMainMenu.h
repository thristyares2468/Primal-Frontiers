#pragma once
#include "Blueprint/UserWidget.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "PFMainMenu.generated.h"
class UEditableTextBox;
class UComboBoxString;
class UTextBlock;
class UVerticalBox;
class UPFSettingsMenu;

UCLASS()
class PRIMALFRONTIER_API UPFMainMenu : public UUserWidget
{
    GENERATED_BODY()
public:
    UFUNCTION()
    void RefreshWorlds();
protected:
    virtual void NativeOnInitialized() override;
    virtual FReply NativeOnPreviewKeyDown(const FGeometry&,const FKeyEvent&) override;
    virtual void NativeDestruct() override;
private:
    UPROPERTY() TObjectPtr<UVerticalBox> WelcomePanel;
    UPROPERTY() TObjectPtr<UVerticalBox> SoloPanel;
    UPROPERTY() TObjectPtr<UVerticalBox> MultiplayerPanel;
    UPROPERTY() TObjectPtr<UPFSettingsMenu> SettingsMenu;
    UPROPERTY() TObjectPtr<UEditableTextBox> WorldName;
    UPROPERTY() TObjectPtr<UEditableTextBox> RenameInput;
    UPROPERTY() TObjectPtr<UComboBoxString> Worlds;
    UPROPERTY() TObjectPtr<UTextBlock> Status;
    UFUNCTION() void CreateWorld();
    UFUNCTION() void LoadWorld();
    UFUNCTION() void RenameWorld();
    UFUNCTION() void SelectionChanged(FString SelectedWorldName,ESelectInfo::Type Type);
    UFUNCTION() void Quit();
    UFUNCTION() UWidget* WorldOption(FString WorldLabel);
    UFUNCTION() void SinglePlayer();
    UFUNCTION() void Multiplayer();
    UFUNCTION() void Back();
    UFUNCTION() void Settings();
    void ShowPanel(int32 Index);
};
UCLASS()
class PRIMALFRONTIER_API APFMainMenuPlayerController : public APlayerController
{
    GENERATED_BODY()
protected:
    virtual void BeginPlay() override;
private:
    UPROPERTY() TObjectPtr<UPFMainMenu> Menu;
};
UCLASS()
class PRIMALFRONTIER_API APFMainMenuGameMode : public AGameModeBase
{
    GENERATED_BODY()
public:
    APFMainMenuGameMode();
    virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
};
