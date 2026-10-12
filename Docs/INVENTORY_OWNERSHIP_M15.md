# M15 foreign inventory ID and malformed RPC verification

October13,2026. Independent technical diagnostic built on published d510927 inventory race. No gameplay fault found; only gated development fixture/runner changed. M13 PersonalG9 remains open; no M14 expansion or fullM15 acceptance.

## Adversarial contract

Separate PF.Inventory.OwnershipLive selector, opt-in PFRunInventoryOwnershipLiveTests, existing /Game/PrimalFrontier/Maps/L_M6Creatures. Real dedicated server and one client first, then two NullRHI clients. Existing pickup race/split/drop conservation precedes additional tests. Fixture publishes the winner's live bag stack ID in a synthetic public PlayerName marker, then refreshes to the surviving4-food stack after dropping the original7. Losing client explicitly waits for a changed marker before attacking the live foreign ID. This deliberate disposable-test disclosure models an attacker already knowing a stack GUID; production peer inventory/name/credential code is unchanged, no new gameplay/test RPC.

Foreign client submits split/drop/consume through its own controller at separate0.5s intervals. Winner submits removed stale ID requests. A sixth phase tests unknown action255, Split0, DropMAX_int32 and Consume2 against a valid owned remaining stack (foreign client also attempts these). Server/client acknowledgements prove phase9..14 executed. Every stage checks all11food, unique live bag/world batch GUIDs, original expiry, exactly one winner, hunger50/no invalid recovery, four in bag/seven in one world drop. Peers' inventory arrays remain empty. Six synthetic water portions per client acknowledge ready plus phases; only food totals are the conservation target.

## Evidence

| Check | Exact report | Result |
| --- | --- | --- |
| Editor | Saved/Logs/PFM15InventoryOwnershipEditorBuild.log | PASS5.68s, no compiler warnings |
| Development Game | Saved/Logs/PFM15InventoryOwnershipGameBuild.log | PASS13.76s, no compiler warnings |
| One client | Saved/AutomationReports/M15InventoryOwnership1_20261012_224415758_1c306a44/run-summary.json | PASS2/2 PF.Inventory.OwnershipLive,54.45s; valid-owner malformed case only |
| Two clients | Saved/AutomationReports/M15InventoryOwnership2_20261012_224540384_afa0334a/run-summary.json | PASS3/3 PF.Inventory.OwnershipLive,63.88s;75ms outgoing lag/1% loss confirmed all roles |
| Shared race regression | Saved/AutomationReports/M15InventoryRace2_20261012_224702565_278b8d6f/run-summary.json | PASS3/3 PF.Inventory.RaceLive,57.76s; same75ms/1% |

Two-client Client2won the food; Client1 logs confirm refreshed live foreign stack requests. One-client cannot prove cross-player refusal. Exact selectors/contracts/exits0, testwarnings/errors0, no timeout/fatal/ensure/assertion. Eight executions of two selectors across repeated roles, not eight unique tests. Existing native3 inventory baseline belongs to prior checkpoint, not a new native run here.

Per-process live peak across these runs1.817working/1.786privateGiB. Private421/422/424existing files and default settings unchanged. Raw eachrole21warnings/14installed experimental Python startup errors; normalized against prior M6 baseline PFM13EcologyVariety2_20261012_210202656_c5f6868bServer.log, zero new category lines. Conventional PrimalFrontier.log remains prior closed Editor; matching unique abslogs Saved/Logs/PFM15InventoryOwnership*.log and PFM15InventoryRace2_20261012_224702565_278b8d6f<role>.log reviewed. No failure/retry in this scope.

## Replay and limits

BuiltEditor/no other Unreal/freeUDP17998/disposable runner profiles; no personal map/save edits. Start here:

```powershell
./Scripts/RunEcologyAutomation.ps1 -TestCase InventoryOwnership -Players 1 -Port 17998
./Scripts/RunEcologyAutomation.ps1 -TestCase InventoryOwnership -Players 2 -SimulateLagLoss -Port 17998
./Scripts/RunEcologyAutomation.ps1 -TestCase InventoryRace -Players 2 -SimulateLagLoss -Port 17998
```

Run sequentially, inspect exact per-role index.json and run-summary, confirmed network profile/phases9..14 and private/default hash guards. Stop on first failure, retain evidence and fix only current scope. No rendering/FPS/human/controller/16GB minimum/packagedServer/Internet auth/transport-flood resistance proof. Storage/crafting races, death/disconnect during work and longer soak remain separate M15 objectives. Deferred1440 and PersonalM13 unchanged.
