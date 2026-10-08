# Milestone 8 — persistence foundation

## Stable structure ownership and current-state checkpoint (October 8)

Added persistent piece GUIDs and replicated server-assigned owner GUIDs. Door, damage, demolition, placement-support and storage reach checks use the stable owner key; `BindPersistentOwner` rebinds the current PlayerState/actor owner only for a matching identity. Native fixtures without that key retain the prior pointer check. The public key is not a reconnect credential. This prepares ownership for restoration but does not certify a network reconnect.

The pending world integration compiled after fixing a unity-build collision: file-store local `Magic` hid the player-codec constant. Renamed the constant, preserving the encoded format. First build failed in 18.32 s; corrected build passed in 16.96 s. Regression `M8CurrentStateRegression/index.json`: 12 passes, one failure, no test warnings. Failed `PF.Building.PlacementAndStorage` had four assertions because its simulated handover changed only Builder, leaving the stable owner key unchanged. Updated that fixture to transfer both fields and test replacement PlayerState, correct rebinding and wrong-key refusal. Final build passed in 16.26 s, no compiler warnings; `M8BuildingIdentityVerified/index.json`: one passed, zero failed/test warnings, engine exit 0. Matching `PFM8BuildingIdentityVerified.log` has no warning/error/ensure/fatal lines. All five existing persistence tests passed in the regression; exact other passes are listed in [CURRENT_STATE.md](CURRENT_STATE.md).

The initial regression monitor overflowed Int32; discard its peak figure. Corrected Int64 monitoring on the retry sampled 2.940 GiB working set / 2.801 GiB private memory. Both runs use NullRHI and provide no rendered performance/manual gameplay evidence. M8 world records/restoration helpers and local development credential code remain in-progress source, without GameMode/command activation or a world/restart/reconnect test pass. M7's manual gate and M8 remain incomplete.

## Server adapter and safe-file checkpoint (October 8)

The user subsequently authorized continuing through the remaining work until their action is required. M7 stays unverified. Added a server-issued public PlayerState GUID, server-only capture/restore, atomic inventory replacement, collision/ground/identity checks and offline food aging. Reloading replaces inventory rather than appending; stale food is omitted. Existing crafting jobs cancel on restoration. No client restoration RPC exists. These APIs are exercised in native worlds; world commands/reconnect are still pending.

`PFSaveFileStore` writes bounded checksummed A/B generations under `Saved/Persistence`. It verifies a pending file before replacing only the inactive generation; the active save survives a failed/interrupted replacement. The prior generation is a backup. Read reports backup recovery explicitly; writes refuse to overwrite corrupt evidence. This is not a claim of transactional filesystem replacement or power-loss durability on every filesystem. Slot identifiers cannot contain paths. Generic file integrity does not replace semantic codec validation.

First Editor build failed because DOREPLIFETIME requires the exact `OutLifetimeProps` parameter name; corrected it. Rebuild passed in **18.19 s**, final fixture-only rebuild **5.70 s**, no compiler warnings. `M8PlayerRuntime/index.json` contains eight Success states: PF.Inventory.Transactions; PF.Inventory.WorldTransfers; PF.Persistence.CorruptPlayerData; PF.Persistence.FileGenerations; PF.Persistence.PlayerRoundTrip; PF.Persistence.PlayerValidation; PF.Persistence.ServerPlayerAdapter; PF.Survival.Lifecycle. Seven were clean, the new adapter fixture had expected missing-socket warnings from native meshless presentation. Matched that known warning explicitly as the existing Lifecycle fixture does. Final `M8PlayerAdapterVerified/index.json`: **ServerPlayerAdapter 1 passed, 0 failed, 0 test warnings**, engine exit 0. Initial sampled working/private peaks **2.949/2.806 GiB**. Logs: `PFM8PlayerRuntime.log`, `PFM8PlayerAdapterVerified.log`. NullRHI cannot certify rendered performance/manual persistence. File tests create and clean only their unique Automation slot through Unreal APIs.

## Current scope

Initial data-only checkpoint, October 8: `FPFPlayerSaveData`, `FPFSavedItemStack`, trusted `FPFPlayerSaveLimits` and `FPFPlayerSaveFormat` in `Source/PrimalFrontier/Persistence`. The later adapter/file checkpoint above extends it. M7 manual route/overnight remains unpassed; M8 is not complete.

