#pragma once
#include "Blueprint/UserWidget.h"
#include "PFHotbarHUD.generated.h"
class UBorder;
class UTextBlock;
class UPFItemPicture;

/** Read-only eight-slot presentation. No inventory, equipment or server mutations. */
UCLASS()
class PRIMALFRONTIER_API UPFHotbarHUD : public UUserWidget
{
    GENERATED_BODY()
protected:
    virtual void NativeOnInitialized() override;
    virtual void NativeTick(const FGeometry&,float Delta) override;
private:
    UPROPERTY() TArray<TObjectPtr<UBorder>> Frames;
    UPROPERTY() TArray<TObjectPtr<UTextBlock>> Counts;
    UPROPERTY() TArray<TObjectPtr<UPFItemPicture>> Pictures;
    UPROPERTY() TObjectPtr<UTextBlock> Hint;
    float Refresh=0;
};
