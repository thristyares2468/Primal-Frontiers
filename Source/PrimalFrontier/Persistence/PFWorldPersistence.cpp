#include "Persistence/PFWorldPersistence.h"
#include "Persistence/PFWorldSaveFormat.h"
#include "Persistence/PFSaveFileStore.h"
#include "Persistence/PFSessionGameInstance.h"
#include "Persistence/PFPlayerSaveAdapter.h"
#include "Inventory/PFInventoryComponent.h"
#include "Inventory/PFInventoryPlayerState.h"
#include "Inventory/PFItemCatalog.h"
#include "Inventory/PFItemPickup.h"
#include "Building/PFBuildPiece.h"
#include "Building/PFBuildingCatalog.h"
#include "Crafting/PFResourceNode.h"
#include "Crafting/PFCraftingCatalog.h"
#include "Creatures/PFCreature.h"
#include "Creatures/PFCreatureSpawner.h"
#include "Creatures/PFCreatureCatalog.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "World/PFWorldClock.h"
#include "PFAssetPaths.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/DateTime.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

namespace
{
    int64 PersistenceUtcNow() { return FDateTime::UtcNow().ToUnixTimestamp(); }
    FString PersistenceMapName(UWorld* W) { return UWorld::RemovePIEPrefix(W->GetMapName()); }
    bool ParseReconnectCredential(const FString& Options, FGuid& Out)
    {
        const FString Text = UGameplayStatics::ParseOption(Options, TEXT("PFReconnect"));
        return Text.IsEmpty() || (FGuid::ParseExact(Text, EGuidFormats::Digits, Out) && Out.IsValid());
    }
}

bool UPFWorldPersistence::Authority(FString& Error)
{
    if (!IsInGameThread() || !GetWorld()->IsGameWorld() || GetWorld()->GetNetMode() == NM_Client || !GetWorld()->GetAuthGameMode())
    { Error = TEXT("Requires authoritative gameplay world"); return false; }
    if (!Items) { Items = LoadObject<UPFItemCatalog>(nullptr, PFAssetPaths::ItemCatalog); }
    if (!Buildings) { Buildings = LoadObject<UPFBuildingCatalog>(nullptr, PFAssetPaths::BuildingCatalog); }
    if (!Crafting) { Crafting = LoadObject<UPFCraftingCatalog>(nullptr, PFAssetPaths::CraftingCatalog); }
    if (!Creatures) { Creatures = LoadObject<UPFCreatureCatalog>(nullptr, PFAssetPaths::CreatureCatalog); }
    if (!Items || !Buildings || !Crafting || !Creatures) { Error = TEXT("Missing persistence catalogs"); return false; }
    return true;
}
bool UPFWorldPersistence::PackPlayer(const FPFPlayerSaveData& Player, double Weight, FString& Out, FString& Error)
{ return FPFWorldSaveFormat::PackPlayer(Player, *Items, Weight, Out, Error); }
bool UPFWorldPersistence::UnpackPlayer(const FString& Text, double Weight, FPFPlayerSaveData& Out, FString& Error)
{ return FPFWorldSaveFormat::UnpackPlayer(Text, *Items, Weight, Out, Error); }
bool UPFWorldPersistence::Validate(const FPFWorldSaveData& Data, FString& Error)
{ return Authority(Error) && FPFWorldSaveFormat::Validate(Data, *Items, *Buildings, *Crafting, *Creatures, Error); }
bool UPFWorldPersistence::Encode(const FPFWorldSaveData& Data, TArray<uint8>& Out, FString& Error)
{ return Validate(Data, Error) && FPFWorldSaveFormat::Encode(Data, Out, Error); }
bool UPFWorldPersistence::Decode(const TArray<uint8>& Bytes, FPFWorldSaveData& Out, FString& Error)
{
    FPFWorldSaveData Candidate;
    if (!Authority(Error) || !FPFWorldSaveFormat::DecodeValidated(Bytes, *Items, *Buildings, *Crafting, *Creatures, Candidate, Error)) { return false; }
    // Compatibility bytes can be verified before player progression restoration exists.
    // Never accept V2 and silently lose its fields during today's V1 capture.
    if(Candidate.Version==2){Error=TEXT("World V2 progression restoration is not integrated; load refused to preserve saved data");return false;}
    Out = MoveTemp(Candidate); return true;
}

