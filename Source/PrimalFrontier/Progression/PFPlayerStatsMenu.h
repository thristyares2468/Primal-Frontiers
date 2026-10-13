#pragma once
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Progression/PFProgressionRecord.h"
#include "PFPlayerStatsMenu.generated.h"
class UTextBlock;
class UPFPlayerStatsMenu;

UCLASS()
class PRIMALFRONTIER_API UPFStatUpgradeButton : public UButton
{
    GENERATED_BODY()
public:
    void InitializeChoice(UPFPlayerStatsMenu* Menu,EPFPlayerStat Stat);
private:
    UPROPERTY() TObjectPtr<UPFPlayerStatsMenu> OwnerMenu;
    EPFPlayerStat Choice=EPFPlayerStat::Attack;
    UFUNCTION() void Choose();
};

/** Owner presentation only. Every click sends a stat ID, never points/bonuses/XP. */
UCLASS()
class PRIMALFRONTIER_API UPFPlayerStatsMenu : public UUserWidget
{
    GENERATED_BODY()
public:
    void RefreshMenu();
    void RequestUpgrade(EPFPlayerStat Stat);
    UFUNCTION() void CloseMenu();
protected:
    virtual void NativeOnInitialized() override;
    virtual void NativeTick(const FGeometry&,float Delta) override;
    virtual FReply NativeOnPreviewKeyDown(const FGeometry&,const FKeyEvent&) override;
private:
    UPROPERTY() TObjectPtr<UTextBlock> Heading;
    UPROPERTY() TObjectPtr<UTextBlock> Budgets;
    UPROPERTY() TObjectPtr<UTextBlock> Feedback;
    UPROPERTY() TObjectPtr<UTextBlock> Help;
    UPROPERTY() TObjectPtr<class UScrollBox> ChoicesScroll;
    UPROPERTY() TArray<TObjectPtr<UPFStatUpgradeButton>> Buttons;
    UPROPERTY() TArray<TObjectPtr<UTextBlock>> Labels;
    int32 Selected=0;
    float UntilRefresh=0;
};
