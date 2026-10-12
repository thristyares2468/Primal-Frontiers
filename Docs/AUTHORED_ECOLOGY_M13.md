# M13 authored open-world ecology - October 13, 2026

The existing open world now uses its validated biome/day-night policy without a developer preview command. This is a bounded M13 implementation, not full milestone acceptance. Two original greybox species remain; additional types/loot and optional predator/prey are subsequent scopes. Taming/companions remain unapproved. The deferred 1440p traversal check is unchanged.

## What changed

`/Game/PrimalFrontier/Creatures/DA_OpenWorldSpawnPolicies` stores the existing native Shore/Woodland/Ridge policy. In `/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld`, the **existing** `PFCreatureSpawner_0` (M7_Forager, -1500/1700/60) uses Shore, and `_1` (M7_Prowler, 1300/4400/60) uses Ridge. Both remain nonspatially loaded, sharing one policy/catalog domain. Clock, actor names, positions, navigation, resources, geometry and existing save layout are preserved. No new spawn point, creature species, art or template-map change.

Fresh daytime starts with one Forager and no Ridge Prowler. Night permits a Prowler once the empty point's retry is due. Dawn retains that living resident/corpse; it does not destroy it or grant loot. Existing per-biome/global eight-creature bounds and server-only nav/clearance/stamina-independent creature rules remain. A new server reloads the saved resident GUIDs, enabled state, clock and bounded delay; configuration belongs to the map, not client requests or arbitrary save class paths. Existing saves retain their already saved Prowler at dawn.

`Scripts/AuditOpenWorldEcology.py` is read-only. `ActivateOpenWorldEcology.py` narrowly preflights the audited map, creates/reuses the policy through AssetTools, assigns only the existing two points, saves through supported Editor APIs and checks reload identity/configuration. It refuses different placement/lifetimes or a custom existing policy. Git includes one new data asset, the two external actor packages and associated map metadata only. No raw binary edits or unrelated assets.

New native/live/rendered fixtures read **authored** settings; they do not reassign spawn policy. The existing persistence fixture selects night through the authoritative clock before waiting for its two test residents, preserving its previous full-loop assertions. Its prior unconditional daytime two-resident assumption is now obsolete. Dedicated restart runner gains `-AuthoredEcology`, exact contract/role checks, unique slot/profile, private-save/settings hashes and process-only120FPS pacing. The controls runner gains `AuthoredEcology` on the real open-world map with uncapped720p rendering.

## Evidence

Report directories below are under `Saved/AutomationReports`; unique logs under `Saved/Logs`. Native/rendered reports contain `index.json` and `run-summary.json`; restart role directories append Create/Restart and Server/Client1/Client2, with exact paths in the parent summary.

| Check | Result | Report/log |
| --- | --- | --- |
| Read-only placement audit | PASS on corrected retry, no asset mutations | `M13AuthoredAuditRetry_20261013/audit.json`; `PFM13AuthoredAuditRetry.log` |
| Supported activation and disk reload | PASS on retry2, names/positions/config preserved | `M13AuthoredActivationRetry2_20261013/activation.json`; `PFM13AuthoredActivationRetry2.log` |
| Editor fixture compilation | PASS15.22s, no compiler warnings | `PFM13AuthoredTestsEditorBuild.log` |
| Final rendered/legacy-fixture Editor build | PASS11.31s, no compiler warnings | `PFM13AuthoredPreviewEditorBuild.log` |
| Native regression | PASS6/6,37.15s, raw/test severity0 | `Automation_M13Authored_20261012_202900035_824d6a47` |
| Dedicated one-client create/restart | PASS4/4, phases44.45/41.27s | `M13AuthoredLive1_20261012_202953109_ec46a55f` |
| Dedicated two-client create/restart, actual75ms outgoing lag/1%loss per role | PASS6/6, phases44.05/43.40s | `M13AuthoredLive2_20261012_203355117_8ea2eb49` |
| Actual authored720p/HUD1.5 night/dawn render | PASS1/1,31.63s; two PNG inspected | `M11AuthoredEcology720_20261012_203545180_33e788c1` |
| Previous full persistence one-client create/restart | PASS4/4, phases58.01/38.70s; original full-loop/privacy assertions intact | `M8Live1_20261012_203645442_31af36c2` |
| Development Game compilation | PASS34.39s, no compiler warnings | `PFM13AuthoredGameBuild.log` |

Six exact native tests: **PF.Creatures.AuthoredPolicy**, **PF.Creatures.EcologySpawner**, **PF.Creatures.SpawnPolicy**, **PF.Persistence.CorpseLootRestore**, **PF.Persistence.WorldRuntime**, **PF.World.OpenWorldAsset**. Authored-policy test loads the actual data asset, validates the whole table, proves day Shore/night Ridge selection and dormant refusal with preserved output.

