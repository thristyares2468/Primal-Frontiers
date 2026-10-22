# M12 — Field survey scope

Task: https://trello.com/c/YlK19xC8. October23 bounded optical consumer. Editor/Game, native, one-/two-client restart and rendered technical checks PASS. Full M12 remains incomplete. No new owner playtest is mandatory for this bounded technical handoff.

## Behavior

**Field survey scope** (`Item_SurveyScope`): 1.25 weight, one per stack, reusable **2x optical magnification**. Recipe `Recipe_SurveyScope`, **Assemble field survey scope**, consumes **1 Quartz lens + 2 Copper ingots + 1 Cord + 2 Wood**, produces one scope. Requires a usable Workbench and existing **Gem cutting** knowledge. Base12 seconds, normal workbench9.6 seconds before player crafting-speed upgrades. Inputs remain reserved until successful atomic completion; cancelling or losing station access preserves them.

Pin its exact batch to the existing session hotbar. Close inventory, select that slot and press **Q**, or **gamepad D-pad down**, to raise/lower it. It narrows the real first-person camera using `2*atan(tan(baseFOV/2)/2)`, so supported base FOV70–110 retains exact2x magnification. It never saves the narrowed value to preferences. Opening a menu, changing hotbar slot, removing the bound batch, dying or changing pawn clears optical state. The hotbar and zoom are session shortcuts and are not restored from world saves. A different scope of the same item type never silently replaces the missing bound batch. Repin deliberately.

The scope has an original code-drawn placeholder picture in crafting, inventory and hotbar. There is no imported model/texture, scope mesh, target scanner or protection/harvesting/damage/reach bonus. The neutral-default item metadata affects only explicitly optical items. All previous36 item IDs,23 recipe IDs,15 resource definitions and six knowledge branches retain order/meaning. The appended scope brings totals to37 items/24 recipes. Existing learned Gem cutting also unlocks it; no point reset or new save format is required.

## Optional owner check — no new mandatory input

**Level:** `/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld`.
**Launch:** one rendered **Standalone Game**, Net Mode **Standalone**, one player. Use a disposable new world named `ScopeCheck` to keep personal progress separate. No second rendered client is needed.

**Reachable fixture:** existing **CAMP CRAFTING STATIONS** sign `(350,-650,160)` cm beside spawn; public workbench `(700,-500,0)` and public refinery `(1100,-500,0)`. No new authored actors/layout or persistent identities are added. These simple station primitives provide visible targets for comparing the normal and scoped view. Any owned workbench also works for its owner.

**Prerequisites:** carry one Quartz lens, two Copper ingots, one Cord, two Wood, with one free inventory slot. Learn **Field tools → Iron metalworking → Gem cutting** in **C > Technology**, using separate technology points at the existing level requirements. Gem cutting requires level3 and Metalwork. If already learned, buy nothing again. In C > Material, **Polish quartz lens** uses3Quartz+1Flint. Use existing Cord and copper-refining recipes to prepare remaining inputs; their ingredient lists show actual bag counts. No developer grants are required for ordinary play. Mining quartz needs the existing appropriate pick; this is not a new starter item.

**Start here:** walk to the CAMP CRAFTING STATIONS workbench, within2.5m and with clear line of sight. Stand on open ground beside it, not inside the cube.

