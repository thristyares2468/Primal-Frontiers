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
        TArray<uint8> Bytes;
        Bytes.SetNumUninitialized(static_cast<int32>(File->TotalSize()));
        File->Serialize(Bytes.GetData(), Bytes.Num());
        if (File->IsError() || !File->Close()) { return false; }
        FString Error;
        return FPFSaveFileStore::DecodeEnvelope(Bytes, Out, Error);
    }
}

bool FPFSaveFileStore::DecodeEnvelope(const TArray<uint8>& Bytes, FPFSavedFile& Out, FString& Error)
{
    Error.Reset();
    if (Bytes.Num() < HeaderBytes || Bytes.Num() > HeaderBytes + MaxPayloadBytes)
    { Error = TEXT("Invalid save envelope size"); return false; }
    FMemoryReader Reader(Bytes, true);
    uint32 Magic=0, Version=0, Length=0, Crc=0;
    FPFSavedFile Candidate;
    Reader << Magic << Version << Candidate.Generation << Candidate.SavedUtc << Length << Crc;
    if (Reader.IsError() || Magic!=FileMagic || Version!=FileVersion || Candidate.Generation==0 ||
        Candidate.SavedUtc<=0 || Candidate.SavedUtc>FDateTime::UtcNow().ToUnixTimestamp()+60 ||
        Length==0 || Length!=static_cast<uint32>(Bytes.Num()-HeaderBytes))
    { Error=TEXT("Invalid save envelope header"); return false; }
    Candidate.Payload.SetNumUninitialized(Length);
    Reader.Serialize(Candidate.Payload.GetData(),Length);
    if (Reader.IsError() || FCrc::MemCrc32(Candidate.Payload.GetData(),Length)!=Crc)
    { Error=TEXT("Save envelope checksum mismatch"); return false; }
    Out=MoveTemp(Candidate); return true;
}

bool FPFSaveFileStore::EncodeEnvelope(const FPFSavedFile& File, TArray<uint8>& Out, FString& Error)
{
    Error.Reset();
    if (File.Payload.IsEmpty() || File.Payload.Num()>MaxPayloadBytes)
    { Error=TEXT("Invalid save payload size"); return false; }
    TArray<uint8> Bytes;
    FMemoryWriter Writer(Bytes,true);
    uint32 Magic=FileMagic, Version=FileVersion, Length=File.Payload.Num();
    uint64 Generation=File.Generation;
    int64 Utc=File.SavedUtc;
    uint32 Crc=FCrc::MemCrc32(File.Payload.GetData(),File.Payload.Num());
    Writer << Magic << Version << Generation << Utc << Length << Crc;
    Bytes.Append(File.Payload);
    FPFSavedFile Checked;
    if (!DecodeEnvelope(Bytes,Checked,Error)) { return false; }
    Out=MoveTemp(Bytes); return true;
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

FString FPFSaveFileStore::OriginalPath(const FString& Slot,const FString& Edge)
{
    return ValidSlot(Slot) && ValidSlot(Edge) ? FPaths::Combine(FPaths::ProjectSavedDir(),TEXT("Persistence/Originals"),Slot,Edge+TEXT(".pfs")) : FString();
}

bool FPFSaveFileStore::ReadOriginal(const FString& Slot,const FString& Edge,FPFSavedFile& Out,FString& Error)
{
    Error.Reset();const FString Original=OriginalPath(Slot,Edge);FPFSavedFile Candidate;
    if(Original.IsEmpty() || !ReadGeneration(Original,Candidate))
    {Error=TEXT("Original backup missing, invalid or corrupt; files preserved");return false;}
    Out=MoveTemp(Candidate);return true;
}

bool FPFSaveFileStore::PreserveOriginal(const FString& Slot,const FString& Edge,FString& Error)
{
    Error.Reset();const FString Original=OriginalPath(Slot,Edge);
    if(!IsInGameThread() || Original.IsEmpty())
    {Error=TEXT("Requires game thread and bounded slot/migration edge identifiers");return false;}
    if(IFileManager::Get().DirectoryExists(*Original))
    {Error=TEXT("Original backup destination occupied by a directory; source preserved");return false;}
    if(IFileManager::Get().FileExists(*Original))
    {
        FPFSavedFile Existing;
        if(!ReadGeneration(Original,Existing)){Error=TEXT("Corrupt original backup preserved; explicit recovery required");return false;}
        return true; // Retain the FIRST original, even after ordinary A/B rotation.
    }
    FPFSavedFile A,B;
    const bool GoodA=ReadGeneration(Path(Slot,false),A),GoodB=ReadGeneration(Path(Slot,true),B);
    if((!GoodA && IFileManager::Get().FileExists(*Path(Slot,false))) || (!GoodB && IFileManager::Get().FileExists(*Path(Slot,true))))
    {Error=TEXT("Corrupt source generation preserved; explicit recovery before migration backup");return false;}
    if(!GoodA && !GoodB){Error=TEXT("No valid source generation; no original created");return false;}
    const bool Second=GoodB && (!GoodA || B.Generation>A.Generation);
    const auto& Source=Second?B:A;
    if(!IFileManager::Get().MakeDirectory(*FPaths::GetPath(Original),true))
    {Error=TEXT("Cannot create original backup directory; source preserved");return false;}
    // No replacement. A partial/failed destination is deliberately retained and
    // blocks a later attempt instead of being deleted or overwritten. Copy keeps
    // the exact source envelope/generation/time, rather than re-encoding as a save.
    TUniquePtr<FArchive> Reader(IFileManager::Get().CreateFileReader(*Path(Slot,Second),FILEREAD_Silent));
    if(!Reader || Reader->TotalSize()<HeaderBytes || Reader->TotalSize()>HeaderBytes+MaxPayloadBytes)
    {Error=TEXT("Cannot read bounded source envelope; source preserved");return false;}
    TArray<uint8> Bytes;Bytes.SetNumUninitialized(static_cast<int32>(Reader->TotalSize()));
    Reader->Serialize(Bytes.GetData(),Bytes.Num());
    const bool Read=!Reader->IsError() && Reader->Close();Reader.Reset();
    if(!Read){Error=TEXT("Source envelope read failed; source preserved");return false;}
    // Write through bounded archives, not platform CopyFile cleanup/replacement
    // policy. Our failure path never deletes a partial original.
    TUniquePtr<FArchive> File(IFileManager::Get().CreateFileWriter(*Original,FILEWRITE_NoReplaceExisting|FILEWRITE_Silent));
    if(!File){Error=TEXT("Cannot create original backup; source and any destination preserved");return false;}
    File->Serialize(Bytes.GetData(),Bytes.Num());
    const bool Written=!File->IsError() && File->Close();File.Reset();
    if(!Written){Error=TEXT("Original write failed; source and partial destination preserved");return false;}
    FPFSavedFile Verified;
    if(!ReadGeneration(Original,Verified) || Verified.Generation!=Source.Generation || Verified.SavedUtc!=Source.SavedUtc || Verified.Payload!=Source.Payload)
    {Error=TEXT("Original backup verification failed; source/destination preserved");return false;}
    return true;
}
