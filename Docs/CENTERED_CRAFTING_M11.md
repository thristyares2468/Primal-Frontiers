# Centered crafting menu — M11

October9,2026. Bounded technical verification, not full human M11 acceptance. Trello: https://trello.com/c/VGeuYmtb.

C opens a centered list/detail menu. Validated catalog recipes have original procedural greybox pictures; selected details show output, duration, fresh ingredient counts, unit weight, stack limit, nutrition and shelf life. Existing soft catalog texture icons are supported asynchronously; no new textures/art/assets/dependencies were added. Current placeholder catalog uses the procedural pictures. Real custom texture loading is not certified by these fixtures.

Select with a row button or Up/Down/D-pad; Enter/A/X crafts the selected recipe through the existing server RPC. Craft selected/Cancel job/Close are actual UButtons. Tab/Space supports focused buttons. Existing1/2/3 quick recipes remain; removed/invalid shortcuts never fall back to another selection. R/D-left cancels; C/Y/B closes. P/Menu closes crafting and transfers to Pause. Movement/look/cursor/focus are captured only when the real local menu is visible, and balanced on Close/Pause/Resume. UI-only input prevents gameplay actions while browsing. The world keeps running during crafting; solo Pause is separate. Headless fixtures do not lock controls for invisible widgets.

Server recipes, batch transactions, output validation, ownership and save format are unchanged. No new recipe/output/resource, gameplay authority or multiplayer certification is implied.

## Evidence

Editor final implementation build: Saved/Logs/PFM11CenteredCraftActivationEditorBuild.log,6.58s,exit0,no compiler warnings. Game: PFM11CenteredCraftGameBuild.log,23.62s,exit0,no compiler warnings. Earlier failed build PFM11CenteredCraftEditorBuild.log retained: unsupported focus/child-size API, member shadow and TObjectPtr loop deduction, corrected only in this UI scope. Retry/final earlier builds14.81/14.41s passed.

Native final report Automation_M11CenteredCraftFinal_20261009_050801380_a7568746: PF.UI.RecipeDetails, PF.Crafting.Transactions, PF.Input.Gamepad, PF.Input.InventorySelection4/4 passed,16.17s,working/private2.935/2.770GiB,raw/test severity0. Earlier pre-activation native report Automation_M11CenteredCraft_20261009_045525928_8d0135c9 also passed4/4.

| Rendered report under Saved/AutomationReports | Test | Result | Seconds | Working/private GiB | Unrelated saves unchanged |
| --- | --- | --- | --- | --- | --- |
| M11Overlays720_20261009_050418353_87a188b9 | PF.UI.ActionOverlaysLive |1/1 pass|30.30|3.064/4.900|161|
| M11Overlays1440_20261009_050500980_715d75be | PF.UI.ActionOverlaysLive |1/1 pass|30.33|3.264/5.375|162|
| M11Controls720_20261009_050619564_34eb646e | PF.UI.ControlsLive |1/1 pass|33.87|3.056/5.094|163|

Overlay runs use maxHUDScale1.5, uncapped/VSync0, normal shadow settings. Exact logs: Saved/Logs/<report>.log. Actual default settings unchanged. All final test warnings/errors0,engine/report/runner0. Twelve final overlay PNGs inspected in Saved/AutomationReports/ControlsUI/<report>: craft_refused,craft_busy,craft_cancelled,craft_food_selected,build_refused,build_storage. Centered bounds, text fit, selected picture ID, nutrition/freshness, selection without crafting, actual refusal/start/busy/cancel/conservation, close input restoration and craft→Pause→Resume counter balance pass.

Retained initial rendered M11Overlays720_20261009_045608820_9f3bf012 failed3 assertions: synthesized background-window pointer activation did not select/Craft/Close. Screenshot also exposed incorrect draw-element tint, corrected; Cancel label wrapping corrected. Like the existing world-menu fixture, final tests use actual Slate focus+Space button activation, never OnClicked broadcasts or direct callbacks. This does not certify real mouse usability; G2 keeps that check open. Keyboard navigation and button callbacks have independent assertions.

Final raw overlay logs retain26 known startup warnings (21EditorDataStorageUI,3uncooked HLOD including multiline path,2renderer CVar priority) and14experimental engine Python error lines. No new gameplay error/ensure/fatal/crash. Native raw log clean. Installedengine5.8.3 differs from requested5.8.2. Actual machine31.93GiB RAM; stationary UI tests cannot certify sustained FPS, stuttering,16GB minimum, hardware gamepad, traversal, rendered multiplayer or full human acceptance. Personal M8 failed save/reopen replay remains open.

## Replay

Level /Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld. One standalone player. Open C, browse without crafting, inspect pictures/details, try missing ingredients, gather3wood+2stone and start tool; duplicate start must refuse, R cancellation must retain ingredients. Close must return controls; P must open Pause then resume cleanly. Test real mouse row/Craft/Close in the same existing G2 session. No new separate Personal task.

AI runners with Editor closed:

```powershell
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunControlsAutomation.ps1 -TestCase Overlays -Resolution 720 -HUDScale 1.5
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunControlsAutomation.ps1 -TestCase Overlays -Resolution 1440 -HUDScale 1.5
```

Changed source: Crafting/PFCraftingHUD.cpp/.h; UI/PFItemPicture.cpp/.h; Survival/PFSurvivalPlayerController.cpp/.h and PFGamepadInput.cpp; Tests/PFActionOverlayLiveTests.cpp. Blueprint presentation remains replaceable; C++ supplies authoritative state/actions.
