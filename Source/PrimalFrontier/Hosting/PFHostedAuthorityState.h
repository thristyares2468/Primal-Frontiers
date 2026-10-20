#pragma once
#include "CoreMinimal.h"

enum class EPFHostedAuthorityPhase : uint8
{
    Idle, Acquiring, Restoring, Ready, CheckpointPending, Frozen, Stopped
};

/** Game-thread value guard for future trusted server adapters. No networking,
 * authentication or world mutation. NOT wired to GameMode: hosted entry remains
 * closed. Caller-supplied RPC/JSON booleans are never identity/restore proof.
 * Request numbers correlate local async callbacks, not wire credentials. */
class PRIMALFRONTIER_API FPFHostedAuthorityState
{
public:
    static constexpr uint64 MaxGeneration = 9007199254740991ULL;
    bool BeginAcquire(const FString& World, double SentAt, double TtlSeconds, uint64& Request, FString& Error);
    bool AcceptLease(uint64 Request, const FGuid& Lease, const FString& Fence,
        uint64 DurableHead, double Now, FString& Error);
    bool CompleteRestore(const FGuid& Lease, const FString& Fence, uint64 RestoredGeneration,
        bool bTrustedServerIdentityValidated, double Now, FString& Error);
    bool BeginRenew(double SentAt, double TtlSeconds, uint64& Request, FString& Error);
    bool AcceptRenewal(uint64 Request, const FGuid& Lease, const FString& Fence, double Now, FString& Error);
    bool BeginCheckpoint(uint64 Generation, const FString& Checksum, double Now, uint64& Request, FString& Error);
    bool AcceptDurableAck(uint64 Request, uint64 Generation, const FString& Checksum, double Now, FString& Error);
    /** Only a currently pending request can freeze authority. Old callbacks are ignored. */
    void MarkUncertain(uint64 Request);
    void Stop();
    /** Checks monotonic time/expiry on every query; never waits for Tick. */
    bool CanRunGameplay(double Now);
    EPFHostedAuthorityPhase Phase() const { return CurrentPhase; }
    uint64 DurableGeneration() const { return Head; }
    const FString& Reason() const { return LastReason; }

private:
    bool Observe(double Now);
    bool Live(double Now);
    bool Allocate(uint64& Request, FString& Error);
    bool Refuse(FString& Error, const TCHAR* Message) const;
    bool Freeze(FString& Error, const TCHAR* Message);
    void ClearPending();
    EPFHostedAuthorityPhase CurrentPhase = EPFHostedAuthorityPhase::Idle;
    FString WorldId, CurrentFenceText, PendingChecksum, LastReason;
    FGuid CurrentLease;
    uint64 Counter = 0, AcquireRequest = 0, RenewRequest = 0, CommitRequest = 0;
    uint64 HighestFence = 0, Head = 0, PendingGeneration = 0;
    double LastNow = 0, AcquireUntil = 0, RenewUntil = 0, AuthorityUntil = 0;
};
