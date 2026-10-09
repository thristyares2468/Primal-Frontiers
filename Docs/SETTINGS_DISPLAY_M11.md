# M11 windowed display confirmation and rollback

October 9, 2026. Bounded normal 1440p launch test passed. The attempted minimum-size 720p launch failed the fixture's smaller-mode prerequisite and is retained, not counted as a display pass. Full M11 and Personal gates remain unverified.

## Scope

Added opt-in `PF.UI.SettingsDisplayLive` and runner case `Display`. Real Slate input opens Settings, selects Windowed before applying, and chooses an engine-reported resolution no larger than the launch dimensions. The fixture checks the actual rendered viewport and actual Slate window mode, not just the settings caption. A controller Keep confirms the smaller size; a later keyboard Back restores it from a pending larger size. A third change waits the existing real 15-second deadline while the world is paused, then checks the timeout message, viewport rollback and focus. Finally it alters only the live resolution field and forces the supported disk reload to recover the kept resolution and last-confirmed mode. Non-display preferences/quality, uncapped state, inventory and resumed movement must remain valid.

All writes use the unique supported `GameUserSettingsINI` destination and unique identity profile. The same actual-branch guard, reused-evidence refusal and default GameUserSettings/Scalability SHA256/presence gate apply. Only guarded Apply/Display cases omit the legacy NoSaveConfig argument and console VSync override; the test verifies actual VSync off and unlimited FPS. No physical fullscreen, monitor, HDR, quality, UI-floor or project rendering settings were changed. No runtime implementation, assets, schema or gameplay authority changes.

## Build

Editor passed in **14.59 s**, Game in **20.97 s**, with zero compiler warnings. Logs: `Saved/Logs/M11SettingsDisplayEditor_20261009.log` and `M11SettingsDisplayGame_20261009.log`. PowerShell AST parse and diff whitespace checks passed.

## Retained 720p prerequisite failure

`M11Display720_20261008_235745355_a2534830`: one failed assertion, zero test warnings, engine exit 0 / report and runner exit 1. The fixture found no smaller supported resolution under its 1280×720 launch bounds. It stopped before menu Apply; no display pass is claimed. 22.47 s, sampled working/private **2.992 / 4.841 GiB**; default settings file unchanged.

Diagnosis from installed primary engine source: `KismetSystemLibrary.cpp` `GetSupportedFullscreenResolutions` filters below `GetMinYResolutionForUI`, whose default CVar is 720. `GameEngine.cpp` convenient-window generation also uses a 1280×720 minimum. The fixture incorrectly assumed a minimum-sized launch could shrink further using standard menu options on this host. No menu bug was established. We did not reduce the UI floor, alter assets/settings, skip the assertion into a false pass, or silently mark the 720p flow verified. The unchanged fixture was run from 1440p after documenting this limitation; its temporary windows never exceeded that launch size.

## Successful normal flow

Report: `Saved/AutomationReports/M11Display1440_20261009_000013129_362f677c/index.json` and `run-summary.json`.

- **PF.UI.SettingsDisplayLive: 1/1 passed**, zero test errors/warnings, engine/report/runner exit 0, no timeout.
- Actual launch **2560×1440**, confirmed smaller window **1920×1440**. Temporary unconfirmed changes back to launch size reverted to the confirmed 1920×1440 on both Back and the actual 15-second timer.
- All actual viewport, Windowed mode, last-confirmed values and disposable disk reload assertions passed. Pause focus, resumed movement, inventory and non-display settings were conserved.
- **46.29 s**, sampled working/private **3.203 / 5.393 GiB**. One existing default settings file stayed unchanged; no default settings file was added.
- Three PNGs inspected: `pending.png`, `confirmed.png`, `timeout.png` in `Saved/AutomationReports/ControlsUI/M11Display1440_20261009_000013129_362f677c`. They show the smaller window, real countdown, saved confirmation and timeout result. Graphics rows use the existing scroll area.

Unique log: `Saved/Logs/M11Display1440_20261009_000013129_362f677c.log`. It contains two real unconfirmed-revert messages. Raw logs retain the known 21 editor-widget warnings, 3 linker/HLOD warnings, 14 installed-engine Python error lines and 2 startup blur/DOF CVar-priority warnings. There were no other warning/error categories, ensures, fatals, crashes or VSM overflows in the successful run. The retained failure also has its expected automation-error lines. The stale default PrimalFrontier.log is not these runs' evidence.

## Repeat the check

Level: `/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld`. One rendered standalone window, normal quality, uncapped; close existing Unreal processes first.

Start here, from the project root:

```powershell
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunControlsAutomation.ps1 -TestCase Display -Resolution 1440
```

1. Wait for the one client to finish. It will use menu input, change only its own window/config, and wait the real timeout. Do not interact during automation.
2. Require Success, zero test errors/warnings, engine/report/runner 0 and `DefaultConfigsUnchanged=true` in its summary.
3. Check `index.json`, unique raw log and all three screenshots. Retain failure artifacts; unsupported host modes are a failed prerequisite, not a pass.
4. Use 1440p for this shrink/rollback fixture on this host. The 720p attempted shrink is deliberately still recorded as failed; ordinary Apply/Cancel at720p passed separately. A new host needs supported-mode evidence before claiming coverage.

## Limits and next step

This is an actual single-process rendered windowed flow, not physical controller feel, fullscreen/borderless monitor behavior, HDR, user acceptance or settings persistence across a process restart. No multiplayer rerun is needed for local test/launcher additions. No sustained FPS, stutter or 16 GB minimum certification is inferred: host RAM is 31.93 GiB; installed engine is 5.8.3 versus requested5.8.2.

Trello: https://trello.com/c/rZT9QNCL. Next independent settings check: a guarded separate-process preference reload; manual UI/open-world/controller/audio cards remain open. No M12 implementation begins from this bounded result.