bool UPFWorldPersistence::CheckLogin(const FString& Options, FString& Error) const
{
    if (bStartupBlocked) { Error = TEXT("Server save failed startup validation"); return false; }
    FGuid Token;
    if (!ParseReconnectCredential(Options, Token)) { Error = TEXT("Invalid reconnect credential"); return false; }
    if (!Token.IsValid())
    {
        if (Roster.Players.Num() >= 32) { Error = TEXT("Development save limit: 32 player identities"); return false; }
        return true;
    }
    const auto* Entry = Roster.Players.FindByPredicate([&](const auto& P) { return P.ReconnectCredential == Token; });
    // Solo worlds share a local endpoint profile. Opening a different world can
    // replace that credential; its sole saved owner is still this local player.
    // Match Login's existing standalone adoption rule while retaining the
    // connected-identity check below. Network servers never adopt unknown tokens.
    if(!Entry && GetWorld()->GetNetMode()==NM_Standalone && Roster.Players.Num()==1){Entry=&Roster.Players[0];}
    if (!Entry)
    {
        if (!bLoadedWorld && Roster.Players.Num() < 32) { return true; } // Fresh world, credentials from an old session are not claims.
        Error = TEXT("Unknown reconnect credential for this server save"); return false;
    }
    for (auto It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        const auto* PS = It->Get() ? It->Get()->GetPlayerState<APFInventoryPlayerState>() : nullptr;
        if (PS && PS->PersistentPlayerId == Entry->PlayerId) { Error = TEXT("Player identity already connected"); return false; }
    }
    return true;
}

void UPFWorldPersistence::Login(APFSurvivalPlayerController* PC, const FString& Options)
{
    FString Error;
    if (!Authority(Error) || !PC || !PC->GetPlayerState<APFInventoryPlayerState>()) { return; }
    FGuid Token; ParseReconnectCredential(Options, Token);
    auto* Entry = Roster.Players.FindByPredicate([&](const auto& P) { return P.ReconnectCredential == Token && Token.IsValid(); });
    // Standalone has one local owner; server may adopt the sole saved record without a network identity claim.
    if (!Entry && GetWorld()->GetNetMode() == NM_Standalone && Roster.Players.Num() == 1) { Entry = &Roster.Players[0]; }
    if (!Entry)
    {
        FPFWorldPlayerRecord Fresh; Fresh.PlayerId = FGuid::NewGuid(); Fresh.ReconnectCredential = FGuid::NewGuid();
        Fresh.CapturedUtc = PersistenceUtcNow(); Roster.Players.Add(Fresh); Entry = &Roster.Players.Last();
    }
    PC->GetPlayerState<APFInventoryPlayerState>()->PersistentPlayerId = Entry->PlayerId;
    RestoredLogins.Remove(Entry->PlayerId);
    if (!Entry->Data.IsEmpty()) { PendingRestores.Add(Entry->PlayerId); }
    PC->GetPlayerState<APFInventoryPlayerState>()->ForceNetUpdate();
    for (TActorIterator<APFBuildPiece> It(GetWorld()); It; ++It) { It->BindPersistentOwner(PC->PlayerState); }
}

