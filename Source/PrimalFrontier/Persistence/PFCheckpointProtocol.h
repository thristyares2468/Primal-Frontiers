#pragma once
#include "CoreMinimal.h"
#include "Persistence/PFSaveFileStore.h"

/** Development API adapter. Carries the original .pfs envelope, never re-dates food. */
class PRIMALFRONTIER_API FPFCheckpointProtocol
{
public:
    static constexpr uint64 MaxGeneration = 9007199254740991ULL; // JSON safe integer.
    static constexpr int32 MaxResponseBytes = 6 * 1024 * 1024;
    static bool ValidWorldId(const FString& Id);
    static bool BuildUpload(const FString& WorldId, const FPFSavedFile& File, FString& Out, FString& Error);
    static bool ReadDownload(const FString& WorldId, uint64 MinimumGeneration, const FString& Json,
        FPFSavedFile& Out, FString& Error);
    static bool ReadAcknowledgement(const FString& WorldId, const FPFSavedFile& Sent,
        const FString& Json, FString& Error);
};
