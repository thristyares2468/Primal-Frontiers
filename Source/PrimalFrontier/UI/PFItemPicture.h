// Cheap original greybox pictures; configured soft catalog icons replace sketches.
#pragma once
#include "Blueprint/UserWidget.h"
#include "PFItemPicture.generated.h"
struct FPFItemDefinition;
class UImage;
UCLASS()
class PRIMALFRONTIER_API UPFItemPicture : public UUserWidget
{
    GENERATED_BODY()
public:
    void SetItem(const FPFItemDefinition* Item);
    FName GetItemId() const {return ItemId;}
protected:
    virtual void NativeOnInitialized() override;
    virtual int32 NativePaint(const FPaintArgs&,const FGeometry&,const FSlateRect&,FSlateWindowElementList&,int32,const FWidgetStyle&,bool) const override;
private:
    UPROPERTY() TObjectPtr<UImage> CatalogImage;
    FName ItemId;
    FSoftObjectPath IconPath;
    bool bSketch=true;
};