bool UPFWorldPersistence::RestorePlayer(APFSurvivalPlayerController* PC, bool* bOutRestored)
{
    if(bOutRestored){*bOutRestored=false;}
    FString Error;
    if (!Authority(Error) || !IsValid(PC) || !PC->GetPlayerState<APFInventoryPlayerState>()) { return false; }
    const FGuid Id = PC->GetPlayerState<APFInventoryPlayerState>()->PersistentPlayerId;
    const auto* Entry = Roster.Players.FindByPredicate([&](const auto& P) { return P.PlayerId == Id; });
    if(!Entry){return false;} // No setup success for an unregistered survivor.
    // PostLogin's deferred call runs after SetPlayer has attached the network connection.
    // Sending this RPC during InitNewPlayer silently executes before it can reach the client.
    if (Entry) { PC->ClientRememberReconnectCredential(Entry->ReconnectCredential); }
    if (!PendingRestores.Contains(Id))
    {
        // Startup Apply may already have successfully restored this controller.
        if(bOutRestored){*bOutRestored=RestoredLogins.Contains(Id);}
        return true;
    }
    if (Entry->Data.IsEmpty()) { return false; } // Pending restoration requires a saved record.
    FPFPlayerSaveData Data;
    if (!UnpackPlayer(Entry->Data, 30, Data, Error) || !FPFPlayerSaveAdapter::Restore(PC, Data, FMath::Max<int64>(0, PersistenceUtcNow() - Entry->CapturedUtc), Error))
    {
        UE_LOG(LogPFSurvival, Error, TEXT("[PrimalPersistence] Reconnect restoration refused: %s"), *Error); return false;
    }
    PendingRestores.Remove(Id);
    RestoredLogins.Add(Id);
    if(bOutRestored){*bOutRestored=true;}
    UE_LOG(LogPFSurvival, Display, TEXT("[PrimalPersistence] Reconnect restored server-owned player record"));
    return true;
}

void UPFWorldPersistence::Logout(APFSurvivalPlayerController* PC)
{
    FString Error; FPFPlayerSaveData Player;
    const auto* PS = PC ? PC->GetPlayerState<APFInventoryPlayerState>() : nullptr;
    if(PS){RestoredLogins.Remove(PS->PersistentPlayerId);}
    // A failed restore must never replace its prior save with the default spawn's empty bag.
    if (PS && PendingRestores.Contains(PS->PersistentPlayerId)) { PendingRestores.Remove(PS->PersistentPlayerId); return; }
    if (!Authority(Error) || !FPFPlayerSaveAdapter::Capture(PC, Player, Error)) { return; }
    auto* Entry = Roster.Players.FindByPredicate([&](const auto& P) { return P.PlayerId == Player.PlayerId; });
    if (!Entry || !PackPlayer(Player, 30, Entry->Data, Error)) { return; }
    Entry->CapturedUtc = PersistenceUtcNow();
    if (!ActiveSlot.IsEmpty() && !Save(ActiveSlot, Error))
    { UE_LOG(LogPFSurvival, Error, TEXT("[PrimalPersistence] Disconnect save refused: %s"), *Error); }
}

