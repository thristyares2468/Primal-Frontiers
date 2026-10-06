// Copyright Epic Games, Inc. All Rights Reserved.
// Project note: unused template GameMode. Survival maps use APFSurvivalGameMode
// (Survival/PFSurvivalGameMode.h) via BP_SurvivalGameMode.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PrimalFrontierGameMode.generated.h"

/**
 *  Simple GameMode for a first person game
 */
UCLASS(abstract)
class APrimalFrontierGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	APrimalFrontierGameMode();
};



