#include "Persistence/PFPlayerSaveFormat.h"

#include "Inventory/PFItemCatalog.h"
#include "Misc/Crc.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"

namespace
{
    constexpr uint32 PlayerSaveMagic = 0x50504650; // PFPP, archive little-endian on Win64
    constexpr int32 MaxStacks = 64;
    constexpr int32 MaxItemIdLength = 64;

    bool Refuse(FString& Error, const TCHAR* Reason)
    {
        Error = Reason;
        return false;
    }

    bool ValidItemText(const FString& Text)
    {
        if (Text.IsEmpty() || Text.Len() > MaxItemIdLength) { return false; }
        for (const TCHAR C : Text)
        {
            if (!((C >= 'A' && C <= 'Z') || (C >= 'a' && C <= 'z') ||
                (C >= '0' && C <= '9') || C == '_')) { return false; }
        }
        return true;
    }

    bool InRange(float Value, float Maximum)
    {
        return FMath::IsFinite(Value) && Value >= 0 && Value <= Maximum;
    }

    // Serialize explicit scalar fields; avoid reflection/object references and arbitrary
    // FString/TArray length allocation from corrupt input. Scalars have fixed wire widths.
    void Fields(FArchive& Ar, FPFPlayerSaveData& Data)
    {
        Ar << Data.PlayerId;
        Ar << Data.Location.X << Data.Location.Y << Data.Location.Z;
        Ar << Data.Rotation.Pitch << Data.Rotation.Yaw << Data.Rotation.Roll;
        Ar << Data.Health << Data.Stamina << Data.Hunger << Data.Thirst;
    }
}

bool FPFPlayerSaveFormat::Validate(const FPFPlayerSaveData& Data, const UPFItemCatalog& Catalog,
    const FPFPlayerSaveLimits& Limits, FString& Error)
{
    Error.Reset();
    if (Limits.Slots < 1 || Limits.Slots > MaxStacks || !FMath::IsFinite(Limits.Weight) ||
        Limits.Weight <= 0 || !FMath::IsFinite(Limits.MaxHealth) || Limits.MaxHealth <= 0 ||
        !FMath::IsFinite(Limits.MaxStamina) || Limits.MaxStamina <= 0)
    {
        return Refuse(Error, TEXT("Invalid trusted save limits"));
    }
    if (!Data.PlayerId.IsValid()) { return Refuse(Error, TEXT("Missing stable player ID")); }
    for (const double Axis : {Data.Location.X, Data.Location.Y, Data.Location.Z})
    {
        if (!FMath::IsFinite(Axis) || FMath::Abs(Axis) > 1.e9)
        {
            return Refuse(Error, TEXT("Invalid player location"));
        }
    }
    for (const double Axis : {Data.Rotation.Pitch, Data.Rotation.Yaw, Data.Rotation.Roll})
    {
        if (!FMath::IsFinite(Axis) || FMath::Abs(Axis) > 360)
        {
            return Refuse(Error, TEXT("Invalid normalized player rotation"));
        }
    }
    if (!InRange(Data.Health, Limits.MaxHealth) || !InRange(Data.Stamina, Limits.MaxStamina) ||
        !InRange(Data.Hunger, 100) || !InRange(Data.Thirst, 100))
    {
        return Refuse(Error, TEXT("Invalid survival attributes"));
    }
    if (Data.Inventory.Num() > Limits.Slots) { return Refuse(Error, TEXT("Inventory exceeds slot capacity")); }
    TSet<FGuid> SeenStacks;
    double Weight = 0;
    for (const FPFSavedItemStack& Stack : Data.Inventory)
    {
        if (!Stack.StackId.IsValid() || SeenStacks.Contains(Stack.StackId))
        {
            return Refuse(Error, TEXT("Missing or duplicate stack ID"));
        }
        SeenStacks.Add(Stack.StackId);
        const FPFItemDefinition* Definition = Catalog.Find(Stack.ItemId);
        if (!Definition || !ValidItemText(Stack.ItemId.ToString()))
        {
            return Refuse(Error, TEXT("Unknown or invalid catalog item"));
        }
        if (Stack.Quantity < 1 || Stack.Quantity > Definition->StackLimit)
        {
            return Refuse(Error, TEXT("Invalid stack quantity"));
        }
        const double Remaining = Stack.RemainingFreshnessSeconds;
        if (!FMath::IsFinite(Remaining) || (Definition->ShelfLifeSeconds == 0 ? Remaining != 0 :
            Remaining <= 0 || Remaining > Definition->ShelfLifeSeconds))
        {
            return Refuse(Error, TEXT("Invalid or expired food freshness"));
        }
        Weight += static_cast<double>(Definition->Weight) * Stack.Quantity;
    }
    if (!FMath::IsFinite(Weight) || Weight > Limits.Weight + 1.e-4)
    {
        return Refuse(Error, TEXT("Inventory exceeds weight capacity"));
    }
    return true;
}