bool UPFWorldPersistence::Capture(FPFWorldSaveData& Out, FString& Error)
{
    if (bStartupBlocked)
    {
        Error = TEXT("Startup restoration failed; capture/save refused to preserve saved data. Resolve the load failure and restart.");
        return false;
    }
    if (!Authority(Error)) { return false; }
    FPFWorldSaveData Data; Data.Map = PersistenceMapName(GetWorld()); Data.Players = Roster.Players;
    int32 ClockCount = 0;
    for (TActorIterator<APFWorldClock> It(GetWorld()); It; ++It) { Data.Hour = It->Hour; ++ClockCount; }
    if (ClockCount != 1) { Error = TEXT("Save requires exactly one world clock"); return false; }
    for (auto It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        auto* PC = Cast<APFSurvivalPlayerController>(It->Get()); FPFPlayerSaveData Player;
        const auto* PS = PC ? PC->GetPlayerState<APFInventoryPlayerState>() : nullptr;
        if (PS && PendingRestores.Contains(PS->PersistentPlayerId)) { Error = TEXT("Connected player restoration is pending or failed; save refused"); return false; }
        if (!PC || !FPFPlayerSaveAdapter::Capture(PC, Player, Error)) { return false; }
        auto* Entry = Data.Players.FindByPredicate([&](const auto& P) { return P.PlayerId == Player.PlayerId; });
        if (!Entry) { Error = TEXT("Player has no server-issued reconnect registration"); return false; }
        if (!PackPlayer(Player, 30, Entry->Data, Error)) { return false; } Entry->CapturedUtc = PersistenceUtcNow();
    }
    for (TActorIterator<APFBuildPiece> It(GetWorld()); It; ++It)
    {
        FPFStructureSaveRecord S; S.Id = It->PersistentId; S.Owner = It->PersistentOwnerId;
        S.Support = It->Support ? It->Support->PersistentId : FGuid(); S.Definition = It->DefinitionId.ToString();
        S.Location = It->GetActorLocation(); S.Rotation = It->GetActorRotation().GetNormalized(); S.Health = It->Health; S.DoorOpen = It->bDoorOpen;
        FPFPlayerSaveData Bag; Bag.PlayerId = S.Id; Bag.Inventory = FPFPlayerSaveAdapter::CaptureInventory(*It->Storage);
        if (!PackPlayer(Bag, 60, S.Storage, Error)) { return false; } Data.Structures.Add(S);
    }
    const double Now = UPFInventoryComponent::ServerTime(GetWorld());
    for (TActorIterator<APFResourceNode> It(GetWorld()); It; ++It)
    {
        FPFResourceSaveRecord R; R.Name = It->GetName(); R.Definition = It->ResourceId.ToString(); R.Hits = It->HitsRemaining;
        R.Respawn = R.Hits == 0 ? FMath::Max(0., It->RespawnAt - Now) : 0; Data.Resources.Add(R);
    }
    for (TActorIterator<APFCreature> It(GetWorld()); It; ++It)
    {
        FPFCreatureSaveRecord C; C.Id = It->PersistentId; C.Definition = It->DefinitionId.ToString(); C.Location = It->GetActorLocation();
        C.Home = It->Home; C.Health = It->Health; C.CorpseSeconds = It->IsDead() ? FMath::Clamp(It->GetLifeSpan(), 0.f, 12.f) : 0; Data.Creatures.Add(C);
    }
    for (TActorIterator<APFCreatureSpawner> It(GetWorld()); It; ++It)
    {
        FPFSpawnerSaveRecord S; S.Name = It->GetName(); S.Resident = IsValid(It->Resident) ? It->Resident->PersistentId : FGuid();
        S.Respawn = It->PersistenceRespawnRemaining(); S.Enabled = It->bAutoSpawn; Data.Spawners.Add(S);
    }
    for (TActorIterator<APFItemPickup> It(GetWorld()); It; ++It)
    {
        const auto& Contents = It->GetContents(); if (Contents.ExpiresAt > 0 && Contents.ExpiresAt <= Now) { continue; }
        FPFPickupSaveRecord P; P.Location = It->GetActorLocation(); FPFPlayerSaveData Bag; Bag.PlayerId = Contents.StackId;
        Bag.Inventory.Add({Contents.StackId, Contents.ItemId, Contents.Quantity, Contents.ExpiresAt > 0 ? Contents.ExpiresAt - Now : 0});
        if (!PackPlayer(Bag, 100000, P.Data, Error)) { return false; } Data.Pickups.Add(P);
    }
    if (!Validate(Data, Error)) { return false; } Out = MoveTemp(Data); return true;
}

