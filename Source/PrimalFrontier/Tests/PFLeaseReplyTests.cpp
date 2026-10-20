#include "Hosting/PFLeaseReplyProtocol.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Hosting/PFHostedAuthorityState.h"
#include "Dom/JsonObject.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace PFLeaseReplyTestsPrivate
{
    const FString World = TEXT("native-lease-fixture");
    const FString Fence = TEXT("9223372036854775807");
    const FString Sum = TEXT("sha256:abababababababababababababababababababababababababababababababab");
    constexpr uint64 Generation = 9007199254740991ULL;
    FString Encode(const TSharedPtr<FJsonObject>& Object)
    {
        FString Text;
        FJsonSerializer::Serialize(Object, TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&Text));
        return Text;
    }
    TSharedPtr<FJsonObject> Fixture()
    {
        FString Text; TSharedPtr<FJsonObject> Root;
        if (!FFileHelper::LoadFileToString(Text, *FPaths::Combine(FPaths::ProjectDir(),
            TEXT("Tests/Fixtures/M15LeaseReplies/fixture.json"))) ||
            !FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Text), Root)) { return nullptr; }
        return Root;
    }
    FString Reply(const TSharedPtr<FJsonObject>& Root, const TCHAR* Name)
    {
        const TSharedPtr<FJsonObject>* Value = nullptr;
        return Root && Root->TryGetObjectField(Name, Value) ? Encode(*Value) : FString();
    }
    FString Replace(const FString& Json, const TCHAR* Field, const FString& RawValue)
    {
        TSharedPtr<FJsonObject> Root;
        FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json), Root);
        Root->RemoveField(Field);
        FString Text = Encode(Root); Text.LeftChopInline(1);
        return Text + TEXT(",\"") + Field + TEXT("\":") + RawValue + TEXT("}");
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFLeaseSqlFixtures, "PF.Hosting.LeaseReply.SqlFixtures",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFLeaseSqlFixtures::RunTest(const FString&)
{
    using namespace PFLeaseReplyTestsPrivate;
    auto Root = Fixture();
    if (!TestTrue(TEXT("Read immutable actual SQL-result fixture"), Root.IsValid())) { return false; }
    FString Error; FPFLeaseReply Lease;
    for (const TCHAR* Name : {TEXT("acquire"), TEXT("acquireReplay")})
    {
        TestTrue(Name, FPFLeaseReplyProtocol::ReadAcquire(200, Reply(Root, Name), World, Lease, Error));
        TestEqual(TEXT("MAX_int64 fence remains an exact string"), Lease.Fence, Fence);
        TestFalse(TEXT("Null empty head remains distinct from persisted zero"), Lease.Head.IsSet());
        TestEqual(TEXT("Replay metadata preserved"), Lease.bReplayed, FString(Name).EndsWith(TEXT("Replay")));
    }
    const FGuid ExpectedLease = Lease.Lease;
    for (const TCHAR* Name : {TEXT("renew"), TEXT("renewReplay")})
    {
        TestTrue(Name, FPFLeaseReplyProtocol::ReadRenew(200, Reply(Root, Name), ExpectedLease, Fence, Lease, Error));
        TestEqual(TEXT("Renewal lease binding preserved"), Lease.Lease, ExpectedLease);
        TestEqual(TEXT("Renewal replay flag preserved"), Lease.bReplayed, FString(Name).EndsWith(TEXT("Replay")));
    }
    for (const TCHAR* Name : {TEXT("commit"), TEXT("commitReplay")})
    {
        TestTrue(Name, FPFLeaseReplyProtocol::ReadCommit(200, Reply(Root, Name), World, Fence, Generation, Sum, Error));
    }
    TestTrue(TEXT("Successful decode clears old errors"), Error.IsEmpty());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFLeaseReplyNumbers, "PF.Hosting.LeaseReply.NumericBoundaries",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFLeaseReplyNumbers::RunTest(const FString&)
{
    using namespace PFLeaseReplyTestsPrivate;
    auto Root = Fixture();
    if (!TestTrue(TEXT("Fixture loaded"), Root.IsValid())) { return false; }
    const FString Json = Reply(Root, TEXT("acquire")); FString Error; FPFLeaseReply Value;
    for (const FString& Head : {FString(TEXT("1")), FString(TEXT("\"1\"")), FString(TEXT("9007199254740991")),
        FString(TEXT("\"9007199254740991\""))})
    {
        TestTrue(TEXT("Canonical safe number or Neon int8 string"), FPFLeaseReplyProtocol::ReadAcquire(200,
            Replace(Json, TEXT("head_generation"), Head), World, Value, Error));
        TestTrue(TEXT("Positive head retained"), Value.Head.IsSet() && Value.Head.GetValue() > 0);
    }
    for (const FString& Head : {FString(TEXT("0")), FString(TEXT("\"0\"")), FString(TEXT("-1")),
        FString(TEXT("1.0")), FString(TEXT("1e0")), FString(TEXT("9007199254740991.1")),
        FString(TEXT("9007199254740992")), FString(TEXT("\"01\"")), FString(TEXT("true")), FString(TEXT("{}"))})
    {
        Value.Fence = TEXT("sentinel"); Value.Head = 42;
        TestFalse(TEXT("Unsafe, rounded, zero or malformed head refused"), FPFLeaseReplyProtocol::ReadAcquire(200,
            Replace(Json, TEXT("head_generation"), Head), World, Value, Error));
        TestEqual(TEXT("Failed decode preserves output fence"), Value.Fence, FString(TEXT("sentinel")));
        TestEqual(TEXT("Failed decode preserves output head"), Value.Head.GetValue(), uint64(42));
    }
    for (const FString& Token : {FString(TEXT("1")), FString(TEXT("\"0\"")), FString(TEXT("\"01\"")),
        FString(TEXT("\"9223372036854775808\"")), FString(TEXT("\"1e1\"")), FString(TEXT("\"-1\""))})
    {
        TestFalse(TEXT("Fence requires positive canonical signed-int64 decimal string"),
            FPFLeaseReplyProtocol::ReadAcquire(200, Replace(Json, TEXT("fence"), Token), World, Value, Error));
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFLeaseReplyBinding, "PF.Hosting.LeaseReply.Binding",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFLeaseReplyBinding::RunTest(const FString&)
{
    using namespace PFLeaseReplyTestsPrivate;
    auto Root = Fixture();
    if (!TestTrue(TEXT("Fixture loaded"), Root.IsValid())) { return false; }
    FString Error; FPFLeaseReply Value;
    const FString Acquire = Reply(Root, TEXT("acquire")), Renew = Reply(Root, TEXT("renew")), Commit = Reply(Root, TEXT("commit"));
    TestTrue(TEXT("Read expected lease"), FPFLeaseReplyProtocol::ReadAcquire(200, Acquire, World, Value, Error));
    const FGuid ExpectedLease = Value.Lease;
    TestFalse(TEXT("Foreign world acquire refused"), FPFLeaseReplyProtocol::ReadAcquire(200, Acquire, TEXT("other"), Value, Error));
    TestFalse(TEXT("Case-distinct world acquire refused"), FPFLeaseReplyProtocol::ReadAcquire(200, Acquire, World.ToUpper(), Value, Error));
    TestFalse(TEXT("Foreign lease renewal refused"), FPFLeaseReplyProtocol::ReadRenew(200, Renew, FGuid::NewGuid(), Fence, Value, Error));
    TestFalse(TEXT("Foreign epoch renewal refused"), FPFLeaseReplyProtocol::ReadRenew(200, Renew, ExpectedLease, TEXT("1"), Value, Error));
    TestFalse(TEXT("Foreign world commit refused"), FPFLeaseReplyProtocol::ReadCommit(200, Commit, TEXT("other"), Fence, Generation, Sum, Error));
    TestFalse(TEXT("Case-distinct world commit refused"), FPFLeaseReplyProtocol::ReadCommit(200, Commit, World.ToUpper(), Fence, Generation, Sum, Error));
    TestFalse(TEXT("Noncanonical uppercase checksum refused"), FPFLeaseReplyProtocol::ReadCommit(200,
        Replace(Commit, TEXT("checksum"), TEXT("\"") + Sum.ToUpper() + TEXT("\"")), World, Fence, Generation, Sum, Error));
    TestFalse(TEXT("Foreign epoch commit refused"), FPFLeaseReplyProtocol::ReadCommit(200, Commit, World, TEXT("1"), Generation, Sum, Error));
    TestFalse(TEXT("Wrong generation refused"), FPFLeaseReplyProtocol::ReadCommit(200, Commit, World, Fence, Generation - 1, Sum, Error));
    TestFalse(TEXT("Wrong checksum refused"), FPFLeaseReplyProtocol::ReadCommit(200, Commit, World, Fence, Generation, Sum.Replace(TEXT("ab"), TEXT("cd")), Error));
    TestFalse(TEXT("Missing committed proof refused"), FPFLeaseReplyProtocol::ReadCommit(200,
        Replace(Commit, TEXT("committed"), TEXT("false")), World, Fence, Generation, Sum, Error));
    TestFalse(TEXT("Acquire cannot masquerade as renewal"), FPFLeaseReplyProtocol::ReadRenew(200, Acquire, ExpectedLease, Fence, Value, Error));
    for (int32 Status : {0, 201, 202, 204, 301, 401, 409, 500})
    {
        TestFalse(TEXT("Only completed contracted HTTP200 success can decode"), FPFLeaseReplyProtocol::ReadAcquire(Status, Acquire, World, Value, Error));
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFLeaseReplyRefusals, "PF.Hosting.LeaseReply.Refusals",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFLeaseReplyRefusals::RunTest(const FString&)
{
    using namespace PFLeaseReplyTestsPrivate;
    auto Root = Fixture();
    if (!TestTrue(TEXT("Fixture loaded"), Root.IsValid())) { return false; }
    const FString Json = Reply(Root, TEXT("acquire")); FString Error; FPFLeaseReply Value;
    for (const FString& Bad : {FString(), FString(TEXT("[]")), Json + TEXT("{}"), Json.LeftChop(1),
        Json.LeftChop(1) + TEXT(",\"ok\":true}"), Json.LeftChop(1) + TEXT(",\"\\u006fk\":true}"),
        Json.Replace(TEXT("\"ok\":"), TEXT("\"OK\":")),
        Json.LeftChop(1) + TEXT(",\"REPLAYED\":true}"),
        Json.LeftChop(1) + TEXT(",\"error\":\"secret-from-service\"}"),
        Replace(Json, TEXT("ok"), TEXT("\"true\"")), Replace(Json, TEXT("ok"), TEXT("false")),
        Json.LeftChop(1) + TEXT(",\"replayed\":false}"), Json + FString::ChrN(FPFLeaseReplyProtocol::MaxResponseBytes, ' ')})
    {
        TestFalse(TEXT("Malformed, duplicate, conflicting, extra or oversized response refused"),
            FPFLeaseReplyProtocol::ReadAcquire(200, Bad, World, Value, Error));
        TestTrue(TEXT("Refusal has tooling prefix"), Error.StartsWith(TEXT("[PrimalAgentTools]")));
        TestFalse(TEXT("Refusal never echoes service-controlled content"), Error.Contains(TEXT("secret-from-service")));
    }
    for (const FString& Expiry : {FString(TEXT("2026-02-30T00:00:00Z")), FString(TEXT("2026-13-01T00:00:00Z")),
        FString(TEXT("2026-10-21T24:00:00Z")), FString(TEXT("2026-10-21T00:00:00.Z")),
        FString(TEXT("2026-10-21T00:00:00")), FString(TEXT("2026-10-21T00:00:00+10:00"))})
    {
        TestFalse(TEXT("Invalid/non-UTC calendar metadata refused"), FPFLeaseReplyProtocol::ReadAcquire(200,
            Replace(Json, TEXT("expires_at"), TEXT("\"") + Expiry + TEXT("\"")), World, Value, Error));
    }
    for (const FString& Lease : {FString(TEXT("00000000-0000-0000-0000-000000000000")),
        FString(TEXT("{11111111-1111-1111-1111-111111111111}")), FString(TEXT("AAAAAAAA-AAAA-AAAA-AAAA-AAAAAAAAAAAA"))})
    {
        TestFalse(TEXT("Only canonical nonzero PostgreSQL UUID accepted"), FPFLeaseReplyProtocol::ReadAcquire(200,
            Replace(Json, TEXT("lease_id"), TEXT("\"") + Lease + TEXT("\"")), World, Value, Error));
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFLeaseReplyGuardBridge, "PF.Hosting.LeaseReply.GuardBridge",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFLeaseReplyGuardBridge::RunTest(const FString&)
{
    using namespace PFLeaseReplyTestsPrivate;
    auto Root = Fixture();
    if (!TestTrue(TEXT("Fixture loaded"), Root.IsValid())) { return false; }
    FPFHostedAuthorityState Guard; FPFLeaseReply Lease; FString Error; uint64 Request = 0;
    TestTrue(TEXT("Start synthetic local request"), Guard.BeginAcquire(World, 0, 30, Request, Error));
    TestTrue(TEXT("Decode actual SQL fixture"), FPFLeaseReplyProtocol::ReadAcquire(200, Reply(Root, TEXT("acquire")), World, Lease, Error));
    TestTrue(TEXT("Exact metadata passes only into restoring"), Guard.AcceptLease(Request, Lease.Lease, Lease.Fence,
        Lease.Head.Get(0), 0.1, Error));
    TestFalse(TEXT("Decoded lease never opens gameplay without restore/identity"), Guard.CanRunGameplay(0.2));
    // Value-only simulated proofs. No actual world or authenticated identity is created.
    TestTrue(TEXT("Synthetic trusted restore permits value readiness"), Guard.CompleteRestore(Lease.Lease, Fence, 0, true, 0.3, Error));
    TestTrue(TEXT("Begin exact synthetic write"), Guard.BeginCheckpoint(Generation, Sum, 1, Request, Error));
    TestTrue(TEXT("Validate exact SQL commit receipt"), FPFLeaseReplyProtocol::ReadCommit(200, Reply(Root, TEXT("commit")), World, Fence, Generation, Sum, Error));
    TestTrue(TEXT("Only matched receipt updates native head"), Guard.AcceptDurableAck(Request, Generation, Sum, 1.1, Error));
    TestEqual(TEXT("Safe integer upper bound retained exactly"), Guard.DurableGeneration(), Generation);
    TestFalse(TEXT("Historical UTC never grants extra monotonic budget"), Guard.CanRunGameplay(29));
    return true;
}
#endif
