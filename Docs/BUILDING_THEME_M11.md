# M11 — building and storage theme

October9,2026. The existing native UMG building overlay now uses the original texture-free PFUITheme: rounded surface/inset, accent heading, readable selected-piece/storage details, a separate server-result line and smaller controls footer. It preserves the actual advisory preview, target health, four stored batches, omitted-row count and countdown. Storage still provides no preservation. Stable controls, ownership, collision, authority, transactions, save format and assets are unchanged. Refresh remains10Hz; finite HUDScale clamps0.75–1.5. Body18/title20/result16/control14 base fonts deliberately leave room for maximum-scale storage/refusal text.

Changed source: Building/PFBuildingHUD.cpp/.h and Tests/PFActionOverlayLiveTests.cpp. Existing all-text desired-height/screen bounds checks now also cover the building panel/footer. No new gameplay tests or implementation were added.

Editor15.85s/Game23.15s PASSED, zero compiler warnings. Saved/Logs/PFM11BuildingThemeEditorBuild.log and PFM11BuildingThemeGameBuild.log. No build/test failure in this slice. [AI Trello task](https://trello.com/c/EzlIjLtl) immediately updated and read back Done after all evidence reviewed.

| Test | Report under Saved/AutomationReports | Result | Seconds | Working/private GiB |
| --- | --- | --- | --- | --- |
| PF.Building.PlacementAndStorage + PF.Input.Gamepad | Automation_M11BuildingTheme_20261009_042204137_da6dbbbd |2/2 PASS|16.30|3.053/2.954|
| PF.UI.ActionOverlaysLive720p | M11Overlays720_20261009_042245136_489c133e |1/1 PASS|27.65|3.085/5.066|
| PF.UI.ActionOverlaysLive1440p | M11Overlays1440_20261009_042354573_efb3797c |1/1 PASS|27.59|3.245/5.398|

Native raw/test severity0, engine/runner0. The existing real-world fixture verifies cost, invalid ID/rotation, grid snapping/support, duplicate footprint, ownership, door state, storage deposit/withdraw, freshness/conservation/full-bag/replay refusal, demolition/damage/rebuild and client-role/reach refusal. Input.Gamepad covers contextual routing, not physical hardware.

Rendered fixture: /Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld, one standalone editor-game process at a time,1280x720 and2560x1440,HUDScale1.5,uncapped/VSync0,normal settings/no low-shadow diagnostic. Bound keyboard delegates exercise crafting refusal/start/busy/cancel and real invalid placement; transient authoritative storage fixture shows six distinct expiring batches as four rows plus two omitted. All text/panel bounds leave centre aim/left vitals/interaction prompt clear. Ten screenshots inspected: craft_refused/craft_busy/craft_cancelled/build_refused/build_storage per resolution. Actual default settings and153/154 unrelated saves unchanged.

Artifacts: Saved/AutomationReports/<prefix>/index.json and run-summary.json; Saved/Logs/<rendered-prefix>.log (native PFAutomation_M11BuildingTheme_20261009_042204137_da6dbbbd.log); PNGs in Saved/AutomationReports/ControlsUI/<rendered-prefix>/. All verdict/engine/runner exits0, test errors/warnings0. Rendered raw known26warnings (21EditorDataStorageUI,3uncooked HLOD,2blur/DOF priority) and14experimental engine Python startup error lines; no other/new gameplay severity/ensure/fatal/crash. Ordinary PrimalFrontier.log remains older; unique launch logs are authoritative.

No sustained FPS/stutter,16GB minimum spec, human usability/full shelter/storage, physical controller or multiplayer acceptance is inferred. Actual host31.93GiB; installed engine reports5.8.3. Personal M8 failed save/reload replay and full M11 acceptance remain open.

Start here for independent replay: close Unreal processes, open project PowerShell, run sequentially:

```powershell
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunNativeAutomation.ps1 -TestFilter 'PF.Building.PlacementAndStorage+PF.Input.Gamepad' -Label M11BuildingTheme
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunControlsAutomation.ps1 -TestCase Overlays -Resolution 720 -HUDScale 1.5
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunControlsAutomation.ps1 -TestCase Overlays -Resolution 1440 -HUDScale 1.5
```

Ordinary play: named map, one player/Play Standalone net mode/Selected Viewport+F11. B opens building; N chooses piece; T rotates; Click places; E opens an aimed-at owned storage/door; U deposits the existing selected item; O takes the first stored item. Expect unchanged cost/preview/authority results, four storage rows plus omitted count, real countdown and explicit no-preservation label. Fail text overlap, invalid server success, unauthorized action or changed food deadline/duplication. This fits existing Personal G2 building/storage steps; no extra human task. Never upload private save/profile contents.

Next independent M11 scope: general recipe browsing/selection, distinct from resource/recipe expansion planned M12. Current bounded technical evidence does not close full milestone acceptance.
