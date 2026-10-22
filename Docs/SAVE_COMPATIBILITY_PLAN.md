# Save compatibility and migration plan — M18

October22 follow-up [ORIGINAL_COPY_COMMAND_M18.md](ORIGINAL_COPY_COMMAND_M18.md)
exposes a separately tested non-Shipping server-console wrapper for exact copy
into an unused slot. Older API-only "not exposed" statements below are historical;
live loading, semantic migration, active-slot changes and full M18 remain separate.

October22 [ORIGINAL_RECOVERY_COPY_M18.md](ORIGINAL_RECOVERY_COPY_M18.md) supplies
a tested trusted backend primitive to copy a verified original into an unused
slot without resetting bytes/generation/time. It preserves source/destination
evidence and is not an automatic live rollback, semantic migration or exposed
operator command. Six native regressions/builds pass; fullM18 remains open.

October15 [FROZEN_WRITER_M18.md](FROZEN_WRITER_M18.md) adds one genuine historical-source writer fixture: c3d221f rebuilt independently on installed5.8.3, production source hashes unchanged, synthetic data only. Current tests consume immutable V1 payload/envelope directly and pass startup/login/repeatedLoad/currentV2save, neutral progression/chest defaults, expired-food and refusal checks. This supersedes the blanket "only current-generated synthetic legacy records" gap for this ONE fixture. No archived historical executable, multi-engine/version/catalog matrix, generic content migration/rollback or fullM18 claim. No personal save processed.

October15 [MIGRATION_REFUSAL_M18.md](MIGRATION_REFUSAL_M18.md): six actual manual/startup backup-refusal scenarios preserve source/evidence/live state/active craft, block default-world saves after startup failure. New native test and three related regressions PASS, both builds clean; no production code repair or OS-crash/disk-full/general migration claim. Genuine older-writer fixtures remain open.

October15: `PF.CheckSave [slot]` now provides read-only file/catalog/map/layout/connected-membership inspection and explicit reviewed Copper-migration/recovery indication. It does not apply or publish a migration, archive an original, guarantee complete restoration or replace the separate load/restart gates. [SAVE_INSPECTION_M18.md](SAVE_INSPECTION_M18.md) records bounded verification. General migration/archived-writer/rollback work remains open. User defers hosting/account/database/voice setup; independent backend work can continue.

October14 Copper checkpoint supersedes the older no-integration statements: one exact old15->current16 OpenWorld node edge is implemented in PFCopperLayout.cpp and called by server Load/post-BeginPlay startup. Original source envelope is verified/preserved outside A/B before apply; Load retains sourceUTC/food age and does not rewrite source bytes. Subsequent normal Save captures aged runtime. Real native Load and actual one-/two-client new-server restart tests pass using synthetic old-layout test sources, not private worlds or an archived executable. Unknown/changed authored manifests refuse. COPPER_TIERS_M12.md records exact evidence and limits; no general content migration, rollback, archive-writer matrix or full M18 acceptance is implied.

October14 additive contract update: RESOURCE_LAYOUT_TRANSFORM_M18.md records a whole-record, exact trusted-manifest offline addition primitive with neutral new-node defaults and atomic refusal/conservation tests. It is not wired to startup/load, and no actual ore edge is registered. Resource-count runtime checks remain strict; authored manifest, original backup and time-anchor-safe publication/application/restart still required. Original source bytes must retain food/UTC age semantics; do not reset age simply by writing a transformed payload with a new timestamp.

October14 bounded implementation update: ORIGINAL_SAVE_BACKUPS_M18.md records a native-tested no-overwrite original-envelope primitive, outside ordinary A/B rotation. It is NOT integrated into normal load/startup/migration; no private save automatically receives this protection yet. General transforms/layout mappings, permanent-original migration transaction, archived writer fixtures, rollback and fullM18 remain unimplemented. Older blanket statements below describe the earlier audit. Inner progression now also supports V3 stat allocations (PLAYER_STATS_M12.md), with neutral-rank V1/V2 bytes retained; outerworldV2 unchanged.

