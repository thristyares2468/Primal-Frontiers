#include "Persistence/PFWorldSaveFormat.h"
#include "Persistence/PFSaveFileStore.h"
#include "Inventory/PFItemCatalog.h"
#include "Building/PFBuildingCatalog.h"
#include "Crafting/PFCraftingCatalog.h"
#include "Creatures/PFCreatureCatalog.h"
#include "Progression/PFProgressionSaveFormat.h"
#include "Progression/PFProgressionCatalog.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"
#include "JsonObjectConverter.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/Base64.h"
#include "Misc/DateTime.h"

namespace
{
    bool RefuseWorldRecord(FString& Error, const TCHAR* Message) { Error = Message; return false; }
    bool ValidWorldPoint(FVector V) { return !V.ContainsNaN() && V.GetAbsMax() <= 1.e9; }
    bool ValidWorldScalar(double V, double Max) { return FMath::IsFinite(V) && V >= 0 && V <= Max; }
    bool WorldMapAllowed(const FString& Map)
    {
        return Map == TEXT("L_PrimalFrontier_OpenWorld") || Map == TEXT("L_M7SurvivalArena") || Map == TEXT("L_Automation");
    }
}

bool FPFWorldSaveFormat::PackPlayer(const FPFPlayerSaveData& Data, const UPFItemCatalog& Items,
    double Weight, FString& Out, FString& Error)
{
    TArray<uint8> Bytes; FPFPlayerSaveLimits Limits; Limits.Weight = Weight;
    if (!FPFPlayerSaveFormat::Encode(Data, Items, Limits, Bytes, Error)) { return false; }
    Out = FBase64::Encode(Bytes); return true;
}
bool FPFWorldSaveFormat::UnpackPlayer(const FString& Text, const UPFItemCatalog& Items,
    double Weight, FPFPlayerSaveData& Out, FString& Error)
{
    if (Text.Len() > (FPFPlayerSaveFormat::MaxEncodedBytes + 2) / 3 * 4) { return RefuseWorldRecord(Error, TEXT("Player record too large")); }
    TArray<uint8> Bytes;
    if (!FBase64::Decode(Text, Bytes)) { return RefuseWorldRecord(Error, TEXT("Invalid player encoding")); }
    FPFPlayerSaveLimits Limits; Limits.Weight = Weight;
    return FPFPlayerSaveFormat::Decode(Bytes, Items, Limits, Out, Error);
}

