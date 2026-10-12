# M15 competing pickup and stale inventory RPC verification

October13,2026. Independent technical hardening while M13 PersonalG9 remains open; no M14 expansion or full M15 acceptance. Only development automation and its isolated runner changed. No gameplay fault found, production mutation code unchanged.

## Contract and fixture

PF.Inventory.RaceLive runs on /Game/PrimalFrontier/Maps/L_M6Creatures, uncooked dedicated server plus real owning NullRHI clients. It pauses existing spawners, removes runtime creatures/pickups, places immobilized participants within reach, and creates one eleven-food batch with one original180s server deadline. These are disposable runtime actors, no map or personal save changes. A frozen replicated clock coordinates phases9..13. Each client's real ordered Consume-water RPC acknowledges readiness and actions; server waits for every participant. Five synthetic water portions per client are coordination inputs, not counted as conserved food. Server resets thirst10 before each acknowledgement to avoid consumption saturation.

Both clients attempt E/Interact twice against the same server-derived target. Exactly one receives all11food. Winner submits same-frame Split4 twice; cooldown permits one, leaving7+4. It submits Drop7 twice; exactly4remain in bag/7in one world drop. Removed-ID split/drop/consume at separate0.5s intervals refuse; loser submits absent IDs. Every stage checks all11food across every player/world pickup, unique live batch GUIDs, unchanged original deadline and hunger50. Owner sees4or0food; peer bags replicate no contents. Direct client pickup/bag mutation refuses. No server-only action substituted for tested client RPCs; setup and coordination alone are server fixture work.

## Results

| Check | Evidence | Result |
| --- | --- | --- |
| Editor build | Saved/Logs/PFM15InventoryRaceEditorBuild.log | PASS12.91s, explicit fixture compilation, no compiler warnings |
| Development Game build | Saved/Logs/PFM15InventoryRaceGameBuild.log | PASS19.02s, no compiler warnings |
| Native baseline | Saved/AutomationReports/Automation_M15RaceBaseline_20261012_223542516_d0459b71/run-summary.json | PASS3/3: PF.Inventory.Transactions, PF.Inventory.WorldTransfers, PF.Inventory.LargeStacks;17.05s; raw/test errors/warnings0 |
| One client | Saved/AutomationReports/M15InventoryRace1_20261012_223638203_d5590f90/run-summary.json | PASS2/2 server/client PF.Inventory.RaceLive;49.37s; no emulation |
| Two clients | Saved/AutomationReports/M15InventoryRace2_20261012_223746868_5fb5b500/run-summary.json | PASS3/3 server/client1/client2 PF.Inventory.RaceLive;58.55s;75ms outgoing lag/1% loss confirmed per role |

Native working/private peak2.966/2.825GiB. One-client max1.814/1.790GiB per process; two-client max1.819/1.785GiB. Private418/419existing save files and default settings unchanged. Exact selections/exit0/contract markers/client phases9..13/testseverity0/no timeout/fatal/ensure/assertion. Eight reported executions, four unique test names; repeated roles are not distinct tests. Two-client logs confirm Client1winner and Client2loser.

Raw network logs each21warnings/14errors match existing M6 baseline PFM13EcologyVariety2_20261012_210202656_c5f6868bServer.log after timestamp normalization: zero new category lines. Includes installed experimental Python API startup errors; not called raw-clean. Conventional Saved/Logs/PrimalFrontier.log remains October12 03:54UTC closed Editor log; these runs use unique abslog files. No new failure/retry in this bounded scope.

## Replay

Prerequisites: builtEditor, no other Unreal process, freeUDP17998, no personal save/profile. Start here:

```powershell
./Scripts/RunNativeAutomation.ps1 -TestFilter 'PF.Inventory.Transactions+PF.Inventory.WorldTransfers+PF.Inventory.LargeStacks' -Label M15RaceBaseline
./Scripts/RunEcologyAutomation.ps1 -TestCase InventoryRace -Players 1 -Port 17998
./Scripts/RunEcologyAutomation.ps1 -TestCase InventoryRace -Players 2 -SimulateLagLoss -Port 17998
```

Run sequentially. Require all exact indices and run-summary contracts/exits/private guards PASS; inspect matching Saved/Logs/PFM15InventoryRace<...><role>.log and native log. Failure stops current scope. No screenshots needed for this backend diagnostic; no rendering/FPS/16GB minimum/physicalcontroller/rendered multiplayer/packagedServer/Internet authentication or reliable-transport flood-resistance proof. Foreign-known live GUIDs, storage/crafting races and death/disconnect soak are separate future checks. M13 human encounter and1440 remain unverified; no repeated accepted personal tests requested.
