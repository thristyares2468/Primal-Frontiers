# M11 — selected inventory item details

October9,2026. Bounded presentation step verified; full M11 usability and the failed Personal M8 replay remain open.

## Change and use

Inventory uses the original texture-free Pause/menu palette and a separate selected-item inset. Tab opens the bag; Up/Down or controller D-pad selects an existing stack. Details show the validated catalog name/category, quantity/stack limit, unit/batch weight, actual batch freshness and per-portion food/water recovery. Water remains drinkable even though its catalog category is Resource. Zero deadline means nonperishable; expiry at the current server time shows an explicit refusal before the server prunes the batch. Missing selection/catalog never substitutes another item or displays stale detail.

Nutrition is the catalog effect for one portion, not a promise of the eventual actual gain: the existing server still checks living ownership, freshness and capped survival attributes. Queries cannot consume/grant anything. Stable GUID input/RPCs, four-row scrolling, owner-only inventory replication, save format and10Hz refresh are unchanged. No assets/dependencies were added. UI remains keyboard/controller-driven and read-only; clickable item grids, remapping and recipe navigation are separate future work.

Changed source: Inventory/PFInventoryHUD.cpp/.h, UI/PFInventoryDetails.cpp/.h, Tests/PFInventoryDetailsTests.cpp and PFInventorySelectionLiveTests.cpp. Native UMG presentation remains replaceable; validated catalog data supplies the fields without loading icons.

## Verification

Editor initial26.72s and final6.09s PASSED; Development Game31.48s PASSED. No compiler warnings. Logs under Saved/Logs: PFM11InventoryDetailsEditorBuild.log, PFM11InventoryDetailsFinalEditorBuild.log, PFM11InventoryDetailsGameBuild.log.

| Check | Report directory under Saved/AutomationReports | Result | Seconds | Working/private GiB |
| --- | --- | --- | --- | --- |
| PF.UI.InventoryDetails + PF.Input.InventorySelection + PF.Inventory.Transactions | Automation_M11InventoryDetails_20261009_035056210_36edc1d2 |3/3 PASS|38.87|3.145/3.106|
| PF.UI.InventorySelectionLive initial720p, before brush-tint/water fixture follow-up | M11Inventory720_20261009_035248640_463120a1 |1/1 PASS|36.95|3.210/5.427|
| PF.UI.InventorySelectionLive final720p | M11Inventory720_20261009_035458173_4eba9f3d |1/1 PASS|36.13|3.163/5.414|
| PF.UI.InventorySelectionLive final1440p | M11Inventory1440_20261009_035559629_0dcf4cc4 |1/1 PASS|36.94|3.235/5.159|

Every test errors/warnings0 and engine/report/runner0. No build/test failed in this step. Native raw logs warning/error/ensure/fatal0. Live fixtures check actual widget title/body after expiry/reselection, full eight-slot bag, last-row selection, earlier-row removal and real catalog water; existing drop refusal/conservation assertions retained. Inputs use existing bound keyboard/controller delegates, not hardware. Final ten PNGs inspected: expired_selection, reselected_stack, full_bag_first, full_bag_last, water_details at each resolution. Text height and panel bounds passed at maximum HUDScale1.5 without hiding aim/vitals.

Rendered map: /Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld, one standalone editor-game process,1280x720 or2560x1440, uncapped/VSync0, no low-shadow diagnostic. Runners isolate profiles/preferences and prove149/150 unrelated save files plus actual default settings unchanged in final runs. Generated reports remain local/untracked.

Raw log names are the rendered report names above plus .log in Saved/Logs; native log is PFAutomation_M11InventoryDetails_20261009_035056210_36edc1d2.log. Reports contain index.json and run-summary.json. PNGs: Saved/AutomationReports/ControlsUI/<rendered-report>/*.png. Ordinary PrimalFrontier.log was checked but predates these launches; the unique logs are authoritative evidence.

Rendered raw baseline:21EditorDataStorageUI factory warnings,3uncooked HLOD import warnings,2blur/DOF console-priority warnings and14engine Python startup error lines per process. No new gameplay severity, ensure/fatal or crash. Installed engine identifies5.8.3 rather than requested5.8.2. Actual host31.93GiB; this is not16GB certification. Stationary uncapped UI checks do not measure traversal FPS or stutter. Readability observed in generated images does not complete physical controller or Personal human acceptance; no new multiplayer run is claimed for this read-only UI step.

## Replay

Start here: close Unreal processes, then from the project PowerShell run:

```powershell
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunNativeAutomation.ps1 -TestFilter 'PF.UI.InventoryDetails+PF.Input.InventorySelection+PF.Inventory.Transactions' -Label M11InventoryDetails
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunControlsAutomation.ps1 -TestCase Inventory -Resolution 720 -HUDScale 1.5
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunControlsAutomation.ps1 -TestCase Inventory -Resolution 1440 -HUDScale 1.5
```

Each runner owns fresh generated evidence. For ordinary play use PLAYTEST.md: open L_PrimalFrontier_OpenWorld, one player/Play Standalone net mode/Selected Viewport+F11. Gather normally with E, Tab then Up/Down. Expect truthful selected name, weight, freshness and per-portion effects; removing the selected stack clears details. Report a wrong/stale field, unreadable overlap or changed item total as FAIL. No mandatory additional Personal task is added; this fits the existing G2 UI check. Do not upload private saves/profiles.

Next independent step: M11 catalog-driven crafting selection/details. Additional actual resources/outputs remain M12 scope and need separate authority/network/save verification.
