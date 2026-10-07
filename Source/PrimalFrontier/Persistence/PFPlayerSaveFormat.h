#pragma once

#include "CoreMinimal.h"

class UPFItemCatalog;

/** Plain records only: no actors, object paths, transient clocks or client requests. */
struct PRIMALFRONTIER_API FPFSavedItemStack
{
    FGuid StackId;
    FName ItemId;
    int32 Quantity = 0;
    // Capture adapters must subtract server time; loading must not grant a new shelf life.
    double RemainingFreshnessSeconds = 0;
};

/** V1 covers one player. World/structure saves and reconnect identity are not implemented. */
struct PRIMALFRONTIER_API FPFPlayerSaveData
{
    // Must eventually be assigned and matched by the server, never a display name/client claim.
    FGuid PlayerId;
    FVector Location = FVector::ZeroVector;
    FRotator Rotation = FRotator::ZeroRotator;
    float Health = 100;
    float Stamina = 100;
    float Hunger = 100;
    float Thirst = 100;
    TArray<FPFSavedItemStack> Inventory;
};

/** Trusted server configuration. Capacity and maximum vitals are not read from a save. */
struct PRIMALFRONTIER_API FPFPlayerSaveLimits
{
    int32 Slots = 8;
    double Weight = 30;
    float MaxHealth = 100;
    float MaxStamina = 100;
};

/** Bounded native Unreal archive codec. Does not access disk or mutate gameplay.
 *  CRC detects accidental corruption, not tampering/authentication. Only trusted server saves
 *  may be used by a future restoration adapter. Failed operations leave outputs unchanged. */
class PRIMALFRONTIER_API FPFPlayerSaveFormat
{
public:
    static constexpr uint32 CurrentVersion = 1;
    static constexpr int32 EnvelopeBytes = 16;
    static constexpr int32 MaxEncodedBytes = 64 * 1024;

    static bool Validate(const FPFPlayerSaveData& Data, const UPFItemCatalog& Catalog,
        const FPFPlayerSaveLimits& Limits, FString& Error);
    static bool Encode(const FPFPlayerSaveData& Data, const UPFItemCatalog& Catalog,
        const FPFPlayerSaveLimits& Limits, TArray<uint8>& OutBytes, FString& Error);
    static bool Decode(const TArray<uint8>& Bytes, const UPFItemCatalog& Catalog,
        const FPFPlayerSaveLimits& Limits, FPFPlayerSaveData& OutData, FString& Error);
};
