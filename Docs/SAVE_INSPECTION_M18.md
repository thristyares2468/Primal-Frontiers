# M18 — read-only save compatibility inspection

`PF.CheckSave [slot]` inspects an existing world save against the current authoritative gameplay world. The default slot is `Survival`. It is a non-Shipping PrimalAgentTools command, listed in `PF.Help`, with the normal `[PrimalAgentTools]` log prefix and command-history export. No hosting, database, provider, asset, map or rendering dependency was added.

## Contract

The game-thread server API `UPFWorldPersistence::InspectSave` checks slot syntax/reserved namespaces, bounded A/B file integrity, supported world/player/progression semantics, trusted item/recipe/building/creature catalogs, current map/resource/spawner layout and membership of connected player records. It shares the same layout predicate as actual restoration. A successful result reports world schema, original generation/UTC, record counts, explicit backup recovery and whether the reviewed old fifteen-node Copper layout needs its existing migration. The prepared resource count includes that reviewed addition; it does not claim the source already contains sixteen records.

Inspection prepares a local value copy only. It never invokes Apply, Load, Save or original-backup publication; never changes the active slot, roster, actor set, inventory, crafting, vitals, clock, profiles or source files. Catalog caches may be initialized. Failed API calls leave the output unchanged and return a reason. Clients, wrong worlds, invalid/reserved slots, unknown definitions/layouts, absent connected player records and files without a healthy generation refuse. Corrupt sibling evidence is retained; a healthy selected generation can be inspected with `backupRecovery=1`. No silent semantic fallback or repair is introduced.

The structured command report includes Players, Structures, Resources, Spawners, Creatures, Pickups, BackupRecovery and RequiresCopperMigration counters. It contains no player identities, reconnect credentials or save payload. A `Passed` result means the listed inspection checks passed, including explicitly reported recovery/migration conditions. It is **not** a guarantee of actor allocation, placement, complete restoration, authentication, crash durability or full M18 acceptance. Existing Load and restart gates remain separate.

## Reproduction

Backend native fixtures require no authored level or user action. Start with a closed Editor/game and run from the project root:

```powershell
.\Scripts\RunNativeAutomation.ps1 -TestFilter 'PF.Persistence.SaveInspection+PF.Persistence.CopperLayout+PF.Persistence.WorldRuntime+PF.Persistence.StartupFailurePreservesSave+PF.Persistence.RejectedLoadPreservesWorld' -Label M18InspectionFinal -TimeoutSeconds 180
.\Scripts\RunPersistenceAutomation.ps1 -Players 1 -Port 17989 -TimeoutSeconds 210
.\Scripts\RunPersistenceAutomation.ps1 -Players 2 -SimulateLagLoss -Port 17989 -TimeoutSeconds 210
```

The last two commands run `PF.Persistence.Live` on `/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld` using uncooked dedicated server and NullRHI clients. Create and restart use separate processes and unique disposable slots/profiles. Run one client first. The server saves, changes unsaved stone/health/time, inspects, verifies those unsaved values are unchanged, then performs the existing restore/restart checks. Clients must refuse the command and direct inspection API without changing the caller's output. Both restored ownership/storage and private progression remain covered by the existing fixture. These are backend network tests, not rendered gameplay/FPS evidence.

For optional use in an already running standalone development world or server console: `PF.CheckSave YourExistingSlot`, then `PF.ExportTestReport SaveInspection`. The active map must match the save; connected players must exist in its records. Missing slots fail rather than creating a world. Do not substitute `PF.LoadWorld` for this command when only inspection is intended. Output is under `Saved/AutomationReports`; no editor map save occurs. No new Personal test is required for this backend feature.

## Evidence

First Editor build `Saved/Logs/PFM18SaveInspectionEditorBuild.log` passed52.35s; final `PFM18SaveInspectionFinalEditorBuild.log` passed15.65s. No compiler warnings.

