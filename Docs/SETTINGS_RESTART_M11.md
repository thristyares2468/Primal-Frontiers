# M11 settings across a real process restart

October 10, 2026. Bounded technical gate passed; full M11/M12 and personal acceptance remain open. Trello: https://trello.com/c/jeLo0isc.

## Behavior and isolation

Two sequential rendered Standalone processes use /Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld. Preparation uses actual Pause/Settings input and Apply to save FOV95, blur On, master90%, HUD125% through supported settings APIs to a unique run-owned GameUserSettings.ini. The process exits before verification starts. Verification checks the independently constructed full expected preferences before any fixture Apply, LoadSettings or mutation, then displays all four categories and verifies actual camera/renderer/HUD consumption. A changed draft is cancelled; the source INI SHA256 is unchanged across the whole fresh process.

The actual Unreal config branch must point exclusively to the run-owned INI. Reused evidence paths, invalid source names, wrong resolution, failed/incomplete preparation and source use in unrelated cases are rejected. Default settings and all existing private saves are hash-guarded. Fixtures seed three wood solely to test modal input conservation; this is not an earned gameplay route. No personal configuration, save, asset, schema, gameplay or transport changes.

## Reproduce

Start here: close Unreal Editor, then run from the project PowerShell terminal:

1. `./Scripts/RunSettingsRestartAutomation.ps1 -Resolution 720 -TimeoutSeconds 240`.
2. Require Passed=true, both exact selectors Success, all engine/verdict/runner exit codes0, no timeout and all hash guards true. Preparation alone is not restart proof. Stop on any failure.
3. Run the same command with `-Resolution 1440` after720 passes.
4. Inspect each run's index.json, run-summary.json, unique log and ControlsUI PNGs. The preparation report also contains settings-restart-summary.json. Never manually edit the diagnostic INI.
5. Shared runner regression: `./Scripts/RunControlsAutomation.ps1 -TestCase SettingsApply -Resolution 720 -TimeoutSeconds 240`.

Native preference validation needs no level: `./Scripts/RunNativeAutomation.ps1 -TestFilter PF.Settings.Preferences -Label M11SettingsRestart`.

## Exact evidence

Editor PrimalFrontierEditor Development Win64 passed34.94s; Game PrimalFrontier Development Win64 passed40.51s. No compiler warnings. Logs Saved/Logs/PFM11SettingsRestartEditorBuild.log and PFM11SettingsRestartGameBuild.log. PowerShell AST and git diff whitespace checks passed.

| Run under Saved/AutomationReports | Exact selector / result | Seconds | Peak working/private GiB |
|---|---|---:|---:|
| Automation_M11SettingsRestart_20261010_060227311_79e078a0 | PF.Settings.Preferences 1/1 |49.56|2.958 /2.837|
| M11RestartP720_20261010_060348761_bdc4f2b8 | PF.UI.SettingsRestartPrepareLive 1/1 |32.94|3.272 /5.224|
| M11RestartV720_20261010_060422026_57c66bef | PF.UI.SettingsRestartVerifyLive 1/1 |33.81|3.368 /4.365|
| M11RestartP1440_20261010_060535099_5c88c066 | PF.UI.SettingsRestartPrepareLive 1/1 |31.01|3.443 /5.643|
| M11RestartV1440_20261010_060606409_9a25b5ed | PF.UI.SettingsRestartVerifyLive 1/1 |33.17|3.361 /4.665|
| M11SettingsApply720_20261010_060740638_f18cb69f | PF.UI.SettingsApplyLive 1/1 |34.24|3.299 /5.231|

All six exact test selections passed, engine/verdict/runner exit0, test errors/warnings0 and no timeout. Default configuration file and311/312/313/314/315 existing private saves respectively unchanged in the five rendered runs. Both verify source INIs unchanged. Four expected negative preflight probes passed before launching Unreal: traversal source, wrong-resolution source, source on Controls, nonexistent exact source. No failed build/gameplay gate in this task.

All17 PNG inspected: two per preparation (prepared/resumed), five per verification (restart_game/graphics/audio/accessibility/resumed), three Apply regression (applied/reloaded/resumed). Located Saved/AutomationReports/ControlsUI/<run>.

Unique raw logs Saved/Logs/<rendered-run>.log; native Saved/Logs/PFAutomation_<native-run>.log. Native raw severity0. Both preparation and Apply logs retain the known26 warnings/14 experimental installed-engine Python error lines. Both fresh verification logs retain24/14: the two known startup blur/DOF CVar priority warnings are absent. Normalized comparison against M12Protection1440_20261009_103335442_3f0bea5f.log finds0 new categories; no assertion/fatal/ensure. These are not clean raw engine startup logs. Default PrimalFrontier.log is stale; unique logs are the evidence.

## Limits and next work

Launch uses -windowed -ForceRes, actual persisted menu mode remains Borderless; this test preserves display mode/resolution/quality and does not certify a display-mode restart or monitor behavior. Unlimited FPS and VSync off are asserted, but no sustained frame-rate benchmark or stutter route is measured. -nosound proves volume preference persistence, not audibility. Synthetic Slate/gamepad input is not physical-controller usability or human visual acceptance.

Installed engine5.8.3 versus requested5.8.2; host31.93GiB physical RAM, build commit40.17GiB. Sampled process peaks cannot certify the16GB minimum. No multiplayer replay required for local test/runner changes. Remaining personal rendered-save/controller/readability/pacing/audio/FPS checks stay unverified. Next independently actionable UI work: reconcile fresh Trello with current docs before improving settings presentation using existing primitives/theme only.
