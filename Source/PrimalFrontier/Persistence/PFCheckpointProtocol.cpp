#include "Persistence/PFCheckpointProtocol.h"
#include "Dom/JsonObject.h"
#include <openssl/sha.h>
#include "Misc/Base64.h"
#include "Misc/DateTime.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace
{
    bool RefuseCheckpointRecord(FString& Error, const TCHAR* Reason) { Error=Reason; return false; }

    bool Checksum(const TArray<uint8>& Bytes, FString& Out, FString& Error)
    {
        uint8 Signature[SHA256_DIGEST_LENGTH];
        if (!SHA256(Bytes.GetData(),Bytes.Num(),Signature))
        { return RefuseCheckpointRecord(Error,TEXT("SHA256 unavailable; checkpoint refused")); }
        Out=TEXT("sha256:")+BytesToHex(Signature,SHA256_DIGEST_LENGTH).ToLower(); return true;
    }

    bool SafeGeneration(const TSharedPtr<FJsonObject>& Object, uint64& Out)
    {
        // Postgres int8 is returned as a decimal string by Neon; accept safe JSON integers too.
        FString Text;
        if (Object->TryGetStringField(TEXT("generation"),Text))
        {
            if (Text.IsEmpty() || Text.Len()>16) { return false; }
            uint64 Value=0;
            for (TCHAR C:Text) { if (C<'0' || C>'9') { return false; } Value=Value*10+(C-'0'); }
            if (Value==0 || Value>FPFCheckpointProtocol::MaxGeneration) { return false; }
            Out=Value; return true;
        }
        double Value=0;
        if (!Object->TryGetNumberField(TEXT("generation"),Value) || !FMath::IsFinite(Value) ||
            Value<1 || Value>static_cast<double>(FPFCheckpointProtocol::MaxGeneration) || FMath::FloorToDouble(Value)!=Value) { return false; }
        Out=static_cast<uint64>(Value); return true;
    }

    bool ReadRecord(const FString& Json,const FString& WorldId,TSharedPtr<FJsonObject>& Out,uint64& Generation,FString& Error)
    {
        Error.Reset();
        if (!FPFCheckpointProtocol::ValidWorldId(WorldId) || Json.IsEmpty() || Json.Len()>FPFCheckpointProtocol::MaxResponseBytes)
        { return RefuseCheckpointRecord(Error,TEXT("Invalid checkpoint identifier or response size")); }
        TSharedPtr<FJsonObject> Root;
        const TSharedPtr<FJsonObject>* Record=nullptr;
        bool Ok=false; FString Id; double Version=0;
        if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json),Root) || !Root ||
            !Root->TryGetBoolField(TEXT("ok"),Ok) || !Ok || !Root->TryGetObjectField(TEXT("checkpoint"),Record) ||
            !(*Record)->TryGetStringField(TEXT("world_id"),Id) || Id!=WorldId ||
            !(*Record)->TryGetNumberField(TEXT("envelope_version"),Version) || Version!=1 || !SafeGeneration(*Record,Generation))
        { return RefuseCheckpointRecord(Error,TEXT("Invalid checkpoint identity, version or generation")); }
        const TSharedPtr<FJsonValue> Expiry=(*Record)->TryGetField(TEXT("expires_at"));
        if (!Expiry) { return RefuseCheckpointRecord(Error,TEXT("Missing checkpoint expiration metadata")); }
        if (Expiry->Type!=EJson::Null)
        {
            FString Text; FDateTime Date;
            if (!Expiry->TryGetString(Text) || !FDateTime::ParseIso8601(*Text,Date) || Date<=FDateTime::UtcNow())
            { return RefuseCheckpointRecord(Error,TEXT("Expired or invalid checkpoint expiration")); }
        }
        Out=*Record; return true;
    }
}

bool FPFCheckpointProtocol::ValidWorldId(const FString& Id)
{
    if (Id.IsEmpty() || Id.Len()>128) { return false; }
    for (TCHAR C:Id)
    { if (!((C>='A' && C<='Z') || (C>='a' && C<='z') || (C>='0' && C<='9') || C=='.' || C=='_' || C==':' || C=='-')) { return false; } }
    return true;
}

