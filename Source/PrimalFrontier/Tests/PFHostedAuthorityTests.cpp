#include "Hosting/PFHostedAuthorityState.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include <limits>

namespace PFHostedAuthorityTestsPrivate
{
    const FString Sum = TEXT("sha256:47d0a2035da74393511ea2f39463de0a4eea822fc0d822584e3799ab899a17ed");
    bool Ready(FPFHostedAuthorityState& Guard, const FGuid& Lease, uint64 Head = 7, double Ttl = 30)
    {
        FString Error; uint64 Request = 0;
        return Guard.BeginAcquire(TEXT("world-a"), 0, Ttl, Request, Error) &&
            Guard.AcceptLease(Request, Lease, TEXT("1"), Head, 0.1, Error) &&
            Guard.CompleteRestore(Lease, TEXT("1"), Head, true, 0.2, Error);
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFHostedAuthorityInitialization, "PF.Hosting.Authority.Initialization",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFHostedAuthorityInitialization::RunTest(const FString&)
{
    FPFHostedAuthorityState Guard; FString Error; uint64 Request = 77;
    const FGuid Lease = FGuid::NewGuid();
    TestFalse(TEXT("Default state cannot run gameplay"), Guard.CanRunGameplay(0));
    TestFalse(TEXT("Invalid world refused"), Guard.BeginAcquire(TEXT("../world"), 0, 30, Request, Error));
    TestEqual(TEXT("Failed start preserves caller request"), Request, uint64(77));
    TestTrue(TEXT("Acquire starts with fresh local correlation"), Guard.BeginAcquire(TEXT("world-a"), 0, 30, Request, Error));
    TestFalse(TEXT("Acquire pending is closed"), Guard.CanRunGameplay(0));
    TestFalse(TEXT("Unrelated acquire callback ignored"), Guard.AcceptLease(Request + 1, Lease, TEXT("1"), 7, 0.1, Error));
    TestTrue(TEXT("Correct callback accepts lease"), Guard.AcceptLease(Request, Lease, TEXT("1"), 7, 0.1, Error));
    TestFalse(TEXT("Lease without restoration is closed"), Guard.CanRunGameplay(0.15));
    TestFalse(TEXT("Wrong epoch restore callback ignored"), Guard.CompleteRestore(Lease, TEXT("2"), 7, true, 0.2, Error));
    TestTrue(TEXT("Exact restore and trusted identity produce readiness in value guard only"),
        Guard.CompleteRestore(Lease, TEXT("1"), 7, true, 0.2, Error));
    TestTrue(TEXT("Ready lease is live"), Guard.CanRunGameplay(0.3));
    TestFalse(TEXT("Late duplicate acquire cannot rewrite state"), Guard.AcceptLease(Request, Lease, TEXT("2"), 88, 100, Error));
    Guard.MarkUncertain(Request);
    TestTrue(TEXT("Completed request failure callback is ignored"), Guard.CanRunGameplay(0.4));
    TestEqual(TEXT("Exact remote head retained"), Guard.DurableGeneration(), uint64(7));

    FPFHostedAuthorityState MissingIdentity; uint64 Other = 0;
    MissingIdentity.BeginAcquire(TEXT("world-a"), 0, 30, Other, Error);
    MissingIdentity.AcceptLease(Other, Lease, TEXT("1"), 0, 0.1, Error);
    TestFalse(TEXT("Unverified identity cannot complete empty-world restoration"),
        MissingIdentity.CompleteRestore(Lease, TEXT("1"), 0, false, 0.2, Error));
    TestTrue(TEXT("Identity refusal freezes"), MissingIdentity.Phase() == EPFHostedAuthorityPhase::Frozen);
    TestFalse(TEXT("Identity refusal is explicit and prefixed"), Error.IsEmpty());
    TestTrue(TEXT("Clear tooling prefix"), Error.StartsWith(TEXT("[PrimalAgentTools]")));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFHostedAuthorityExpiry, "PF.Hosting.Authority.ExpiryAndRenewal",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFHostedAuthorityExpiry::RunTest(const FString&)
{
    using namespace PFHostedAuthorityTestsPrivate;
    FPFHostedAuthorityState Guard; FString Error; uint64 Request = 0;
    const FGuid Lease = FGuid::NewGuid();
    if (!TestTrue(TEXT("Prepare short lease"), Ready(Guard, Lease, 7, 5))) { return false; }
    TestTrue(TEXT("Before conservative send+TTL-1 deadline"), Guard.CanRunGameplay(3));
    TestTrue(TEXT("Renew begins without changing acknowledged deadline"), Guard.BeginRenew(3, 30, Request, Error));
    TestTrue(TEXT("Matching timely renewal accepted"), Guard.AcceptRenewal(Request, Lease, TEXT("1"), 3.5, Error));
    TestFalse(TEXT("Duplicate old renewal cannot extend again"), Guard.AcceptRenewal(Request, Lease, TEXT("1"), 31, Error));
    TestTrue(TEXT("Extended bound is send3+TTL30-1"), Guard.CanRunGameplay(31.9));
    TestFalse(TEXT("Exact deadline refuses without waiting for Tick"), Guard.CanRunGameplay(32));
    TestTrue(TEXT("Expired diagnostic state freezes"), Guard.Phase() == EPFHostedAuthorityPhase::Frozen);

    FPFHostedAuthorityState Late;
    Ready(Late, Lease, 7, 5); Late.BeginRenew(3, 30, Request, Error);
    TestFalse(TEXT("Response after old conservative deadline cannot reopen"), Late.AcceptRenewal(Request, Lease, TEXT("1"), 4.1, Error));
    TestFalse(TEXT("Late renewal stays closed"), Late.CanRunGameplay(4.2));
    FPFHostedAuthorityState Clock;
    Ready(Clock, Lease); TestTrue(TEXT("Advance monotonic observation"), Clock.CanRunGameplay(2));
    TestFalse(TEXT("Clock rollback cannot restore time budget"), Clock.CanRunGameplay(1));
    TestFalse(TEXT("NaN clock cannot grant readiness"), Clock.CanRunGameplay(std::numeric_limits<double>::quiet_NaN()));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFHostedAuthorityDurability, "PF.Hosting.Authority.Durability",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFHostedAuthorityDurability::RunTest(const FString&)
{
    using namespace PFHostedAuthorityTestsPrivate;
    FPFHostedAuthorityState Guard; FString Error; uint64 Request = 0;
    const FGuid Lease = FGuid::NewGuid();
    if (!TestTrue(TEXT("Prepare restored authority"), Ready(Guard, Lease))) { return false; }
    TestFalse(TEXT("Old generation cannot start write"), Guard.BeginCheckpoint(7, Sum, 1, Request, Error));
    TestFalse(TEXT("Malformed checksum cannot start write"), Guard.BeginCheckpoint(8, TEXT("barehex"), 1, Request, Error));
    TestTrue(TEXT("New immutable checkpoint starts"), Guard.BeginCheckpoint(8, Sum, 1, Request, Error));
    TestFalse(TEXT("Pending checkpoint never counts as saved/readiness"), Guard.CanRunGameplay(1.1));
    TestEqual(TEXT("No premature durable-head update"), Guard.DurableGeneration(), uint64(7));
    TestFalse(TEXT("Other callback ignored"), Guard.AcceptDurableAck(Request + 1, 8, Sum, 1.2, Error));
    TestTrue(TEXT("Exact durable receipt accepted"), Guard.AcceptDurableAck(Request, 8, Sum, 1.3, Error));
    TestEqual(TEXT("Only exact acknowledgment advances durable head"), Guard.DurableGeneration(), uint64(8));
    TestTrue(TEXT("Successful durable receipt restores readiness"), Guard.CanRunGameplay(1.4));
    const uint64 Previous = Request;
    TestTrue(TEXT("Next operation gets a nonreused local correlation"), Guard.BeginCheckpoint(9, Sum, 2, Request, Error));
    TestTrue(TEXT("Correlation increases"), Request > Previous);
    Guard.MarkUncertain(Previous);
    TestTrue(TEXT("Old failure leaves current write pending"), Guard.Phase() == EPFHostedAuthorityPhase::CheckpointPending);
    Guard.MarkUncertain(Request);
    TestTrue(TEXT("Unknown write outcome freezes"), Guard.Phase() == EPFHostedAuthorityPhase::Frozen);
    TestFalse(TEXT("Late success cannot undo uncertainty"), Guard.AcceptDurableAck(Request, 9, Sum, 2.1, Error));
    TestEqual(TEXT("Unknown write never acknowledged durable"), Guard.DurableGeneration(), uint64(8));

    FPFHostedAuthorityState Wrong; Ready(Wrong, Lease);
    Wrong.BeginCheckpoint(8, Sum, 1, Request, Error);
    TestFalse(TEXT("Wrong generation acknowledgment freezes"), Wrong.AcceptDurableAck(Request, 9, Sum, 1.1, Error));
    TestTrue(TEXT("Wrong receipt stays frozen"), Wrong.Phase() == EPFHostedAuthorityPhase::Frozen);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFHostedAuthorityRecovery, "PF.Hosting.Authority.Recovery",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFHostedAuthorityRecovery::RunTest(const FString&)
{
    using namespace PFHostedAuthorityTestsPrivate;
    FPFHostedAuthorityState Guard; FString Error; uint64 Request = 0;
    const FGuid Old = FGuid::NewGuid(), Next = FGuid::NewGuid();
    if (!TestTrue(TEXT("Prepare durable generation7"), Ready(Guard, Old))) { return false; }
    Guard.BeginRenew(1, 30, Request, Error); Guard.MarkUncertain(Request);
    TestTrue(TEXT("Recovery starts only after freeze"), Guard.BeginAcquire(TEXT("world-a"), 2, 30, Request, Error));
    TestFalse(TEXT("Returned head cannot regress acknowledged history"), Guard.AcceptLease(Request, Next, TEXT("2"), 6, 2.1, Error));
    Guard.BeginAcquire(TEXT("world-a"), 3, 30, Request, Error);
    TestFalse(TEXT("Retired fence is not reusable"), Guard.AcceptLease(Request, Next, TEXT("1"), 7, 3.1, Error));
    Guard.BeginAcquire(TEXT("world-a"), 4, 30, Request, Error);
    TestTrue(TEXT("New exact large fence parsed without JSON precision loss"),
        Guard.AcceptLease(Request, Next, TEXT("9007199254740993"), 8, 4.1, Error));
    TestFalse(TEXT("Old lease restore ignored"), Guard.CompleteRestore(Old, TEXT("1"), 8, true, 4.2, Error));
    TestFalse(TEXT("Recovery still requires exact restored head"),
        Guard.CompleteRestore(Next, TEXT("9007199254740993"), 7, true, 4.2, Error));
    TestTrue(TEXT("Wrong restored head freezes"), Guard.Phase() == EPFHostedAuthorityPhase::Frozen);
    Guard.Stop();
    TestFalse(TEXT("Stopped guard cannot restart"), Guard.BeginAcquire(TEXT("world-a"), 5, 30, Request, Error));
    TestFalse(TEXT("Stopped guard cannot admit"), Guard.CanRunGameplay(5));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFHostedAuthorityRefusals, "PF.Hosting.Authority.Refusals",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFHostedAuthorityRefusals::RunTest(const FString&)
{
    FString Error; uint64 Request = 99;
    for (double Ttl : {0.0, 4.0, 5.5, 121.0, std::numeric_limits<double>::infinity()})
    {
        FPFHostedAuthorityState Guard;
        TestFalse(TEXT("Unbounded/fractional TTL refused"), Guard.BeginAcquire(TEXT("world-a"), 0, Ttl, Request, Error));
    }
    for (const FString& Fence : {FString(TEXT("0")), FString(TEXT("01")), FString(TEXT("-1")),
        FString(TEXT("1e1")), FString(TEXT("9223372036854775808")), FString(TEXT(""))})
    {
        FPFHostedAuthorityState Guard;
        Guard.BeginAcquire(TEXT("world-a"), 0, 30, Request, Error);
        TestFalse(TEXT("Invalid/signed-overflow fence refused"), Guard.AcceptLease(Request, FGuid::NewGuid(), Fence, 0, 0.1, Error));
        TestFalse(TEXT("Malformed proof never ready"), Guard.CanRunGameplay(0.2));
    }
    FPFHostedAuthorityState Head;
    Head.BeginAcquire(TEXT("world-a"), 0, 30, Request, Error);
    TestFalse(TEXT("Remote unsafe JSON head refused"), Head.AcceptLease(Request, FGuid::NewGuid(), TEXT("1"),
        FPFHostedAuthorityState::MaxGeneration + 1, 0.1, Error));
    FPFHostedAuthorityState Late;
    Late.BeginAcquire(TEXT("world-a"), 0, 5, Request, Error);
    TestFalse(TEXT("Delayed acquire cannot use reply-time TTL"), Late.AcceptLease(Request, FGuid::NewGuid(), TEXT("1"), 0, 4, Error));
    TestFalse(TEXT("Recovery cannot change worlds and forget history"), Late.BeginAcquire(TEXT("world-b"), 5, 30, Request, Error));
    return true;
}
#endif
