#pragma once
#include "Blueprint/UserWidget.h"
#include "PFInventoryHUD.generated.h"
class UTextBlock;
class UBorder;
UCLASS()
class PRIMALFRONTIER_API UPFInventoryHUD : public UUserWidget
{
    GENERATED_BODY()
protected:
    virtual void NativeOnInitialized() override;
    virtual void NativeTick(const FGeometry& Geometry,float Delta) override;
private:
    UPROPERTY() TObjectPtr<UTextBlock> Text;
    UPROPERTY() TObjectPtr<UBorder> Panel;
};
