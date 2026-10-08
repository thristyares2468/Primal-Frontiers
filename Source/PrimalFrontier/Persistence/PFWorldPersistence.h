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

/** Server-only orchestration; records contain values, never arbitrary actor classes. */
UCLASS()
class PRIMALFRONTIER_API UPFWorldPersistence : public UWorldSubsystem
{
    GENERATED_BODY()
public:
    bool Save(const FString& Slot, FString& Error);
    bool Load(const FString& Slot, FString& Error);
    bool Capture(FPFWorldSaveData& Out, FString& Error);
    bool Validate(const FPFWorldSaveData& Data, FString& Error);
    bool Encode(const FPFWorldSaveData& Data, TArray<uint8>& Out, FString& Error);
    bool Decode(const TArray<uint8>& Bytes, FPFWorldSaveData& Out, FString& Error);
    bool CheckLogin(const FString& Options, FString& Error) const;
    void Login(APFSurvivalPlayerController* PC, const FString& Options);
    bool RestorePlayer(APFSurvivalPlayerController* PC);
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
    bool Authority(FString& Error);
    bool Apply(const FPFWorldSaveData& Data, int64 SavedUtc, FString& Error);
    bool PackPlayer(const FPFPlayerSaveData& Player, double Weight, FString& Out, FString& Error);
    bool UnpackPlayer(const FString& Text, double Weight, FPFPlayerSaveData& Out, FString& Error);
};
