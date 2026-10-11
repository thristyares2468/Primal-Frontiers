# M13 authoritative ecology spawning checkpoint

October 12, 2026 (Brisbane). Bounded technical task: https://trello.com/c/lDtUfjgi. **This spawning integration passed its technical gates; full M13 is not complete.** Authored open-world activation, additional creatures/loot, predator/prey and human acceptance remain separate. Taming/companions require explicit approval. Installed engine reports 5.8.3 under UE_5.8; these results do not certify the requested 5.8.2 executable.

## Current behavior

`APFCreatureSpawner` can explicitly opt into the validated `UPFCreatureSpawnCatalog` using `bUseSpawnPolicy`, `SpawnPolicies` and a native biome tag. Defaults keep existing fixed-ID spawning. No binary map/data asset, creature definition, module dependency, save schema, private save or default user setting was changed.

Only an authoritative game world with exactly one world clock can select/spawn. The server derives the clock phase, validates the whole policy/catalog, then draws a bounded weighted ticket. Zero eligible tickets is a valid dormant phase: no fallback creature and no spawn. Clients are refused even if a locally created actor reports an Authority role. Selection refusals preserve outputs; successful spawning still requires navmesh projection, capsule clearance and the existing global eight-creature cap.

All opted-in points share one policy/catalog domain. Per-biome budgets count living residents and corpses, including those owned by disabled points. Duplicate resident ownership, unknown biomes and mixed policy/catalog domains refuse new spawning. A point retains its resident at dawn or configuration failure; phase changes never destroy creatures or grant loot. Policy respawn duration feeds the existing cooldown record. Failed spawn attempts retry after three seconds; identical refusal reasons log only when they change, using `[PrimalAgentTools]`.

The existing world save path restores resident identity and remaining delay. Configuration is map-owned, not stored in the save. The new live fixture verifies **two save/load operations in the same server process**, not a new-server restart with authored ecology configuration. Existing M8 restart evidence is separate; ecology startup/reconnect with authored open-world assignments still needs its own gate.

`PF.PreviewEcology` offers a disposable rendered preview on standalone `/Game/PrimalFrontier/Maps/L_M7SurvivalArena`. It preflights the existing two points/one clock/project catalog, refuses personal loaded saves, clears runtime creatures without loot, configures Shore/Ridge and starts 09:00. Day has a Shore Forager; night permits a Ridge Prowler; dawn retains it. Repeating the command resets this preview. It neither saves a map nor grants player items. **Stop PIE without saving the preview world.** The command appears in `PF.Help`, records normal command history and remains excluded from Shipping with the existing tooling plugin.

## Passed verification

Final catalog-domain review compares every point's effective catalog, including the project-catalog fallback, rather than treating an unassigned catalog as any custom domain. Native checks refuse explicit mixed/custom-fallback domains with preserved outputs and permit the restored common domain. No asset/schema change. Final Editor build: `Saved/Logs/PFM13EcologyCatalogEditorBuild.log`, 6.63 seconds; Development Game: `Saved/Logs/PFM13EcologyCatalogGameBuild.log`, 14.20 seconds. Both succeeded without compiler warnings. Earlier pre-review Editor6.32s/Game27.06s logs remain as history.

Post-review native report `Automation_M13CatalogReview_20261011_215516260_9ab02c6a`: the same exact six native tests below passed 6/6, 16.57s, 3.027/2.881GiB, test/raw severity and exits zero, no timeout/assertion/fatal/ensure. One-client `M13EcologyEcology1_20261011_215643794_63576c0f`: 2/2, 33.76s; Server1.719/1.621GiB, C1 1.798/1.777GiB; 339 unrelated saves/preferences unchanged. Two-client `M13EcologyEcology2_20261011_215733436_1b371547`: 3/3, 42.53s, 75ms/1% confirmed; Server1.722/1.618GiB, C1 1.810/1.774GiB, C2 1.807/1.787GiB; 342 files/preferences unchanged. All selections/exits/test severity zero, no timeout/assertion/fatal/ensure; each raw role21warnings14knownPythonerrors, zero new normalized categories. The original passed gates follow; unchanged command/M6 legacy tests need no duplicate replay for this opt-in-only comparison.

Final rendered post-review `M11EcologyPreview720_20261011_215848791_128d9665`: exact PF.Creatures.EcologyPreviewLive passed 1/1, 32.32s, 3.156/4.072GiB working/private. All exits/test severity zero, no timeout/assertion/fatal/ensure; 346 unrelated saves/default settings/source settings unchanged. Raw23warnings14knownPythonerrors, zero new normalized categories. Both final PNGs inspected: `Saved/AutomationReports/ControlsUI/M11EcologyPreview720_20261011_215848791_128d9665/night_resident.png` and `day_retained_resident.png`. The same Prowler label/100 health, first-person HUD and changed day/night lighting are visible. No human or FPS pass inferred.

