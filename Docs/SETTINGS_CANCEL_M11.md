# M11 rendered settings draft and Cancel verification

October9,2026. Test/evidence addition only; no settings/gameplay implementation fix was needed. Full M11 human usability remains unverified.

PF.UI.SettingsCancelLive opens Pause → Settings through real Slate gamepad focus. It edits actual Game/FOV, Graphics/motion-blur, Audio/master and Accessibility/HUD-scale drafts, navigates tabs/rows, invokes Defaults, cancels with controller B, reopens to discard the draft, cancels with keyboard Escape and checks parent teardown. It verifies every reflected live preference, native quality levels, display/FPS/VSync and renderer blur remain unchanged. Three real seeded wood items are conserved. The existing user config's presence/text contents remain unchanged. It never presses Apply, invokes SaveSettings or changes display mode/resolution. Defaults changes only the draft.

Fixtures require opt-in, a unique UI-prefixed local profile and a fresh evidence directory. They are excluded from Shipping. The launcher remains one rendered process with normal quality,ForceRes,uncapped FPS/VSync0 and no config saving. Requested HUDScale1.5 is recorded by the runner, but this fixture deliberately does not apply it to live preferences; the existing menu typography is fixed. Do not call these maximum-scale HUD tests. Actual menu/footer bounds and draft behavior are checked at the two output resolutions.

## Exact evidence

Editor PASSED20.27s and Development Game PASSED26.49s,without compiler warnings: Saved/Logs/M11SettingsCancelEditorBuild_20261009.log and M11SettingsCancelGameBuild_20261009.log. Installed engine5.8.3,requested5.8.2.

| Test | Report directory under Saved/AutomationReports | Result | Seconds | Sampled working/private GiB |
| --- | --- | --- | --- | --- |
| PF.UI.SettingsCancelLive,1280x720 | M11Settings720_20261008_223515493_7f54212c | 1/1 passed | 30.53 | 2.979/3.937 |
| PF.UI.SettingsCancelLive,2560x1440 | M11Settings1440_20261008_223614943_b5d89f62 | 1/1 passed | 30.69 | 3.185/4.389 |
| PF.Settings.Preferences,NullRHI | Automation_M11SettingsCancel_20261008_223928509_5042bcdb | 1/1 passed | 21.72 | 2.959/2.866 |

Each directory has index.json/run-summary.json. All engine/strict exits0,test warnings/errors0. Native raw warning/error/ensure0. Each rendered log contains24 known widget/HLOD startup warnings and14 engine Python startup error lines,zero other warning/error/ensure/fatal/crash. Logs are Saved/Logs/<rendered-run>.log and PFAutomation_M11SettingsCancel_20261008_223928509_5042bcdb.log.

Eight PNGs inspected: Saved/AutomationReports/ControlsUI/<rendered-run>/{game_draft,graphics_draft,audio_draft,accessibility_draft}.png. The long Graphics list scrolls inside its region; action/status/footer remain visible. Changes shown in screenshots are unapplied drafts,not persistent player settings. Timings/memory are short process samples,not traversal FPS or minimum-spec/stutter certification.

```powershell
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunControlsAutomation.ps1 -TestCase Settings -Resolution 720
```

Trello https://trello.com/c/WwM89qzB records the bounded task Done. No new multiplayer test is necessary for this local draft-only fixture; real network-profile verification remains in RECONNECT_FEEDBACK_M11.md. Applying/saving live preferences, display confirmation/timeout, hardware-controller feel and actual audio output are outside this test. Personal M7/M8/M11 gates stay open. Next eligible M11 implementation: explicit server-owned reconnect-restoration acknowledgment, separate from local credential saving.