Dedicated roles use **PF.Creatures.AuthoredLive**. Fresh server verifies actual point names/policy/biomes, waits for real day/night spawning, keeps both at dawn, and saves. Restart reads the first process's real save and compares resident GUIDs/enabled state/cooldown against restored actors without duplicates. Each client observes both living replicated residents, receives correct fresh/restored setup, and fails direct spawning/save requests. Later changes made by disconnect capture use existing server lifecycle; no fake/client save writes or schema changes.

Rendered **PF.Creatures.AuthoredPreviewLive** uses the actual authored points and clock. Inspected `ControlsUI/M11AuthoredEcology720_20261012_203545180_33e788c1/authored_night.png` and `authored_dawn.png`: same living100HP primitive Prowler, first-person HUD, contrasting night/day lighting and no phase duplication. Test temporarily freezes that creature and moves the camera for reproducible framing. It is not sustained human traversal or combat/FPS acceptance.

Passing reports require exact tests, engine/report/runner exits0, zero test warnings/errors, no timeout/fatal/ensure/assertion and protected private state unchanged. Native raw severity0. Authored network raw24warnings/14known installed experimental-Python errors are the established open-world widget/HLOD/Python baseline; the older small-map ecology had21warnings because it lacked three HLOD import warnings. Rendered raw baseline must be inspected before final publication. Commandlet world teardown retains the known Recast/Crowd warning; it is not a new creature runtime error. Conventional `PrimalFrontier.log` was checked but remains the prior editor exit; current isolated evidence uses unique logs.

## Memory and limits

Final raw review: all14 authored/legacy network role logs match known24warning14Pythonerror open-world startup/HLOD baseline with zero new normalized categories. Rendered26warning14Pythonerror matches previous720p baseline with zero new categories. The earlier statement that final rendered review is pending below is superseded. Legacy persistence replay additionally guarded398private files/preferences unchanged, peak1.814working1.776privateGiB. Game34.39s passed/no compiler warnings; no Unreal process remains.

Native peak2.922GiBworking/2.839private. One-client maximum1.788/1.723GiB; two-client maximum1.796/1.776GiB across per-role sampled peaks. Rendered720p3.218/5.406GiB. Private files/default preference guards:387one-client,391two-client,396rendered, unchanged. Host31.93GiBRAM and installedUE5.8.3; requested5.8.2 and minimum16GB are not certified. Only one rendered window; NullRHI for servers/clients. Rendered uncapped; headless transport process-only120FPS. No sustained FPS/human stuttering, physical-controller/audio or packaged Server pass (installed engine cannot build Server target).

## Failures retained

Initial audit failed because Python removes the `b` prefix (`b_use_spawn_policy` missing); corrected `use_spawn_policy` and replay passed with zero mutations. First activation saved the intended packages but Python struct `!=` comparison failed value equality after reload. Follow-up used the native equality helper but first requested its C++ class name; engine header exposes ScriptName `GameplayTagLibrary`. Corrected to the supported Python library; retry2 reload passed. Earlier logs remain `PFM13AuthoredAudit.log`, `PFM13AuthoredActivation.log`, `PFM13AuthoredActivationRetry.log`; none is falsely labelled pass.

Initial Editor build21.92s succeeded before the new fixtures were discovered outside Source; moved only newly created files into `Source/PrimalFrontier/Tests`, then the15.22s build explicitly compiled them. The six exact-selector native tests passed, so an unrelated successful target build was never used as test-discovery evidence. Subsequent rendered fixture compiled11.31s clean.

## Replay and next scope

No user action is required for these automated commands. Close Unreal before isolated runs; never use personal slots. From project PowerShell:

```powershell
./Scripts/RunNativeAutomation.ps1 -TestFilter 'PF.Creatures.AuthoredPolicy+PF.World.OpenWorldAsset+PF.Creatures.SpawnPolicy+PF.Creatures.EcologySpawner+PF.Persistence.WorldRuntime+PF.Persistence.CorpseLootRestore' -Label M13Authored
./Scripts/RunPersistenceAutomation.ps1 -AuthoredEcology -Players 1 -Port 17997
./Scripts/RunPersistenceAutomation.ps1 -AuthoredEcology -Players 2 -SimulateLagLoss -Port 17997
./Scripts/RunControlsAutomation.ps1 -TestCase AuthoredEcology -Resolution 720 -HUDScale 1.5
./Scripts/RunPersistenceAutomation.ps1 -Players 1 -Port 17997
```

Native: no saved level required except read-only OpenWorldAsset. Live/rendered: `/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld`; dedicated NullRHI or one standalone rendered process respectively. Start with the native runner, then one client before two, wait for each, inspect exact reports/logs/captures/hash guards; stop/fix failure before expanding. This bounded checkpoint does not finish M13. More original creature/loot definitions are future AI work; human ecology feel, controller, rendered multiplayer, minimum spec and asset provenance stay distinct. 1440 remains deferred by user choice. Existing accepted G8 and solo persistence checks need no repeat.