Report paths below are directories beneath `Saved/AutomationReports`; each native/rendered directory has `index.json` and strict `run-summary.json`. Network directories have `run-summary.json`, with exact role `index.json` paths in that summary (role directories append Server/Client1/Client2 to the run name).

| Run | Exact successful records | Seconds | Peak working/private GiB |
| --- | --- | --- | --- |
| Automation_M13EcologyRetry_20261011_212658315_b610c77c | PF.Creatures.EcologySpawner: 1/1 | 16.20 | 2.987 / 2.851 |
| Automation_M13EcologyRegression_20261011_213737080_1133ceea | PF.Creatures.EcologySpawner, PF.Creatures.SpawnPolicy, PF.Creatures.Lifecycle, PF.World.Clock, PF.Persistence.WorldRuntime, PF.Persistence.CorpseLootRestore: 6/6 | 16.63 | 2.986 / 2.875 |
| Automation_M13EcologyCommands_20261011_213822825_2369400c | PF.PrimalAgentTools.CommandArguments, PF.PrimalAgentTools.MissingSystemsAreBlocked: 2/2 | 16.15 | 3.026 / 2.882 |
| M11EcologyPreview720_20261011_213924786_e8cb0464 | PF.Creatures.EcologyPreviewLive: 1/1 | 34.96 | 3.183 / 4.124 |
| M13EcologyEcology1_20261011_214122691_47fc9eb5 | PF.Creatures.EcologyLive: server + client, 2/2 | 33.33 | Server 1.718 / 1.632; C1 1.799 / 1.770 |
| M13EcologyEcology2_20261011_214214969_7795887c | PF.Creatures.EcologyLive: server + two clients, 3/3, outgoing 75ms lag / 1% packet loss confirmed | 41.49 | Server 1.716 / 1.617; C1 1.813 / 1.771; C2 1.805 / 1.790 |
| M13EcologyLifecycle1_20261011_214701436_b55c5cdf | Existing PF.Creatures.Live: server + client, 2/2 | 41.07 | Server 1.719 / 1.622; C1 1.807 / 1.783 |

Every final record is clean Success, exact selector/record count confirmed, all engine/strict runner exits zero, no timeout, test warning/error, assertion, fatal or ensure. This is narrow coverage, not a combined 55-test source checkpoint.

Native coverage includes invalid configuration/clock/ticket, dormant phase, server/client refusal, resident retention, disabled-point/corpse/biome/global budgets, mixed domains, duplicate ownership and real no-nav refusal. Live ecology checks actual nav spawning, client mutation refusal, dormant day/night/day retention, twice-restored stable identity/no duplication and cooldown. The client waits for a post-restore 09:15 marker, so the original map's 09:00 residents cannot satisfy the completion check. Existing M6 live coverage also passed real obstacle navigation, flee/chase, attack, death and client recovery of loot.

Rendered automation uses one 1280x720 process with uncapped FPS requested (`t.MaxFPS 0`, VSync off). Both actual PNGs were inspected: first-person HUD, placeholder Prowler and night/day lighting are visible. Files:

- `Saved/AutomationReports/ControlsUI/M11EcologyPreview720_20261011_213924786_e8cb0464/night_resident.png`
- `Saved/AutomationReports/ControlsUI/M11EcologyPreview720_20261011_213924786_e8cb0464/day_retained_resident.png`

The reused controls runner retains its historical M11 filename prefix; the exact selector is M13's `PF.Creatures.EcologyPreviewLive`. Rendered actor movement was frozen for screenshot identity checks; this is not human combat/navigation feel. The separate unchanged M6 live test exercises movement.

Raw logs: native `Saved/Logs/PF<run>.log`; network `Saved/Logs/PF<run><role>.log`; rendered `Saved/Logs/M11EcologyPreview720_20261011_213924786_e8cb0464.log`. Native logs have zero warnings/errors. Final network roles each retain 21 startup warnings and 14 installed experimental StateTree Python/toolset errors; rendered retains 23/14. Normalized severity comparison against retained M8 network/M12 rendered baselines found **zero new categories**; these logs are not error-free. No gameplay assertion/fatal/ensure. `Saved/Logs/PrimalFrontier.log` was checked but remains October9's ordinary PIE log; unique abslogs are current evidence.

Unrelated save/preference hashes remained unchanged: rendered 330 files, final one-client 331, final two-client 334 and legacy regression 338. Counts grow because earlier isolated automation files are included in subsequent untouched-file guards. Each run uses disposable slot/profile(s), bounded deadlines and cleanup of only its owned processes. No Unreal process remains from the tests.

Host has 31.93 GiB physical RAM; these are process peaks, not a minimum-16GB certificate. No sustained FPS or human stuttering observation was measured. Two rendered clients were avoided. Installed engine cannot build a packaged Server target; tested servers are uncooked NullRHI editor processes.

## Failures and corrective replays retained

