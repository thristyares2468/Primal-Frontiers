# M12 — Fuel-efficient refinery batches

Task: https://trello.com/c/jvU88JFY. October22 scoped implementation/build/native/network/rendered technical gates PASS. This is a bounded M12 addition, not completion of all combat/tools/progression. The owner requests STOP after its regression, two-client, rendered and Game/docs/Trello/publication steps. No new owner playtest is mandatory for this technical handoff.

## Implemented behavior

| Recipe | Ingredients per completed job | Output | Base / refinery time | Knowledge |
| --- | --- | --- | --- | --- |
| Recipe_CopperBatch — Batch refine copper (wood fuel) | 4 Copper ore + 1 Wood | 2 Copper ingots | 12 / 8 seconds | Available from start |
| Recipe_IronBatch — Batch refine iron (coal fuel) | 4 Iron ore + 1 Coal + 2 Flint | 2 Iron ingots | 15 / 10 seconds | Existing Iron metalworking |

Times above assume no player crafting-speed allocations and the normal 150% refinery. Coal/wood per ingot is half the existing portable recipe cost. Iron still requires the same flint per ingot. Existing 21 recipes keep their IDs, costs and portable availability; total recipes are now 23. No item/resource/knowledge-node IDs are added. Iron batch is an extra recipe on the existing Metalwork branch, so already learned Metalwork remains sufficient. New first-craft credit follows the existing once-per-recipe rules.

The new neutral-default `RequiredStation` recipe tag accepts only the registered Workbench or Refinery types. Server recipe resolution chooses a real usable provider before starting, then checks its authority, owner, health, type, range and visibility throughout the job. Public camp stations are public; another player's built refinery is unavailable. Ingredients, including fuel, stay in their exact reserved inventory batches until atomic successful completion. Cancel, leaving/occluding/demolishing the station, death/pawn change, changed inputs or insufficient output capacity cannot consume a partial batch or produce free ingots. Clients send the existing recipe ID/cancel request only.

The crafting panel shows two-output quantities, actual ingredient counts, a required-refinery line and the current station bonus. Away from a suitable station its Craft button is disabled; direct owning-client RPCs also receive the authoritative refusal. Existing ingot pictures are reused. There is no new external art, authored map/layout change, fuel inventory/tank, unattended furnace, power, charge, Frontier or save-format migration in this slice.

## Optional owner guide — no new mandatory input

Level: `/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld`.
Launch: one rendered **Standalone Game**, Net Mode **Standalone**, one player. Use the main menu to create a disposable world named `BatchCheck`; retain personal worlds. The automated native test below uses temporary worlds and requires no manually opened level.

Existing reachable fixture: **CAMP CRAFTING STATIONS** sign at `(350,-650,160)` cm; public refinery at `(1100,-500,0)`, workbench at `(700,-500,0)`. Copper vein is `(0,-850,60)`. Mining row sign **ERYNDOR MINING TEST** is `(-2400,-1450,180)`; coal/iron/flint nodes are at X=-2600, Y=-1200/-400/-800, Z=60. No new permanent actors or persistent IDs are required. These are integration fixtures, not final resource distribution.

Prerequisites: craft/carry the existing Stone gathering tool (3 Wood + 2 Stone, C menu) to mine copper/coal/iron/flint. For the copper check have at least 4 Copper ore and 1 Wood plus one free inventory slot. For iron have 4 Iron ore, 1 Coal and 2 Flint, and learn **Iron metalworking** in C > Technology (level 3; Field tool prerequisite; separate technology points). Gather/craft normally; no grant command is needed. Test iron separately if the bag is full. Crafting continues in real time; stay alive and leave sufficient room for output.

**Start here:** gather the copper ingredients, then stand over 5m away from both camp stations and any owned refinery.

