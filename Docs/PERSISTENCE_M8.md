# Milestone 8 — persistence foundation

## Current scope

Data-only checkpoint, October 8: `FPFPlayerSaveData`, `FPFSavedItemStack`, trusted `FPFPlayerSaveLimits` and `FPFPlayerSaveFormat` in `Source/PrimalFrontier/Persistence`. No gameplay call sites, Blueprint mutation, network request or file write. M7 manual route/overnight remains unpassed; the user explicitly authorized independent preparation. M8 is not complete.

V1 records one stable player GUID, location, normalized look rotation, health/stamina/hunger/thirst and distinct inventory batches (stack GUID, catalog item ID, quantity, remaining food lifetime). Dead health=0 is representable; respawn/load behavior is not implemented. Exposure is derived from the environment and not persisted here. Maximum vitals and inventory capacity come from trusted configuration, not the file. This format is not a complete world save.

## Format and validation

The little-endian Win64 envelope contains four uint32 fields: signature `0x50504650`, version `1`, payload length and payload CRC32. Explicit Unreal archive scalar serialization uses GUIDs, double-precision position/rotation, float vitals, int32 counts/quantities and double food lifetime. Item IDs are length-prefixed ASCII letters/digits/underscores, maximum 64 bytes. There is no object/reflection deserialization or arbitrary asset load.

- Maximum encoded record: 64 KiB. Decode checks exact envelope length and CRC, then bounded counts (maximum 64 slots) and item lengths before allocation. Trailing/truncated payloads are refused.
- Validate stable/nonduplicate GUIDs, finite bounded transforms/vitals, existing unambiguous catalog definitions, positive quantities within stack limits and trusted slot/weight limits. Location bound is +/-1e9 cm per axis; rotation must be within +/-360 degrees per axis. These checks do not certify navigable spawn locations; a future server restore adapter must do that.
- Nonperishable items require remaining lifetime=0. Perishable batches require a finite positive lifetime no greater than the catalog shelf life. Expired batches must be removed by a future capture adapter. Split/transfer/save/load must never refresh freshness or collapse differing batches.
- Both encoding and decoding preserve previous outputs on failure and return a specific error. Unsupported old/new versions and missing catalog IDs fail; no silent migration, item creation or false pass.
- CRC detects accidental damage only. Files are not client-authoritative; authenticated identity and any hostile-input/security policy belong to the future server adapter.

## Pending integration and policy

No disk-save durability, backup rotation, authoritative world transaction, inventory injection, server restart/reconnect, structure ownership/support IDs, storage, resources, creatures, offline spoilage or migration has been implemented. Remaining lifetime is represented, but no offline aging policy is active yet. The current PlayerState actor/display name cannot serve as an authenticated save identity. PF.SaveWorld, PF.LoadWorld and PF.TestPersistence remain unavailable until real adapters and integration tests exist. No assets or rendering settings change in this checkpoint.

## Verification

Editor build passed (12.53 s); after correcting a test-only TArray aliasing assertion, rebuild passed (5.58 s). No compiler warnings. Installed engine: 5.8.3; it was not changed.

Final `Saved/AutomationReports/M8PlayerFormatVerified/index.json` reports **3 passed, 0 failed, 0 test warnings**, process exit 0:

- `PF.Persistence.PlayerRoundTrip`: stable IDs, location/look, all four survival values, distinct food batches, remaining freshness, deterministic re-encoding and a dead/empty-inventory record.
- `PF.Persistence.PlayerValidation`: invalid identity/vitals/transforms, duplicate stack IDs/catalog definitions, unknown items, quantities, capacity and food lifetime; failed encoding preserves previous bytes.
- `PF.Persistence.CorruptPlayerData`: every truncated prefix of a valid empty-inventory record, signature/version/length/checksum errors, extra payload, oversized files, checksummed invalid counts/ID lengths/health/quantity and changed/removed catalog items; failed decoding preserves destination state.

Final log `Saved/Logs/PFM8PlayerFormatVerified.log` has no warnings/errors/ensures/fatal lines. Peak sampled process working set/private memory: **2.991/2.848 GiB**. NullRHI tests cannot assess rendered FPS or stuttering. No live save/restart/reconnect, manual gameplay or multiplayer persistence pass is claimed.

The earlier `Saved/Logs/PFM8PlayerFormat.log` shows both `PF.Inventory.Transactions` and `PF.Inventory.WorldTransfers` completed Success before the new validation fixture crashed at its duplicate-catalog setup (exit 3). Copying the source definition before `TArray::Add` repaired the fixture; the final persistence suite above reran all three tests from the corrected source. The initial process as a whole did not pass and exported no complete report.

Reproduce with UnrealEditor-Cmd, quoted project path, `-nullrhi -unattended -nosplash -nosound -NoLiveCoding -ExecCmds="Automation RunTests PF.Persistence" -TestExit="Automation Test Queue Empty" -ReportExportPath="C:/UnrealProjects/PrimalFrontier/Saved/AutomationReports/M8PlayerFormatVerified" -abslog="C:/UnrealProjects/PrimalFrontier/Saved/Logs/PFM8PlayerFormatVerified.log"`. Use a fresh report/log name for new runs to preserve this evidence. Engine-generated reports/logs remain outside Git.
