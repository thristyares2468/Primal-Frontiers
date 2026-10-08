# Current native regression checkpoint — M11

October9,2026. Source checkpoint `df648eb`. The supported35-test native selection has now passed at one source checkpoint in two sequential NullRHI Editor processes. This closes the previously documented combined-coverage gap; it does not accept full M11, M7 traversal/overnight, M8 rendered persistence or later milestones. No C++/asset/settings change was needed.

Editor target check passed1.14s, up to date with zero compile actions/warnings. Log: `Saved/Logs/M11ConsolidatedEditorCheck_20261009.log`. The actual Editor/Game compilations of this source remain24.88s/31.52s from [M11_REVIEW.md](M11_REVIEW.md). Installed engine reports5.8.3, not a verified5.8.2 executable.

## Reports and outcomes

| Report directory under Saved/AutomationReports | Result | Seconds | Sampled working/private GiB |
| --- | --- | --- | --- |
| Automation_M11Consolidated_20261008_214710018_c1987e48 |32/32 passed|20.23|2.920/2.761|
| Automation_M11ConsolidatedCommands_20261008_214805753_240b69b1 |3/3 passed|16.11|3.064/2.947|

Each directory contains engine `index.json` and runner `run-summary.json`. Matching logs are `Saved/Logs/PF<full report directory name>.log`. Both engine and strict runner exit0; no timeout, test/raw warning/error, ensure or fatal. Log tails confirm32 and3 performed tests and TestExit queue empty. The default `Saved/Logs/PrimalFrontier.log` remains the older October7 local-time log; it is not evidence for these launches.

Reconciled all reported identifiers against the30-test manifest in M8_POST_FIX_VERIFICATION.md plus DeadPlayerRespawn, OfflineFoodAging, ActiveCraftCancellation, InventorySelection and ShadowBudget: **35 expected,35 records,35 unique,0 missing/extra**. Every record is clean Success; there is no absent test hidden behind process exit0.

Exact passed tests:

```text
PF.Building.PlacementAndStorage
PF.Crafting.Gathering
PF.Crafting.Transactions
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
PF.Settings.Preferences
PF.Settings.ShadowBudget
PF.Survival.Component
PF.Survival.Environment
PF.Survival.Lifecycle
PF.Survival.Needs
PF.World.Clock
PF.World.OpenWorldAsset
PF.World.Resources
```

## Repeat and limits

```powershell
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunNativeAutomation.ps1 -TestFilter 'PF.Building+PF.Crafting+PF.Creatures+PF.Input+PF.Interaction+PF.Inventory+PF.Persistence+PF.Settings+PF.Survival+PF.World' -Label M11Consolidated -TimeoutSeconds 180
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunNativeAutomation.ps1 -TestFilter 'PF.PrimalAgentTools.CommandArguments+PF.PrimalAgentTools.MissingSystemsAreBlocked+PF.PrimalAgentTools.TeleportAndRuntimeReset' -Label M11ConsolidatedCommands -TimeoutSeconds 120
```

No user Editor process was running; each owned process finished before the next began. Opt-in live server/client, streaming, screenshot and editor scenario-reset tests are excluded; their prerequisites and evidence remain separate. The supported native tests use isolated fixtures, including unique persistence files, and do not edit authored maps or unrelated assets. Generated artifacts remain local and outside Git.

NullRHI provides no rendered FPS/stutter, UI usability, VRAM or physical-controller evidence. Host has31.93GiB RAM; these sampled process peaks do not certify the16GiB minimum. No new screenshot or two-client rendered result is claimed. The next mandatory Personal M11 usability gate remains [iH8I2Qfc](https://trello.com/c/iH8I2Qfc); no M12 implementation is started. Bounded checkpoint task: https://trello.com/c/ZKdNDaOC.
