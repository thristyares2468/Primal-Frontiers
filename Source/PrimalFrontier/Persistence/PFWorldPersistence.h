#pragma once
#include "Subsystems/WorldSubsystem.h"
#include "Persistence/PFWorldSaveData.h"
#include "Persistence/PFPlayerSaveFormat.h"
#include "PFWorldPersistence.generated.h"
class APFSurvivalPlayerController;
class UPFItemCatalog;
class UPFBuildingCatalog;
class UPFCraftingCatalog;
class UPFCreatureCatalog;
struct FPFProgressionRecord;

/** Sanitized read-only preflight; never contains player identities, credentials or save bytes. */
struct FPFSaveInspection
{
    FString Map;
    int32 WorldVersion=0;
    uint64 Generation=0;
    int64 SavedUtc=0;
    int32 Players=0,Structures=0,Resources=0,Spawners=0,Creatures=0,Pickups=0;
    bool bRecoveredBackup=false;
    bool bRequiresCopperMigration=false;
};

/** Server-only orchestration; records contain values, never arbitrary actor classes. */
UCLASS()
class PRIMALFRONTIER_API UPFWorldPersistence : public UWorldSubsystem
{
    GENERATED_BODY()
public:
    bool Save(const FString& Slot, FString& Error);
    bool Load(const FString& Slot, FString& Error);
    /** File/catalog/map/layout and connected membership only, not a guarantee of actor restoration.
     *  No apply, save, original backup or active-slot change; failed inspection preserves Out. */
    bool InspectSave(const FString& Slot,FPFSaveInspection& Out,FString& Error);
    bool Capture(FPFWorldSaveData& Out, FString& Error);
    bool Validate(const FPFWorldSaveData& Data, FString& Error);
    bool Encode(const FPFWorldSaveData& Data, TArray<uint8>& Out, FString& Error);
    bool Decode(const TArray<uint8>& Bytes, FPFWorldSaveData& Out, FString& Error);
    bool CheckLogin(const FString& Options, FString& Error) const;
    void Login(APFSurvivalPlayerController* PC, const FString& Options);
    /** Optional outcome describes successful server restoration; never local credential storage. */
    bool RestorePlayer(APFSurvivalPlayerController* PC, bool* bOutRestored=nullptr);
    void Logout(APFSurvivalPlayerController* PC);
    void ConfigureStartup();
    void ApplyStartup();
    FString ActiveSlot;
private:
    UPROPERTY() TObjectPtr<UPFItemCatalog> Items;
    UPROPERTY() TObjectPtr<UPFBuildingCatalog> Buildings;
    UPROPERTY() TObjectPtr<UPFCraftingCatalog> Crafting;
    UPROPERTY() TObjectPtr<UPFCreatureCatalog> Creatures;
    FPFWorldSaveData Roster;
    FPFWorldSaveData Pending;
    int64 PendingUtc = 0;
    bool bPending = false;
    bool bStartupBlocked = false;
    bool bLoadedWorld = false;
    TSet<FGuid> PendingRestores;
    /** Actual successful restoration in the current login; saving alone never sets it. */
    TSet<FGuid> RestoredLogins;
    bool Authority(FString& Error);
    bool Apply(const FPFWorldSaveData& Data, int64 SavedUtc, FString& Error);
    bool PrepareCopperLayout(FPFWorldSaveData& Data,bool& Changed,FString& Error);
    bool CheckLayout(const FPFWorldSaveData& Data,FString& Error);
    bool PackPlayer(const FPFPlayerSaveData& Player, double Weight, FString& Out, FString& Error, bool bEnforceWeight=true,float MaxStamina=100);
    bool UnpackPlayer(const FString& Text, double Weight, FPFPlayerSaveData& Out, FString& Error, bool bEnforceWeight=true,float MaxStamina=100);
    bool UnpackProgression(int32 Version,const FPFWorldPlayerRecord& Player,FPFProgressionRecord& Out,FString& Error);
};