1. Press **C**, select **Tool**, scroll to **Assemble field survey scope**. Expect a telescope picture, the2.0x optical description, four ingredient counts, **Requires workbench**, and **Camp workbench:125%**. Without upgrades the time is9.6 seconds. If locked, read the named Gem cutting prerequisite rather than spending player stat points.
2. Record the four input counts. Press **Enter**, wait about a second, then **R** while crafting is open. Expect **Cancelled**, no scope, and all inputs retained. R here cancels crafting; it rotates only in build mode.
3. Press Enter again, stay nearby and wait for **Completed**. Expect exactly **+1 scope, -1 lens, -2 copper ingots, -1 cord, -2 wood**. Waiting must not produce a second scope. If carrying older inputs/items, compare deltas rather than expecting zero.
4. Press **Esc** to close crafting. Press **Tab**, click the Field survey scope stack, then press **1** to pin it to hotbar1. Expect a pinned-batch message and the telescope picture in that slot. Close inventory with Tab/Esc. Press1 once to select that hotbar slot.
5. Look toward the adjacent refinery or another stationary landmark. Press **Q**. Expect a visibly enlarged view and **SURVEY2.0x** hotbar hint. Move the mouse: looking remains first-person. E does not gain longer reach and the scope count stays unchanged. PressQ again: normal view returns.
6. Raise it again, then press **P**. Expect the pause menu and cleared zoom. Resume: normal view stays normal until you deliberately pressQ. Repeat with Tab or C: opening either panel clears scope view. Selecting another hotbar slot also clears it.
7. Optional exact-batch check in this disposable world: while scoped, open inventory and drop that scope using the existing selected-item Drop control. Close inventory and pressQ on the old shortcut. Expect a missing/empty-batch explanation and no zoom. Pick the dropped scope up withE, then select and pin it again; recovery of an item is not automatic shortcut rebinding.
8. Optional existing-world save/reload: use **P > Save World**, return to the menu and load that world. Expect the scope and material counts preserved, and a normal camera. Repin the scope after load because hotbar shortcuts are per session. Do not repeat a previously accepted save test unless checking this new item specifically.

**PASS** means expected costs, picture, zoom and resets match. **FAIL** means wrong costs/output, zoom persisting through a menu, absent controls, an unexpected batch substitution or a new gameplay error. Optional report: `SURVEY_SCOPE PASS` or `SURVEY_SCOPE FAIL`, failing step and observed counts/view. Physical controller feel, ordinary earned pacing and sustained frame rate are separate from programmatically injected button routes.

## Automated reproduction

From `C:/UnrealProjects/PrimalFrontier`, close other Unreal processes first. Build `PrimalFrontierEditor Win64 Development` with the installed engine. Run one Unreal test process group at a time:

```powershell
./Scripts/RunNativeAutomation.ps1 -TestFilter 'PF.Optics.Lifecycle+PF.Optics.Persistence' -Label M12ScopeNative -TimeoutSeconds 180
./Scripts/RunNativeAutomation.ps1 -TestFilter 'PF.Input.Hotbar+PF.Eryndor.Catalog+PF.UI.InventoryDetails+PF.UI.RecipeDetails+PF.Progression.Records' -Label M12ScopeRegression -TimeoutSeconds 180
./Scripts/RunPersistenceAutomation.ps1 -Players 1 -SurveyScope -Port 18081
./Scripts/RunPersistenceAutomation.ps1 -Players 2 -SurveyScope -SimulateLagLoss -Port 18081
./Scripts/RunControlsAutomation.ps1 -Resolution 720 -TestCase SurveyScope
```

Native fixtures require no manually opened level. They create temporary game worlds and unique owned save slots; use actual catalog definitions, start/cancel/capacity/knowledge/authority refusals, real bound-key delegates/camera, exact stackGUID and repeated real disk restoration. Restores check scope/material quantities, original batch identity, knowledge/XP/first-craft credit and neutral local zoom.

Network fixture uses the exact OpenWorld package above, one uncooked dedicated server and then1/2 NullRHI clients. A test-only250XP/three-knowledge seed and exact ingredient seed avoid retesting the already verified entire mining progression. Server spawns a public workbench per isolated player; clients use real owning craft/cancel/restart RPCs and real replicated inventory to pin/zoom. Tests check once-only270XP/recipe credit, private foreign bags/knowledge, refused direct client grant/craft/save, and server never acquiring local zoom. Then every process exits, a new server loads the same isolated slot, and new clients reconnect using their isolated profiles. No fresh seed is applied during Restart; the saved item/knowledge/ledger must return, local shortcuts must not. Two clients use confirmed75ms outgoing lag/1% loss. These headless movement fixtures are capped120FPS to avoid the known NullRHI saved-move queue overflow; rendered tests remain uncapped.

Rendered fixture uses the saved reachable camp workbench, actual timed crafting/Slate Enter, real inventory1 pin and boundQ/D-pad routes, actual camera and PlayerCameraManager FOV, pause reset, inspected crafting/inventory/normal/zoom screenshots. Materials/knowledge and positioning are assisted, creatures/drains/time frozen only in that disposable run. It requests1280x720, `t.MaxFPS0/r.VSync0`, no personal settings writes. No human input/physical device, sustained FPS,1440p or natural pacing claim is inferred. Each runner hashes unrelated saves/settings/profiles and records sampled process memory. No private owner world is loaded.