1. Press **C**, select **Material**, scroll down and select **Batch refine copper (wood fuel)**. Expect the Copper ingot picture, **Makes 2**, Wood/Copper ore counts and **Requires refinery**. Craft is disabled here; Enter must leave the job idle and all quantities unchanged.
2. Close with **Esc**. Walk next to the public refinery, within 2.5m with a clear view. Open C and select the same batch. Expect **Camp refinery: 150%** and an 8-second duration without stat upgrades. If required-refinery feedback remains, step clear of the primitive and approach its open side.
3. Record ore, wood and ingot counts. Press **Enter**, wait about one second, then press **R** while the crafting menu is open. Expect **Cancelled**, unchanged inputs and no new ingots. R rotates pieces only in building mode; here it cancels crafting.
4. Press Enter again, remain nearby and wait for **Completed**. Expect exactly **-4 Copper ore, -1 Wood, +2 Copper ingots**. Waiting longer must not add another output. Existing ingots in the bag are counted as a delta, not assumed absent.
5. Optional interruption: with another batch's inputs available, start it, press **Esc** to close crafting, then walk over 5m away. Reopen C to inspect the result: expect cancellation and all those inputs retained. Reapproach and start a fresh job deliberately; moving back must not resume the cancelled job.
6. Optional iron: learn Metalwork, choose **Batch refine iron (coal fuel)** at the refinery. A completed job consumes exactly **4 Iron ore + 1 Coal + 2 Flint** and produces **2 Iron ingots**, normally 10 seconds. Without that knowledge it remains locked. Portable single-ingot recipes remain separate choices.
7. Optional owned station: **B**, foundation (2 Wood), then **B > 0** Refinery (12 Wood), **R** to rotate, left-click valid supported placement. Owner receives the same batch support/150%; other players cannot use that owned provider. H demolition works in building mode on the owner's aimed piece and cancels its running craft without consuming its ingredients. The public camp refinery stays public.

PASS means expected counts, station/knowledge reasons, cancel and completion match. FAIL means unexpected consumption/output, absent controls or a new gameplay error. Return `BATCH_REFINING PASS` or `BATCH_REFINING FAIL` with the failing step and counts. This is optional usability reproduction; automated assisted playtesting does not prove ordinary earning pace, physical controller feel or sustained frame rate.

## Automated reproduction

From `C:/UnrealProjects/PrimalFrontier`, close other Unreal instances first. Build `PrimalFrontierEditor Win64 Development` with the installed engine Build.bat. Run one Unreal test process group at a time:

```powershell
./Scripts/RunNativeAutomation.ps1 -TestFilter 'PF.Crafting.BatchRefining+PF.Crafting.Stations+PF.Crafting.OwnedStations+PF.Crafting.OwnedStationsRestore+PF.Crafting.Transactions' -Label M12BatchRegression -TimeoutSeconds 180
./Scripts/RunNativeAutomation.ps1 -TestFilter 'PF.Eryndor.Catalog+PF.Eryndor.Runtime+PF.Persistence.WorldRuntime+PF.Progression.Records+PF.UI.RecipeDetails' -Label M12BatchCatalog -TimeoutSeconds 180
./Scripts/RunPersistenceAutomation.ps1 -Players 1 -BatchRefining -Port 18081
./Scripts/RunPersistenceAutomation.ps1 -Players 2 -BatchRefining -SimulateLagLoss -Port 18081
./Scripts/RunControlsAutomation.ps1 -Resolution 720 -TestCase BatchRefining
```

Native `PF.Crafting.BatchRefining` uses the actual loaded item/crafting catalogs and verifies both recipes, invalid station tags, missing/wrong/non-authoritative stations, client direct refusal, fuel removal, capacity, range/occlusion/destruction, exact output and knowledge. `OwnedStationsRestore` now performs a real owned-refinery batch before disk Save, then two Loads: same structure owners/IDs/support, exactly two station helpers, preserved ore/fuel/output/new recipe reward ledger, and usable required refinery after each restoration.

`PF.Crafting.BatchLive` runs actual owner RPCs against an uncooked dedicated server on the stated level. Server fixture seeds exactly 8 Copper ore/2 Wood per player, then waits for real missing-station refusals before spawning each owned foundation/refinery. It bypasses construction costs (already tested separately). Clients receive the real helper, start/cancel/restart/send a duplicate, and finish with 4 ore/1 wood/2 ingots. The server verifies every player's station and cancellation; foreign inventories remain private. One client precedes two; latter uses confirmed 75ms outgoing lag/1% loss. Only these headless fixtures cap 120 FPS to avoid NullRHI saved-move overflow; there is no production frame cap or rendered-performance improvement claim. This station-only runner intentionally has no process-restart phase; actual disk restoration is verified by the native test.

Rendered `PF.Crafting.StationsUI` with the BatchRefining opt-in uses actual authored camp refineries and the real Slate C/Enter/R controls, seeded ingredients/700XP and assisted positions. It checks the missing-station disabled button, station/fuel/output details, cancel and exact completion. One 1280x720 window requests **t.MaxFPS 0 / r.VSync 0**; no sustained FPS/stutter or 1440p acceptance is inferred. Runners hash unrelated saves/profiles/default settings and isolate new test credentials/worlds. No private owner save is loaded or edited.

