#pragma once
#include "CoreMinimal.h"

struct PRIMALFRONTIER_API FPFSavedFile
{
    TArray<uint8> Payload;
    uint64 Generation = 0;
    int64 SavedUtc = 0;
    bool bRecoveredBackup = false;
};

/** Two bounded, checksummed generations under Saved/Persistence. A failed write never
 *  deletes the active generation. No client RPC, asset/map save or arbitrary path input. */
class PRIMALFRONTIER_API FPFSaveFileStore
{
public:
    static constexpr int32 MaxPayloadBytes = 4 * 1024 * 1024;
    static bool ValidSlot(const FString& Slot);
    static FString Path(const FString& Slot, bool bSecond);
    static bool Read(const FString& Slot, FPFSavedFile& Out, FString& Error);
    static bool Write(const FString& Slot, const TArray<uint8>& Payload, FString& Error);
};
