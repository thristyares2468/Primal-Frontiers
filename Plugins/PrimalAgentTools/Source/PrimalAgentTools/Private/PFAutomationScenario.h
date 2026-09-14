#pragma once

#include "PFResults.h"

class UWorld;

namespace PF::AgentTools
{
inline const FName OwnerTag(TEXT("PrimalAgentTools.Scenario.v1"));
bool IsAutomationMap(const FString& PackageName);
FResult ResetScenario(UWorld* World);
FResult PlaceTestActor(UWorld* World);
}
