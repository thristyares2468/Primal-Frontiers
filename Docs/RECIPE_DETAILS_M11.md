# M11 — themed recipe cards

October9,2026. Read-only native UMG cards for the three existing recipe shortcuts now share the original UI theme. Names, output quantities/items, duration and ingredient requirements come from validated catalogs. Have/need counts exclude batches at/past their actual server-time expiry even before server pruning; the view never prunes or spends inventory. Missing data is explicit. Ingredients-present text is advisory: existing server checks still validate life, exact batches, output capacity, completion and duplicate requests. Running job and last authoritative feedback stay separate. No new recipes/resources, controls/RPCs, save schema, dependencies or assets. General recipe browsing/selection is not implemented by these cards.

Changed source: Crafting/PFCraftingHUD.cpp/.h, UI/PFRecipeDetails.cpp/.h, Tests/PFRecipeDetailsTests.cpp, Tests/PFActionOverlayLiveTests.cpp. Existing building/storage overlay assertions remain regressions; that UI was not restyled by this step.

Editor16.67s/Game24.19s PASSED, no compiler warnings. Build logs: Saved/Logs/PFM11RecipeDetailsEditorBuild.log and PFM11RecipeDetailsGameBuild.log. No build/test failure in this step.

| Test | Report directory under Saved/AutomationReports | Result | Seconds | Working/private GiB |
| --- | --- | --- | --- | --- |
| PF.UI.RecipeDetails + PF.Crafting.Transactions + PF.Input.Gamepad | Automation_M11RecipeDetails_20261009_040903822_3c70ce19 |3/3 PASS|17.33|3.030/2.909|
| PF.UI.ActionOverlaysLive720p | M11Overlays720_20261009_041011256_0e490c77 |1/1 PASS|28.82|3.064/4.919|
| PF.UI.ActionOverlaysLive1440p | M11Overlays1440_20261009_041112037_4c0f6ee3 |1/1 PASS|28.67|3.261/5.413|

All test errors/warnings0; engine/report/runner0. Native real-batch model checks deadline equality before pruning, tuned cost/time/output, busy/unavailable data and read-only conservation. Existing crafting transaction tests retain client-role refusal, single completion, cancel, moved/expired inputs, capacity, unknown ID and death checks. Gamepad contextual routing passed.

Rendered fixtures use /Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld, one standalone editor-game process at1280x720 and2560x1440, HUDScale1.5, uncapped/VSync0, no low-shadow diagnostic. Bound keyboard/controller delegates exercise real missing-input refusal, start, busy and cancellation; widgets show real output/count/availability and feedback. Invalid placement/storage tests retained. All recipe text/panel bounds leave centre aim and left vitals clear. Ten PNGs inspected: craft_refused, craft_busy, craft_cancelled, build_refused, build_storage per resolution.151/152 unrelated saves and actual default settings unchanged.

Exact generated evidence: Saved/AutomationReports/<prefix>/index.json and run-summary.json; Saved/Logs/<rendered-prefix>.log (native PFAutomation_M11RecipeDetails_20261009_040903822_3c70ce19.log); screenshots Saved/AutomationReports/ControlsUI/<rendered-prefix>/*.png. Native raw warning/error/ensure/fatal0. Rendered raw baseline26warnings (21widget,3HLOD,2blur/DOF priorities) and14engine Python startup error lines;no new gameplay severity/ensure/fatal/crash. Ordinary PrimalFrontier.log remains older; unique launch logs are authoritative.

This is a stationary UI check, not traversal FPS/stutter, physical device or human usability acceptance. Actual host31.93GiB does not certify16GB. Installed engine identifies5.8.3. No new multiplayer acceptance is claimed for this read-only UI step; earlier network evidence remains separate. Personal M8 failed save/reload replay and full M11 acceptance remain open.

Start here for replay: close Unreal processes, open project PowerShell, run in order:

```powershell
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunNativeAutomation.ps1 -TestFilter 'PF.UI.RecipeDetails+PF.Crafting.Transactions+PF.Input.Gamepad' -Label M11RecipeDetails
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunControlsAutomation.ps1 -TestCase Overlays -Resolution 720 -HUDScale 1.5
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunControlsAutomation.ps1 -TestCase Overlays -Resolution 1440 -HUDScale 1.5
```

For ordinary play: open the named map, one player/Play Standalone net mode/Selected Viewport+F11; C opens craft,1 tool/2 cook/3 dry, R cancels. Expect catalog output, fresh have/need counts and explicit missing/running state, with actual server refusal/progress below. Cancellation must retain ingredients and produce no output. This fits existing Personal G2 step3; no separate human task added. Fail wrong/stale counts, text overlap or duplicated/spent cancellation items. Do not upload private saves/profiles.

Next independent UI step: apply the same theme to building/storage while retaining placement/ownership authority. General recipe browsing/selection remains future M11 scope; actual resource/recipe expansion remains M12.