Evidence is local: `Saved/AutomationReports/<run>/index.json` and `run-summary.json`; logs `Saved/Logs/PF<run>.log` for native/network or `<run>.log` for rendered; PNGs `Saved/AutomationReports/ControlsUI/<rendered-run>/`. Build transcripts are `C:/UnrealSource/BuildLogs/M12Batch_*_20261022.log`. Do not upload private saves/credentials as evidence.

## Results — October22

- Editor builds: first 30.90s, live-fixture17.01s, final runtime/restore14.40s, catalog-contract5.34s, all PASS. No compiler warning/error found.
- Focused first native `Automation_M12BatchFirst_20261022_011044845_8fe48940`:1/1 PASS47.03s; raw/test severity0, engine/runner0, peak2.998working/2.919privateGiB.
- Final native `Automation_M12BatchRegression_20261022_011636613_f65804a1`:5/5 PASS18.20s, including actual batch repeated disk restore; raw/test severity0/exits0,2.987working/2.850privateGiB.
- Catalog/runtime native `Automation_M12BatchCatalog_20261022_011948025_27dd7997`:5/5 PASS17.56s, raw/test severity0/exits0,2.947working/2.808privateGiB. Original21 portable definitions/addition order and unchanged earned-tier contract pass.
- One client `M12BatchLive1_20261022_011427426_2be85048`:2/2 server/client PASS47.46s,650 protected files unchanged, per-role1.733–1.790working/1.639–1.782privateGiB.
- Two clients `M12BatchLive2_20261022_011729799_4a23d9e7`:3/3 PASS49.24s with lag/loss,651 protected files unchanged, per-role1.734–1.796working/1.628–1.791privateGiB. All five role exits0/test severity0, no timeout/fatal/ensure. Each raw log has33unique normalized severity messages, exactly equal the successful prior owned-station baseline (24warning/14experimental-startup-error lines per role). No new gameplay severity.
- First rendered `M11BatchRefining720_20261022_012038643_4b7828cb`:1/1 PASS40.65s,3.178working/5.424privateGiB,652protected files/default settings unchanged. Inspection caught a stale completion screenshot (before the next UI refresh), despite correct server counts. Retained the run and fixed only the fixture timing, adding refreshed fuel and Completed-result assertions. No production behavior or assertion relaxation.
- Artifact Editor6.84s PASS; rendered replay `M11BatchRefining720_20261022_012412679_eeb11b4a`:1/1 PASS39.50s,3.162working/5.153privateGiB,653protected files/default settings unchanged. Inspected all three PNGs: required refinery/disabled button, ready150% station and actual Completed with ore4/wood5/reward720XP are readable. Existing ingot pictures retained.
- Large-text replay `M11BatchRefining720_20261022_012522291_daa47656`:1/1 PASS39.47s at720p/HUD1.5,3.041working/4.962privateGiB,654protected files/default settings unchanged. All three PNGs inspected: key counts/reason/result/actions visible, long titles wrap; lower detail stats use the existing scroll area. Initial and both final renderer logs have35unique severity messages, exactly equal the prior owned-station renderer (26warning/14experimental-startup-error lines), no new gameplay severity/fatal/ensure. Actual `t.MaxFPS=0`/`r.VSync=0` confirmed in final logs.
- Development Game33.20s PASS, final artifact-fixture Game14.89s PASS; compiler severity0. All planned automated gates pass with no build/test failures; the initial stale screenshot is retained as the corrected artifact issue. No post-test production C++ changes. No sourceServer/cook/stage update for this slice.

Changed source: recipe catalog/header, Eryndor recipes/knowledge, station support, crafting authority and HUD, recipe-details view; new BatchRefining native/live fixtures, extended owned-station disk restore/catalog/UI fixtures; isolated native/network/rendered runner options and accompanying documentation. Binary assets, authored actor identities, private saves and default rendering settings are unchanged. Scoped Milestone12 commit/push and Trello completion receipt close this task; no next task starts automatically.

Known limits: installed engine5.8.3 CL58210709 differs from requested5.8.2 (Build.version refreshed). Current OS reports31.93GiB visible RAM,14.35GiB available after the rendered check; prior GPU observation is RTX4060 8GiB. These short tests certify neither16GiB nor sustained1440p performance. No sustained route/frame-time sampling or human stutter result is claimed. Default `Saved/Logs/PrimalFrontier.log` is the historical October14 GPU crash, so unique abslogs above are the current evidence. No cooked Server/client rebuild, cloud deployment, paid resource, live voice, full M12 completion or manual acceptance is claimed. Owner requires stop after this task's four steps.
