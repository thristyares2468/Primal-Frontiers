#pragma once
#include "Blueprint/UserWidget.h"
#include "PFBuildingHUD.generated.h"
class UBorder;
class UTextBlock;
UCLASS()
class PRIMALFRONTIER_API UPFBuildingHUD : public UUserWidget
{
    GENERATED_BODY()
protected:
    virtual void NativeOnInitialized() override;
    virtual void NativeTick(const FGeometry& Geometry,float Delta) override;
private:
    UPROPERTY() TObjectPtr<UBorder> Panel;
    UPROPERTY() TObjectPtr<UTextBlock> Text;
};