bool FPFWorldSaveFormat::Validate(const FPFWorldSaveData& Data, const UPFItemCatalog& Items,
    const UPFBuildingCatalog& Buildings, const UPFCraftingCatalog& Crafting, const UPFCreatureCatalog& Creatures, FString& Error,
    const UPFProgressionCatalog* Progression)
{
    Error.Reset();
    if ((Data.Version != 1 && Data.Version != 2) || !WorldMapAllowed(Data.Map) || !ValidWorldScalar(Data.Hour, 24) || Data.Hour == 24 ||
        Data.Players.Num() > 32 || Data.Structures.Num() > 128 || Data.Resources.Num() > 128 ||
        Data.Creatures.Num() > 8 || Data.Spawners.Num() > 32 || Data.Pickups.Num() > 128)
    { return RefuseWorldRecord(Error, TEXT("Invalid world version/map/clock/record limits")); }
    TSet<FGuid> Players, Credentials, Stacks, Structures, CreatureIds;
    auto Batches = [&](const FString& Text, double Weight, FGuid Expected, bool bSingle)
    {
        FPFPlayerSaveData Record;
        if (!UnpackPlayer(Text, Items, Weight, Record, Error)) { return false; }
        if (Record.PlayerId != Expected || (bSingle && Record.Inventory.Num() != 1)) { return RefuseWorldRecord(Error, TEXT("Invalid inventory identity or pickup count")); }
        for (const auto& S : Record.Inventory)
        {
            if (Stacks.Contains(S.StackId)) { return RefuseWorldRecord(Error, TEXT("Duplicate stack across world containers")); }
            Stacks.Add(S.StackId);
        }
        return true;
    };
    for (const auto& P : Data.Players)
    {
        if (!P.PlayerId.IsValid() || Players.Contains(P.PlayerId) || !P.ReconnectCredential.IsValid() || Credentials.Contains(P.ReconnectCredential) ||
            P.CapturedUtc <= 0 || P.CapturedUtc > FDateTime::UtcNow().ToUnixTimestamp() + 60)
        { return RefuseWorldRecord(Error, TEXT("Invalid/duplicate player identity or capture time")); }
        Players.Add(P.PlayerId); Credentials.Add(P.ReconnectCredential);
        if (!Batches(P.Data, 30, P.PlayerId, false)) { return false; }
        if(Data.Version==1 && !P.Progression.IsEmpty()){return RefuseWorldRecord(Error,TEXT("V1 cannot contain progression"));}
        if(Data.Version==2)
        {
            FPFProgressionRecord Record;const auto* Catalog=Progression?Progression:GetDefault<UPFProgressionCatalog>();
            if(!UnpackProgression(P.Progression,P.PlayerId,*Catalog,Crafting,Items,Record,Error)){return false;}
        }
    }
    for (const auto& S : Data.Structures)
    {
        const auto* D = Buildings.Find(FName(*S.Definition, FNAME_Find));
        if (!D || !S.Id.IsValid() || Structures.Contains(S.Id) || !Players.Contains(S.Owner) || !ValidWorldPoint(S.Location) ||
            S.Rotation.ContainsNaN() || FMath::Abs(S.Rotation.Yaw) > 360 || S.Rotation.Pitch != 0 || S.Rotation.Roll != 0 || !ValidWorldScalar(S.Health, D->MaxHealth) || S.Health == 0 ||
            (S.DoorOpen && D->Kind != EPFBuildKind::Door)) { return RefuseWorldRecord(Error, TEXT("Invalid structure definition/ownership/transform/health")); }
        Structures.Add(S.Id);
        if (!Batches(S.Storage, 60, S.Id, false)) { return false; }
        FPFPlayerSaveData Bag; UnpackPlayer(S.Storage, Items, 60, Bag, Error);
        if (D->Kind != EPFBuildKind::Storage && !Bag.Inventory.IsEmpty()) { return RefuseWorldRecord(Error, TEXT("Non-storage piece contains items")); }
        if ((D->Kind == EPFBuildKind::Foundation) == S.Support.IsValid()) { return RefuseWorldRecord(Error, TEXT("Invalid foundation/support rule")); }
    }
    for (const auto& S : Data.Structures)
    {
        TSet<FGuid> Seen; const FPFStructureSaveRecord* Parent = &S;
        while (Parent->Support.IsValid())
        {
            if (Seen.Contains(Parent->Id)) { return RefuseWorldRecord(Error, TEXT("Cyclic structure support")); }
            Seen.Add(Parent->Id);
            const auto* ChildDefinition = Buildings.Find(FName(*Parent->Definition, FNAME_Find));
            const FGuid ChildOwner = Parent->Owner;
            const FGuid Id = Parent->Support;
            Parent = Data.Structures.FindByPredicate([&](const auto& P) { return P.Id == Id; });
            if (!Parent) { return RefuseWorldRecord(Error, TEXT("Missing support piece")); }
            const auto* D = Buildings.Find(FName(*Parent->Definition, FNAME_Find));
            const bool bRoof = ChildDefinition->Kind == EPFBuildKind::Floor || ChildDefinition->Kind == EPFBuildKind::Ceiling;
            const bool bCompatible = bRoof ? (D->Kind == EPFBuildKind::Wall || D->Kind == EPFBuildKind::Door) :
                (D->Kind == EPFBuildKind::Foundation || D->Kind == EPFBuildKind::Floor || D->Kind == EPFBuildKind::Ceiling);
            if (Parent->Owner != ChildOwner || !bCompatible)
            { return RefuseWorldRecord(Error, TEXT("Support ownership or piece kind mismatch")); }
        }
    }
    TSet<FString> ResourceNames, SpawnerNames;
    for (const auto& R : Data.Resources)
    {
        const auto* D = Crafting.Resource(FName(*R.Definition, FNAME_Find), &Items);
        if (!D || R.Name.IsEmpty() || R.Name.Len() > 128 || ResourceNames.Contains(R.Name) || R.Hits < 0 || R.Hits > D->Hits ||
            !ValidWorldScalar(R.Respawn, D->RespawnSeconds) || (R.Hits > 0 && R.Respawn != 0)) { return RefuseWorldRecord(Error, TEXT("Invalid resource state")); }
        ResourceNames.Add(R.Name);
    }
    for (const auto& C : Data.Creatures)
    {
        const auto* D = Creatures.Find(FName(*C.Definition, FNAME_Find));
        if (!D || !C.Id.IsValid() || CreatureIds.Contains(C.Id) || !ValidWorldPoint(C.Location) || !ValidWorldPoint(C.Home) ||
            !ValidWorldScalar(C.Health, D->Health) || !ValidWorldScalar(C.CorpseSeconds, 12) || (C.Health > 0 && C.CorpseSeconds != 0))
        { return RefuseWorldRecord(Error, TEXT("Invalid creature state")); }
        CreatureIds.Add(C.Id);
    }
    TSet<FGuid> Residents;
    for (const auto& S : Data.Spawners)
    {
        if (S.Name.IsEmpty() || S.Name.Len() > 128 || SpawnerNames.Contains(S.Name) || !ValidWorldScalar(S.Respawn, 300) ||
            (S.Resident.IsValid() && (!CreatureIds.Contains(S.Resident) || Residents.Contains(S.Resident))))
        { return RefuseWorldRecord(Error, TEXT("Invalid spawner or duplicate resident")); }
        SpawnerNames.Add(S.Name); if (S.Resident.IsValid()) { Residents.Add(S.Resident); }
    }
    for (const auto& P : Data.Pickups)
    {
        FPFPlayerSaveData Bag;
        if (!ValidWorldPoint(P.Location)) { return RefuseWorldRecord(Error, TEXT("Invalid pickup location")); }
        if (!UnpackPlayer(P.Data, Items, 100000, Bag, Error)) { return false; }
        if (Bag.Inventory.Num() != 1 || Bag.PlayerId != Bag.Inventory[0].StackId) { return RefuseWorldRecord(Error, TEXT("Invalid pickup inventory")); }
        if (!Batches(P.Data, 100000, Bag.PlayerId, true)) { return false; }
    }
    return true;
}

