# M11 Settings Apply verification

October 9, 2026. The bounded non-resolution Apply and same-process disk-reload check passed. Full M11 and Personal gameplay gates remain unverified.

## What changed

Added opt-in `PF.UI.SettingsApplyLive`. It uses actual Pause/Settings Slate keyboard and controller routing to change FOV, motion blur, master volume and HUD scale. Before Apply, all live preferences must stay unchanged. Apply must show the real saved-status message, update the renderer, preserve resolution/mode/quality/FPS, and write its disposable INI. The fixture then replaces only the live in-memory preference values, calls supported `LoadSettings(true)`, and checks that the saved values return without rewriting the file. Reopening Settings verifies all four displayed values. After resume, the real first-person camera and vitals font must consume FOV/blur/HUD changes; modal input must conserve inventory and restore movement/focus.

The runner adds `SettingsApply` with an explicit opt-in flag. The actual config branch must point to this run's unique `Saved/AutomationReports/<run>/GameUserSettings.ini` before any fixture actions. The runner rejects reused destinations, uses a unique reconnect profile, and checks default GameUserSettings/Scalability file presence and SHA256 before/after. Only this case omits the legacy `-NoSaveConfig` argument to explicitly permit its disposable writes. Apply also omits the console VSync override so GameUserSettings owns it; the fixture verifies actual `r.VSync=0`, `t.MaxFPS=0`, unlimited frame limit and VSync disabled. No warnings are suppressed.

No runtime gameplay/settings implementation, assets, networking authority, schema or dependencies changed. Existing non-Apply cases retain their previous launch arguments.

## Build and retained failures

- Initial Editor build failed in 13.98 s: C2666, new test compared a float font size to an integer expected value. Corrected only the expected type. Log: `Saved/Logs/M11SettingsApplyEditorBuild_20261009.log`.
- Corrected Editor passed in 5.74 s; Game passed in 22.17 s. Logs: `M11SettingsApplyEditorRetry_20261009.log`, `M11SettingsApplyGameBuild_20261009.log`.
- First rendered 720p run retained: `M11SettingsApply720_20261008_234536772_45b31a86`. All assertions passed but strict verdict failed on one warning: the runner's console VSync value blocked the supported lower-priority GameUserSettings write. Engine exit 0 was not a pass; report/runner exit 1. Default config unchanged; 29.04 s; sampled working/private 3.076/4.889 GiB. Removed only this case's console override and added actual uncapped-state assertions.
- Final Editor passed in 5.92 s; Game passed in 13.95 s. No compiler warnings. Logs: `Saved/Logs/M11SettingsApplyVSyncEditor_20261009.log` and `M11SettingsApplyVSyncGame_20261009.log`.
- PowerShell AST parse and `git diff --check` passed.

## Final tests

| Test / resolution | Report under Saved/AutomationReports | Result | Seconds | Sampled working/private GiB |
| --- | --- | --- | --- | --- |
| PF.UI.SettingsApplyLive / 1280×720 | M11SettingsApply720_20261008_234745899_aee28374 | 1/1 passed | 28.94 | 3.084 / 4.430 |
| PF.UI.SettingsApplyLive / 2560×1440 | M11SettingsApply1440_20261008_234843853_f7e5155a | 1/1 passed | 28.74 | 3.177 / 5.649 |

Both engine/report/runner exit codes were 0, with zero test errors/warnings and no timeout. Both summaries confirm the one existing default settings file remained unchanged; no default settings file was added. Six PNGs were inspected: `applied.png`, `reloaded.png`, `resumed.png` under `Saved/AutomationReports/ControlsUI/<run>`. Applied status, reopened values and resumed scaled HUD were visible. This is 125% HUD-scale evidence, not all accessibility scale/extreme-layout coverage.

Each raw log retains the known 21 editor-widget warnings, 3 linker/HLOD warnings, 14 installed-engine Python error lines, and 2 startup ConsoleManager priority warnings: lower-priority scalability blur/DOF requests are ignored after game settings disabled them. The tested Apply itself has zero warnings. No other severity category, ensure/fatal/crash or VSM overflow occurred. Unique logs are `Saved/Logs/<run>.log`; the stale default PrimalFrontier.log is not these runs' evidence.

## Repeat the check

Level: `/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld`. One rendered standalone `-game` process, windowed, normal quality, uncapped. Close existing Unreal processes first.

Start here, from the project root:

```powershell
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunControlsAutomation.ps1 -TestCase SettingsApply -Resolution 720
```

1. Wait for the client to finish. Require test Success, zero errors/warnings, all three exit codes 0 and `DefaultConfigsUnchanged=true` in its `run-summary.json`.
2. Inspect `index.json`, the unique log, and the three screenshots. On failure, retain artifacts and fix only this task.
3. Only after the first pass, repeat with `-Resolution 1440`.

This writes only the unique generated game-settings destination through supported Unreal APIs. Do not use a personal profile/config or copy/delete private files to force a result.

## Limits and next work

This verifies same-process real-file reload; it does not certify settings across a process restart. `-nosound` means master volume preference persistence is checked, not audio audibility or device gain. Resolution/display confirmation, cancellation and timeout need a separate guarded test. No new multiplayer run is needed for this local fixture/launcher change. Manual walking/overnight, physical controller/audio and sustained FPS/stutter remain separate Personal gates. The host has 31.93 GiB RAM and installed UE 5.8.3, distinct from the requested 5.8.2/16 GB target; short runs do not certify that minimum.

Trello: https://trello.com/c/sIfh4PuZ. Next independent M11 step: disposable windowed display confirmation and rollback; no M12 implementation follows automatically.
