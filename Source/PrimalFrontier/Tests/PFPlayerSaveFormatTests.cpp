#include "Persistence/PFPlayerSaveFormat.h"
#include "Inventory/PFItemCatalog.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Misc/Crc.h"
#include <limits>

namespace
{
    FPFPlayerSaveData ExamplePlayer()
    {
        FPFPlayerSaveData Data;
        Data.PlayerId = FGuid(1, 2, 3, 4);
        Data.Location = FVector(125, -200, 90);
        Data.Rotation = FRotator(-20, 90, 0);
        Data.Health = 65;
        Data.Stamina = 42;
        Data.Hunger = 24;
        Data.Thirst = 73;
        Data.Inventory = {
            {FGuid(5, 6, 7, 8), TEXT("Item_Wood"), 10, 0},
            {FGuid(9, 10, 11, 12), TEXT("Item_Food"), 2, 12.5},
            {FGuid(13, 14, 15, 16), TEXT("Item_Food"), 3, 280}
        };
        return Data;
    }

    void PutWord(TArray<uint8>& Bytes, int32 Offset, uint32 Value)
    {
        // Wire format is little-endian, independent of host byte order.
        for (int32 Index = 0; Index < 4; ++Index)
        {
            Bytes[Offset + Index] = static_cast<uint8>(Value >> (8 * Index));
        }
    }

