#pragma once
#include "CoreMinimal.h"
#include "Persistence/PFSaveFileStore.h"
class UWorld;

struct PRIMALFRONTIER_API FPFCheckpointResult
{
    bool bSucceeded=false;
    int32 HttpStatus=0;
    FString Details;
    FPFSavedFile File;
};

/** Opt-in development bridge. Dedicated server only; no RPC, replication or automatic restore.
 *  The secret is read from the process environment AFTER authority checks, never from config assets. */
class PRIMALFRONTIER_API FPFCheckpointClient
{
public:
    using FCompletion = TFunction<void(FPFCheckpointResult)>;
    static bool UploadSavedSlot(UWorld* World,const FString& Slot,const FString& WorldId,FCompletion Complete,FString& Error);
    /** Returns validated bytes only. Caller must separately enforce catalog/map/layout and session admission. */
    static bool Fetch(UWorld* World,const FString& WorldId,uint64 MinimumGeneration,FCompletion Complete,FString& Error);
    static bool EligibleWorld(const UWorld* World);
private:
    friend class FPFCheckpointHttp;
    static bool Start(UWorld* World,const FString& WorldId,const FPFSavedFile* Upload,uint64 MinimumGeneration,
        FCompletion Complete,FString& Error);
    /** Private transport seam: tests use loopback/fake credentials; production only calls after Start authority checks. */
    static bool Send(const FString& Url,const FString& Secret,const FString& WorldId,const FPFSavedFile* Upload,uint64 MinimumGeneration,
        TFunction<bool()> IsActive,FCompletion Complete,FString& Error);
};