`Automation_M18SaveInspection_20261015_020210855_92d0209f` passed4/4: PF.Persistence.SaveInspection, PF.Persistence.RejectedLoadPreservesWorld, PF.PrimalAgentTools.CommandArguments, PF.PrimalAgentTools.MissingSystemsAreBlocked.55.52s, sampled working/private3.028/2.954GiB, raw/test warnings/errors0, exit0.

`Automation_M18InspectionFinal_20261015_020714143_4d6958e5` passed5/5: PF.Persistence.SaveInspection, PF.Persistence.CopperLayout, PF.Persistence.WorldRuntime, PF.Persistence.StartupFailurePreservesSave, PF.Persistence.RejectedLoadPreservesWorld.19.30s, sampled working/private3.024/2.894GiB, raw/test warnings/errors0, no timeout/fatal/ensure, exit0. Both repeat/current/refusal/recovery tests use real disposable files; Copper coverage uses a synthetic prior-layout source written by current test code, not an archived previous executable or personal save.

Final Game `Saved/Logs/PFM18SaveInspectionGameBuild.log` passed40.97s without compiler warnings. Seven distinct native selectors passed across the two runs (nine native execution records); no build/test failure in this bounded task.

| Actual NullRHI PF.Persistence.Live run | Result | Create / restart seconds | Sampled working / private GiB per process |
| --- | --- | --- | --- |
| M8Live1_20261015_020756496_e54eff29 |4/4 roles passed|62.55 /44.07|1.733–1.824 /1.635–1.798|
| M8Live2_20261015_021024532_9bb4eb8d,75ms outgoing lag and1% loss |6/6 roles passed, Client2 did not crash|66.65 /45.57|1.728–1.830 /1.625–1.803|

All ten live role reports have zero test errors/warnings, engine/report/runner exits0, no fatal/ensure, correct exact selector and role/RPC/privacy/network evidence. Server CheckSave returned saved player counts1/2, structures2, resources16, spawners2 while retaining unsaved stone1/health90/hour9; later real Load/restart conserved owners, stored items and progression. Client commands/direct inspection were refused without changing output. `PF.Help` ran and lists the new help description. Exported CheckSave records include exact test name, timestamp, map, Passed status, structured counts and explicit restoration limits.

Native reports are `Saved/AutomationReports/<run>/index.json` and `run-summary.json`; logs are `Saved/Logs/PF<run>.log`. Live summary is `Saved/AutomationReports/<run>/run-summary.json`; role reports append CreateServer/CreateClient1/CreateClient2/RestartServer/RestartClient1/RestartClient2 to that run name as applicable. Actual structured server exports:

- `Saved/AutomationReports/PF_M8PersistenceCreateServer_20261015T020841_1B3CA0054F87E5A0D7790FB5D967F2B7.json`
- `Saved/AutomationReports/PF_M8PersistenceCreateServer_20261015T021109_DD2FEEAD4C814C11232E32A5C08B2856.json`

Live raw severity is24warnings/14Python error lines per role:21EditorDataStorageUI warnings, three uncooked HLOD linker warnings and14engine Python errors. Compared all ten logs' normalized unique severity text to the existing `PFM15VoiceMetadata1_20261014_233831191_1f24e9c9Server.log`:33unique messages, zero new messages per role. These known startup issues remain warnings/limitations, not silently hidden or claimed fixed. Ordinary `Saved/Logs/PrimalFrontier.log` still contains the separate pre-task October14,23:00UTC D3D12 device crash; unique current NullRHI logs do not reproduce it.

The live runner verified598(one-client)/602(two-client) unrelated `.pfs` and game/scalability settings snapshot entries unchanged. No personal save was loaded or inspected. No assets, map layouts, schemas, source credentials or provider settings changed. Reports remain generated/untracked. There is no sustained-FPS, rendering, stutter or minimum16GiB certification; measured host is31.93GiB. No human observation or whole milestone pass inferred.

Installed engine is5.8.3 CL58210709; requested baseline remains5.8.2. Launcher distribution still cannot produce the packaged Server target. NullRHI tests measure neither rendering nor stutter. Source access/cloud/database/voice setup is deferred by the user; Copper's ordinary earned-loop and unrelated Personal/device/performance gates remain separate.