V1 records one stable player GUID, location, normalized look rotation, health/stamina/hunger/thirst and distinct inventory batches (stack GUID, catalog item ID, quantity, remaining food lifetime). Dead health=0 is representable; respawn/load behavior is not implemented. Exposure is derived from the environment and not persisted here. Maximum vitals and inventory capacity come from trusted configuration, not the file. This format is not a complete world save.

## Format and validation

The little-endian Win64 envelope contains four uint32 fields: signature `0x50504650`, version `1`, payload length and payload CRC32. Explicit Unreal archive scalar serialization uses GUIDs, double-precision position/rotation, float vitals, int32 counts/quantities and double food lifetime. Item IDs are length-prefixed ASCII letters/digits/underscores, maximum 64 bytes. There is no object/reflection deserialization or arbitrary asset load.

- Maximum encoded record: 64 KiB. Decode checks exact envelope length and CRC, then bounded counts (maximum 64 slots) and item lengths before allocation. Trailing/truncated payloads are refused.
- Validate stable/nonduplicate GUIDs, finite bounded transforms/vitals, existing unambiguous catalog definitions, positive quantities within stack limits and trusted slot/weight limits. Location bound is +/-1e9 cm per axis; rotation must be within +/-360 degrees per axis. These checks do not certify navigable spawn locations; a future server restore adapter must do that.
- Nonperishable items require remaining lifetime=0. Perishable batches require a finite positive lifetime no greater than the catalog shelf life. Expired batches must be removed by a future capture adapter. Split/transfer/save/load must never refresh freshness or collapse differing batches.
- Both encoding and decoding preserve previous outputs on failure and return a specific error. Unsupported old/new versions and missing catalog IDs fail; no silent migration, item creation or false pass.
- CRC detects accidental damage only. Files are not client-authoritative; authenticated identity and any hostile-input/security policy belong to the future server adapter.

## Pending integration and policy

The player adapter subtracts nonnegative offline age from remaining food lifetime. World transactions, server restart/reconnect credentials, persistent structure ownership/support, storage/resources/creatures and migration are pending. The public PlayerState GUID cannot authenticate a reconnect by itself. PF.SaveWorld, PF.LoadWorld and PF.TestPersistence remain unavailable until whole-world adapters and integration tests exist. No assets or rendering settings changed.

## Verification

Editor build passed (12.53 s); after correcting a test-only TArray aliasing assertion, rebuild passed (5.58 s). No compiler warnings. Installed engine: 5.8.3; it was not changed.

Final `Saved/AutomationReports/M8PlayerFormatVerified/index.json` reports **3 passed, 0 failed, 0 test warnings**, process exit 0:

- `PF.Persistence.PlayerRoundTrip`: stable IDs, location/look, all four survival values, distinct food batches, remaining freshness, deterministic re-encoding and a dead/empty-inventory record.
- `PF.Persistence.PlayerValidation`: invalid identity/vitals/transforms, duplicate stack IDs/catalog definitions, unknown items, quantities, capacity and food lifetime; failed encoding preserves previous bytes.
- `PF.Persistence.CorruptPlayerData`: every truncated prefix of a valid empty-inventory record, signature/version/length/checksum errors, extra payload, oversized files, checksummed invalid counts/ID lengths/health/quantity and changed/removed catalog items; failed decoding preserves destination state.

Final log `Saved/Logs/PFM8PlayerFormatVerified.log` has no warnings/errors/ensures/fatal lines. Peak sampled process working set/private memory: **2.991/2.848 GiB**. NullRHI tests cannot assess rendered FPS or stuttering. No live save/restart/reconnect, manual gameplay or multiplayer persistence pass is claimed.

The earlier `Saved/Logs/PFM8PlayerFormat.log` shows both `PF.Inventory.Transactions` and `PF.Inventory.WorldTransfers` completed Success before the new validation fixture crashed at its duplicate-catalog setup (exit 3). Copying the source definition before `TArray::Add` repaired the fixture; the final persistence suite above reran all three tests from the corrected source. The initial process as a whole did not pass and exported no complete report.

Reproduce with UnrealEditor-Cmd, quoted project path, `-nullrhi -unattended -nosplash -nosound -NoLiveCoding -ExecCmds="Automation RunTests PF.Persistence" -TestExit="Automation Test Queue Empty" -ReportExportPath="C:/UnrealProjects/PrimalFrontier/Saved/AutomationReports/M8PlayerFormatVerified" -abslog="C:/UnrealProjects/PrimalFrontier/Saved/Logs/PFM8PlayerFormatVerified.log"`. Use a fresh report/log name for new runs to preserve this evidence. Engine-generated reports/logs remain outside Git.