bool FPFPlayerSaveFormat::Encode(const FPFPlayerSaveData& Data, const UPFItemCatalog& Catalog,
    const FPFPlayerSaveLimits& Limits, TArray<uint8>& OutBytes, FString& Error)
{
    if (!Validate(Data, Catalog, Limits, Error)) { return false; }
    TArray<uint8> Payload;
    FMemoryWriter Writer(Payload, true);
    FPFPlayerSaveData Copy = Data;
    Fields(Writer, Copy);
    int32 Count = Copy.Inventory.Num();
    Writer << Count;
    for (FPFSavedItemStack& Stack : Copy.Inventory)
    {
        Writer << Stack.StackId;
        const FString ItemText = Stack.ItemId.ToString();
        int32 Length = ItemText.Len();
        Writer << Length;
        for (const TCHAR C : ItemText)
        {
            uint8 Ascii = static_cast<uint8>(C);
            Writer << Ascii;
        }
        Writer << Stack.Quantity << Stack.RemainingFreshnessSeconds;
    }
    if (Writer.IsError() || Payload.Num() > MaxEncodedBytes - EnvelopeBytes)
    {
        return Refuse(Error, TEXT("Player payload exceeds size limit"));
    }
    TArray<uint8> Encoded;
    FMemoryWriter Envelope(Encoded, true);
    uint32 WireMagic = PlayerSaveMagic;
    uint32 Version = CurrentVersion;
    uint32 Size = Payload.Num();
    uint32 Crc = FCrc::MemCrc32(Payload.GetData(), Payload.Num());
    Envelope << WireMagic << Version << Size << Crc;
    Envelope.Serialize(Payload.GetData(), Payload.Num());
    if (Envelope.IsError()) { return Refuse(Error, TEXT("Unable to encode save envelope")); }
    OutBytes = MoveTemp(Encoded);
    return true;
}

bool FPFPlayerSaveFormat::Decode(const TArray<uint8>& Bytes, const UPFItemCatalog& Catalog,
    const FPFPlayerSaveLimits& Limits, FPFPlayerSaveData& OutData, FString& Error)
{
    Error.Reset();
    if (Bytes.Num() < EnvelopeBytes || Bytes.Num() > MaxEncodedBytes)
    {
        return Refuse(Error, TEXT("Invalid save file size"));
    }
    FMemoryReader Reader(Bytes, true);
    uint32 WireMagic = 0, Version = 0, Size = 0, Crc = 0;
    Reader << WireMagic << Version << Size << Crc;
    if (WireMagic != PlayerSaveMagic) { return Refuse(Error, TEXT("Invalid save signature")); }
    if (Version != CurrentVersion) { return Refuse(Error, TEXT("Unsupported save version; migration required")); }
    if (Size != static_cast<uint32>(Bytes.Num() - EnvelopeBytes))
    {
        return Refuse(Error, TEXT("Save length mismatch"));
    }
    if (FCrc::MemCrc32(Bytes.GetData() + EnvelopeBytes, Size) != Crc)
    {
        return Refuse(Error, TEXT("Save checksum mismatch"));
    }
    FPFPlayerSaveData Candidate;
    Fields(Reader, Candidate);
    int32 Count = 0;
    Reader << Count;
    if (Reader.IsError() || Count < 0 || Count > MaxStacks)
    {
        return Refuse(Error, TEXT("Invalid encoded stack count"));
    }
    Candidate.Inventory.Reserve(Count);
    for (int32 Index = 0; Index < Count; ++Index)
    {
        FPFSavedItemStack Stack;
        int32 Length = 0;
        Reader << Stack.StackId << Length;
        if (Reader.IsError() || Length < 1 || Length > MaxItemIdLength ||
            Reader.TotalSize() - Reader.Tell() < static_cast<int64>(Length) + 12)
        {
            return Refuse(Error, TEXT("Invalid encoded item ID length"));
        }
        FString ItemText;
        ItemText.Reserve(Length);
        for (int32 Char = 0; Char < Length; ++Char)
        {
            uint8 Ascii = 0;
            Reader << Ascii;
            ItemText.AppendChar(static_cast<TCHAR>(Ascii));
        }
        if (!ValidItemText(ItemText)) { return Refuse(Error, TEXT("Invalid encoded item ID")); }
        // Resolve only existing IDs; never intern arbitrary save-file strings in the FName pool.
        for (const FPFItemDefinition& Definition : Catalog.Items)
        {
            if (Definition.Id.ToString().Equals(ItemText, ESearchCase::IgnoreCase))
            {
                Stack.ItemId = Definition.Id;
                break;
            }
        }
        if (Stack.ItemId.IsNone()) { return Refuse(Error, TEXT("Unknown catalog item in save")); }
        Reader << Stack.Quantity << Stack.RemainingFreshnessSeconds;
        if (Reader.IsError()) { return Refuse(Error, TEXT("Truncated inventory record")); }
        Candidate.Inventory.Add(Stack);
    }
    if (Reader.IsError() || Reader.Tell() != Reader.TotalSize())
    {
        return Refuse(Error, TEXT("Truncated or trailing player payload"));
    }
    if (!Validate(Candidate, Catalog, Limits, Error)) { return false; }
    OutData = MoveTemp(Candidate);
    return true;
}
