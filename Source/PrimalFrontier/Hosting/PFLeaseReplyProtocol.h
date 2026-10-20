#pragma once
#include "CoreMinimal.h"

/** Decoded draft-service metadata only. Null head means no checkpoint; a saved
 * generation zero is refused rather than silently treated as an empty world. */
struct PRIMALFRONTIER_API FPFLeaseReply
{
    FGuid Lease;
    FString Fence;
    FDateTime ExpiresUtc;
    TOptional<uint64> Head;
    bool bReplayed = false;
};

/** Pure decoder for pf_fencing_draft SQL results, NOT an authenticated HTTP
 * adapter. Caller must bind the authenticated response to the exact pending
 * request/server/world/operation before using it. No GameMode integration. */
class PRIMALFRONTIER_API FPFLeaseReplyProtocol
{
public:
    static constexpr int32 MaxResponseBytes = 4096;
    static bool ReadAcquire(int32 HttpStatus, const FString& Json, const FString& ExpectedWorld,
        FPFLeaseReply& Out, FString& Error);
    static bool ReadRenew(int32 HttpStatus, const FString& Json, const FGuid& ExpectedLease,
        const FString& ExpectedFence, FPFLeaseReply& Out, FString& Error);
    static bool ReadCommit(int32 HttpStatus, const FString& Json, const FString& ExpectedWorld,
        const FString& ExpectedFence, uint64 ExpectedGeneration, const FString& ExpectedChecksum,
        FString& Error);
};
