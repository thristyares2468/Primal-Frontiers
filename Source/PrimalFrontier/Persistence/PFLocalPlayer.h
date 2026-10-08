#pragma once
#include "Engine/LocalPlayer.h"
#include "PFLocalPlayer.generated.h"
UCLASS()
class PRIMALFRONTIER_API UPFLocalPlayer : public ULocalPlayer
{
    GENERATED_BODY()
public:
    virtual FString GetGameLoginOptions() const override;
    static bool RememberCredential(UWorld* World, FGuid Credential);
private:
    static FString ProfileSlot(UWorld* World);
};
