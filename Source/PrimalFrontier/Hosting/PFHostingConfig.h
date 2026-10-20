#pragma once
#include "CoreMinimal.h"

/** Bounded validation before loading the vendor SDK (whose parser fatals on bad input). */
struct PRIMALFRONTIER_API FPFHostingAgentConfig
{
    int32 ListenPort=0;
    static bool Parse(const FString& Json,FPFHostingAgentConfig& Out,FString& Error);
};
