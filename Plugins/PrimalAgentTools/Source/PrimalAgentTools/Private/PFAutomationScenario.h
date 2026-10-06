// PFAutomationScenario.h
//
// Editor test fixtures for the approved L_Automation map (PF.ResetTestWorld,
// PF.PlaceTestActor). Fixture actors are tagged with OwnerTag plus their role name,
// so only actors this tool created are ever touched. History: ccbad56.
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
