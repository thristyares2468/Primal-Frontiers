# Milestone 8 — persistence and multiplayer

## Current development contract

UPFWorldPersistence is a server-only WorldSubsystem. V1 contains value records/catalog IDs, never arbitrary actor classes. Approved maps: L_PrimalFrontier_OpenWorld, L_M7SurvivalArena and L_Automation. Bounded to 32 player identities, 128 structures, 128 resources, eight creatures, 32 spawners and 128 pickups; JSON payload at most 4 MiB. Player/batch codec is bounded and checksummed. Unknown versions/catalogs, invalid vitals/transforms, duplicate IDs, cyclic/missing/cross-owner/wrong-kind building supports and mismatched authored map layouts are refused. Roofs use wall/door support. Map-content changes require a migration.

Records preserve public player IDs separately from private reconnect credentials; player vitals/location/item batches; structure owner/support/health/door/storage; resource depletion and remaining respawn; creature identity/health/home/corpse duration; spawner resident/enabled/timer; pickups and world time. Food uses remaining lifetime minus offline UTC age, removing expired batches. Other world timers pause offline. Crafting is cancelled on restore. No learned/progression state exists yet. Creature AI transient targets/path state are not serialized.

Restoration stages hidden actors and validates records/player ground/capsule before replacing runtime objects and inventories. Repeated load replaces rather than appends. Pending/failed player restores block empty default-player capture. This is bounded development orchestration, **not globally rollback-atomic restoration across multiplayer callbacks** or universal power-loss protection.

PFSaveFileStore publishes verified checksummed A/B generations at Saved/Persistence/<slot>.a.pfs and .b.pfs. The active generation survives an inactive-file write failure. Read explicitly reports backup recovery; writes refuse to overwrite existing corrupt evidence. No silent repair/delete. Slot: 1–64 ASCII letters/digits/underscore, no paths or hyphens. Saved files are ignored by Git. Keep a separate backup before deliberate corruption experiments.

PFSurvivalGameMode loads on startup before accepting joins with -PFSaveSlot=<slot> -PFLoadSave; deferred PostLogin restores after the engine attaches the connection. PlayerController::Destroyed captures departure before pawn removal. A configured active slot is saved on client logout. **Manual PF.SaveWorld is required before standalone/server exit; no timed autosave is added.** Connected players absent from a selected loaded save are refused: restart/load before joining.

PFLocalPlayer stores a private GUID capability under Identity_<profile>_<endpoint hash>, using A/B file APIs. -PFIdentityProfile defaults Local (maximum 16 valid slot characters). Reconnect must use the same profile, host spelling and port; aliases create distinct profiles. Server-assigned public ownership IDs are not credentials. Unknown credentials are replaced only in a fresh unsaved world; malformed/duplicate-connected credentials and unknown credentials after loading are refused. This is trusted local/LAN development identity, **not production account authentication**. Profiles/world files and engine login URLs can contain private credentials: do not commit, paste or upload them.

PF.SaveWorld [slot] / PF.LoadWorld [slot] default Survival and run only in an authoritative game/PIE world. They never save an editor map or forward a client request. PF.TestPersistence performs read-only Capture/Encode/Decode/Re-encode validation; it is not disk/restart certification. PF.Help reports their current backend. Runtime tooling remains excluded from Shipping; game persistence has no reverse dependency on the plugin. Built-in Json/JsonUtilities are the only new module dependencies.

## 2026-10-08 M8 world persistence and reconnect verification

**M8 implementation and automated verification advanced; milestone acceptance is still pending.** M7's sustained manual open-world route/overnight result and M8's rendered gather/craft/build/storage/save/close/restart/reconnect playtest remain unverified. The user authorized independent M8 work while deferring M7. No M9 work starts from this checkpoint.

Implemented bounded V1 whole-world records, server-only save/load, startup restoration and development reconnect identities. Saves include player vitals/location/inventory, structure health/support/ownership/storage/door state, authored resource depletion/respawn, creature identities/health/home/corpse timers, spawner residents and pickups, plus world time. Food ages offline; expired batches are omitted; repeated restoration replaces inventories. A failed player restoration cannot overwrite the saved player with an empty default spawn. Login/restore uses a deferred PostLogin step; disconnect captures the pawn before destruction. Commands PF.SaveWorld [slot], PF.LoadWorld [slot] and read-only PF.TestPersistence now call real APIs and reject clients. No map/assets were saved or changed.

Builds: PrimalFrontierEditor **passed (6.33 s)**; PrimalFrontier Development **passed (69.65 s)**, no compiler warnings. Installed engine reports **5.8.3**, not the originally requested 5.8.2. PrimalFrontierServer was **blocked before compilation (0.73 s)**: "Server targets are not currently supported from this engine distribution." The existing Server target is retained; uncooked Editor -server is the tested dedicated-server route. A post-commit working-set build is still pending.

