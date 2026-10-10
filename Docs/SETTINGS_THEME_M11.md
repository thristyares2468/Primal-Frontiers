# M11 themed settings and category guidance

October 10, 2026. Bounded technical presentation gate; full M11/M12 and personal usability remain open. Trello https://trello.com/c/PzdtVFJF.

The existing local settings screen now shares PFUITheme with Pause/main/crafting: texture-free rounded surfaces, equal category and action buttons, outlined active row/category, wrapped category guidance and a separate status/controls footer. Exact values, settings backends, first-person focus, draft transactions, Apply/Cancel/Defaults and display confirmation remain unchanged. No art, assets, dependencies, gameplay authority, RPC, save schema or personal preference changes.

Source: Settings/PFSettingsMenu.cpp/.h and Tests/PFSettingsRestartLiveTests.cpp. The latter checks real painted title/category guidance/action/help bounds and desired text height after screenshot/paint waits; it never seeds expected settings from the live object. Existing isolation and server/save guards remain.

## Reproduce and expected behavior

Level: /Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld. One rendered Standalone -game process at a time, uncapped/VSync0, unique disposable config/profile. No native map required.

Start here: close Unreal Editor and use project PowerShell:

1. Run `./Scripts/RunNativeAutomation.ps1 -TestFilter PF.Settings.Preferences -Label M11SettingsTheme`; require exact1/1 Success, all exits0, no timeout.
2. Run `./Scripts/RunSettingsRestartAutomation.ps1 -Resolution 720 -TimeoutSeconds 240`. Actual menu Apply writes FOV95/blurOn/master90%/HUD125%, exits; a fresh process must load independent full expected preferences before fixture mutation. Inspect all four category screens, resumed camera/HUD, painted bounds, Cancel and source/personal hash checks.
3. Only after720p passes run the same restart command with `-Resolution 1440`. Require Passed=true and exact Prepare/Verify selectors. Preparation alone is not restart proof.
4. Run `./Scripts/RunControlsAutomation.ps1 -TestCase Settings -Resolution 720 -TimeoutSeconds 240`. Four drafts, Defaults, pad/keyboard Cancel, reopening, focus, teardown, inventory and applied configuration must remain unchanged.
5. Run `./Scripts/RunControlsAutomation.ps1 -TestCase Display -Resolution 1440 -TimeoutSeconds 240`. Real bounded smaller window, Keep, Back and actual15-second paused timeout must restore confirmed viewport/disk state. Do not launch this fixture from720p: its retained previous smaller-mode prerequisite failure is documented separately.
6. Review index.json/run-summary.json and unique logs for exact selection, errors/warnings, timeout, process exits and hash guards; inspect PNGs for legible, unclipped guidance/title/actions. Stop/fix/replay affected scope on failure.

Human convenience, not another assignment: in one-player Selected Viewport PIE on the same map click the view, press P, choose Settings. Mouse tabs/arrows or LB/RB and D-pad change drafts; Apply saves, Cancel discards unapplied edits, Defaults needs Apply. Esc may stop PIE; P opens the full Pause menu. Window size/mode are controlled by the Editor in PIE.

## Exact technical evidence

Editor Development Win64 passed40.20s, Game 51.03 s, no compiler warnings. Saved/Logs/PFM11SettingsThemeEditorBuild.log and PFM11SettingsThemeGameBuild.log. Native/run whitespace checks passed.

| Report directory in Saved/AutomationReports | Exact selector / passed | Seconds | Sampled peak working/private GiB |
|---|---|---:|---:|
| Automation_M11SettingsTheme_20261010_061944568_e46ba2bd | PF.Settings.Preferences1/1 |45.20|2.963 /2.915|
| M11RestartP720_20261010_062052947_b3e98330 | PF.UI.SettingsRestartPrepareLive1/1 |48.77|3.342 /5.169|
| M11RestartV720_20261010_062142358_b3c35aa7 | PF.UI.SettingsRestartVerifyLive1/1 |33.89|3.316 /4.294|
| M11RestartP1440_20261010_062317952_eef2e09d | PF.UI.SettingsRestartPrepareLive1/1 |31.57|3.393 /5.796|
| M11RestartV1440_20261010_062349795_197c42a1 | PF.UI.SettingsRestartVerifyLive1/1 |33.97|3.405 /4.721|
| M11Settings720_20261010_062445656_cc18fc2a | PF.UI.SettingsCancelLive1/1 |35.50|3.261 /5.277|
| M11Display1440_20261010_062559394_8da921f1 | PF.UI.SettingsDisplayLive1/1 |51.56|3.346 /5.656|

All seven exact test records passed; all engine/verdict/runner exits0, test errors/warnings0, no timeout. Six rendered runs preserve default settings and respectively316/317/318/319/320/321 existing private saves; both fresh-process source INIs unchanged. No failed build/test gate in this task.

All21 PNG inspected under Saved/AutomationReports/ControlsUI/<run>: fourteen restart screenshots, four Cancel category drafts, three display pending/confirmed/timeout. Graphics has17 rows in an inner scroll region; partially visible unselected rows at its edge are deliberate clipping, while headers/actions/footer stay visible. Actual smaller display is1920x1440 from2560x1440 launch. Source status/captions remain readable.

Native unique Saved/Logs/PFAutomation_<native-run>.log has0 raw severity. Six rendered unique Saved/Logs/<run>.log: Prepare/Cancel/Display retain known26warnings14experimental installed-engine Python errors; Verify24/14 removes the two known blur/DOF startup CVar warnings. Normalized comparison with retained M12Protection1440_20261009_103335442_3f0bea5f.log gives0new severity categories; no assertion/fatal/ensure. Raw startup is not completely clean. Default PrimalFrontier.log remains October9 stale; unique logs are evidence.

## Limits

Installed5.8.3 versus requested5.8.2; host31.93GiB physical RAM, build commit41.33GiB. Peak process memory up to3.405working/5.796privateGiB, not16GB minimum certification or total machine budget. UnlimitedFPS does not establish a sustained FPS/stutter benchmark. No audible audio (-nosound), physical controller, human usability, fullscreen/HDR/monitor behavior or display-mode restart certification. Synthetic input and screenshots are technical evidence only. Network replay is not required for styling and test-only bounds changes.

Next: immediate bounded Trello evidence/Done/readback and scoped Sol publication; fresh board/current-state audit before another independently eligible objective. Existing Personal rendered-save, controller, combat/pacing, readability and FPS gates remain open.