Reports: `Saved/AutomationReports/<run>/index.json` and `run-summary.json`; native/network logs `Saved/Logs/PF<run>.log`, rendered `Saved/Logs/<run>.log`; PNGs `Saved/AutomationReports/ControlsUI/<rendered-run>/`. Build logs `C:/UnrealSource/BuildLogs/M12Scope_*_20261023.log`. UTC artifact names can contain20261022 while the local Brisbane result date isOctober23. Do not publish private saves/credentials.

## Verification results

First native `Automation_M12ScopeFirst_20261022_215249175_52cb8b82`:2/2PASS47.19s, raw/test severity0, engine/runner exits0, peak2.977working/2.875privateGiB. First network Create both roles pass, but the runner stops on an unrelated baseline chest-role condition; retained `M12ScopeLive1_20261022_215839968_0f6a9bc8` and corrected only fixture selection. Restart was not attempted in that failed run. Its default `PrivateFilesUnchanged:false` was not evaluated because execution stopped earlier; do not interpret it as an observed file difference.

- Editor baseline27.84s, runtime36.91s, live17.98s and rendered-fixture14.39s allPASS, no compiler warning/error observed. No production C++ changes after the first native pass.
- Final related native `Automation_M12ScopeRegression_20261022_220600424_15ca562e`:5/5PASS20.67s, exact Hotbar/Eryndor.Catalog/InventoryDetails/RecipeDetails/Progression.Records selection, raw/test severity0/exits0/no timeout,2.977working/2.848privateGiB.
- One client replay `M12ScopeLive1_20261022_220119434_297c563c`:4/4 server/client Create+Restart reportsPASS,48.09/36.59s,659 protected files unchanged. Per-role1.732–1.795working/1.632–1.779privateGiB. Actual new-server restoration, independent client identity/material/knowledge/once-only credit and optical reset pass.
- Two clients `M12ScopeLive2_20261022_220304478_741f5fea`:6/6 Create+Restart reportsPASS,50.70/38.60s,663 protected files unchanged; confirmed75ms outgoing lag/1%loss. Per-role1.725–1.802working/1.625–1.798privateGiB. Client2 does not crash. All ten final network reports/exits/test warnings/errors0, no timeout/fatal/ensure; each raw log24warnings/14experimental Python startup-error lines,33unique normalized severity messages exactly equal `PFM12OwnedStationLive2_20261021_235132056_59eb9f4fCreateServer.log`. No new gameplay severity.
- Rendered `M11SurveyScope720_20261022_220455148_9e01c8e3`:1/1PASS44.77s, engine/strict exits0/test severity0,3.160working/4.538privateGiB,668 unrelated saves/default settings unchanged. Actual `t.MaxFPS=0/r.VSync=0` confirmed. All four PNGs inspected: readable crafting2x/required bench/costs, inventory picture/pin instructions, identical landmark visibly enlarged, normal and SURVEY2x hotbar states. Actual CameraComponent and PlayerCameraManager match2x derivation; pause restores normal view immediately and D-pad route toggles without consuming item/settings. Raw35unique severities exactly equal previous successful owned-station renderer (26warning/14startup-error lines), no new gameplay severity.
- Development Game41.22s PASS, no compiler warnings/errors. No sourceServer/cook/stage update is claimed. OS free memory16.87GiB after short tests; no sustained frame-time sampling or human stutter result. Scoped milestone/Sol commit and remote-equality receipt follow on Trello.

Known limits: actual installed engine5.8.3CL58210709 versus requested5.8.2. OS31.93GiB visible RAM; earlier GPU observation RTX4060/8GiB. Short isolated tests do not certify16GiB/1440p/sustained stutter. Default PrimalFrontier.log remains an oldOctober14 GPU crash; current tests use unique logs. No asset import, authored map/binary/settings/private-world modification, sourceServer cook/package, external deployment, paid service or fullM12 acceptance is part of this task.
