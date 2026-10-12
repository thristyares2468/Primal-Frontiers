# M13 weighted ecology integration and human gate - October 13, 2026

Four original greybox types are now eligible in ordinary open-world ecology. This completes the tested technical integration, not the human milestone gate. **Before M14 expansion, the user must run G9 in TRELLO_TEST_GUIDES.md.** No pass is assumed. The separate1440p diagnostic stays deferred.

## Active policy

| Existing point | Biome | Day pool | Night pool |
| --- | --- | --- | --- |
| PFCreatureSpawner_0 | Shore | Forager1 / Scrabbler1 | Forager1 / Scrabbler1 |
| PFCreatureSpawner_1 | Ridge | Dormant, zero tickets | Prowler1 / DuskPouncer1 |

Numbers are relative weights, not simultaneous creature counts. Each existing point owns one resident; same per-biome/global cap8 rules. Dawn retains living/corpse residents; normal corpse expiry and remaining cooldown still apply. Save/restore keeps original resident GUIDs, types, enabled state and remaining delay. Old Forager/Prowler saves remain valid; no actor names, point count/position, geometry, resources or schema changed. Natural replacements/new worlds can choose new types, while an existing saved resident is preserved.

`Scripts/ExtendOpenWorldEcologyVariety.py` uses supported Editor APIs to append only known alternate IDs to DA_OpenWorldSpawnPolicies. It preserves original entries/weights and refuses custom/duplicate/unexpected layout. Setup passed exit0, `Saved/AutomationReports/M13WeightedSetup_20261013/policy.json`, `Saved/Logs/PFM13WeightedSetup.log`. Git asset scope is **one existing policy data asset**, not map/external actor packages. No additional art or asset-source input needed for primitives.

## Final verification

Reports below are directories under Saved/AutomationReports. Native/rendered contain index.json/run-summary.json. Network parent summary links each Create/Restart Server/Client role index and unique log.

| Check | Result | Evidence |
| --- | --- | --- |
| Final Editor compilation | PASS5.52s, no compiler warnings | Saved/Logs/PFM13WeightedCaptureEditorBuild.log |
| Final Game compilation | PASS13.64s, no compiler warnings | Saved/Logs/PFM13WeightedCaptureGameBuild.log |
| Actual weighted asset/native regression | PASS5/5,17.03s | Automation_M13Weighted_20261012_212239465_eca7980b |
| One-client new-server restart | PASS4/4, phases44.62/41.29s | M13AuthoredLive1_20261012_212431992_eedc3500 |
| Two-client new-server restart,75ms outgoing lag/1%loss each role | PASS6/6, phases43.97/43.34s | M13AuthoredLive2_20261012_212652311_7c706cee |
| Corrected actual authored720p/HUD1.5 night/dawn | PASS1/1,29.73s; two PNG inspected | M11AuthoredEcology720_20261012_213352839_a9f221ae |

Five exact native tests: **PF.Creatures.AuthoredPolicy, PF.Creatures.Variety, PF.Creatures.EcologySpawner, PF.Persistence.WorldRuntime, PF.Persistence.CorpseLootRestore**. Authored policy checks actual asset ticket0 baseline/ticket1 new IDs and dormant daytime Ridge. Variety covers actual loaded defaults/loot guards/death duplicates/new-ID actual file roundtrip; other cases retain server/domain/corpse/world compatibility checks.

Network PF.Creatures.AuthoredLive compares each new server's real resident GUIDs/enabled state/cooldown with previous saved records; client verifies exactly one valid passive/hostile pool member, positive replicated health, fresh/restored setup and direct mutation refusals. One-client Create server actually selected Forager and **DuskPouncer**, then restored that new variant. Two-client Create chose baseline Forager/Prowler, proving legacy pool members still work. Random selection is not a guaranteed sample of every species: Scrabbler's alternate ticket is proven deterministically native, and both new types had real AI/navigation/combat/loot/replication passes in CREATURE_VARIETY_M13.md. No invented claim that random two-client chose Scrabbler.