bool UPFWorldPersistence::Save(const FString& Slot, FString& Error)
{
    if(!FPFSaveFileStore::ValidSlot(Slot) || Slot.StartsWith(TEXT("Identity_"),ESearchCase::IgnoreCase) || Slot.StartsWith(TEXT("Metadata_"),ESearchCase::IgnoreCase))
    {Error=TEXT("Invalid or reserved world-save slot.");return false;}
    FPFWorldSaveData Data; TArray<uint8> Bytes;
    if (!Capture(Data, Error) || !Encode(Data, Bytes, Error) || !FPFSaveFileStore::Write(Slot, Bytes, Error)) { return false; }
    ActiveSlot = Slot; Roster.Players = Data.Players;
    UE_LOG(LogPFSurvival, Display, TEXT("[PrimalPersistence] Saved slot=%s players=%d structures=%d pickups=%d bytes=%d"),
        *Slot, Data.Players.Num(), Data.Structures.Num(), Data.Pickups.Num(), Bytes.Num()); return true;
}

bool UPFWorldPersistence::Load(const FString& Slot, FString& Error)
{
    if(!FPFSaveFileStore::ValidSlot(Slot) || Slot.StartsWith(TEXT("Identity_"),ESearchCase::IgnoreCase) || Slot.StartsWith(TEXT("Metadata_"),ESearchCase::IgnoreCase))
    {Error=TEXT("Invalid or reserved world-save slot.");return false;}
    FPFSavedFile File; FPFWorldSaveData Data;
    if (!Authority(Error) || !FPFSaveFileStore::Read(Slot, File, Error) || !Decode(File.Payload, Data, Error) || !Apply(Data, File.SavedUtc, Error)) { return false; }
    ActiveSlot = Slot; bLoadedWorld = true;
    UE_LOG(LogPFSurvival, Display, TEXT("[PrimalPersistence] Loaded slot=%s generation=%llu backupRecovery=%d"), *Slot, File.Generation, File.bRecoveredBackup);
    return true;
}

void UPFWorldPersistence::ConfigureStartup()
{
    FString Slot;bool bLoad=false;
    auto* Session=GetWorld()->GetGameInstance<UPFSessionGameInstance>();
    const bool bMenuRequest=Session && Session->ConsumeWorldRequest(Slot,bLoad);
    if(!bMenuRequest)
    {
        if (!FParse::Value(FCommandLine::Get(), TEXT("PFSaveSlot="), Slot)) { return; }
        bLoad=FParse::Param(FCommandLine::Get(),TEXT("PFLoadSave"));
    }
    if (!FPFSaveFileStore::ValidSlot(Slot) || Slot.StartsWith(TEXT("Identity_"),ESearchCase::IgnoreCase) || Slot.StartsWith(TEXT("Metadata_"),ESearchCase::IgnoreCase))
    {
        bStartupBlocked = true; UE_LOG(LogPFSurvival, Error, TEXT("[PrimalPersistence] Invalid startup save-slot identifier")); return;
    }
    ActiveSlot = Slot;
    if (!bLoad) { return; }
    FPFSavedFile File; FString Error;
    if (!FPFSaveFileStore::Read(Slot, File, Error) || !Decode(File.Payload, Pending, Error))
    {
        bStartupBlocked = true; UE_LOG(LogPFSurvival, Error, TEXT("[PrimalPersistence] Startup load refused: %s"), *Error); return;
    }
    if (Pending.Map != PersistenceMapName(GetWorld()))
    {
        bStartupBlocked = true; UE_LOG(LogPFSurvival, Error, TEXT("[PrimalPersistence] Startup save/map identity mismatch")); return;
    }
    PendingUtc = File.SavedUtc; Roster.Players = Pending.Players; bPending = true; bLoadedWorld = true;
}

void UPFWorldPersistence::ApplyStartup()
{
    if (!bPending) { return; } FString Error;
    if (!Apply(Pending, PendingUtc, Error)) { bStartupBlocked = true; UE_LOG(LogPFSurvival, Error, TEXT("[PrimalPersistence] Startup world restore refused: %s"), *Error); }
    bPending = false;
}

