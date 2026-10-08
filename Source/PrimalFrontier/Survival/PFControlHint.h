#pragma once
#include "CoreMinimal.h"
#include "InputCoreTypes.h"

/** Read-only presentation of an actually registered binding, not a remapping preference. */
struct FPFControlHint
{
    FKey Key;
    FString Context;
    FString Action;
};