Rendered PF.Creatures.AuthoredPreviewLive uses the real authored points without preview-policy reassignment. Two inspected PNGs at `ControlsUI/M11AuthoredEcology720_20261012_213352839_a9f221ae/authored_night.png` and `authored_dawn.png` show the same100HP baseline Prowler, readable current-camera caption, first-person HUD and changed lighting. Test freezes movement and frames camera; ordinary ticking settles labels before capture. This is presentation/retention evidence, not a human sustained route or combat-feel certificate.

All16 final test records have exact selectors/exits0, test warnings/errors0, no timeout/fatal/ensure/assertion. Native raw severity0. Ten network logs match24warnings/14known widget/HLOD/installed-Python errors baseline, zero new normalized categories. Corrected render matches26warnings/14knownPythonerrors baseline, zero new categories. Raw warnings are retained, not hidden. Conventional PrimalFrontier.log checked but remains prior editor exit; these runs deliberately use unique absolute log paths.

Native peak3.004GiBworking/2.877private; one-client maximum1.792/1.794GiB; two-client1.797/1.793GiB per-role peaks; corrected render3.116/4.903GiB. Private-save/default-preference guards407one-client,411two-client,417rendered all unchanged. Host31.93GiBRAM/RTX4060 observed8GB/installedUE5.8.3; no minimum16GB, requested5.8.2 binary, sustainedFPS, human stutter/controller or packaged Server certificate. Only one rendered window, uncapped; headless transport120FPS process-only. No unnecessary editor instance or1440 retry.

Earlier Editor20.87s/Game27.88s passed before capture-only fix. First weighted rendered M11AuthoredEcology720_20261012_212918628_ded7fbb2 passed assertions but its frozen caption was edge-on; visual gate was rejected. Corrected ordinary label ticking/capture freeze, rebuilt Editor5.52s, replayed successfully and inspected final PNG; final Game13.64s clean. No failed gameplay assertion was hidden. No production behavior/policy changed during the visual repair.

## Replay / remaining gate

AI: close Unreal and run sequentially, stop/fix first failure:

```powershell
./Scripts/RunNativeAutomation.ps1 -TestFilter 'PF.Creatures.AuthoredPolicy+PF.Creatures.Variety+PF.Creatures.EcologySpawner+PF.Persistence.WorldRuntime+PF.Persistence.CorpseLootRestore' -Label M13Weighted
./Scripts/RunPersistenceAutomation.ps1 -AuthoredEcology -Players 1 -Port 17997
./Scripts/RunPersistenceAutomation.ps1 -AuthoredEcology -Players 2 -SimulateLagLoss -Port 17997
./Scripts/RunControlsAutomation.ps1 -TestCase AuthoredEcology -Resolution 720 -HUDScale 1.5
```

Native no launched level required; live/rendered `/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld`, dedicated NullRHI/new-server restart or standalone720 respectively. Use runner-owned slots/profiles only; inspect exact summaries/indices/logs/captures/private hashes. See AUTHORED_ECOLOGY_M13.md and CREATURE_VARIETY_M13.md for preceding stable-layout, combat/loot and failure evidence.

Personal: **G9**, same OpenWorld, one normal standalone fresh unused world. Observe daytime passive, set21:00 for a short night encounter, escape/fight/loot if chosen, then immediate dawn retention/readability. Report M13 encounters PASS or first failure, with loot NOT TESTED if avoided and approximate moving FPS. Existing M7/G8/solo persistence/chest/Pause/Settings acceptance needs no repeat;1440 remains deferred. Full M13 stays incomplete until actual encounter acceptance; M14 expansion does not start automatically before it.

Optional predator/prey remains unimplemented: a separate targeting/economy/authority scope would be needed and is deferred rather than casually combining it with survivor combat. Taming/companions need explicit approval; adaptation/biomod research/progression remains planned. These optional/future systems are not falsely counted as done. M9 provenance still gates art integration. No external asset is needed for this manual greybox gate.
