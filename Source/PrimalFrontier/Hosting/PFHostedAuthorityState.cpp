#include "Hosting/PFHostedAuthorityState.h"
#include "Persistence/PFCheckpointProtocol.h"

namespace PFHostedAuthorityStatePrivate
{
    bool Deadline(double Sent, double Ttl, double& Out)
    {
        if (!FMath::IsFinite(Sent) || Sent < 0 || !FMath::IsFinite(Ttl) || Ttl < 5 || Ttl > 120 ||
            FMath::FloorToDouble(Ttl) != Ttl) { return false; }
        // Conservative causal bound: DB creates/renews a lease AFTER request send.
        // Use send-time + requested TTL minus reserve, never reply-time + TTL or
        // an untrusted local/remote wall-clock offset. Clock-rate drift still
        // needs a real deployment bound before this guard is integrated.
        Out = Sent + Ttl - 1;
        return FMath::IsFinite(Out) && Out > Sent;
    }
    bool Fence(const FString& Text, uint64& Value)
    {
        if (Text.IsEmpty() || Text.Len() > 19 || Text[0] == '0') { return false; }
        uint64 Parsed = 0;
        for (TCHAR C : Text)
        {
            if (C < '0' || C > '9') { return false; }
            const uint64 Digit = C - '0';
            if (Parsed > (static_cast<uint64>(MAX_int64) - Digit) / 10) { return false; }
            Parsed = Parsed * 10 + Digit;
        }
        Value = Parsed;
        return true;
    }
    bool Checksum(const FString& Text)
    {
        if (Text.Len() != 71 || !Text.StartsWith(TEXT("sha256:"), ESearchCase::CaseSensitive)) { return false; }
        for (int32 I = 7; I < Text.Len(); ++I)
        {
            const TCHAR C = Text[I];
            if (!((C >= '0' && C <= '9') || (C >= 'a' && C <= 'f'))) { return false; }
        }
        return true;
    }
}

