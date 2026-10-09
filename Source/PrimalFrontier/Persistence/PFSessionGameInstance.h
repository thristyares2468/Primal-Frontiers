#pragma once
#include "Engine/GameInstance.h"
#include "PFSessionGameInstance.generated.h"

/** Explicit solo world travel intent; independent of persistent process arguments. */
UCLASS()
class PRIMALFRONTIER_API UPFSessionGameInstance : public UGameInstance
{
    GENERATED_BODY()
public:
    bool StartWorld(const FString& Slot,bool bLoad,FString& Error);
    bool ConsumeWorldRequest(FString& Slot,bool& bLoad);
    void ReturnToWorldMenu();
    FString MenuMessage;
private:
    FString PendingSlot;
    bool bPendingLoad=false,bHasRequest=false;
};
