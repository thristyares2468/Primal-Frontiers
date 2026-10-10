# M13 biome/day-night spawn policy prerequisite

October 10, 2026. Trello https://trello.com/c/MGOxKJEE. **Data-only technical gate passed; no live spawning change or full M13 acceptance.** The user permits independent work while M11 usability/M12 pacing remain unverified.

## Implemented contract

UPFCreatureSpawnCatalog is a focused Unreal UDataAsset type, with Blueprint-readable/editable structs for native biome tag, existing creature IDs, day/night weights, resident budget and respawn duration. No binary asset, map assignment, new creature or live spawner consumer is created.

Source: Creatures/PFCreatureSpawnCatalog.h; implementation in existing PFCreatureCatalog.cpp; new PF.Creatures.SpawnPolicy test in existing PFCreatureTests.cpp. Existing creature definitions remain unchanged. No external module/dependency, arbitrary class load, RPC, spawn command or save-schema change.

Proposed inactive defaults:

| Native tag | Day weights | Night weights | Maximum residents | Respawn seconds |
| --- | --- | --- | --- | --- |
| Ecology.Biome.Shore | Forager1 | Forager1 |2|30|
| Ecology.Biome.Woodland | Forager3 / Prowler1 | Forager1 / Prowler3 |3|30|
| Ecology.Biome.Ridge | None | Prowler1 |2|30|

These are configuration examples, not authored biome placement or tested gameplay balance. Combined seven slots leave one below the existing eight-creature global cap. Future resident accounting must include corpses; no population increase is authorized by this data type.

Validate checks the entire catalog:1..8 policies/entries, unique valid Ecology.Biome child tags, existing nonduplicate validated creature IDs,1..8 residents/table, combined budget≤8, finite5..300s duration and0..100 integer weights with at least one positive phase per entry. Invalid unrelated rows also refuse selection; no overflow or partial output commit.

Choose accepts exact World.Time.Day/Night and a caller-supplied zero-based ticket strictly below the eligible weight total. Same data/phase/ticket yields the same ID without mutable RNG, world state or actor creation. Invalid catalog/biome/phase/ticket and a dormant zero-total phase return an explicit reason and preserve OutId. Future integration must derive tickets/biome/time on the server; this pure API itself does not mutate gameplay or imply client authority.

## Exact verification

Initial Editor compile failed56.31s at PFCreatureTests.cpp:89: C2039/C3861, TNumericLimits<float>::QuietNaN unavailable. Production catalog compiled. Only the fixture switched to standard <limits>/std::numeric_limits<float>::quiet_NaN; corrected Editor passed8.32s, zero compiler warnings. Retained logs:
- Saved/Logs/PFM13SpawnPolicyEditorBuild.log (failure)
- Saved/Logs/PFM13SpawnPolicyEditorRetry.log (pass)
- Saved/Logs/PFM13SpawnPolicyGameBuild.log (Development Game pass54.91s, zero compiler warnings)

| Report directory under Saved/AutomationReports | Exact passed tests | Seconds | Sampled peak working/private GiB |
| --- | --- | --- | --- |
| Automation_M13SpawnPolicy_20261010_064823306_9746c8a4 | PF.Creatures.SpawnPolicy1/1 |30.24|2.994/2.873|
| Automation_M13SpawnPolicyRegression_20261010_064944765_d5fb6071 | PF.Creatures.Lifecycle, PF.Persistence.WorldRecords, PF.World.Clock3/3 |21.72|2.964/2.813|

Engine index.json and strict run-summary.json in each directory confirm all records clean Success, each selector matched, all process/runner exits0, no test warnings/errors or timeout. Matching raw logs Saved/Logs/PF<full report name>.log reviewed:0warnings/errors/assertions/fatals/ensures; tails confirm1 and3 performed tests/TestExit. PrimalFrontier.log remains the October9 14:53:04 Brisbane-time file, not the current launch evidence.

Tests exhaust all four Woodland day/night tickets and repeat results; check Shore/Ridge/no daytime fallback, budget, malformed/duplicate/unknown rows, nonfinite/out-of-range duration, invalid weights/phase/biome/tickets, whole-catalog refusal, explicit reasons and preserved output. Existing lifecycle includes damage/loot/corpse/global cap, save validation and server clock remain regression-tested.

This introduces the54th Editor-context core/runtime-command registration after the source-specific53-test consolidation; no combined54-test replay is claimed.

## Reproduce

No gameplay level required. One hidden NullRHI Editor process, -NoSaveConfig, existing disposable native fixtures only. Prerequisite: close Unreal Editor.

Start here: project PowerShell:

1. `./Scripts/RunNativeAutomation.ps1 -TestFilter PF.Creatures.SpawnPolicy -Label M13SpawnPolicy -TimeoutSeconds 120`; require exact1/1, exits0.
2. Only after pass, `./Scripts/RunNativeAutomation.ps1 -TestFilter 'PF.Creatures.Lifecycle+PF.Persistence.WorldRecords+PF.World.Clock' -Label M13SpawnPolicyRegression -TimeoutSeconds 120`; require exact3/3.
3. Inspect generated reports and unique raw logs for clean Success/selection, no timeout and no new warning/error/fatal/ensure. Failed gates stop affected scope.
4. Editor/Game builds must pass. This data-only type has no human action to test in the current map.

## Next integration gate and limits

Existing baseline is /Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld. Authored biome mapping, policy binary asset and runtime consumer are **TBD**. Do not ask the user to playtest absent ecology behavior. Next independent integration must preserve fixed-ID defaults, derive biome/time from authoritative state, validate navigation/clearance and per-table/global resident bounds, avoid spawn bursts/phase-fallback, preserve residents/cooldowns across save/load/restart, and prove one-client then two-client NullRHI behavior before activation. Changing phase must not delete existing residents or duplicate loot.

No live biome/time-of-day spawning, extra creatures, ecology navigation, predator/prey, adaptation, taming, new loot, multiplayer or persistence integration is accepted by this prerequisite. No screenshots/rendering run needed for inactive data; no FPS/stuttering/minimum16GB/human pacing claim. Host31.93GiB, installed engine5.8.3; packaged Server distribution limitation remains. Native memory peaks do not measure active AI performance. Full M11/M12 Personal gates remain open and full M13 stays future.