Current contracts reconciled October13,2026 against published c2890ee. **General content/layout migration and permanent original-backup publication are not implemented.** Specific backwards-compatible readers/defaults below already exist; this distinction supersedes the original October8 planning-only overview. Full M18 remains To Do. This document audit changes no source, schema, assets or private saves and certifies no production authentication/crash durability. Accepted M7 and solo M8 observations remain accepted; rendered multiplayer, M13 G9 and deferred1440 remain separate open gates.

## Current contracts checked in source

| Layer | Version and bounds | Present behavior |
| --- | --- | --- |
| File store | V1, 32-byte header, at most 4 MiB payload | Magic/version, generation, UTC, length and CRC; verified A/B inactive-generation publication. |
| World metadata | Normal runtime writer V2; reader supports V1/V2 JSON (plain struct still defaults V1) | V1 player records omit/default zero progression; V2 requires owner-bound progression for every player. Approved maps, bounded arrays, strict fields and whole-candidate validation. |
| Player/container codec | `CurrentVersion=1`, 16-byte envelope, at most 64 KiB | Explicit scalar/GUID/ASCII item fields, CRC, trusted catalog/capacity checks; no arbitrary object loading. |
| Progression codec | Inner V1/V2, 16-byte envelope, maximum16 KiB, owner GUID wrapper | No gather windows writes byte-compatible V1; active windows use V2. Preserve XP, learned/credited history and bounded active-time windows; future/corrupt/foreign owner refuses. |
| Local reconnect profile | File-store V1, 16-byte GUID payload | Endpoint/profile-specific capability, separate from public player ID; no independent profile schema field yet. |
| Content/world compatibility | Catalog IDs and authored resource/spawner names | Unknown definitions/layouts and invalid owner/support/batch identities refuse load; no revision/mapping system yet. |

Sources: `PFSaveFileStore.{h,cpp}`, `PFWorldSaveData.h`, `PFWorldSaveFormat.cpp`, `PFPlayerSaveFormat.{h,cpp}`, `PFPlayerSaveAdapter.cpp`, `PFWorldPersistence.cpp`, `PFLocalPlayer.cpp`. Current world limits: 32 players, 128 structures/resources/pickups each, eight creatures and 32 spawners. Player bags/storage have eight slots. Bag30kg is a trusted soft carry threshold (overweight can save/load); storage60kg remains a hard capacity. Stack quantity is bounded999000, gear stacks1, freshness batches still separate. Capacity/enforcement comes from trusted runtime configuration, never a save. Future expansion must change and test codec/runtime limits together.

These version layers are independent. Current worldV2 can contain either valid inner progression version. Adding a world field does not itself justify replacing the file envelope or reconnect profile. Engine patch numbers are not save versions. Installed engine is 5.8.3; originally requested 5.8.2.

Current fallback checks **file integrity**, not semantic migration: Read chooses the highest valid checksummed generation and explicitly reports an invalid sibling as backup recovery. Unsupported selected world semantics refuse Load; no silent older-version fallback occurs. Write refuses existing corrupt generations. Failed startup restoration blocks login/capture/save until deliberate recovery and restart.

## Implemented compatibility and remaining gaps

- V1 world/player load gives zero XP/empty knowledge/credit/window records; inventory is not proof of an earned craft. No retrospective reward or free unlock. Current capture/disconnect roster upgrades all owner records together to worldV2 before ordinary save. This is a known compatible edge, not a general migration registry or permanent-original archive.
- Old chest records missing BOTH filter fields default unrestricted/empty allowed list. Partial/malformed fields refuse. Existing filters retain exact permitted item IDs, ownership and stored quantities; no rename guessing.
- Four native original creature IDs now validate, including Scrabbler/DuskPouncer; weighted authored policies retain the same two point names/count/positions. Old saved residents retain type/GUID/enabled/cooldown and do not respawn duplicate loot. Same schema number does not promise an older engine/catalog can read newly introduced creature/item IDs.
- Live apply still requires exactly one clock and exact resource/spawner record counts/names/definitions. A new M14 node/point/renamed object is a content-layout migration, even when JSON version is unchanged. Do not bypass this refusal, delete records or overwrite a failed restored world with defaults. No content revision/ID mapping registry exists yet.
- Existing A/B publication protects bounded current generations/integrity. It does NOT retain a permanent V1 original: subsequent ordinary V2 saves can rotate both generations. The original-backup policy proposed below belongs to a future explicit migration transaction; do not claim it already protects every compatible V1 upgrade.
- Existing native compatibility tests generate synthetic V1/V2 records using current codecs and mutate fields. They verify known semantics, but are NOT immutable fixtures produced by an archived previous executable. Frozen previous-writer fixtures, content mappings, permanent backups and adversarial publication/rollback still need their own future gates.

