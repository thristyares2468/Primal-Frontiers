# Save compatibility and migration plan — M18

October 8, 2026. **Planning complete; migration is not implemented.** Full M18 remains To Do. This independent source/document contract prepares for future progression/content changes. It does not close M7/M8 manual gates, change a version, rewrite saves or certify production authentication/crash durability.

## Current contracts checked in source

| Layer | Version and bounds | Present behavior |
| --- | --- | --- |
| File store | V1, 32-byte header, at most 4 MiB payload | Magic/version, generation, UTC, length and CRC; verified A/B inactive-generation publication. |
| World metadata | `FPFWorldSaveData.Version=1`, JSON | Approved maps, bounded arrays, strict field conversion and semantic validation. |
| Player/container codec | `CurrentVersion=1`, 16-byte envelope, at most 64 KiB | Explicit scalar/GUID/ASCII item fields, CRC, trusted catalog/capacity checks; no arbitrary object loading. |
| Local reconnect profile | File-store V1, 16-byte GUID payload | Endpoint/profile-specific capability, separate from public player ID; no independent profile schema field yet. |
| Content/world compatibility | Catalog IDs and authored resource/spawner names | Unknown definitions/layouts and invalid owner/support/batch identities refuse load; no revision/mapping system yet. |

Sources: `PFSaveFileStore.{h,cpp}`, `PFWorldSaveData.h`, `PFWorldSaveFormat.cpp`, `PFPlayerSaveFormat.{h,cpp}`, `PFPlayerSaveAdapter.cpp`, `PFWorldPersistence.cpp`, `PFLocalPlayer.cpp`. Current world limits: 32 players, 128 structures/resources/pickups each, eight creatures and 32 spawners. Bags/storage have eight slots and trusted weight limits 30/60 respectively. Future expansion must change and test codec/runtime limits together, never trust capacity from the save.

These version layers are independent. Adding a world field does not itself justify replacing the file envelope or reconnect profile. Engine patch numbers are not save versions. Installed engine is 5.8.3; originally requested 5.8.2.

Current fallback checks **file integrity**, not semantic migration: Read chooses the highest valid checksummed generation and explicitly reports an invalid sibling as backup recovery. Unsupported selected world semantics refuse Load; no silent older-version fallback occurs. Write refuses existing corrupt generations. Failed startup restoration blocks login/capture/save until deliberate recovery and restart.

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

For planned progression, [PROGRESSION_PLAN.md](PROGRESSION_PLAN.md) initializes V1 players with zero XP/credits, baseline knowledge and empty modifier loadout; existing inventory/structures never grant retrospective XP. Add the record/version extension only with that runtime component and tests. No V2 schema is introduced here.

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

Existing M8 anchors under `PF.Persistence`: PlayerRoundTrip, PlayerValidation, CorruptPlayerData, FileGenerations, ServerPlayerAdapter, WorldRecords, WorldRuntime, StartupFailurePreservesSave and RejectedLoadPreservesWorld. PF.Persistence.Live remains opt-in/separate. Latest rejected-load run passed 1/1 in Automation_M8RejectedLoad_20261008_044007915_1b40fffb. These prove current behavior, **not an implemented migration suite or M18 acceptance**. See [AUTOMATION_VERIFICATION.md](AUTOMATION_VERIFICATION.md).

Planning checked against actual fields/versions/limits, offline anchors, backup/lookup behavior, tests and installed JSON converter source. No runtime/schema/assets/saves/settings changed; no new build/test/FPS measurement is claimed. Full M18 still depends on preceding acceptance. M7/M8 manual and packaged Server gates remain open.
