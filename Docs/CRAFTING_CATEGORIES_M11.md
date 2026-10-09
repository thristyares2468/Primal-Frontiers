# Crafting category navigation — M11/M12

October 9, 2026. Bounded local UI work, not full M11/M12 or human acceptance. Task: https://trello.com/c/Fw7nOnMS.

The centered crafting menu now builds All, Tool, Food and Material tabs from the validated recipes' Gameplay Tags. All shows the five current recipes; Tool shows the two tools, Food the two food conversions, and Material fibre cord. Pictures, fresh ingredient counts, costs, time, nutrition and tool statistics remain catalog queries.

Click a category or use PgUp/PgDn (controller LB/RB). Changing category clears selection and displays **CHOOSE A RECIPE**. Choose a row with Up/Down or the D-pad, then Enter/A/X or Craft selected. Browsing cannot send a crafting request, spend ingredients, cancel/restart a queue or grant output. Unknown category requests are refused. The existing 1/2/3 shortcuts act only when that baseline recipe is visible; they never fall back to another recipe. R/D-left still deliberately cancels the active job. Close and Pause retain the established modal behavior.

Pause > Controls & help includes the same modal navigation text as the actual crafting footer. Category state is local, absent from saves and RPC parameters. No gameplay authority, recipes, inventory, save schema, assets or dependencies changed. C++ supplies native replaceable UMG presentation.

## Verification

Editor implementation builds passed: PFM11CraftCategoriesEditorBuild.log (16.30s), PFM11CraftCategoriesCaptureEditorBuild.log (6.92s), PFM11CategoryTimeoutDiagnosticEditorBuild.log (7.00s), PFM11CategoriesFinalEditorBuild.log (7.15s), under Saved/Logs. Shared-help final Editor PFM11CategoriesHelpEditorBuild.log passed (16.25s). No compiler warnings. Game PFM11CategoriesGameBuild.log passed (23.20s); final PFM11CategoriesHelpGameBuild.log passed (22.68s), clean.

Native PF.UI.RecipeDetails and PF.Crafting.ToolProgression passed 2/2: Automation_M11CraftCategories_20261009_092000401_9e83c7da, 16.34s, sampled working/private 2.996/2.866GiB, raw and test severity zero. These regressions alone do not certify category input.

| Report under Saved/AutomationReports | Test | Result | Seconds | Working/private GiB | Existing save files unchanged |
| --- | --- | --- | ---: | --- | ---: |
| M12Tool720_20261009_093140779_122a2685 | PF.Crafting.ToolProgressionLive | 1/1 passed | 74.28 | 3.089/5.321 | 184 |
| M12Tool1440_20261009_093324788_ecd7bb92 | PF.Crafting.ToolProgressionLive | 1/1 passed | 74.25 | 3.200/5.124 | 185 |
| M11Overlays720_20261009_093523562_fda1e2a7 | PF.UI.ActionOverlaysLive | 1/1 passed | 30.40 | 3.074/5.270 | 186 |
| M11Controls720_20261009_094045969_730e4a5d | PF.UI.ControlsLive | 1/1 passed | 33.63 | 3.102/4.910 | 187 |

All final executions above use maximum HUDScale 1.5, uncapped FPS/VSync0, and one rendered Standalone process. Engine/report/runner exits and test errors/warnings are zero; actual default settings are unchanged. Eight final category PNGs, six overlay PNGs and three Pause/help PNGs were inspected under Saved/AutomationReports/ControlsUI/<run>/. Bounds/text fit, real Slate category-button focus+Space, keyboard/controller shoulder navigation, cleared selection/no request, unchanged queue deadline, exact cord/tool crafting costs, regrowth/finite yield, shared help text, and existing Close/Pause/Resume input balance passed. No direct OnClicked broadcast or fabricated job result. Help screenshots show the scrollable binding list; the new section's contents are separately asserted, not claimed visible in its initial scroll position.

Matching raw logs are Saved/Logs/<run>.log. They retain 26 known startup warnings (21 EditorDataStorageUI, three uncooked HLOD imports, two renderer CVar priority warnings) and 14 engine experimental Python error lines. No other errors, ensure, fatal or crash found. The task does not repair those unrelated engine/content findings.

Retained first run **M12Tool720_20261009_092038684_f8c40e1a failed** at the runner's 180.79s timeout, engine -1 and no finished report. The log reached completed bound tool, node depletion and regrowth; four PNGs exist. Settings/182 existing save files were unchanged, working/private 3.076/4.886GiB. Added opt-in stage/last-observed diagnostics and reduced the test deadline to 140s to leave report-flush time before the runner cap. Diagnostic replay M12Tool720_20261009_092851469_3ac0a06a passed 1/1 in 74.48s, 3.101/4.961GiB, 183 saves/settings unchanged. The first timeout did not reproduce in that replay or the two final runs. **Its original cause remains unproven; instrumentation is not a claimed gameplay fix.**

No network rerun was needed for local filtering/help: the unchanged authoritative tool path's separate one-/two-client restart evidence remains in TOOL_TIERS_M12.md. This task does not extend that evidence to human rendered multiplayer. Actual host has 31.93GiB RAM and installed engine 5.8.3, differing from the requested 16GB minimum/5.8.2. Stationary uncapped scripted exercises do not establish sustained walking FPS, stutter, hardware gamepad or real mouse usability. Personal M8 failed replay and M11 usability remain open.

## Replay

Level: /Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld. One Standalone player; use existing Guide G2 in TRELLO_TEST_GUIDES.md. Start here: open C, choose Food then confirm no recipe is selected and Enter does nothing. Choose a row explicitly to inspect it. Start a valid craft, change categories and confirm the same job continues; R must be a separate deliberate cancellation. Check actual mouse category/row/action buttons during the existing session. No new separate Personal task.

AI runs require Editor closed and create their own disposable identity/settings/evidence:

```powershell
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunNativeAutomation.ps1 -TestFilter 'PF.UI.RecipeDetails+PF.Crafting.ToolProgression' -Label M11CraftCategories
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunControlsAutomation.ps1 -TestCase ToolProgression -Resolution 720 -HUDScale 1.5
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunControlsAutomation.ps1 -TestCase ToolProgression -Resolution 1440 -HUDScale 1.5
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunControlsAutomation.ps1 -TestCase Overlays -Resolution 720 -HUDScale 1.5
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunControlsAutomation.ps1 -TestCase Controls -Resolution 720 -HUDScale 1.5
```

Changed source: Crafting/PFCraftingHUD.cpp/.h, Survival/PFControlsMenu.cpp, Tests/PFToolProgressionLiveTests.cpp and PFControlsLiveTests.cpp. Documentation covers evidence, decisions, current state and existing player/Trello guides.
