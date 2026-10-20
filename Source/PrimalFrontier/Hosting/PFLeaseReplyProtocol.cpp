#include "Hosting/PFLeaseReplyProtocol.h"
#include "Persistence/PFCheckpointProtocol.h"
#include "Containers/StringConv.h"
#include "Serialization/JsonReader.h"

namespace PFLeaseReplyProtocolPrivate
{
    struct Field
    {
        EJsonNotation Type = EJsonNotation::Null;
        FString Text;
        bool bValue = false;
    };
    using Fields = TMap<FString, Field>;
    bool Refuse(FString& Error)
    {
        // Never copy service-controlled strings, secrets or response bodies to logs.
        Error = TEXT("[PrimalAgentTools] Lease reply refused: invalid context, status, shape or proof");
        return false;
    }
    bool Decimal(const FString& Text, uint64 Limit, uint64& Out)
    {
        if (Text.IsEmpty() || Text.Len() > 19 || Text[0] == '0') { return false; }
        uint64 Value = 0;
        for (TCHAR C : Text)
        {
            if (C < '0' || C > '9') { return false; }
            const uint64 Digit = C - '0';
            if (Value > (Limit - Digit) / 10) { return false; }
            Value = Value * 10 + Digit;
        }
        Out = Value;
        return true;
    }
    bool Fence(const FString& Text)
    {
        uint64 Ignored = 0;
        return Decimal(Text, static_cast<uint64>(MAX_int64), Ignored);
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
    bool Parse(int32 Status, const FString& Json, const TArray<FString>& Required, Fields& Out)
    {
        if (Status != 200 || Json.IsEmpty() || Json.Len() > FPFLeaseReplyProtocol::MaxResponseBytes ||
            FTCHARToUTF8(*Json, Json.Len()).Length() > FPFLeaseReplyProtocol::MaxResponseBytes) { return false; }
        const auto Reader = TJsonReaderFactory<>::Create(Json);
        EJsonNotation Token;
        if (!Reader->ReadNext(Token) || Token != EJsonNotation::ObjectStart) { return false; }
        bool bEnded = false;
        while (Reader->ReadNext(Token))
        {
            if (Token == EJsonNotation::ObjectEnd) { bEnded = true; break; }
            const FString Name = Reader->GetIdentifier();
            // Bounded flat replies only: reject unknown, duplicate (including
            // escaped spelling), nested, conflicting error/success fields.
            const bool bRequired = Required.ContainsByPredicate([&Name](const FString& Key)
                { return Key.Equals(Name, ESearchCase::CaseSensitive); });
            if (Out.Contains(Name) || (!bRequired && !Name.Equals(TEXT("replayed"), ESearchCase::CaseSensitive))) { return false; }
            Field Value; Value.Type = Token;
            switch (Token)
            {
            case EJsonNotation::String: Value.Text = Reader->GetValueAsString(); break;
            case EJsonNotation::Number: Value.Text = Reader->GetValueAsNumberString(); break;
            case EJsonNotation::Boolean: Value.bValue = Reader->GetValueAsBoolean(); break;
            case EJsonNotation::Null: break;
            default: return false;
            }
            Out.Add(Name, MoveTemp(Value));
        }
        if (!bEnded || Reader->ReadNext(Token) || !Reader->GetErrorMessage().IsEmpty()) { return false; }
        for (const FString& Name : Required) { if (!Out.Contains(Name)) { return false; } }
        const Field& Ok = Out[TEXT("ok")];
        const Field* Replay = Out.Find(TEXT("replayed"));
        return Ok.Type == EJsonNotation::Boolean && Ok.bValue &&
            (!Replay || (Replay->Type == EJsonNotation::Boolean && Replay->bValue));
    }
    bool Text(const Fields& Values, const TCHAR* Name, FString& Out)
    {
        const Field* Value = Values.Find(Name);
        if (!Value || Value->Type != EJsonNotation::String) { return false; }
        Out = Value->Text;
        return true;
    }
    bool Generation(const Field& Value, uint64& Out)
    {
        return (Value.Type == EJsonNotation::String || Value.Type == EJsonNotation::Number) &&
            Decimal(Value.Text, FPFCheckpointProtocol::MaxGeneration, Out);
    }
    bool Expiration(const FString& TextValue, FDateTime& Out)
    {
        // PostgreSQL UTC JSON timestamp: seconds, optional 1..6 fractional
        // digits, Z or +00:00. Validate the calendar before UE parsing.
        if (TextValue.Len() < 20 || TextValue.Len() > 32) { return false; }
        const int32 Zone = TextValue.EndsWith(TEXT("Z"), ESearchCase::CaseSensitive) ? TextValue.Len() - 1 :
            (TextValue.EndsWith(TEXT("+00:00"), ESearchCase::CaseSensitive) ? TextValue.Len() - 6 : -1);
        if (Zone < 19 || Zone > 26 || TextValue[4] != '-' || TextValue[7] != '-' || TextValue[10] != 'T' ||
            TextValue[13] != ':' || TextValue[16] != ':') { return false; }
        for (int32 I = 0; I < Zone; ++I)
        {
            if (I == 4 || I == 7 || I == 10 || I == 13 || I == 16) { continue; }
            if (I == 19) { if (TextValue[I] != '.' || Zone == 20) { return false; } continue; }
            if (TextValue[I] < '0' || TextValue[I] > '9') { return false; }
        }
        const int32 Year = FCString::Atoi(*TextValue.Mid(0, 4));
        if (Year < 1970 || !FDateTime::Validate(Year, FCString::Atoi(*TextValue.Mid(5, 2)),
            FCString::Atoi(*TextValue.Mid(8, 2)), FCString::Atoi(*TextValue.Mid(11, 2)),
            FCString::Atoi(*TextValue.Mid(14, 2)), FCString::Atoi(*TextValue.Mid(17, 2)), 0)) { return false; }
        // Metadata only; readiness uses monotonic send-time in the authority
        // guard, never a comparison between service UTC and local wall clock.
        return FDateTime::ParseIso8601(*TextValue, Out);
    }
    bool LeaseMetadata(const Fields& Values, FPFLeaseReply& Candidate)
    {
        FString LeaseText, Expiry;
        if (!Text(Values, TEXT("lease_id"), LeaseText) ||
            !FGuid::ParseExact(LeaseText, EGuidFormats::DigitsWithHyphensLower, Candidate.Lease) ||
            !Candidate.Lease.IsValid() || !Candidate.Lease.ToString(EGuidFormats::DigitsWithHyphensLower).Equals(LeaseText, ESearchCase::CaseSensitive) ||
            !Text(Values, TEXT("fence"), Candidate.Fence) || !Fence(Candidate.Fence) ||
            !Text(Values, TEXT("expires_at"), Expiry) || !Expiration(Expiry, Candidate.ExpiresUtc)) { return false; }
        Candidate.bReplayed = Values.Contains(TEXT("replayed"));
        return true;
    }
}

bool FPFLeaseReplyProtocol::ReadAcquire(int32 HttpStatus, const FString& Json, const FString& ExpectedWorld,
    FPFLeaseReply& Out, FString& Error)
{
    using namespace PFLeaseReplyProtocolPrivate;
    Error.Reset(); Fields Values; FPFLeaseReply Candidate; FString World;
    if (!FPFCheckpointProtocol::ValidWorldId(ExpectedWorld) ||
        !Parse(HttpStatus, Json, {TEXT("ok"), TEXT("world_id"), TEXT("lease_id"), TEXT("fence"),
            TEXT("expires_at"), TEXT("head_generation")}, Values) ||
        !Text(Values, TEXT("world_id"), World) || !World.Equals(ExpectedWorld, ESearchCase::CaseSensitive) || !LeaseMetadata(Values, Candidate))
    { return Refuse(Error); }
    const Field& Head = Values[TEXT("head_generation")];
    if (Head.Type != EJsonNotation::Null)
    {
        uint64 GenerationValue = 0;
        if (!Generation(Head, GenerationValue)) { return Refuse(Error); }
        Candidate.Head = GenerationValue;
    }
    Out = MoveTemp(Candidate);
    return true;
}

bool FPFLeaseReplyProtocol::ReadRenew(int32 HttpStatus, const FString& Json, const FGuid& ExpectedLease,
    const FString& ExpectedFence, FPFLeaseReply& Out, FString& Error)
{
    using namespace PFLeaseReplyProtocolPrivate;
    Error.Reset(); Fields Values; FPFLeaseReply Candidate;
    if (!ExpectedLease.IsValid() || !Fence(ExpectedFence) ||
        !Parse(HttpStatus, Json, {TEXT("ok"), TEXT("lease_id"), TEXT("fence"), TEXT("expires_at")}, Values) ||
        !LeaseMetadata(Values, Candidate) || Candidate.Lease != ExpectedLease || Candidate.Fence != ExpectedFence)
    { return Refuse(Error); }
    Out = MoveTemp(Candidate);
    return true;
}

bool FPFLeaseReplyProtocol::ReadCommit(int32 HttpStatus, const FString& Json, const FString& ExpectedWorld,
    const FString& ExpectedFence, uint64 ExpectedGeneration, const FString& ExpectedChecksum, FString& Error)
{
    using namespace PFLeaseReplyProtocolPrivate;
    Error.Reset(); Fields Values; FString World, FenceText, ChecksumText; uint64 GenerationValue = 0;
    if (!FPFCheckpointProtocol::ValidWorldId(ExpectedWorld) || !Fence(ExpectedFence) ||
        ExpectedGeneration == 0 || ExpectedGeneration > FPFCheckpointProtocol::MaxGeneration || !Checksum(ExpectedChecksum) ||
        !Parse(HttpStatus, Json, {TEXT("ok"), TEXT("committed"), TEXT("world_id"), TEXT("generation"),
            TEXT("checksum"), TEXT("fence")}, Values) ||
        Values[TEXT("committed")].Type != EJsonNotation::Boolean || !Values[TEXT("committed")].bValue ||
        !Text(Values, TEXT("world_id"), World) || !World.Equals(ExpectedWorld, ESearchCase::CaseSensitive) ||
        !Text(Values, TEXT("fence"), FenceText) || FenceText != ExpectedFence ||
        !Text(Values, TEXT("checksum"), ChecksumText) || !ChecksumText.Equals(ExpectedChecksum, ESearchCase::CaseSensitive) ||
        !Generation(Values[TEXT("generation")], GenerationValue) || GenerationValue != ExpectedGeneration)
    { return Refuse(Error); }
    return true;
}