bool FPFCheckpointProtocol::BuildUpload(const FString& WorldId,const FPFSavedFile& File,FString& Out,FString& Error)
{
    Error.Reset(); TArray<uint8> Bytes; FString Digest;
    if (!ValidWorldId(WorldId) || File.Generation>MaxGeneration) { return RefuseCheckpointRecord(Error,TEXT("Invalid world identity or unsafe remote generation")); }
    if (!FPFSaveFileStore::EncodeEnvelope(File,Bytes,Error) || !Checksum(Bytes,Digest,Error)) { return false; }
    auto Root=MakeShared<FJsonObject>(); auto Payload=MakeShared<FJsonObject>();
    Root->SetStringField(TEXT("world_id"),WorldId);
    Root->SetNumberField(TEXT("generation"),static_cast<double>(File.Generation));
    Root->SetNumberField(TEXT("envelope_version"),1);
    Root->SetStringField(TEXT("checksum"),Digest);
    Root->SetField(TEXT("expires_at"),MakeShared<FJsonValueNull>());
    Payload->SetStringField(TEXT("format"),TEXT("pf-pfs-base64-v1"));
    Payload->SetStringField(TEXT("envelope"),FBase64::Encode(Bytes));
    Root->SetObjectField(TEXT("payload"),Payload);
    FString Json;
    if (!FJsonSerializer::Serialize(Root,TJsonWriterFactory<TCHAR,TCondensedJsonPrintPolicy<TCHAR>>::Create(&Json)))
    { return RefuseCheckpointRecord(Error,TEXT("Checkpoint serialization failed")); }
    Out=MoveTemp(Json); return true;
}

bool FPFCheckpointProtocol::ReadDownload(const FString& WorldId,uint64 MinimumGeneration,const FString& Json,FPFSavedFile& Out,FString& Error)
{
    TSharedPtr<FJsonObject> Record; uint64 Generation=0;
    if (!ReadRecord(Json,WorldId,Record,Generation,Error)) { return false; }
    if (MinimumGeneration>MaxGeneration || Generation<MinimumGeneration) { return RefuseCheckpointRecord(Error,TEXT("Stale checkpoint generation refused")); }
    const TSharedPtr<FJsonObject>* Payload=nullptr; FString Format,Encoded,Expected;
    if (!Record->TryGetObjectField(TEXT("payload"),Payload) || !(*Payload)->TryGetStringField(TEXT("format"),Format) ||
        Format!=TEXT("pf-pfs-base64-v1") || !(*Payload)->TryGetStringField(TEXT("envelope"),Encoded) ||
        Encoded.Len()>((FPFSaveFileStore::MaxPayloadBytes+34)/3)*4 || !Record->TryGetStringField(TEXT("checksum"),Expected))
    { return RefuseCheckpointRecord(Error,TEXT("Invalid checkpoint payload format or size")); }
    TArray<uint8> Bytes; FString Actual;
    if (!FBase64::Decode(Encoded,Bytes) || FBase64::Encode(Bytes)!=Encoded) { return RefuseCheckpointRecord(Error,TEXT("Invalid checkpoint base64 envelope")); }
    if (!Checksum(Bytes,Actual,Error)) { return false; }
    FPFSavedFile Candidate;
    if (Expected!=Actual) { return RefuseCheckpointRecord(Error,TEXT("Remote checkpoint SHA256 mismatch")); }
    if (!FPFSaveFileStore::DecodeEnvelope(Bytes,Candidate,Error)) { return false; }
    if (Candidate.Generation!=Generation) { return RefuseCheckpointRecord(Error,TEXT("Remote and envelope generation differ")); }
    Out=MoveTemp(Candidate); return true;
}

bool FPFCheckpointProtocol::ReadAcknowledgement(const FString& WorldId,const FPFSavedFile& Sent,const FString& Json,FString& Error)
{
    TSharedPtr<FJsonObject> Record; uint64 Generation=0;
    if (!ReadRecord(Json,WorldId,Record,Generation,Error)) { return false; }
    TArray<uint8> Bytes; FString Actual,Expected;
    if (!FPFSaveFileStore::EncodeEnvelope(Sent,Bytes,Error) || !Checksum(Bytes,Actual,Error)) { return false; }
    if (Generation!=Sent.Generation || !Record->TryGetStringField(TEXT("checksum"),Expected) || Expected!=Actual)
    { return RefuseCheckpointRecord(Error,TEXT("Checkpoint acknowledgement does not match sent generation/checksum")); }
    return true;
}
