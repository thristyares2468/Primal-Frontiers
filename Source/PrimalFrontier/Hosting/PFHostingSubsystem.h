#pragma once
#include "Subsystems/GameInstanceSubsystem.h"
#include "Containers/Ticker.h"
#include "PFHostingSubsystem.generated.h"

/** Explicit opt-in agent lifecycle only; hosted gameplay admission remains closed. */
UCLASS()
class PRIMALFRONTIER_API UPFHostingSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    static bool IsHostedRequested();
private:
    bool TickHosting(float DeltaSeconds);
    void StopHosting();
    void FailHosting(const TCHAR* Reason);
    FTSTicker::FDelegateHandle TickHandle;
    double StartedAt=0;
    int32 AgentPort=0;
    bool bSdkLoaded=false,bReady=false,bActive=false,bStopping=false;
};
