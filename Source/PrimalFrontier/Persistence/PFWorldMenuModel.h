#pragma once
#include "CoreMinimal.h"

struct FPFWorldMenuEntry
{
    FString Slot, DisplayName, Map, Issue;
    int64 SavedUtc=0;
    bool bLoadable=false;
};
/** Local disk catalog. Never returns private player records or arbitrary map paths. */
namespace PFWorldMenuModel
{
    PRIMALFRONTIER_API FString MapPackage(const FString& Name);
    PRIMALFRONTIER_API bool Inspect(const FString& Slot,FPFWorldMenuEntry& Out);
    PRIMALFRONTIER_API TArray<FPFWorldMenuEntry> List();
    PRIMALFRONTIER_API bool CanCreate(const FString& Slot,FString& Error);
    PRIMALFRONTIER_API bool Rename(const FString& Slot,const FString& DisplayName,FString& Error);
    PRIMALFRONTIER_API FString NameFor(const FString& Slot);
    PRIMALFRONTIER_API FString NamesSlot();
}
