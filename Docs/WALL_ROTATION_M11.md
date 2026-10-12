# M11 — Forward wall placement and R rotation

Current navigation update: MENU_NAVIGATION_M11.md supersedes B-exits-placement instructions in this historical wall verification record. B now returns directly to the picker; Esc exits placement or backs out before opening Pause. Wall/R snapping and the recorded results below remain valid.

October12,2026. User reported walls only landing left/right and requested R.

## Behavior

Wall/Door/Window offset0 now selects the nearest cardinal platform edge ahead of the possessed pawn's eye yaw. R cycles clockwise offsets0/90/180/270. Keep looking the same way while selecting an edge; turning across a cardinal boundary changes its starting edge. Aim down at your visible owned platform to attach a frame, rather than at a previously built wall. Other pieces retain world-grid orientation; stairs still use their existing exterior-flight rules. B selects/exits, left click requests placement. R remains Cancel while crafting is open, is inert in other contexts, and never cancels a job while rotating a building. T is unbound; gamepad Down still rotates.

The server receives only ID/validated quarter turn and recomputes the transform through the shared Candidate routine. Reach/support/ownership/collision/cost checks, stable IDs, saved actual transforms and schema remain. No assets, maps, private saves or personal settings were edited. Added standard InputCore private dependency to the non-Shipping developer module for its real-key network test; no new plugin/service.

## Exact verification

Editor `PrimalFrontierEditor Win64 Development` with `-WaitMutex -NoHotReloadFromIDE -MaxParallelActions=4`: initial42.13s and final8.89s PASSED, no compiler warnings. Logs `Saved/Logs/PFWallRotationEditorBuild.log`, `PFWallRotationEditorInputCoreBuild.log`. Development Game `PrimalFrontier Win64 Development` same options PASSED42.68s/no warnings; `Saved/Logs/PFWallRotationGameBuild.log`.

Run from project root with Editor/game closed:

```powershell
./Scripts/RunNativeAutomation.ps1 -TestFilter 'PF.Building.PlacementAndStorage+PF.Input.Gamepad+PF.Persistence.WorldRecords+PF.Persistence.WorldRuntime' -Label WallRotation -TimeoutSeconds 180
./Scripts/RunControlsAutomation.ps1 -TestCase BuildingMenus -Resolution 720 -TimeoutSeconds 180
./Scripts/RunControlsAutomation.ps1 -TestCase BuildingMenus -Resolution 1440 -HUDScale 1.5 -TimeoutSeconds 180
./Scripts/RunEcologyAutomation.ps1 -TestCase Building -Players 1 -Port 17994 -TimeoutSeconds 180
./Scripts/RunEcologyAutomation.ps1 -TestCase Building -Players 2 -SimulateLagLoss -Port 17994 -TimeoutSeconds 180
./Scripts/RunControlsAutomation.ps1 -TestCase Overlays -Resolution 720 -HUDScale 1.5 -TimeoutSeconds 180
```

Native uses isolated test worlds/NullRHI; rendered uses `/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld`, one standalone `-game`, unique reports/profile/settings, `t.MaxFPS 0,r.VSync 0`. Network uses `/Game/PrimalFrontier/Maps/L_M5Building`, uncooked dedicated server plus one client first, then two NullRHI clients. Lag75ms/loss1% confirmed in all three roles. R delivery is programmatically through registered delegates; Slate buttons use real widget focus/activation, not physical keyboard/mouse/controller hardware acceptance.

| Final report prefix under Saved/AutomationReports | Exact tests/results | Seconds | Peak working/private GiB |
| --- | --- | --- | --- |
| Automation_WallRotation_20261012_040352427_3115b942 | PlacementAndStorage, Input.Gamepad, Persistence.WorldRecords, Persistence.WorldRuntime:4/4 |29.06|3.010/2.976|
| M11BuildingMenus720_20261012_040822182_810d15f0 | PF.UI.BuildingMenusLive:1/1 |36.75|3.076/4.919|
| M11BuildingMenus1440_20261012_040930497_94cee26e | PF.UI.BuildingMenusLive:1/1, HUDScale1.5 |36.77|3.382/5.609|
| M11BuildingBuilding1_20261012_041030735_c3b0d41a | PF.Building.Live:Server+Client1,2/2 |45.19|1.820/1.781 max per role|
| M11BuildingBuilding2_20261012_041151212_64c53108 | PF.Building.Live:Server+Client1+Client2,3/3 |53.85|1.816/1.781 max per role|
| M11Overlays720_20261012_041312864_0aa07281 | PF.UI.ActionOverlaysLive:1/1, HUDScale1.5 |30.56|3.086/4.892|