bool UPFWorldPersistence::Apply(const FPFWorldSaveData& Data, int64 SavedUtc, FString& Error)
{
    if (!Validate(Data, Error) || Data.Map != PersistenceMapName(GetWorld())) { Error = TEXT("Save validation or map identity mismatch"); return false; }
    TMap<FString, APFResourceNode*> Nodes; TMap<FString, APFCreatureSpawner*> SpawnPoints;
    TArray<APFWorldClock*> Clocks; TArray<APFBuildPiece*> OldPieces; TArray<APFCreature*> OldCreatures; TArray<APFItemPickup*> OldPickups;
    for (TActorIterator<APFResourceNode> It(GetWorld()); It; ++It) { Nodes.Add(It->GetName(), *It); }
    for (TActorIterator<APFCreatureSpawner> It(GetWorld()); It; ++It) { SpawnPoints.Add(It->GetName(), *It); }
    for (TActorIterator<APFWorldClock> It(GetWorld()); It; ++It) { Clocks.Add(*It); }
    if (Clocks.Num() != 1 || Nodes.Num() != Data.Resources.Num() || SpawnPoints.Num() != Data.Spawners.Num())
    { Error = TEXT("Map clock/resource/spawner layout changed; migration required"); return false; }
    for (const auto& R : Data.Resources)
    {
        auto* Node = Nodes.FindRef(R.Name);
        if (!Node || Node->ResourceId.ToString() != R.Definition || !Node->IsConfigurationValid())
        { Error = TEXT("Resource layout/definition mismatch"); return false; }
    }
    for (const auto& S : Data.Spawners) { if (!SpawnPoints.Contains(S.Name)) { Error = TEXT("Spawner layout mismatch"); return false; } }
    struct FPlayerRestore { APFSurvivalPlayerController* PC; FPFPlayerSaveData Data; double Age; };
    TArray<FPlayerRestore> Players;
    for (auto It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        auto* PC = Cast<APFSurvivalPlayerController>(It->Get()); auto* PS = PC ? PC->GetPlayerState<APFInventoryPlayerState>() : nullptr;
        if (!PS) { Error = TEXT("Unsupported connected player"); return false; }
        auto* Record = Data.Players.FindByPredicate([&](const auto& P) { return P.PlayerId == PS->PersistentPlayerId; });
        if (!Record) { Error = TEXT("Connected player absent from save; restart with load enabled before joining"); return false; }
        FPlayerRestore Restore; Restore.PC = PC; Restore.Age = FMath::Max<int64>(0, PersistenceUtcNow() - Record->CapturedUtc);
        if (!UnpackPlayer(Record->Data, 30, Restore.Data, Error) || !FPFPlayerSaveAdapter::CanRestore(PC, Restore.Data, Restore.Age, Error)) { return false; }
        Players.Add(MoveTemp(Restore));
    }
    for (TActorIterator<APFBuildPiece> It(GetWorld()); It; ++It) { OldPieces.Add(*It); }
    for (TActorIterator<APFCreature> It(GetWorld()); It; ++It) { OldCreatures.Add(*It); }
    for (TActorIterator<APFItemPickup> It(GetWorld()); It; ++It) { OldPickups.Add(*It); }
    const double Age = FMath::Max<int64>(0, PersistenceUtcNow() - SavedUtc);
    TArray<AActor*> Staged; TMap<FGuid, APFBuildPiece*> Pieces; TMap<FGuid, APFCreature*> Animals;
    auto Abort = [&] { for (AActor* Actor : Staged) { if (IsValid(Actor)) { Actor->Destroy(); } } return false; };
    // All new actors remain hidden/noncolliding until all allocations and inventories succeed.
    for (const auto& S : Data.Structures)
    {
        const FTransform T(S.Rotation, S.Location);
        auto* Piece = GetWorld()->SpawnActorDeferred<APFBuildPiece>(APFBuildPiece::StaticClass(), T, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
        if (!Piece) { Error = TEXT("Structure allocation failed"); return Abort(); }
        Staged.Add(Piece); Piece->SetActorHiddenInGame(true); Piece->SetActorEnableCollision(false);
        Piece->Initialize(*Buildings->Find(FName(*S.Definition, FNAME_Find)), nullptr, nullptr);
        Piece->PersistentId = S.Id; Piece->PersistentOwnerId = S.Owner; Piece->Health = S.Health; Piece->bDoorOpen = S.DoorOpen;
        Piece->FinishSpawning(T);
        FPFPlayerSaveData Bag;
        if (!UnpackPlayer(S.Storage, 60, Bag, Error) || !Piece->Storage->RestorePersistence(Bag.Inventory, Age, Error)) { return Abort(); }
        Piece->RefreshPersistenceShape(); Pieces.Add(S.Id, Piece);
    }
    for (const auto& S : Data.Structures) { Pieces[S.Id]->Support = Pieces.FindRef(S.Support); }
    for (const auto& C : Data.Creatures)
    {
        const FTransform T(C.Location);
        auto* Animal = GetWorld()->SpawnActorDeferred<APFCreature>(APFCreature::StaticClass(), T, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
        if (!Animal) { Error = TEXT("Creature allocation failed"); return Abort(); }
        Staged.Add(Animal); Animal->SetActorHiddenInGame(true); Animal->SetActorEnableCollision(false);
        Animal->CreatureId = FName(*C.Definition, FNAME_Find); Animal->Catalog = Creatures; Animal->PersistentId = C.Id; Animal->FinishSpawning(T);
        if (!Animal->RestorePersistence(C.Health, C.Home, C.CorpseSeconds)) { Error = TEXT("Creature restoration rejected"); return Abort(); }
        Animals.Add(C.Id, Animal);
    }
    for (const auto& P : Data.Pickups)
    {
        FPFPlayerSaveData Bag; UnpackPlayer(P.Data, 100000, Bag, Error); const auto& S = Bag.Inventory[0];
        if (S.RemainingFreshnessSeconds > 0 && S.RemainingFreshnessSeconds <= Age) { continue; }
        const FTransform T(P.Location);
        auto* Pickup = GetWorld()->SpawnActorDeferred<APFItemPickup>(APFItemPickup::StaticClass(), T, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
        if (!Pickup) { Error = TEXT("Pickup allocation failed"); return Abort(); }
        Staged.Add(Pickup); Pickup->SetActorHiddenInGame(true); Pickup->SetActorEnableCollision(false);
        Pickup->Initialize(S.ItemId, S.Quantity, S.RemainingFreshnessSeconds > 0 ? UPFInventoryComponent::ServerTime(GetWorld()) + S.RemainingFreshnessSeconds - Age : 0, S.StackId);
        Pickup->FinishSpawning(T); if (!IsValid(Pickup)) { Error = TEXT("Pickup restoration rejected"); return Abort(); }
    }
    // Preflight above ran on the same game-thread transaction, with no world tick in between.
    for (const auto& P : Players) { if (!FPFPlayerSaveAdapter::Restore(P.PC, P.Data, P.Age, Error)) { return Abort(); } }
    for (auto* A : OldPieces) { A->Destroy(); } for (auto* A : OldCreatures) { A->Destroy(); } for (auto* A : OldPickups) { A->Destroy(); }
    for (const auto& R : Data.Resources) { Nodes[R.Name]->RestorePersistence(R.Hits, R.Respawn); }
    for (const auto& S : Data.Spawners) { SpawnPoints[S.Name]->RestorePersistence(Animals.FindRef(S.Resident), S.Respawn, S.Enabled); }
    Clocks[0]->SetHour(Data.Hour); Roster.Players = Data.Players;
    RestoredLogins.Reset();
    for (auto* A : Staged) { A->SetActorHiddenInGame(false); A->SetActorEnableCollision(true); A->ForceNetUpdate(); }
    for (const auto& P : Players)
    {
        PendingRestores.Remove(P.Data.PlayerId);
        RestoredLogins.Add(P.Data.PlayerId);
        for (auto& Pair : Pieces) { Pair.Value->BindPersistentOwner(P.PC->PlayerState); }
    }
    return true;
}
