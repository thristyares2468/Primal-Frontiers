#pragma once
#include "CoreMinimal.h"
#include "PFWorldSaveData.generated.h"

USTRUCT()
struct FPFWorldPlayerRecord
{
    GENERATED_BODY()
    UPROPERTY() FString Data;
    UPROPERTY() FGuid PlayerId;
    UPROPERTY() FGuid ReconnectCredential;
    UPROPERTY() int64 CapturedUtc = 0;
};
USTRUCT()
struct FPFStructureSaveRecord
{
    GENERATED_BODY()
    UPROPERTY() FGuid Id;
    UPROPERTY() FGuid Owner;
    UPROPERTY() FGuid Support;
    UPROPERTY() FString Definition;
    UPROPERTY() FVector Location = FVector::ZeroVector;
    UPROPERTY() FRotator Rotation = FRotator::ZeroRotator;
    UPROPERTY() float Health = 0;
    UPROPERTY() bool DoorOpen = false;
    UPROPERTY() FString Storage;
};
USTRUCT()
struct FPFResourceSaveRecord
{
    GENERATED_BODY()
    UPROPERTY() FString Name;
    UPROPERTY() FString Definition;
    UPROPERTY() int32 Hits = 0;
    UPROPERTY() double Respawn = 0;
};
USTRUCT()
struct FPFCreatureSaveRecord
{
    GENERATED_BODY()
    UPROPERTY() FGuid Id;
    UPROPERTY() FString Definition;
    UPROPERTY() FVector Location = FVector::ZeroVector;
    UPROPERTY() FVector Home = FVector::ZeroVector;
    UPROPERTY() float Health = 0;
    UPROPERTY() float CorpseSeconds = 0;
};
USTRUCT()
struct FPFSpawnerSaveRecord
{
    GENERATED_BODY()
    UPROPERTY() FString Name;
    UPROPERTY() FGuid Resident;
    UPROPERTY() double Respawn = 0;
    UPROPERTY() bool Enabled = true;
};
USTRUCT()
struct FPFPickupSaveRecord
{
    GENERATED_BODY()
    UPROPERTY() FVector Location = FVector::ZeroVector;
    UPROPERTY() FString Data;
};
USTRUCT()
struct FPFWorldSaveData
{
    GENERATED_BODY()
    UPROPERTY() int32 Version = 1;
    UPROPERTY() FString Map;
    UPROPERTY() float Hour = 9;
    UPROPERTY() TArray<FPFWorldPlayerRecord> Players;
    UPROPERTY() TArray<FPFStructureSaveRecord> Structures;
    UPROPERTY() TArray<FPFResourceSaveRecord> Resources;
    UPROPERTY() TArray<FPFCreatureSaveRecord> Creatures;
    UPROPERTY() TArray<FPFSpawnerSaveRecord> Spawners;
    UPROPERTY() TArray<FPFPickupSaveRecord> Pickups;
};
