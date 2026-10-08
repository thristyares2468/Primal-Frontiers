# M11 review and full inventory readability

October 9, 2026. Requested review of the prior M11 implementation completed. M11 has passing bounded automated checkpoints; it is **not fully accepted**. The next acceptance gate is a hands-on first-person UI check on `L_PrimalFrontier_OpenWorld`.

## Review findings

- Controls/help reads actual registered input bindings; it does not implement remapping or certify physical controllers. Pause semantics keep multiplayer running. Settings use a local draft and display confirmation; audibility and controller feel still require observation.
- Inventory actions continue to resolve stable GUIDs and validate quantities/ownership/liveness on the server. Expiry never silently selects a replacement stack. This review found a real presentation gap: fixed inventory dimensions, unbounded rows and ignored HUD scaling.
- Survival feedback is read-only and possession scoped, clears missing/dead pawn state, and reports health/needs/exposure independently of control hints. Retained normal1440p final shadow-budget run summary (`M11Survival1440_20261008_100640924_df254fd7`) still has engine/strict0; the earlier diagnostic-only limitation was later corrected by commit9c31467. Trello now records that later fix.
- Retained craft/build720p/1440p final summaries have engine/strict0. Their live test covers actual craft refusal/start/busy/cancel, invalid placement and bounded storage presentation. It does not verify completed crafted output or valid placement in the rendered scenario. Native transaction tests cover backend behavior separately. ACTION_OVERLAYS_M11.md now makes this distinction and retains the initial build/layout failures.
- All these UI live tests are isolated standalone runs. Historical network/private-inventory tests support authority contracts; no fresh two-client rendered UI acceptance was performed. No protected/private reconnect credentials are exposed or attached to Trello.

## Bounded change

The bag uses the shared right-side panel with scaled heading/body/result text. Four numbered rows follow the controller's resolved selection; range text shows omitted rows and existing Up/Down or D-pad navigation reaches every stack. Feedback remains in a separate footer, including explicit absence of selection after expiry/removal. Closed/paused panels collapse immediately, while open text refreshes at10Hz. The interaction prompt reflows into the left viewing area alongside the bag. Default eight-slot capacity, item quantities/freshness, input meanings, server APIs, save formats and assets are unchanged.

Changed source: Inventory/PFInventoryHUD.h/.cpp, Survival/PFSurvivalHUD.cpp, Tests/PFInventorySelectionLiveTests.cpp. The existing opt-in rendered test now checks actual eight-batch state, navigation to the final batch, text allocation/scaling and stable selection after removing an earlier row, in addition to real expiry/reselection.

## Exact evidence

Build logs under `Saved/Logs`:

| Build | Log | Result |
| --- | --- | --- |
| Reproduction Editor | M11InventoryReviewReproBuild_20261009.log | Passed28.51s |
| Final Editor | M11InventoryReviewFixBuild_20261009.log | Passed24.88s |
| Final Development Game | M11InventoryReviewGameBuild_20261009.log | Passed31.52s |

All three compiled without compiler warnings. Installed engine is5.8.3 under UE_5.8; this is not a verified5.8.2 binary. Packaged Server remains blocked by the installed engine distribution, as already documented.

Reports under `Saved/AutomationReports`:

| Run | Tests/result | Seconds | Sampled working/private GiB |
| --- | --- | --- | --- |
| M11Inventory720_20261008_212014391_96140ad8 | Reproduction failed1/1;7 assertions |52.06|3.216/4.242|
| M11Inventory720_20261008_212440618_1abe00d7 | PF.UI.InventorySelectionLive passed1/1 |34.22|3.146/4.151|
| M11Inventory1440_20261008_212537649_4ab3e957 | PF.UI.InventorySelectionLive passed1/1 |33.27|3.216/4.435|
| Automation_M11InventoryReview_20261008_212627961_6ba72ccf | PF.Input.Gamepad, PF.Input.InventorySelection, PF.Inventory.Transactions passed3/3 |21.23|2.987/2.859|

Failed run: two text-height and three font22-versus33 assertions demonstrate the gap. Two additional test-only row-number expectations were corrected to identify the actual selected batch by its displayed freshness rather than assume a specific numbering format. Failed evidence is retained. Final runs have process/strict verdict0 and zero test warnings/errors. Native raw log has zero warning/error/ensure lines.

Both final rendered runs use maximum HUD scale1.5 and normal shadow settings. Logs `Saved/Logs/<rendered run>.log` retain24 known widget/HLOD warnings and14 engine StateTree/Python errors, with no VSM overflow, ensure, fatal or crash. Eight final screenshots (expiry, reselection, full-bag first/final selection at each resolution) were inspected under `Saved/AutomationReports/ControlsUI/<rendered run>/`. These are real1280x720 and2560x1440 captures.

Repeat one rendered process at a time:

```powershell
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunControlsAutomation.ps1 -TestCase Inventory -Resolution 720 -HUDScale 1.5
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunControlsAutomation.ps1 -TestCase Inventory -Resolution 1440 -HUDScale 1.5
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunNativeAutomation.ps1 -TestFilter 'PF.Input.InventorySelection+PF.Inventory.Transactions+PF.Input.Gamepad' -Label M11InventoryReview
```

`t.MaxFPS0` and `r.VSync0` are session-only. No sustained FPS/stutter claim follows from short stationary UI tests. Host has31.93GiB RAM; process peaks do not certify a16GiB machine. Current greybox catalog and default eight-slot bag are tested; arbitrary localized/very long names, narrow aspect ratios and all64 possible configured slots are not certified. No art/asset acquisition is needed for this slice.

## Mandatory next acceptance gate

On `L_PrimalFrontier_OpenWorld`, use Selected Viewport + F11, move/gather an item, inspect bag selection, crafting/building feedback, then Pause → Settings/Controls. The player must confirm readability and usability while moving and report FPS. M7 sustained route/overnight and M8 full rendered persistence remain separate unverified gates. Physical controller hot-plug/feel and audio audibility remain unverified. Do not mark full M11 or later milestones passed from these automated results. Trello bounded review: https://trello.com/c/h1bRvrVr; Personal gate: https://trello.com/c/iH8I2Qfc.
