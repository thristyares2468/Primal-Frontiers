// Data-only M13 prerequisite. No live spawner consumes these policies yet.
#pragma once

#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "PFCreatureSpawnCatalog.generated.h"

class UPFCreatureCatalog;

USTRUCT(BlueprintType)
struct FPFCreatureSpawnEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly) FName CreatureId;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0", ClampMax="100")) int32 DayWeight=1;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0", ClampMax="100")) int32 NightWeight=1;
};

USTRUCT(BlueprintType)
struct FPFCreatureSpawnPolicy
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly) FGameplayTag Biome;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<FPFCreatureSpawnEntry> Entries;
    /** Includes living residents and corpses. Combined table budgets must not exceed eight. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="1", ClampMax="8")) int32 MaximumResidents=1;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="5", ClampMax="300")) float RespawnSeconds=30;
};

UCLASS(BlueprintType)
class PRIMALFRONTIER_API UPFCreatureSpawnCatalog : public UDataAsset
{
    GENERATED_BODY()

public:
    /** Proposed Shore/Woodland/Ridge defaults; no asset or active world change. */
    UPFCreatureSpawnCatalog();
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<FPFCreatureSpawnPolicy> Policies;

    /** Validate the entire policy catalog against known creature definitions. */
    bool Validate(const UPFCreatureCatalog& Creatures, FString& Error) const;
    /** Pure selection. Future authority supplies a zero-based roll; invalid input preserves OutId. */
    bool Choose(const UPFCreatureCatalog& Creatures, FGameplayTag Biome, FGameplayTag Phase,
        int32 Roll, FName& OutId, FString& Error) const;
};