    void RefreshEnvelope(TArray<uint8>& Bytes)
    {
        const int32 Size = Bytes.Num() - FPFPlayerSaveFormat::EnvelopeBytes;
        PutWord(Bytes, 8, Size);
        PutWord(Bytes, 12, FCrc::MemCrc32(Bytes.GetData() + FPFPlayerSaveFormat::EnvelopeBytes, Size));
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFSaveRoundTripTest, "PF.Persistence.PlayerRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)
bool FPFSaveRoundTripTest::RunTest(const FString&)
{
    auto* Catalog = NewObject<UPFItemCatalog>();
    const FPFPlayerSaveLimits Limits;
    const auto Original = ExamplePlayer();
    TArray<uint8> Bytes;
    FString Error;
    if (!TestTrue(TEXT("Encode valid player"), FPFPlayerSaveFormat::Encode(Original, *Catalog, Limits, Bytes, Error)))
    {
        AddError(Error);
        return false;
    }
    FPFPlayerSaveData Restored;
    if (!TestTrue(TEXT("Decode valid player"), FPFPlayerSaveFormat::Decode(Bytes, *Catalog, Limits, Restored, Error)))
    {
        AddError(Error);
        return false;
    }
    TestEqual(TEXT("Stable player ID"), Restored.PlayerId, Original.PlayerId);
    TestEqual(TEXT("Position"), Restored.Location, Original.Location);
    TestEqual(TEXT("Look rotation"), Restored.Rotation, Original.Rotation);
    TestEqual(TEXT("Health"), Restored.Health, Original.Health);
    TestEqual(TEXT("Stamina"), Restored.Stamina, Original.Stamina);
    TestEqual(TEXT("Hunger"), Restored.Hunger, Original.Hunger);
    TestEqual(TEXT("Thirst"), Restored.Thirst, Original.Thirst);
    if (!TestEqual(TEXT("Separate food batches remain separate"), Restored.Inventory.Num(), 3)) { return false; }
    for (int32 Index = 0; Index < Original.Inventory.Num(); ++Index)
    {
        TestEqual(TEXT("Stable stack ID"), Restored.Inventory[Index].StackId, Original.Inventory[Index].StackId);
        TestEqual(TEXT("Item ID"), Restored.Inventory[Index].ItemId, Original.Inventory[Index].ItemId);
        TestEqual(TEXT("Quantity"), Restored.Inventory[Index].Quantity, Original.Inventory[Index].Quantity);
        TestEqual(TEXT("Freshness is not renewed"), Restored.Inventory[Index].RemainingFreshnessSeconds,
            Original.Inventory[Index].RemainingFreshnessSeconds);
    }
    TArray<uint8> Again;
    TestTrue(TEXT("Re-encode"), FPFPlayerSaveFormat::Encode(Restored, *Catalog, Limits, Again, Error));
    TestTrue(TEXT("Deterministic unchanged record bytes"), Bytes == Again);
    Restored.Health = 0;
    Restored.Inventory.Reset();
    TestTrue(TEXT("Dead player with empty inventory is representable"),
        FPFPlayerSaveFormat::Encode(Restored, *Catalog, Limits, Again, Error));
    FPFPlayerSaveData Dead;
    TestTrue(TEXT("Dead record decodes without reviving"), FPFPlayerSaveFormat::Decode(Again, *Catalog, Limits, Dead, Error));
    TestEqual(TEXT("Health stays zero"), Dead.Health, 0.f);
    AddInfo(TEXT("[PrimalPersistence] Checked player/attributes/batch IDs/remaining freshness round-trip; no live-world restoration."));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFSaveValidationTest, "PF.Persistence.PlayerValidation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)
bool FPFSaveValidationTest::RunTest(const FString&)
{
    auto* Catalog = NewObject<UPFItemCatalog>();
    FPFPlayerSaveLimits Limits;
    FString Error;
    auto Reject = [&](const TCHAR* Label, const FPFPlayerSaveData& Data)
    {
        TArray<uint8> Output = {1, 2, 3};
        TestFalse(Label, FPFPlayerSaveFormat::Encode(Data, *Catalog, Limits, Output, Error));
        TestTrue(TEXT("Refusal explains why"), !Error.IsEmpty());
        TestTrue(TEXT("Failed encode preserves prior bytes"), Output == TArray<uint8>({1, 2, 3}));
    };
    auto Bad = ExamplePlayer(); Bad.PlayerId.Invalidate(); Reject(TEXT("Missing player ID"), Bad);
    Bad = ExamplePlayer(); Bad.Location.X = std::numeric_limits<double>::quiet_NaN(); Reject(TEXT("NaN position"), Bad);
    Bad = ExamplePlayer(); Bad.Location.Y = 1.e10; Reject(TEXT("Out-of-bounds position"), Bad);
    Bad = ExamplePlayer(); Bad.Rotation.Yaw = std::numeric_limits<double>::infinity(); Reject(TEXT("Infinite rotation"), Bad);
    for (const float Value : {-1.f, 101.f, std::numeric_limits<float>::quiet_NaN()})
    {
        Bad = ExamplePlayer(); Bad.Health = Value; Reject(TEXT("Invalid health"), Bad);
        Bad = ExamplePlayer(); Bad.Stamina = Value; Reject(TEXT("Invalid stamina"), Bad);
        Bad = ExamplePlayer(); Bad.Hunger = Value; Reject(TEXT("Invalid hunger"), Bad);
        Bad = ExamplePlayer(); Bad.Thirst = Value; Reject(TEXT("Invalid thirst"), Bad);
    }
    Bad = ExamplePlayer(); Bad.Inventory[1].StackId = Bad.Inventory[0].StackId; Reject(TEXT("Duplicate stack ID"), Bad);
    Bad = ExamplePlayer(); Bad.Inventory[0].StackId.Invalidate(); Reject(TEXT("Missing stack ID"), Bad);
    Bad = ExamplePlayer(); Bad.Inventory[0].ItemId = TEXT("Item_Missing"); Reject(TEXT("Unknown item"), Bad);
    for (const int32 Value : {0, -1, 21, MAX_int32})
    {
        Bad = ExamplePlayer(); Bad.Inventory[0].Quantity = Value; Reject(TEXT("Invalid quantity"), Bad);
    }
    for (const double Value : {0., -1., 301., std::numeric_limits<double>::quiet_NaN()})
    {
        Bad = ExamplePlayer(); Bad.Inventory[1].RemainingFreshnessSeconds = Value; Reject(TEXT("Invalid food lifetime"), Bad);
    }
    Bad = ExamplePlayer(); Bad.Inventory[0].RemainingFreshnessSeconds = 10; Reject(TEXT("Expiry on nonperishable item"), Bad);
    Limits.Slots = 2; Reject(TEXT("Slot overflow"), ExamplePlayer()); Limits.Slots = 8;
    Limits.Weight = 1; Reject(TEXT("Weight overflow"), ExamplePlayer()); Limits.Weight = 30;
    Limits.Slots = MAX_int32; Reject(TEXT("Invalid trusted capacity"), ExamplePlayer()); Limits.Slots = 8;
    const FPFItemDefinition Duplicate = Catalog->Items[0];
    Catalog->Items.Add(Duplicate); Reject(TEXT("Duplicate catalog definition"), ExamplePlayer());
    AddInfo(TEXT("[PrimalPersistence] Checked atomic rejection of invalid vitals, identity, quantities, capacity, catalog and freshness."));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFSaveCorruptionTest, "PF.Persistence.CorruptPlayerData",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)
bool FPFSaveCorruptionTest::RunTest(const FString&)
{
    auto* Catalog = NewObject<UPFItemCatalog>();
    const FPFPlayerSaveLimits Limits;
    FString Error;
    TArray<uint8> Good;
    auto Empty = ExamplePlayer(); Empty.Inventory.Reset();
    if (!TestTrue(TEXT("Encode empty inventory fixture"), FPFPlayerSaveFormat::Encode(Empty, *Catalog, Limits, Good, Error)))
    {
        return false;
    }
    auto Reject = [&](const TCHAR* Label, const TArray<uint8>& Bytes)
    {
        FPFPlayerSaveData Output = ExamplePlayer();
        TestFalse(Label, FPFPlayerSaveFormat::Decode(Bytes, *Catalog, Limits, Output, Error));
        TestTrue(TEXT("Decode refusal has details"), !Error.IsEmpty());
        TestEqual(TEXT("Failed decode preserves player ID"), Output.PlayerId, ExamplePlayer().PlayerId);
        TestEqual(TEXT("Failed decode preserves health"), Output.Health, 65.f);
        if (TestEqual(TEXT("Failed decode preserves inventory"), Output.Inventory.Num(), 3))
        {
            TestEqual(TEXT("Failed decode preserves food lifetime"), Output.Inventory[1].RemainingFreshnessSeconds, 12.5);
        }
    };
    Reject(TEXT("Empty bytes"), {});
    for (int32 Length = 1; Length < Good.Num(); ++Length)
    {
        TArray<uint8> Truncated = Good; Truncated.SetNum(Length);
        Reject(TEXT("Every truncated prefix rejected"), Truncated);
    }
    auto Bad = Good; Bad[0] ^= 1; Reject(TEXT("Signature mismatch"), Bad);
    Bad = Good; PutWord(Bad, 4, 0); Reject(TEXT("Older version needs migration"), Bad);
    TestTrue(TEXT("Migration refusal explicit"), Error.Contains(TEXT("migration")));
    Bad = Good; PutWord(Bad, 4, FPFPlayerSaveFormat::CurrentVersion + 1); Reject(TEXT("Future version"), Bad);
    Bad = Good; PutWord(Bad, 8, MAX_uint32); Reject(TEXT("Untrusted envelope length"), Bad);
    Bad = Good; Bad.Last() ^= 1; Reject(TEXT("Checksum mismatch"), Bad);
    Bad = Good; Bad.Add(42); Reject(TEXT("Appended bytes"), Bad);
    RefreshEnvelope(Bad); Reject(TEXT("Checksummed trailing payload"), Bad);
    Bad = Good; Bad.SetNum(FPFPlayerSaveFormat::MaxEncodedBytes + 1); Reject(TEXT("Oversized file"), Bad);
    // Empty-record payload ends in a stack count. A valid checksum must not bypass bounded parsing.
    Bad = Good; PutWord(Bad, Bad.Num() - 4, MAX_int32); RefreshEnvelope(Bad);
    Reject(TEXT("Huge stack count rejected before allocation"), Bad);
    Bad = Good; PutWord(Bad, Bad.Num() - 4, MAX_uint32); RefreshEnvelope(Bad);
    Reject(TEXT("Negative stack count"), Bad);
    Bad = Good; PutWord(Bad, Bad.Num() - 20, 0x7FC00000); RefreshEnvelope(Bad);
    Reject(TEXT("Checksummed NaN health"), Bad);
    Bad = Good; PutWord(Bad, Bad.Num() - 4, 1); RefreshEnvelope(Bad);
    Reject(TEXT("Missing stack record with valid checksum"), Bad);
    // Append a GUID and an oversized ID length; no arbitrary FString allocation is allowed.
    Bad.AddZeroed(16); const int32 LengthOffset = Bad.AddZeroed(4);
    PutWord(Bad, LengthOffset, MAX_int32); RefreshEnvelope(Bad);
    Reject(TEXT("Huge item ID length rejected before allocation"), Bad);
    TArray<uint8> WithItems;
    if (!TestTrue(TEXT("Encode item corruption fixture"),
        FPFPlayerSaveFormat::Encode(ExamplePlayer(), *Catalog, Limits, WithItems, Error))) { return false; }
    Bad = WithItems; PutWord(Bad, Bad.Num() - 12, MAX_uint32); RefreshEnvelope(Bad);
    Reject(TEXT("Checksummed invalid quantity"), Bad);
    Catalog->Items[0].StackLimit = 1;
    Reject(TEXT("Changed catalog stack limit revalidated"), WithItems);
    Catalog->Items.RemoveAt(0);
    Reject(TEXT("Removed catalog item cannot be restored"), WithItems);
    AddInfo(TEXT("[PrimalPersistence] Checked all truncated prefixes, corrupt/versioned envelopes and malicious lengths; destination preserved."));
    return true;
}
#endif
