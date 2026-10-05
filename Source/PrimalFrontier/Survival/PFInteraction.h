#pragma once
#include "CoreMinimal.h"
class APawn;
class AActor;
namespace PFInteraction
{
    // Same server-derived view query for prompts and authoritative interaction.
    PRIMALFRONTIER_API AActor* FindTarget(const APawn* Pawn, float Reach=250.f);
}