Final regression: **26 passed, 0 failed, 0 test warnings**, engine exit 0. Report: Saved/AutomationReports/M8FinalRegression/index.json; log: Saved/Logs/PFM8FinalRegression.log. Exact Success tests:

- PF.Building.PlacementAndStorage
- PF.Crafting.Gathering
- PF.Crafting.Transactions
- PF.Creatures.Lifecycle
- PF.Input.Gamepad
- PF.Interaction.TargetAndPickup
- PF.Inventory.Transactions
- PF.Inventory.WorldTransfers
- PF.Persistence.CorruptPlayerData
- PF.Persistence.FileGenerations
- PF.Persistence.PlayerRoundTrip
- PF.Persistence.PlayerValidation
- PF.Persistence.ServerPlayerAdapter
- PF.Persistence.WorldRecords
- PF.Persistence.WorldRuntime
- PF.PrimalAgentTools.CommandArguments
- PF.PrimalAgentTools.MissingSystemsAreBlocked
- PF.PrimalAgentTools.TeleportAndRuntimeReset
- PF.Settings.Preferences
- PF.Survival.Component
- PF.Survival.Environment
- PF.Survival.Lifecycle
- PF.Survival.Needs
- PF.World.Clock
- PF.World.OpenWorldAsset
- PF.World.Resources

Focused prerequisites also passed: M8WorldFixture (WorldRecords and WorldRuntime: 2); M8WorldHookRegression (12); M8DisconnectVerified (WorldRuntime: 1). Actual one-client Create/Restart produced four passing PF.Persistence.Live reports: M8CreateOneVerifiedServer, M8CreateOneVerifiedClient, M8RestartOneVerifiedServer, M8RestartOneVerifiedClient. Actual two-client Create/Restart produced six passing PF.Persistence.Live reports: M8CreateTwoVerifiedServer, M8CreateTwoVerifiedClient1, M8CreateTwoVerifiedClient2, M8RestartTwoVerifiedServer, M8RestartTwoVerifiedClient1, M8RestartTwoVerifiedClient2. All ten have zero failures/test warnings and engine exits 0, under Saved/AutomationReports/<name>/index.json; matching logs are Saved/Logs/PF<name>.log.

Live tests use L_PrimalFrontier_OpenWorld, separate uncooked dedicated server and one/two clients with NullRHI. They perform real gathering, timed tool craft, foundation/storage placement, file save/load, client authority refusals and replicated owner/vitals/items/time. A separate server restart restores the crafted tool, health, storage, ownership and a fibre marker changed after the manual save and captured on disconnect. Fixture teleports and time overrides are **not manual traversal/overnight evidence**.

Preserved failures: first world fixture build lacked the pickup deadline argument (fixed; two tests then passed). Original M8RestartOneServer failed five assertions; client failed with timeout/network aftermath (2 errors/39 warnings). Credential RPC ran before the engine attached the client connection; moving it to deferred PostLogin and capturing departure before pawn destruction fixed the fresh verified runs. Initial M8CreateTwoServer/Client1/Client2 each timed out at a test prerequisite: the test filtered stone nodes to a region containing only one, but required two. Removed that test-only filter; no map change; rebuilt and all six fresh create/restart reports passed. Older failed reports remain evidence. Engine exit 0 alone is not a test verdict.

Final regression sampled working/private memory **2.896/2.769 GiB**. Two-client create sampled server/client1/client2 working sets **1.717/1.808/1.816 GiB**, restart **1.722/1.796/1.795 GiB**. NullRHI has no rendered FPS or stutter measurement; it does not resolve the earlier New Editor Window PIE 14–17 FPS complaint. This host reports about 31.93 GiB installed RAM; the 16 GB target remains a budget, not a measured minimum-spec pass. Known uncooked HLOD, engine Toolsets Python/editor-widget startup and teardown warnings remain outside test events; final regression log has no warning/error/ensure/fatal lines.

Changed areas: Persistence/PFWorldSaveData, PFWorldSaveFormat, PFWorldPersistence, PFLocalPlayer; player-save adapter; GameMode/controller lifecycle; resource/creature/spawner/pickup snapshot helpers; built-in Json/JsonUtilities module dependencies; DefaultEngine.ini LocalPlayer class; plugin adapters/live tests and native world tests; documentation. No unrelated Unreal assets, external art, rendering settings, new gameplay system or MCP server changed.

Remaining: clean-working-set build and scoped commit/push; then M7 manual route/overnight and M8 manual persistence playtest. Production account authentication, globally rollback-atomic restoration, save migration, cooked streaming/HLOD and packaged-server certification are outside this verified development scenario. See PERSISTENCE_M8.md and PLAYTEST.md.

## Historical foundation checkpoints

The entries below preserve earlier results/failures. Statements that world integration or commands were pending describe their original checkpoint and are superseded by the current contract above.

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
