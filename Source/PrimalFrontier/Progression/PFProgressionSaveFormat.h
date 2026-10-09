#pragma once
#include "Progression/PFProgressionRecord.h"
#include "Persistence/PFPlayerSaveFormat.h"

/** Separate bounded progression bytes; no existing player/world writer or file I/O integration.
 *  CRC detects accidental corruption, not authentication. Trusted server records only.
 *  All failed methods preserve output bytes/records. Never interns unknown on-disk names. */
class PRIMALFRONTIER_API FPFProgressionSaveFormat
{
public:
    static constexpr uint32 CurrentVersion=1;
    static constexpr int32 EnvelopeBytes=16;
    static constexpr int32 MaximumBytes=16*1024;
    static bool Encode(const FPFProgressionRecord& Record,const UPFProgressionCatalog& Catalog,
        const UPFCraftingCatalog& Crafting,const UPFItemCatalog& Items,TArray<uint8>& Out,FString& Error);
    static bool Decode(const TArray<uint8>& Bytes,const UPFProgressionCatalog& Catalog,
        const UPFCraftingCatalog& Crafting,const UPFItemCatalog& Items,FPFProgressionRecord& Out,FString& Error);
    /** Validate the original V1 player first, then provide zero-XP/empty-ledger defaults.
     *  Inventory is not proof of completed crafts; no retrospective rewards or file rewrite. */
    static bool DecodeLegacyPlayer(const TArray<uint8>& Bytes,const UPFItemCatalog& Items,
        const FPFPlayerSaveLimits& Limits,const UPFProgressionCatalog& Catalog,const UPFCraftingCatalog& Crafting,
        FPFPlayerSaveData& OutPlayer,FPFProgressionRecord& OutProgression,FString& Error);
};