bool FPFWorldSaveFormat::Encode(const FPFWorldSaveData& Data, TArray<uint8>& Out, FString& Error)
{
    auto Root=MakeShared<FJsonObject>();
    if(!FJsonObjectConverter::UStructToJsonObject(FPFWorldSaveData::StaticStruct(),&Data,Root,0,0))
    {return RefuseWorldRecord(Error,TEXT("Cannot encode world metadata"));}
    if(Data.Version==1)
    {
        for(const auto& P:Data.Players){if(!P.Progression.IsEmpty()){return RefuseWorldRecord(Error,TEXT("V1 cannot discard progression"));}}
        const TArray<TSharedPtr<FJsonValue>>* Players=nullptr;
        if(!Root->TryGetArrayField(TEXT("players"),Players)){return RefuseWorldRecord(Error,TEXT("Cannot encode player array"));}
        for(const auto& Value:*Players){Value->AsObject()->RemoveField(TEXT("progression"));}
    }
    FString Json;
    if (!FJsonSerializer::Serialize(Root,TJsonWriterFactory<TCHAR,TCondensedJsonPrintPolicy<TCHAR>>::Create(&Json)))
    { return RefuseWorldRecord(Error, TEXT("Cannot encode world metadata")); }
    FTCHARToUTF8 Utf8(*Json);
    if (Utf8.Length() > FPFSaveFileStore::MaxPayloadBytes) { return RefuseWorldRecord(Error, TEXT("World metadata exceeds file limit")); }
    TArray<uint8> Bytes; Bytes.Append(reinterpret_cast<const uint8*>(Utf8.Get()), Utf8.Length());
    Out = MoveTemp(Bytes); Error.Reset(); return true;
}

