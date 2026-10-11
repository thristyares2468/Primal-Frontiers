# M11 chest controls and eight-piece building picker

October12,2026. Bounded repair for user-reported inaccessible storage and Foundation-only building UX, plus requested Sort, Quick Store and chest allowed-item settings. [AI task](https://trello.com/c/dUJskDzO). Technical gate passed; full M11/M8 human acceptance remains open.

## Play it

Level: `/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld`. Start here: open that level, Play > one player > Play Standalone net mode > Selected Viewport, click the game view and F11. P opens Pause; Esc may stop PIE. No map saving required. G2 in TRELLO_TEST_GUIDES.md is the complete same-session guide; use steps4–5 for this repair.

1. Gather wood normally with E. Each current piece costs2wood. B opens all eight choices: Foundation, Wall, Floor, Roof/ceiling, Door and frame, Chest, Window wall, Stairs. Click a choice or press1–8; preview returns to first person. T rotates90degrees; left click requests placement. N optionally cycles during preview. B exits placement; B again opens the picker.
2. Foundation requires flat clear ground. Wall/Window/Door/Chest/Stairs require an owned visible platform. Floor/Roof require an owned Wall/Door/Window and sit one320cm storey above its platform. Stairs extend outside the platform, eight40cm risers; rotate toward a clear edge. The resulting landing also must be within7m. Move nearer if Out of reach appears; don't remove validation. A solid upper slab has no interior stair opening.
3. Exit placement with B, aim at your chest within7m and E opens CHEST. Store1/Store stack and Take1/Take stack transfer stable batches; source+destination totals remain constant. Closing with E/Tab/Close releases cursor/movement/look. World simulation continues while browsing. P closes the chest and opens Pause.
4. Sort chest orders display name, then item ID, expiry and GUID. It neither merges batches nor resets food expiry. Normal expiry pruning may remove already expired food.
5. Chest settings opens a local draft. Select Wood ([x]); Apply filter. Quick Store attempts every allowed whole bag batch, leaving disallowed/full batches in the bag; it reports moved items and skipped batches. A skipped large batch can be deposited in smaller amounts manually with Store1. Existing disallowed contents may always be withdrawn. Cancel settings leaves the applied filter unchanged. All items allowed restores unrestricted mode; selected-items mode with zero selections blocks all new deposits. Filters are per chest and saved with its world record.

Fail on wrong counts/deadlines, unauthorized opening/deposit, cancelled draft applying, unreadable controls or stuck input. Report the failed action and expected/actual quantity. Physical mouse/gamepad feel, sustained FPS and rendered save/restart are still human gates; programmatic Slate button activation is not that acceptance.

## Implementation and compatibility

New focused `Building/PFBuildingMenus.cpp/.h` contains the local picker, dual-column chest window, batch action buttons and filter draft. Controller/modal integration and BuildingHUD suppression share existing input/authority paths. No grants or predicted inventory mutations in menus. Server checks living owner, current aimed/reached/open chest, valid unique known IDs (max64), rate limit and inventory capacity. Contents and applied filters replicate owner-only. Closing or loss of reach/death clears the menu.

`Inventory/PFInventoryComponent::SortStacks` preserves GUID, quantity and deadline. Quick Store uses the existing atomic TransferTo for each whole batch. Filters affect deposits only and never delete existing contents. Apply/Cancel are distinct. Scroll lists refresh on changed identities/counts/settings, freshness text10Hz; user HUDScale is clamped0.75–1.5. Rebuild rows release stale focus to the menu so navigation remains usable.

Append Build_Window/Build_Stairs and enum values, preserve old six IDs/values. Windows use four frame cubes; stairs eight solid cubes. Platform support/ownership/save validation accepts windows as vertical support and stairs on platforms. Optional `storageRestricted`/`allowedStorageItems` metadata strictly defaults both absent fields to unrestricted for old V1/V2 saves; partial, unknown, duplicate, non-chest and oversized filters refuse. Decode failure preserves the previous output. Current restore stages/validates before committing, including old disallowed chest contents; no private save migration occurred.

Supported Editor Python `Scripts/UpgradeBuildingCatalogUI.py` confirmed eight native-backed catalog defaults with changed=False; no .uasset/.umap was altered. No new art/material/dependency/plugin/MCP. Existing maps/template assets unchanged. Actual installed engine5.8.3, requested5.8.2 mismatch retained. Installed packaged Server target remains unsupported; uncooked dedicated-server evidence is separate.

## Verification

Final Editor `Saved/Logs/PFBuildingMenusFinalNativeEditorBuild.log` PASSED6.77s and Development Game `Saved/Logs/PFBuildingMenusFinalGameBuild.log` PASSED42.90s, no compiler warnings. Earlier implementation39.98s, network11.38s and layout replay6.79s succeeded. Shared PF.UI.ActionOverlaysLive720p/HUDScale1.5 PASSED1/1 M11Overlays720_20261011_232645319_489ecf0d,30.28s,3.087/5.298GiB. All six regression PNG inspected (craft_food_selected plus original five);355private saves/default preferences unchanged, exact selectors/exits/testseverity0/no timeout/fatalensure, known26/14raw baseline/no new severity. Its storage screenshot deliberately exercises the historical advisory overlay fixture, not the new actual chest popup (covered above). Total final gates14 successful records: six native, three rendered, five network roles. No aggregate full-project suite inferred.

| Gate | Report name under Saved/AutomationReports | Result | Seconds | Peak working/private GiB |
| --- | --- | --- | --- | --- |
| Native exact six below | Automation_BuildingMenusFinal_20261011_232507513_a1279b81 |6/6 PASS|16.55|2.971/2.834|
| Actual720p menu/buttons/character | M11BuildingMenus720_20261011_231950196_18f1e80e |1/1 PASS|34.74|3.120/5.057|
| Actual1440p menu/buttons/character, HUDScale1.5 | M11BuildingMenus1440_20261011_232037013_83a61ea7 |1/1 PASS|34.75|3.309/5.263|
| Dedicated server + one NullRHI client | M11BuildingBuilding1_20261011_232149210_bd13ef16 |2/2 roles PASS|44.56|1.814/1.787 max|
| Dedicated server + two NullRHI clients,75ms/1%loss | M11BuildingBuilding2_20261011_232324145_a7e2e11c |3/3 roles PASS|52.95|1.813/1.775 max|

Native names: PF.Building.PlacementAndStorage, PF.Input.Gamepad, PF.Inventory.Transactions, PF.Persistence.WorldRecords, PF.Persistence.WorldRuntime, PF.Progression.WorldCompatibility. Actual transient-world placement/cost/ownership/refusal, multi-batch alphabetical Sort conservation, filters/Quick Store, repeated disk restore, legacy/partial/malformed metadata, new pieces and support graph covered. Native raw/test severity0; exact records/exits0/no missing/unexpected selectors/timeout.

Rendered selector PF.UI.BuildingMenusLive, fresh standalone actual open-world map. Bound B/E/click inputs and focused Slate Space buttons exercise all eight selections, no selection cost, actual foundation/chest placement, real open menu/input freeze/world continuing, deposit/withdraw, Apply/Cancel, filtered Quick Store, Sort, capacity refusal/no duplication, freshness, unauthorized opening and physical character walking up the exterior stairs to the upper platform. Ten final PNGs inspected at both resolutions in `Saved/AutomationReports/ControlsUI/<rendered-run>/`: eight_piece_picker, chest_open, chest_filter_draft, chest_capacity_refusal, window_stairs_scene. The last scene shows the window/platform/chest; stair traversal is established by the actual character assertions/log, not that camera angle. Raw rendered26warnings/14known experimental engine Python errors, zero new severity categories against M11Overlays720_20261009_042245136_489c133e. No gameplay error/assert/fatal/ensure. Profiles and350/351 unrelated private saves/default preferences unchanged.

Network selector PF.Building.Live on `/Game/PrimalFrontier/Maps/L_M5Building`; actual1-/2-client ownership/RPC/cost/deposit/open-menu notification/filter/invalid-request/Quick Store/Sort/freshness checks. Other client receives no contents or filter metadata; Window/Stairs support and4/8 collision cubes reconstruct from replication. New shape actors are trusted server fixtures after the player places the original four pieces; they aren't proof of client-requested Window/Stair placement (native/rendered tests cover that). Each role has one exact success record/exits0/testseverity0/no timeout/crash/fatal/ensure; lag/loss confirmed in all three two-client roles. Raw21warnings/14known experimentalPythonerrors in every role, zero new severity categories against PFM13EcologyEcology2_20261011_215733436_1b371547Client1.log.352/353 unrelated saves/default preferences unchanged. Role reports use `<network-run>Server|Client1|Client2/index.json`, matching `Saved/Logs/PF<network-run><role>.log`. PF.ExportTestReport additionally emits M5LiveServer/Client results within each isolated process; automation index/summary is the verdict authority.

All rendered launches request t.MaxFPS0/r.VSync0; no sustained frame-time sample or stutter/FPS acceptance inferred. Host31.93GiB physical RAM; peak per rendered process5.298GiB private memory, not a16GB minimum-spec certificate. Ordinary `Saved/Logs/PrimalFrontier.log` remains the earlier08:53 local exit; unique launch logs above are the current evidence. Test artifacts remain generated/local/ignored.

## Retained failures and replays

- Automation_BuildingMenusCore_20261011_230954502_e0262044: building test failed two errors, stair support trace hit the newly placed west window; other three native tests passed. Fixture relocation only.
- Automation_BuildingMenusReplay_20261011_231455910_257844e3: two errors, first relocation placed resulting exterior stairs outside7m. Relocate south of base, preserve rules. Failed-only replay Automation_BuildingMenusReplay2_20261011_231543077_dc08d1a0 PASSED1/1; final6/6 above supersedes it.
- M11BuildingMenus720_20261011_231614055_37d02ee7: one error at exterior stair placement Out of reach after prior menu assertions. Reposition fixture near landing. Replay M11BuildingMenus720_20261011_231814763_e5df2dd9 PASSED1/1 incl actual stair climb; final screenshot review led to wrapping repair/replay above.
- PFBuildingMenusFinalEditorBuild.log FAILED2.80s, C3861/C2039: button alignment setter belongs to UButtonSlot. Use supported slot setter after SetContent; replay6.79s passed. No test launched from failed build. No compiler warning or unresolved gameplay failure retained.

## Replay (AI)

Close existing Unreal processes, then run from project PowerShell, one command at a time. Disposable profiles/report paths are generated; private-save/default-preference guards and exact-selector exit gates apply. Do not use personal slots.

```powershell
./Scripts/RunNativeAutomation.ps1 -TestFilter 'PF.Building.PlacementAndStorage+PF.Persistence.WorldRecords+PF.Persistence.WorldRuntime+PF.Input.Gamepad+PF.Inventory.Transactions+PF.Progression.WorldCompatibility' -Label BuildingMenusFinal
./Scripts/RunControlsAutomation.ps1 -TestCase BuildingMenus -Resolution 720
./Scripts/RunControlsAutomation.ps1 -TestCase BuildingMenus -Resolution 1440 -HUDScale 1.5
./Scripts/RunEcologyAutomation.ps1 -TestCase Building -Players 1 -Port 17994 -TimeoutSeconds 180
./Scripts/RunEcologyAutomation.ps1 -TestCase Building -Players 2 -SimulateLagLoss -Port 17994 -TimeoutSeconds 180
```

The existing ecology network runner now routes Building as an additional bounded case, using the existing M5 map/flag and no world-save startup slot. No changes to unrelated ecology/lifecycle test selection.