- `PFM13EcologyEditorBuild.log` initially succeeded 25.48s. Native `Automation_M13EcologySpawner_20261011_212546178_d9b536d7` failed only because an inherited expected mesh warning did not occur in its no-player fixture. Gameplay assertions passed; removed that expectation, rebuilt 5.66s (`PFM13EcologyEditorRetry.log`), exact retry above passed.
- Live-fixture compile failed C4458/C2280/C2672 in `PFM13EcologyLiveEditorBuild.log` (9.83s): shadowed Clock, deleted weak-pointer bool and missing outer `.Get()`. Test-only corrections; retry `PFM13EcologyLiveEditorRetry.log` (5.80s) then failed LNK2019/LNK1120 from a direct GameplayTag request. The fixture reads the existing native catalog tag instead; no new dependency. Retry2 passed 6.28s.
- Initial `M13EcologyEcology1_20261011_213040337_7441eb74` failed server save with three test errors because M6 is outside the persistence whitelist; client passed. Corrected only the fixture to existing save-supported L_M7SurvivalArena; did not weaken the whitelist. Map retry Editor passed 6.31s. Corrected one-client `...213238011_fe1d5085` and two-client `...213417057_daae42e1` passed before final replays above.
- The earlier two-client run had one extra AutoSDK backup deletion code2 warning during concurrent engine startup, without gameplay failure. Runner now waits for Client1 map load before starting Client2; the extra warning is absent in the final two-client replay.
- Preview Editor build passed 10.65s (`PFM13EcologyPreviewEditorBuild.log`); final marker/client readiness rebuild passed 6.32s. No production spawning failure remained after final verification.

## Replay (AI)

Close Unreal Editor before isolated automation. Start here: project PowerShell. Commands run sequentially; stop and inspect any failure before broadening scope.

1. `./Scripts/RunNativeAutomation.ps1 -TestFilter 'PF.Creatures.EcologySpawner+PF.Creatures.SpawnPolicy+PF.Creatures.Lifecycle+PF.World.Clock+PF.Persistence.WorldRuntime+PF.Persistence.CorpseLootRestore' -Label M13EcologyRegression -TimeoutSeconds 120` — temporary native fixtures, no level required, exact six tests.
2. `./Scripts/RunNativeAutomation.ps1 -TestFilter 'PF.PrimalAgentTools.CommandArguments+PF.PrimalAgentTools.MissingSystemsAreBlocked' -Label M13EcologyCommands -TimeoutSeconds 120` — exact two tests, including actual Help registration/history.
3. `./Scripts/RunEcologyAutomation.ps1 -Players 1`, then `./Scripts/RunEcologyAutomation.ps1 -Players 2 -SimulateLagLoss` — exact PF.Creatures.EcologyLive, L_M7SurvivalArena, two then three successful role records.
4. `./Scripts/RunControlsAutomation.ps1 -TestCase EcologyPreview -Resolution 720` — exact PF.Creatures.EcologyPreviewLive, standalone L_M7SurvivalArena, one rendered record and two PNGs.
5. `./Scripts/RunEcologyAutomation.ps1 -TestCase Lifecycle -Players 1` — exact PF.Creatures.Live on L_M6Creatures. No save-slot argument on this whitelist-excluded navigation fixture.
6. Inspect generated summary/index/raw logs and actual screenshots, compare known startup severity, confirm save/preference guards and process exits; a reported engine exit alone is insufficient.

Manual next step: **Guide G8 in [TRELLO_TEST_GUIDES.md](TRELLO_TEST_GUIDES.md)**. Use the existing small fixture, not the open-world map, for this disposable command. No new asset or source edit is needed from the user. After actual observations, the next AI scope is an explicitly authored open-world biome/policy activation with resident startup/restart checks. Future placement and policy asset are TBD; do not certify them with this fixture.

## Changed files

- `Source/PrimalFrontier/Creatures/PFCreatureSpawnCatalog.h`, `PFCreatureCatalog.cpp`, `PFCreatureSpawner.h/.cpp`: validated phase resolution and opt-in server spawning.
- `Source/PrimalFrontier/Tests/PFCreatureTests.cpp`: native ecology authority/domain/budget checks.
- `Plugins/PrimalAgentTools/Source/PrimalAgentToolsRuntime/Private/PFCommands.cpp`, `Tests/PFCommandTests.cpp`, `Tests/PFCreatureLiveTests.cpp`: command/help/history, refusal and actual rendered/network fixtures.
- `Scripts/RunEcologyAutomation.ps1`, `Scripts/RunControlsAutomation.ps1`: guarded isolated runners and preview selector.
- `Docs/CURRENT_STATE.md`, `MILESTONES.md`, `DECISIONS.md`, `ROADMAP_STATUS.md`, `TRELLO_SYNC.md`, `TRELLO_TEST_GUIDES.md`, `ECOLOGY_SPAWN_POLICY_M13.md`, this evidence file; plugin README/DEVELOPER_COMMANDS: current contract, guides and evidence.

No binary assets, maps, project INIs, external art, private saves or generated reports are included in the commit.
