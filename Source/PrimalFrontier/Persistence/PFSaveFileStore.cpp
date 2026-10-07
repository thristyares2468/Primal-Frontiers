#include "Persistence/PFSaveFileStore.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Misc/Crc.h"
#include "Misc/DateTime.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"

namespace
{
    constexpr uint32 FileMagic = 0x46534650;
    constexpr uint32 FileVersion = 1;
    constexpr int32 HeaderBytes = 32;

    bool ReadGeneration(const FString& Path, FPFSavedFile& Out)
    {
        TUniquePtr<FArchive> File(IFileManager::Get().CreateFileReader(*Path, FILEREAD_Silent));
        if (!File || File->TotalSize() < HeaderBytes || File->TotalSize() > HeaderBytes + FPFSaveFileStore::MaxPayloadBytes) { return false; }
        uint32 Magic = 0, Version = 0, Length = 0, Crc = 0;
        FPFSavedFile Candidate;
        *File << Magic << Version << Candidate.Generation << Candidate.SavedUtc << Length << Crc;
        if (File->IsError() || Magic != FileMagic || Version != FileVersion || Candidate.Generation == 0 ||
            Candidate.SavedUtc <= 0 || Candidate.SavedUtc > FDateTime::UtcNow().ToUnixTimestamp() + 60 ||
            Length == 0 || Length != static_cast<uint32>(File->TotalSize() - HeaderBytes)) { return false; }
        Candidate.Payload.SetNumUninitialized(Length);
        File->Serialize(Candidate.Payload.GetData(), Length);
        if (File->IsError() || FCrc::MemCrc32(Candidate.Payload.GetData(), Length) != Crc || !File->Close()) { return false; }
        Out = MoveTemp(Candidate); return true;
    }
}

bool FPFSaveFileStore::ValidSlot(const FString& Slot)
{
    if (Slot.IsEmpty() || Slot.Len() > 64) { return false; }
    for (const TCHAR C : Slot)
    {
        if (!((C >= 'A' && C <= 'Z') || (C >= 'a' && C <= 'z') || (C >= '0' && C <= '9') || C == '_')) { return false; }
    }
    return true;
}

FString FPFSaveFileStore::Path(const FString& Slot, bool bSecond)
{
    return ValidSlot(Slot) ? FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Persistence"), Slot + (bSecond ? TEXT(".b.pfs") : TEXT(".a.pfs"))) : FString();
}

bool FPFSaveFileStore::Read(const FString& Slot, FPFSavedFile& Out, FString& Error)
{
    Error.Reset();
    if (!ValidSlot(Slot)) { Error = TEXT("Invalid save slot identifier"); return false; }
    FPFSavedFile A, B;
    const bool GoodA = ReadGeneration(Path(Slot, false), A);
    const bool GoodB = ReadGeneration(Path(Slot, true), B);
    if (!GoodA && !GoodB) { Error = TEXT("No valid save generation; missing or corrupt files preserved"); return false; }
    FPFSavedFile Candidate = (GoodB && (!GoodA || B.Generation > A.Generation)) ? MoveTemp(B) : MoveTemp(A);
    Candidate.bRecoveredBackup = (!GoodA && IFileManager::Get().FileExists(*Path(Slot, false))) ||
        (!GoodB && IFileManager::Get().FileExists(*Path(Slot, true)));
    Out = MoveTemp(Candidate); return true;
}

bool FPFSaveFileStore::Write(const FString& Slot, const TArray<uint8>& Payload, FString& Error)
{
    Error.Reset();
    if (!IsInGameThread() || !ValidSlot(Slot) || Payload.IsEmpty() || Payload.Num() > MaxPayloadBytes)
    {
        Error = TEXT("Requires game thread, valid slot and bounded nonempty payload"); return false;
    }
    FPFSavedFile A, B;
    const bool GoodA = ReadGeneration(Path(Slot, false), A);
    const bool GoodB = ReadGeneration(Path(Slot, true), B);
    if ((!GoodA && IFileManager::Get().FileExists(*Path(Slot, false))) ||
        (!GoodB && IFileManager::Get().FileExists(*Path(Slot, true))))
    {
        Error = TEXT("Corrupt generation preserved; restore/archive it explicitly before saving again"); return false;
    }
    const uint64 Latest = FMath::Max(A.Generation, B.Generation);
    if (Latest == MAX_uint64) { Error = TEXT("Save generation counter exhausted"); return false; }
    const bool bSecond = GoodA && (!GoodB || A.Generation > B.Generation);
    const FString Destination = Path(Slot, bSecond);
    const FString Temporary = Destination + TEXT(".pending");
    IFileManager::Get().MakeDirectory(*FPaths::GetPath(Destination), true);
    TArray<uint8> Bytes;
    FMemoryWriter Writer(Bytes, true);
    uint32 Magic = FileMagic, Version = FileVersion, Length = Payload.Num();
    uint64 Generation = Latest + 1;
    int64 Utc = FDateTime::UtcNow().ToUnixTimestamp();
    uint32 Crc = FCrc::MemCrc32(Payload.GetData(), Payload.Num());
    Writer << Magic << Version << Generation << Utc << Length << Crc;
    Bytes.Append(Payload);
    TUniquePtr<FArchive> File(IFileManager::Get().CreateFileWriter(*Temporary));
    if (!File) { Error = TEXT("Cannot create pending save; active generation preserved"); return false; }
    File->Serialize(Bytes.GetData(), Bytes.Num());
    const bool Written = !File->IsError() && File->Close(); File.Reset();
    FPFSavedFile Verified;
    if (!Written || !ReadGeneration(Temporary, Verified) || Verified.Payload != Payload || Verified.Generation != Generation)
    {
        Error = TEXT("Pending save failed verification; active generation preserved"); return false;
    }
    // Unreal's generic Move deletes an existing destination before rename. Only replace
    // the INACTIVE generation, so interrupted replacement cannot erase the current save.
    if (!IFileManager::Get().Move(*Destination, *Temporary, true, false, false, true))
    {
        Error = TEXT("Cannot publish inactive generation; active generation preserved"); return false;
    }
    return true;
}