bool FPFWorldSaveFormat::Decode(const TArray<uint8>& Bytes, FPFWorldSaveData& Out, FString& Error)
{
    if (Bytes.IsEmpty() || Bytes.Num() > FPFSaveFileStore::MaxPayloadBytes) { return RefuseWorldRecord(Error, TEXT("Invalid world metadata size")); }
    const FUTF8ToTCHAR Text(reinterpret_cast<const ANSICHAR*>(Bytes.GetData()), Bytes.Num());
    const FString Json(Text.Length(), Text.Get());
    TSharedPtr<FJsonObject> Root;
    if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json), Root) || !Root.IsValid())
    { return RefuseWorldRecord(Error, TEXT("Malformed world JSON")); }
    double Version = 0;
    if (!Root->TryGetNumberField(TEXT("version"), Version) || (Version != 1 && Version != 2))
    { return RefuseWorldRecord(Error, TEXT("Unsupported world version; migration required")); }
    const TPair<const TCHAR*, int32> Limits[] = {{TEXT("players"), 32}, {TEXT("structures"), 128}, {TEXT("resources"), 128},
        {TEXT("creatures"), 8}, {TEXT("spawners"), 32}, {TEXT("pickups"), 128}};
    for (const auto& Limit : Limits)
    {
        const TArray<TSharedPtr<FJsonValue>>* Array = nullptr;
        if (!Root->TryGetArrayField(Limit.Key, Array) || Array->Num() > Limit.Value) { return RefuseWorldRecord(Error, TEXT("Missing/oversized world record array")); }
    }
    FPFWorldSaveData Candidate; FText Why;
    const TArray<TSharedPtr<FJsonValue>>* Players=nullptr;Root->TryGetArrayField(TEXT("players"),Players);
    for(const auto& Value:*Players)
    {
        if(Value->Type!=EJson::Object){return RefuseWorldRecord(Error,TEXT("Invalid world player record"));}auto Player=Value->AsObject();FString Progression;
        if(Version==1)
        {
            if(Player->HasField(TEXT("progression")) && (!Player->TryGetStringField(TEXT("progression"),Progression) || !Progression.IsEmpty()))
            {return RefuseWorldRecord(Error,TEXT("V1 cannot conceal progression"));}
            Player->SetStringField(TEXT("progression"),TEXT("")); // New reflected field has explicit legacy default.
        }
        else if(!Player->TryGetStringField(TEXT("progression"),Progression) || Progression.IsEmpty() || Progression.Len()>(FPFProgressionSaveFormat::MaximumBytes+18)/3*4)
        {return RefuseWorldRecord(Error,TEXT("Missing or oversized V2 progression record"));}
    }
    if (!FJsonObjectConverter::JsonObjectToUStruct(Root.ToSharedRef(), &Candidate, 0, 0, true, &Why))
    { return RefuseWorldRecord(Error, TEXT("Invalid world metadata fields")); }
    Out = MoveTemp(Candidate); Error.Reset(); return true;
}

bool FPFWorldSaveFormat::PackProgression(FGuid Owner,const FPFProgressionRecord& Record,const UPFProgressionCatalog& Catalog,
    const UPFCraftingCatalog& Crafting,const UPFItemCatalog& Items,FString& Out,FString& Error)
{
    if(!Owner.IsValid()){return RefuseWorldRecord(Error,TEXT("Invalid progression owner"));}TArray<uint8> Payload;
    if(!FPFProgressionSaveFormat::Encode(Record,Catalog,Crafting,Items,Payload,Error)){return false;}
    TArray<uint8> Owned;FMemoryWriter Writer(Owned,true);Writer<<Owner;Writer.Serialize(Payload.GetData(),Payload.Num());
    if(Writer.IsError()){return RefuseWorldRecord(Error,TEXT("Progression owner encoding failed"));}Out=FBase64::Encode(Owned);Error.Reset();return true;
}
bool FPFWorldSaveFormat::UnpackProgression(const FString& Text,FGuid Owner,const UPFProgressionCatalog& Catalog,
    const UPFCraftingCatalog& Crafting,const UPFItemCatalog& Items,FPFProgressionRecord& Out,FString& Error)
{
    if(!Owner.IsValid() || Text.IsEmpty() || Text.Len()>(FPFProgressionSaveFormat::MaximumBytes+18)/3*4)
    {return RefuseWorldRecord(Error,TEXT("Invalid progression owner or encoding length"));}TArray<uint8> Owned;
    if(!FBase64::Decode(Text,Owned) || Owned.Num()<16+28 || Owned.Num()>16+FPFProgressionSaveFormat::MaximumBytes)
    {return RefuseWorldRecord(Error,TEXT("Invalid owner-bound progression bytes"));}
    FMemoryReader Reader(Owned,true);FGuid SavedOwner;Reader<<SavedOwner;
    if(Reader.IsError() || SavedOwner!=Owner){return RefuseWorldRecord(Error,TEXT("Progression owner mismatch"));}
    TArray<uint8> Payload;Payload.Append(Owned.GetData()+16,Owned.Num()-16);return FPFProgressionSaveFormat::Decode(Payload,Catalog,Crafting,Items,Out,Error);
}
bool FPFWorldSaveFormat::DecodeValidated(const TArray<uint8>& Bytes,const UPFItemCatalog& Items,const UPFBuildingCatalog& Buildings,
    const UPFCraftingCatalog& Crafting,const UPFCreatureCatalog& Creatures,FPFWorldSaveData& Out,FString& Error,const UPFProgressionCatalog* Progression)
{
    FPFWorldSaveData Candidate;if(!Decode(Bytes,Candidate,Error) || !Validate(Candidate,Items,Buildings,Crafting,Creatures,Error,Progression)){return false;}
    Out=MoveTemp(Candidate);Error.Reset();return true;
}