Current scope is a source/document reconciliation plus four existing native tests; no load/migration is run on a private world. Latest evidence is appended below after execution. The original October8 future transaction/proposals remain guidance, not completed implementation.

## Proposed compatibility policy

| Change or condition | Future action |
| --- | --- |
| Current schema/catalog/map | Existing server validation/restoration. |
| Explicitly supported older schema | One bounded deterministic value-record transform; validate the complete candidate before publication. |
| Unknown older/future schema | Refuse with layer/version/recovery guidance; preserve source and runtime. No downgrade. |
| Truncation/corruption/checksum failure | Preserve evidence; report supported backup recovery explicitly. No repair/default-world overwrite. |
| Newly introduced optional field | Version-specific neutral default only, such as zero XP/empty loadout. Required V1 fields cannot become optional accidentally. |
| Renamed catalog ID | Reviewed, checked-in old-to-new mapping; preserve quantity, stable IDs and freshness. No display-name guessing. |
| Removed/unknown item without approved mapping | Refuse; do not delete, substitute free resources or create an undocumented overflow container. |
| Reduced stack/capacity/weight limits | Refuse without a reviewed lossless policy. Any split conserves quantity/freshness and uses deterministic distinct IDs. |
| Resource/spawner rename or changed layout | Explicit map revision/mapping with validated targets; no proximity/order matching. |
| Changed structure/support | Declared definition/transform/support rules; validate whole graph and owners. Never discard a piece or its storage to repair support. |
| Unsafe saved player ground | Refuse under current policy. Future approved PlayerStart relocation must preserve items/vitals/identity and report relocation. |
| Invalid/duplicate player, creature or stack identity | Refuse; do not guess replacement owners or merge containers. |

Restoration cancels crafting; transient AI targets are not serialized. Migration must not replay crafts, kills, rewards, gathering or loot. Health=0 is a valid record: preserve it and test death/respawn, never substitute full health.

## Staged migration transaction

1. Start with an offline/native record transform. The server owns it; block joins and save mutations during publication. Reject client migration/save requests. No new plugin or general framework is required.
2. Read a bounded known envelope, preserving bytes/metadata. Select an explicit edge by layer/version/content revision; refuse an absent edge, cycle, oversized candidate or unknown field policy.
3. Make and verify a **separate original-generation backup** before publication. A/B alone cannot provide lasting V1 rollback: the next ordinary save may overwrite its remaining V1 sibling. Backup failure refuses migration. Private backups stay outside Git/public reports.
4. Transform a copy with declared defaults/ID maps. Preserve stable player/structure/batch/creature IDs, owner/support links and time anchors. Validate catalog limits, totals, global duplicate batches and layout before runtime/file changes.
5. Target-encode, decode and validate again; publish only a new candidate generation/slot. Keep the original backup read-only. Prove the candidate in a separate bounded test world before live restart. Existing restoration is not globally rollback-atomic across multiplayer callbacks.
6. Apply at startup before joining. On failure block joins/publication and keep the original backup; provide an explicit restore/restart route. Report version/outcome/counts, never capabilities, login URLs or inventory payloads.

Food time needs special care: player bags age from each `CapturedUtc`; storage/pickups age from file `SavedUtc`. Publishing at a newer UTC cannot renew food. Preserve the old anchor until age is consumed, or subtract age once and establish a matching new anchor. Offline-expired batches may be omitted under existing spoilage policy, with a reported count. Other world timers currently pause offline. Repeated conversion/load must not age twice, extend deadlines, append items or reset cooldowns.

