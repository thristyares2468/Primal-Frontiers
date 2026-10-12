# M11 — B picker return and contextual Escape

October12,2026. User requests B to return to build change menu and Esc to exit current menus before opening Pause. User clarified standalone Esc already does not stop simulation; no Editor shortcut interception/preferences changes were made.

## Current behavior

B opens the eight-piece picker. Choosing a piece enters first-person preview; B during preview returns directly to the picker. B/Esc on the picker exits building; Esc during preview exits it without pausing. R stays contextual rotation/crafting cancellation. Existing selection/rotation and all authoritative placement rules are retained.

Esc closes Inventory or Crafting without opening Pause. A running craft continues when its screen closes; only R/Cancel deliberately cancels. Esc from gameplay with no overlay opens visible Pause; Esc from Pause resumes. Settings/Controls help Back returns to Pause first. Chest settings Esc discards the unapplied draft and returns to chest contents; next Esc closes the chest and clears the existing server open-storage pointer. Modal repeat events are handled without another Back transition. P remains the secondary Pause key. Main-menu/world-browser existing Back behavior is unchanged.

This is local input/UMG navigation. No new RPC, client authority, gameplay transaction, assets/maps, save schema/private save migration, dependency or rendering-setting change. Existing server-validated close-storage RPC is reused. No changes to Editor PIE Esc shortcuts; use standalone for this behavior, P remains safe in PIE.

## Verification

Editor PrimalFrontierEditor Win64 Development: initial33.66s, layout13.71s, final fixture6.38s PASSED/no compiler warnings. Logs Saved/Logs/PFMenuNavigationEditorBuild.log, PFMenuNavigationFinalEditorBuild.log, PFMenuNavigationFinalFixtureEditorBuild.log. PrimalFrontier Win64 Development Game PASSED39.69s/no compiler warnings; PFMenuNavigationGameBuild.log. All use -WaitMutex -NoHotReloadFromIDE -MaxParallelActions=4; initial/new-source Editor and Game additionally -NoUBTMakefiles. No failed build/test in this task.

Run from project root with Unreal closed, sequentially:

```powershell
./Scripts/RunNativeAutomation.ps1 -TestFilter 'PF.Input.Gamepad+PF.Settings.Preferences' -Label MenuNavigation -TimeoutSeconds 180
./Scripts/RunControlsAutomation.ps1 -TestCase MenuNavigation -Resolution 720 -TimeoutSeconds 180
./Scripts/RunControlsAutomation.ps1 -TestCase MenuNavigation -Resolution 1440 -HUDScale 1.5 -TimeoutSeconds 180
./Scripts/RunControlsAutomation.ps1 -TestCase BuildingMenus -Resolution 720 -HUDScale 1.5 -TimeoutSeconds 180
./Scripts/RunControlsAutomation.ps1 -TestCase Overlays -Resolution 720 -HUDScale 1.5 -TimeoutSeconds 180
```

No level required for native fixture test worlds, NullRHI. Rendered tests use /Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld, one fresh standalone -game process, unique profile/report/settings destination, no personal world loaded. Requested t.MaxFPS0/r.VSync0, no simultaneous rendered instances. Gameplay keys execute registered bindings; modal keys travel through real Slate focus, including repeat events. This does not prove physical keyboard/gamepad delivery or human acceptance.

| Final prefix under Saved/AutomationReports | Exact tests | Seconds | Peak working/private GiB |
| --- | --- | --- | --- |
| Automation_MenuNavigation_20261012_043622984_4713be9a | PF.Input.Gamepad + PF.Settings.Preferences:2/2 |16.57|2.940/2.778|
| M11MenuNav720_20261012_044020206_ed521a44 | PF.UI.MenuNavigationLive:1/1 |30.78|3.039/4.884|
| M11MenuNav1440_20261012_044125206_885359aa | PF.UI.MenuNavigationLive:1/1, scale1.5 |30.91|3.280/5.143|
| M11BuildingMenus720_20261012_044328346_3ef53df5 | PF.UI.BuildingMenusLive:1/1, scale1.5 |37.97|3.074/5.155|
| M11Overlays720_20261012_044444778_cfee7b25 | PF.UI.ActionOverlaysLive:1/1, scale1.5 |30.93|3.068/4.883|

Six final successful records, exact selectors/exits0/test errors/warnings0, no missing/extra record, timeout/fatal/ensure/assertion. Native verifies actual bindings/help/contextual Escape/R and existing controller mappings/preferences. New menu fixture checks B return, selection/rotation preservation, held Esc, inventory/craft/preview closure, real solo Pause, nested Settings draft discard/Help return, resume/input balance and zero item transactions. Existing BuildingMenus test now selects all eight with one B between selections and adds real nested chest Escape/cancel/server-pointer checks, retaining actual forward-wall cost, chest conservation/privacy/refusals and physical character stair climb. Overlay replay additionally closes an actual active tool craft with Esc, verifies job/ingredients survive, then R-cancels and verifies conservation/new footer bounds. Six native/rendered records are distinct from the earlier wall task's12 records.

Each prefix has index.json/run-summary.json. Logs Saved/Logs/<prefix>.log for rendered, PFAutomation_MenuNavigation_20261012_043622984_4713be9a.log for native. All19finalPNG inspected under Saved/AutomationReports/ControlsUI/<prefix>: three each navigation, seven building, six overlays. build_picker_return/escape_pause/escape_child_back show the real picker and nested Back to paused parent; existing captures show new B-choose/Esc-close footer, chest draft/refusal and craft busy/cancelled feedback. At720p maximum text scale, picker rows use the existing scroll area; last choices remain selectable. Baseline greybox lighting/black sky persists, not changed here.

Native rawseverity0. All four rendered raw logs26warning14known experimental engine Python startup errors, matching timestamp/frame-normalized Saved/Logs/M11BuildingMenus720_20261012_040822182_810d15f0.log exactly:0newcategories/fatal/ensure/assertion. Widget factories, uncooked HLOD, blur/DOF priority and Python ToolsetRegistry/StateTree/PythonTestRunner startup issues remain; not an overall clean engine-log certificate. Default PrimalFrontier.log read: prior Editor session exited normally03:54UTC, unchanged; isolated tests deliberately use abslog paths above. Private368–371save files/default preference snapshot guards passed in final live runs. No private save bytes/credentials published.

First MenuNavigation720p M11MenuNav720_20261012_043740345_365ffa0d also passed before explicit max-scale fixture parsing; final source replays above supersede it. No failed result was hidden. No new multiplayer run: navigation is local; prior wall/network authority results in WALL_ROTATION_M11.md remain their separate scope. Multiplayer world-pause policy unchanged. No new packaged-server, PIE Esc, audible settings, physical device, sustainedFPS or16GB-minimum certificate. Host31.93GiB; final rendered peak3.280working/5.155privateGiB, one process at a time. Uncapped menus do not establish walking FPS/stuttering.

## Small user check

Same open-world level, one standalone -game window. Start here: B, select a piece, B again should immediately show the picker. Esc closes it; next Esc opens Pause; next resumes without ending the session. Report **B/Esc PASS** or first failed transition during your existing building session. No wood is required just to navigate. Settings/help/chest Back can be observed during that same session; already accepted chest/save/load/Pause/Settings need no repeat. Personal G2/full M11 and remaining window/stair/device acceptance stay open until actual observations. Trello bounded AI https://trello.com/c/Ieu24Nin records technical completion; future M13 work requires fresh current-state/Trello audit.
