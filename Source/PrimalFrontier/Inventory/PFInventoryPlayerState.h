#pragma once
#include "GameFramework/PlayerState.h"
#include "PFInventoryPlayerState.generated.h"
class UPFInventoryComponent;
UCLASS()
class PRIMALFRONTIER_API APFInventoryPlayerState : public APlayerState
{
    GENERATED_BODY()
public:
    APFInventoryPlayerState();
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly) TObjectPtr<UPFInventoryComponent> Inventory;
};
