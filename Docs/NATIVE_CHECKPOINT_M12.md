# Current native regression checkpoint — M11/M12

Later M13 policy prerequisite adds a54th registration, verified1/1 plus three affected regressions in ECOLOGY_SPAWN_POLICY_M13.md. The53-test source-specific result below remains historical evidence for bd9206c; it does not claim a combined54-test run.

October 10, 2026. Source checkpoint `bd9206cd53512b9745eff2b97ec0a1fae43365bf`; no C++ or asset changes in this audit. Trello https://trello.com/c/Jv4jvkRm.

All **53 Editor-context core/runtime-command tests passed at this source checkpoint**, in two sequential NullRHI Editor processes. Compared the registrations in Source/PrimalFrontier/Tests and Plugins/PrimalAgentTools/Source/PrimalAgentToolsRuntime/Private/Tests against actual index.json identifiers: 53 expected, 53 records, 53 unique, zero missing/extra/duplicate. This supersedes the retained 35-test checkpoint for these directories. It is not the entire editor-only plugin suite or ClientContext/ServerContext live suite.

## Build and exact results

PrimalFrontierEditor Development Win64 target check passed **2.05 s**, up to date with zero compile actions or warnings. Log: Saved/Logs/PFM12ConsolidatedEditorBuild.log. Current actual Editor/Game compilations remain 40.20/51.03 s from SETTINGS_THEME_M11.md. No rebuild of unchanged Game source was needed.

| Report directory under Saved/AutomationReports | Passed | Seconds | Sampled peak working/private GiB |
| --- | --- | --- | --- |
| Automation_M12Consolidated_20261010_063602347_d34c49b0 | 50/50 | 62.43 | 2.994/2.869 |
| Automation_M12ConsolidatedCommands_20261010_063732865_4dc3fe11 | 3/3 | 20.93 | 2.985/2.835 |

Both directories contain engine index.json and strict run-summary.json. Matching raw logs: Saved/Logs/PF<full report directory name>.log. Each requested selector matched; engine/process/strict runner exits0, clean Success/test warnings/errors0, no timeout, raw warning/error0 and no assertion/fatal/ensure. Log tails independently confirm 50 and 3 performed tests and TestExit queue empty. The default Saved/Logs/PrimalFrontier.log remains the October9 14:53:04 Brisbane-time file, not evidence for these launches.

Exact passed identifiers:

```text
PF.Building.PlacementAndStorage
PF.Crafting.Gathering
PF.Crafting.Protection
PF.Crafting.ToolProgression
PF.Crafting.Transactions
PF.Crafting.WeaponProgression
PF.Creatures.Lifecycle
PF.Input.Gamepad
PF.Input.InventorySelection
PF.Interaction.TargetAndPickup
PF.Inventory.Transactions
PF.Inventory.WorldTransfers
PF.Persistence.ActiveCraftCancellation
PF.Persistence.CorpseLootRestore
PF.Persistence.CorruptPlayerData
PF.Persistence.DeadPlayerRespawn
PF.Persistence.FileGenerations
PF.Persistence.OfflineFoodAging
PF.Persistence.PlayerRoundTrip
PF.Persistence.PlayerValidation
PF.Persistence.RejectedLoadPreservesWorld
PF.Persistence.ServerPlayerAdapter
PF.Persistence.StartupFailurePreservesSave
PF.Persistence.StructureCollisionRestore
PF.Persistence.WorldRecords
PF.Persistence.WorldRuntime
PF.PrimalAgentTools.CommandArguments
PF.PrimalAgentTools.MissingSystemsAreBlocked
PF.PrimalAgentTools.TeleportAndRuntimeReset
PF.Progression.Codec
PF.Progression.Component
PF.Progression.EarnedUpgrade
PF.Progression.EarnedWeapon
PF.Progression.GatherClock
PF.Progression.GatherEvents
PF.Progression.GatherLifecycle
PF.Progression.GatherWindows
PF.Progression.KnowledgeRequests
PF.Progression.RecipeAccess
PF.Progression.Records
PF.Progression.WorldCompatibility
PF.Settings.Preferences
PF.Settings.ShadowBudget
PF.Survival.Component
PF.Survival.Environment
PF.Survival.Lifecycle
PF.Survival.Needs
PF.UI.InventoryDetails
PF.UI.ProgressionDetails
PF.UI.RecipeDetails
PF.World.Clock
PF.World.OpenWorldAsset
PF.World.Resources
```

## Replay guide

No gameplay level needs to be opened. The tests create bounded disposable native worlds and perform supported asset checks. Prerequisite: close Unreal Editor; use one hidden UnrealEditor-Cmd process at a time.

Start here: from project PowerShell run:

1. `./Scripts/RunNativeAutomation.ps1 -TestFilter 'PF.Building+PF.Crafting+PF.Creatures+PF.Input+PF.Interaction+PF.Inventory+PF.Persistence+PF.Progression+PF.Settings+PF.Survival+PF.UI+PF.World' -Label M12Consolidated -TimeoutSeconds 300`.
2. Only after pass, run `./Scripts/RunNativeAutomation.ps1 -TestFilter 'PF.PrimalAgentTools.CommandArguments+PF.PrimalAgentTools.MissingSystemsAreBlocked+PF.PrimalAgentTools.TeleportAndRuntimeReset' -Label M12ConsolidatedCommands -TimeoutSeconds 120`.
3. Require clean 50+3 records, expected-selector coverage, all exits0, no timeout and matching source-manifest identifiers. Stop on any mismatch or failed record; diagnose only the affected test.
4. Read each generated index.json/run-summary.json and unique raw log. Native fixtures are not substitute rendered or human playtests.

## Status correction and limits

ROADMAP_STATUS.md now distinguishes current implementation/technical evidence/human acceptance and retains original October8–9 notes as historical. CURRENT_STATE.md has a readable current overview before dated checkpoints. Fresh Trello confirms M7 user-reported Done; M8 rendered replay, M11 human usability and M12 overall pacing/combat remain open. No Personal deadline, assignment, priority or acceptance changed.

No new gameplay, RPC, save schema, asset, dependency, private save or personal setting edits. Native temporary-file fixtures use their existing isolated APIs; this runner does not add the rendered runner's personal-file hash certificate. NullRHI establishes no FPS, graphics, stuttering, physical controller, audio or minimum16GB acceptance. Installed engine is 5.8.3, not independently tested 5.8.2. Host physical RAM is31.93GiB; sampled process peaks do not certify machine-wide headroom.

No failed test or compiler warning in this bounded audit. Next independent action is a fresh Trello/source audit of the remaining M12 requirements and M13 ecology prerequisites; do not invent an untested full milestone pass or start asset integration. Current technical progression has two useful earned unlocks; human pacing remains a distinct gate.