bool FPFHostedAuthorityState::Refuse(FString& Error, const TCHAR* Message) const
{
    Error = FString(TEXT("[PrimalAgentTools] Hosted authority: ")) + Message;
    return false;
}
void FPFHostedAuthorityState::ClearPending()
{
    AcquireRequest = RenewRequest = CommitRequest = PendingGeneration = 0;
    PendingChecksum.Reset();
}
bool FPFHostedAuthorityState::Freeze(FString& Error, const TCHAR* Message)
{
    if (CurrentPhase != EPFHostedAuthorityPhase::Stopped) { CurrentPhase = EPFHostedAuthorityPhase::Frozen; }
    ClearPending();
    LastReason = Message;
    return Refuse(Error, Message);
}
bool FPFHostedAuthorityState::Observe(double Now)
{
    if (!FMath::IsFinite(Now) || Now < LastNow)
    {
        FString Ignored;
        return Freeze(Ignored, TEXT("Invalid or regressing monotonic clock"));
    }
    LastNow = Now;
    return true;
}
bool FPFHostedAuthorityState::Live(double Now)
{
    if (!Observe(Now)) { return false; }
    if (CurrentPhase != EPFHostedAuthorityPhase::Restoring && CurrentPhase != EPFHostedAuthorityPhase::Ready &&
        CurrentPhase != EPFHostedAuthorityPhase::CheckpointPending) { return false; }
    if (Now >= AuthorityUntil)
    {
        FString Ignored;
        return Freeze(Ignored, TEXT("Conservative lease deadline expired"));
    }
    return true;
}
bool FPFHostedAuthorityState::Allocate(uint64& Request, FString& Error)
{
    if (Counter == MAX_uint64) { Stop(); return Refuse(Error, TEXT("Request counter exhausted")); }
    Request = ++Counter;
    return true;
}
bool FPFHostedAuthorityState::BeginAcquire(const FString& World, double SentAt, double Ttl, uint64& Request, FString& Error)
{
    Error.Reset();
    double Until = 0;
    if ((CurrentPhase != EPFHostedAuthorityPhase::Idle && CurrentPhase != EPFHostedAuthorityPhase::Frozen) ||
        !FPFCheckpointProtocol::ValidWorldId(World) || (!WorldId.IsEmpty() && WorldId != World) ||
        !PFHostedAuthorityStatePrivate::Deadline(SentAt, Ttl, Until)) { return Refuse(Error, TEXT("Invalid acquire context")); }
    if (!Observe(SentAt)) { return Refuse(Error, TEXT("Invalid acquire clock")); }
    ClearPending();
    if (!Allocate(AcquireRequest, Error)) { return false; }
    WorldId = World; AcquireUntil = Until; LastReason.Reset();
    CurrentPhase = EPFHostedAuthorityPhase::Acquiring;
    Request = AcquireRequest;
    return true;
}
bool FPFHostedAuthorityState::AcceptLease(uint64 Request, const FGuid& Lease, const FString& Fence,
    uint64 DurableHead, double Now, FString& Error)
{
    Error.Reset();
    if (CurrentPhase != EPFHostedAuthorityPhase::Acquiring || Request == 0 || Request != AcquireRequest)
    { return Refuse(Error, TEXT("Stale acquire callback ignored")); }
    uint64 Parsed = 0;
    if (!Observe(Now) || Now >= AcquireUntil || !Lease.IsValid() ||
        !PFHostedAuthorityStatePrivate::Fence(Fence, Parsed) || Parsed <= HighestFence ||
        DurableHead > MaxGeneration || DurableHead < Head)
    { return Freeze(Error, TEXT("Invalid, expired or regressing lease proof")); }
    CurrentLease = Lease; CurrentFenceText = Fence; HighestFence = Parsed; Head = DurableHead;
    AuthorityUntil = AcquireUntil; AcquireRequest = 0;
    CurrentPhase = EPFHostedAuthorityPhase::Restoring;
    return true;
}
bool FPFHostedAuthorityState::CompleteRestore(const FGuid& Lease, const FString& Fence, uint64 RestoredGeneration,
    bool bIdentityValidated, double Now, FString& Error)
{
    Error.Reset();
    if (CurrentPhase != EPFHostedAuthorityPhase::Restoring || Lease != CurrentLease || Fence != CurrentFenceText)
    { return Refuse(Error, TEXT("Stale restore callback ignored")); }
    if (!Live(Now) || !bIdentityValidated || RestoredGeneration != Head)
    { return Freeze(Error, TEXT("Exact validated restore and trusted server identity required")); }
    CurrentPhase = EPFHostedAuthorityPhase::Ready;
    return true;
}
bool FPFHostedAuthorityState::CanRunGameplay(double Now)
{
    return Live(Now) && CurrentPhase == EPFHostedAuthorityPhase::Ready;
}
bool FPFHostedAuthorityState::BeginRenew(double SentAt, double Ttl, uint64& Request, FString& Error)
{
    Error.Reset();
    double Until = 0;
    if (RenewRequest != 0 || !PFHostedAuthorityStatePrivate::Deadline(SentAt, Ttl, Until) ||
        (CurrentPhase != EPFHostedAuthorityPhase::Ready && CurrentPhase != EPFHostedAuthorityPhase::CheckpointPending))
    { return Refuse(Error, TEXT("Renewal is unavailable or already pending")); }
    if (!Live(SentAt)) { return Refuse(Error, TEXT("Live lease required for renewal")); }
    if (!Allocate(RenewRequest, Error)) { return false; }
    RenewUntil = Until; Request = RenewRequest;
    return true;
}
bool FPFHostedAuthorityState::AcceptRenewal(uint64 Request, const FGuid& Lease, const FString& Fence, double Now, FString& Error)
{
    Error.Reset();
    if (Request == 0 || Request != RenewRequest ||
        (CurrentPhase != EPFHostedAuthorityPhase::Ready && CurrentPhase != EPFHostedAuthorityPhase::CheckpointPending))
    { return Refuse(Error, TEXT("Stale renewal callback ignored")); }
    if (!Live(Now) || Now >= RenewUntil || Lease != CurrentLease || Fence != CurrentFenceText)
    { return Freeze(Error, TEXT("Renewal proof/deadline refused")); }
    AuthorityUntil = FMath::Max(AuthorityUntil, RenewUntil); RenewRequest = 0;
    return true;
}
bool FPFHostedAuthorityState::BeginCheckpoint(uint64 Generation, const FString& Checksum, double Now, uint64& Request, FString& Error)
{
    Error.Reset();
    if (!CanRunGameplay(Now)) { return Refuse(Error, TEXT("Ready authority required before checkpoint")); }
    if (Generation <= Head || Generation > MaxGeneration || !PFHostedAuthorityStatePrivate::Checksum(Checksum))
    { return Refuse(Error, TEXT("New safe generation and canonical checksum required")); }
    if (!Allocate(CommitRequest, Error)) { return false; }
    PendingGeneration = Generation; PendingChecksum = Checksum;
    CurrentPhase = EPFHostedAuthorityPhase::CheckpointPending; Request = CommitRequest;
    return true;
}
bool FPFHostedAuthorityState::AcceptDurableAck(uint64 Request, uint64 Generation, const FString& Checksum, double Now, FString& Error)
{
    Error.Reset();
    if (CurrentPhase != EPFHostedAuthorityPhase::CheckpointPending || Request == 0 || Request != CommitRequest)
    { return Refuse(Error, TEXT("Stale durability callback ignored")); }
    if (!Live(Now) || Generation != PendingGeneration || Checksum != PendingChecksum)
    { return Freeze(Error, TEXT("Durable acknowledgment does not match pending checkpoint")); }
    Head = PendingGeneration; PendingGeneration = CommitRequest = 0; PendingChecksum.Reset();
    CurrentPhase = EPFHostedAuthorityPhase::Ready;
    return true;
}
void FPFHostedAuthorityState::MarkUncertain(uint64 Request)
{
    if (Request == 0 || !((CurrentPhase == EPFHostedAuthorityPhase::Acquiring && Request == AcquireRequest) ||
        Request == RenewRequest || Request == CommitRequest)) { return; }
    FString Ignored;
    Freeze(Ignored, TEXT("Pending operation outcome is unknown; authority frozen"));
}
void FPFHostedAuthorityState::Stop()
{
    ClearPending(); CurrentPhase = EPFHostedAuthorityPhase::Stopped;
    LastReason = TEXT("Hosting stopped");
}