Total12 final records, all Success/zero test warnings/errors, exact selections, engine/runner/verdict exits0, no timeout/fatal/ensure/assertion. No missing/unexpected tests. Native frame fixture checks all3frame kinds ×4view headings ×4offsets =48candidates, matching support/rotation/wood conservation. Input checks one R binding, no T help, building/crafting/bag context and existing controller mappings. Persistence tests still cover validated records/repeated runtime restore without duplication; no new private-world migration.

Rendered fixture selects all eight pieces, places a real foundation then default-forward wall with +Y view via bound click, charges exactly2wood, cycles/wraps R without cost, and runs existing chest/filter/Sort/Quick Store/refusals and physical character stair traversal. Shared overlay replay starts a real tool craft and R-cancels with ingredients conserved, checks updated building HUD bounds at maximum scale. Network checks actual +Y authoritative wall location/normal/support, boundR/wrap, costs, storage privacy/filters/conservation and replicated window/stair shapes.

Raw logs: native0warning/error; rendered26warnings/14known experimental engine Python startup errors; network21/14 per role. Removing timestamp/frame prefixes and comparing rendered to `Saved/Logs/M11BuildingMenus720_20261011_231950196_18f1e80e.log` and network to `PFM11BuildingBuilding2_20261011_232324145_a7e2e11cClient1.log` found zero new severity categories. Known widget-factory, uncooked HLOD, blur/DOF priority and Python ToolsetRegistry/StateTree/PythonTestRunner initialization findings remain unresolved engine baseline, not a clean overall engine log. Default PrimalFrontier.log was read: prior session closed normally03:54UTC; these launches deliberately use unique abslog files instead.

Each prefix has run-summary.json and index.json (network role index.json appends Server/Client1/Client2 to prefix). Logs under Saved/Logs use `<prefix>.log` for rendered and `PF<prefix>[role].log` for native/network. Agent command reports for network are project-local `Saved/AutomationReports` as directed by PF.ExportTestReport. Private-save/default-settings snapshot guards passed (361–366 unrelated save files across final live runs). Credentials/save contents were not published.

All20finalPNG inspected: seven each BuildingMenus plus six Overlays in `Saved/AutomationReports/ControlsUI/<prefix>/`. Key files wall_ahead_preview.png/wall_ahead_placed.png show front-facing wall, valid preview, Built result and R controls at both resolutions. After placement the new preview may say incompatible support because it is aiming at the wall instead of the platform; this does not negate the distinct Built server result. Other captures show eight choices/chest draft/capacity refusal/window and stair scene plus busy/cancelled craft. Existing black sky/greybox lighting remains; no renderer/asset change.

Retained corrections: `PFWallRotationFinalEditorBuild.log` failed LNK2019 EKeys::R/LNK1120; `PFWallRotationFinalEditorReplayBuild.log` failed FKey::GetFName/LNK1120. Both exposed missing InputCore import; final standard dependency fixed it before network launch. Earlier rendered runs `M11BuildingMenus720_20261012_040441891_71bd32e8` and `M11BuildingMenus1440_20261012_040540638_9308be68` passed assertions but their immediate post-spawn placed-wall screenshot was black. Fixture now moves back and waits before capturing; final replays above provide useful artifacts. No gameplay validation was relaxed.

## Human retest and limits

Same `/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld`, one-player standalone or Selected Viewport/Play Standalone net mode (P pauses; Esc may stop PIE). Start here: use your existing foundation and2wood. B > Wall; stand just outside it, aim down at its exposed top. Keep looking the same way; R until offset0 should preview across the edge ahead. Cycle four R presses: right/back/left/ahead with no charge. At a valid ahead preview click once: one wall, exactly2wood. B exits. Report **Wall/R PASS** or failed offset/refusal. Full G2 window/floor/stair/user usability remains open; accepted chest, solo persistence, Pause and Settings do not need repeating.

Host reports31.93GiB RAM, not the earlier16GiB minimum. Single rendered runs peaked3.382working/5.609privateGiB; NullRHI roles ≤1.820working/1.781privateGiB. Uncapped tests do not measure sustained walking FPS/hitches, physical feel or minimum-spec acceptance. No two rendered clients or packaged server certification. Installed engine reports5.8.3 vs requested5.8.2. Full M11/M8/M13 remain gated independently. Next action is the focused personal wall retest; future independent M13 authored ecology requires fresh Trello/current-state audit.