Current implemented progression, documented in [PROGRESSION_PLAN.md](PROGRESSION_PLAN.md), initializes V1 players with zero XP/credits and empty earned-knowledge/window records; baseline recipes remain available. Existing inventory/structures never grant retrospective XP. WorldV2 and the owner-bound progression runtime already exist. Adaptation/modifier loadouts remain proposed/unimplemented; no record or new version is introduced by this audit.

Reconnect-profile conversion remains separate: preserve server roster identity/capability relationships. Endpoint spelling already affects profile selection. A client public ID is not authentication; token rotation needs an explicit server/profile handover. Production accounts are outside M8.

## Fixtures and gates

Use deterministic **synthetic** fixtures with clearly fake public GUIDs and primitive test worlds. Freeze baseline bytes before changing the writer, recording source version, expected records and checksum/hash. An old fixture must prove the previous writer, not today's struct re-encoded as “old.” Private saves/profiles/login URLs never enter source, Trello or uploaded reports.

| Gate | Required checks before expanding |
| --- | --- |
| First known edge | Editor build; old fixture to target round trip; neutral defaults; deterministic conversion; unchanged source/destination on refusal. |
| Food/items | Differing batches; bag/storage/pickup offline expiry; no renewal/double aging; quantity/capacity; renamed/unknown IDs. |
| Whole world | Ownership/support, storage, resource/spawner mapping, live/corpse identities, safe ground and absent/extra layout. |
| Publication/rollback | Corrupt/truncated/future candidate; backup/write failure; failed startup/load; subsequent A/B rotation retains original backup. |
| Repeat application | Migration/load/reconnect cannot duplicate items/actors/rewards or stack effects. |
| Live authority | One client first, actual server restart, then two NullRHI clients where practical; wrong identity/authority, join/leave and separate owners. |
| Manual | Rendered first-person gather/craft/build/store/save/restart/reconnect and freshness comparison; logs and uncapped frame-time/memory evidence. |

Historical October8 test anchor inventory (new current evidence below): existing M8 anchors under `PF.Persistence`: PlayerRoundTrip, PlayerValidation, CorruptPlayerData, FileGenerations, ServerPlayerAdapter, WorldRecords, WorldRuntime, StartupFailurePreservesSave and RejectedLoadPreservesWorld. PF.Persistence.Live remains opt-in/separate. Latest rejected-load run passed 1/1 in Automation_M8RejectedLoad_20261008_044007915_1b40fffb. These prove current behavior, **not an implemented migration suite or M18 acceptance**. See [AUTOMATION_VERIFICATION.md](AUTOMATION_VERIFICATION.md).

Original October8 planning evidence: source/limits/offline anchors/backup behavior and installed JSON converter checked without a new runtime build/test then. Its old M7/M8-unverified status is historical; M7 and solo M8 are now accepted. FullM18/current remaining gates are stated in the overview, not completed by this document.

## October13 current-contract verification

Four exact native tests PASSED4/4: PF.Progression.WorldCompatibility, PF.Progression.Codec, PF.Persistence.WorldRecords, PF.Inventory.LargeStacks. Report Saved/AutomationReports/Automation_M18ContractAudit_20261012_225130102_75a16a93/run-summary.json and index.json; log Saved/Logs/PFAutomation_M18ContractAudit_20261012_225130102_75a16a93.log.16.56s,2.956working/2.794privateGiB; exact selection/exit0/raw and testseverity0/no timeout/fatal/ensure. These exercise known V1 defaults/V2 owner/window preservation, future/corrupt refusal, chest filters/ownership/batch validation and large-stack/trusted carry limits. Current-generated synthetic values are not archived old-writer fixtures.

No source/asset/schema/settings/private save changes, no new build needed for this documentation-only scope; preceding Editor5.68s/Game13.76s passed for c2890ee. No disk restart, content migration, rendering/FPS or human acceptance added here. Conventional PrimalFrontier.log remains the previously closed Editor log; unique native abslog was reviewed. Trello bounded rlak115e closes this audit only; MainM18 stays ToDo/full migration absent. Required M13 encounter G9 before M14 remains unverified,1440 explicitly deferred, permanent backups/content mappings/frozen previous-writer fixtures remain future independently scoped work.
