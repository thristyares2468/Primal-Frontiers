# Selected crafting row visibility — M11

October 10, 2026. Bounded technical task https://trello.com/c/lbTkhg0x; full M11/M12 and Personal acceptance remain open.

The earned guard screenshot exposed a partly clipped selected row at 720p/HUD scale 1.5. A selection-time scroll request could run before fonts and wrapped row sizes settled; subsequent multi-line server feedback also shrank the browser without a new request. UPFCraftingHUD now observes cached viewport/selected-row dimensions and selection identity, requesting supported ScrollWidgetIntoView only when those change. Stable layouts preserve deliberate manual scrolling. No recipe, input, authority, inventory, save or asset behavior changes.

## Verification

Level: /Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld. One isolated rendered standalone process, uncapped FPS/VSync0, disposable reconnect/settings destinations with read-only personal hashes. New PF.UI.CraftingScrollLive selects the actual last row, produces real knowledge/craft refusals to grow the footer, measures full button/title/body/picture bounds in the actual browser, preserves manual SetScrollOffset during stable layout, tests Up/Down wrap/category empty selection and Close input balance. No grants or XP seed. Synthetic Slate keyboard and supported manual offset are not physical mouse/controller acceptance.

| Evidence under Saved/AutomationReports | Result |
| --- | --- |
| M11CraftingScroll720_20261010_040535610_70d70cc3 | 1/1, 29.35 seconds, working/private 3.088/5.031 GiB; 280 personal saves unchanged |
| M11Scroll1440_20261010_040753074_e2cbe7e7 | 1/1, 29.58 seconds, 3.262/5.638 GiB; 282 personal saves unchanged |
| M11Overlays720_20261010_041327183_05bd47dd | ActionOverlaysLive 1/1, 30.29 seconds, 3.112/5.340 GiB; 285 personal saves unchanged |

Both exact selector checks, engine/strict/runner exits0, test severity0/no timeout. Six PNGs inspected under AutomationReports/ControlsUI/<run>: last_row_before_feedback, last_row_after_feedback, first_row_wrapped. Full selected row including image and wrapped title/output/duration is visible. Long right-hand details remain deliberately scrollable; not all off-screen rows/details can fit simultaneously. Both raw logs match prior rendered baseline26startupwarnings/14experimental Python definition lines with no new severity/assert/fatal/ensure. Default settings unchanged. Unique Logs/<run>.log are current evidence, ordinary PrimalFrontier.log predates this task.

## Retained failures and corrections

1. Reproduction M11CraftingScroll720_20261010_040241709_03273643 failed six button/title/body assertions before any production fix. Stage1 button bottom675.8 vs browser581.2; after real footer growth browser515.2. Engine0/verdict1,29.73 seconds,279personal saves/settings unchanged. No timeout/crash. Retain all raw/report/PNG evidence.
2. Fix build PFM11CraftingScrollFixEditorBuild.log linked18.38 seconds with two C4305 tolerance-literal warnings. Changed only0.1 to0.1f; clean Editor PFM11CraftingScrollCleanEditorBuild.log succeeded6.90 seconds.
3. Initial1440 launch M11CraftingScroll1440_20261010_040638214_a3f2d8fd refused before menu tests: generated49-character label exceeded48. Shortened only this runner prefix to M11Scroll; successful1440 replay above, no assertion weakening.
4. Existing ActionOverlays720 regression M11Overlays720_20261010_040841472_5b0dbb92 failed nine assertions requiring nonselected off-screen Recipe6/7 text inside the root screen. Corrected fixture to measure fixed screen controls, actual scroll visibility and full selected row. Retry M11Overlays720_20261010_041119958_c3573741 still failed nine allocation checks on culled children with stale cached geometry. Final fixture tests allocations of painted/intersecting children and every fixed control, with mandatory full selected-row bounds; CraftingScroll independently paints/selects the lower rows before checking them. No production change or warning suppression for these fixture corrections. Both failures retained, engine0/verdict1/no timeout/personal hashes unchanged.

Final Editor PFM11CraftingScrollPaintEditorBuild.log succeeded6.48 seconds and Game PFM11CraftingScrollGameBuild.log25.20 seconds without compiler warnings. Existing Overlays replay passed with original craft/refusal/busy/cancel/selection/buttons/Pause/input/build/storage checks and strengthened painted allocation/selected-row bounds. All six final overlay PNGs inspected, readable actual fresh counts/server refusals/storage omissions. Its raw baseline also unchanged26/14/no assert/fatal/ensure. All three final tests technically passed; earlier failures remain evidence, not waived successes.

Start here: close competing Unreal processes, build Editor, run Scripts/RunControlsAutomation.ps1 -TestCase CraftingScroll -Resolution 720 -HUDScale 1.5, then the same1440 and -TestCase Overlays -Resolution 720 -HUDScale 1.5. Require exactly one clean test per run, inspect raw logs/PNG/hash guards, build PrimalFrontier Development Win64. No networking mutation, so no new multiplayer run applies to this local layout slice. Installed5.8.3 vs requested5.8.2, host31.93GiB; stationary synthetic UI does not certify sustained FPS/stutter/minimum16GB or human feel. No external assets/private settings/save edits.
