# M11 controls/help slice

Open **P → Controls & help** on L_PrimalFrontier_OpenWorld. P avoids PIE's Editor Esc stop shortcut. The pages are read-only, not remapping. Left/Right or LB/RB selects keyboard/controller, Up/Down or D-pad scrolls, and Esc/P/B/Menu returns to Pause. Mouse page/back buttons also work. Only Resume closes Pause.

BindControl records descriptions beside the actual legacy delegates. Movement/view/jump labels read the current public Enhanced Input mapping view. Controller descriptions explain inventory/crafting/building contexts. Help includes pickup aim/reach, finite food/freshness, explicit-save guidance and standalone versus multiplayer safety. Page selection is explicit; automatic last-device switching/remapping are not implemented.

Pause owns/removes child help/settings on resume/teardown and sizes its background to wrapped content. Its obsolete “nothing is saved yet” warning is replaced with accurate explicit-save guidance. This UI adds no save RPC, server mutation, asset, plugin or dependency.

## Evidence (October 8)

| Check | Result / exact evidence |
| --- | --- |
| Initial Editor build | FAILED C2248 protected accessor + incomplete mapping type; Saved/Logs/M11ControlsBuild_20261008.log retained |
| Editor final | PASSED6.31 s, no compiler warnings; M11ControlsFinalBuild_20261008.log |
| Development Game | PASSED41.75 s, no compiler warnings; M11ControlsGameBuild_20261008.log |
| PF.Input.Gamepad + PF.Settings.Preferences | 2/2 PASSED, zero test/raw warnings/errors; Automation_M11Controls_20261008_090734672_36578953;16.07 s; working/private2.920/2.782 GiB |
| Initial PF.UI.ControlsLive720p | FAILED three focus assertions; engine0/strict verdict1; M11Controls720_20261008_090915891_b2dca088 retained |
| Final PF.UI.ControlsLive720p | 1/1 PASSED, zero test warnings/errors; engine/verdict0; M11Controls720_20261008_091400974_9c4f8bcf;32.61 s; working/private3.079/4.090 GiB |
| Final PF.UI.ControlsLive1440p | 1/1 PASSED, zero test warnings/errors; engine/verdict0; M11Controls1440_20261008_091446844_c3fb51c8;37.27 s; working/private3.166/4.367 GiB |

Reports: Saved/AutomationReports/<run>/index.json and run-summary.json. Logs: Saved/Logs/<run>.log. Actual keyboard/controller/pause PNGs: Saved/AutomationReports/ControlsUI/<run>/. Both final resolutions inspected. Earlier passing retries retained: M11Controls720_20261008_091134050_5935558b and M11Controls1440_20261008_091217044_429864d9.

The failed focus test delivered navigation in the same frame as queued SetInputMode focus. A frame boundary corrected fixture timing while retaining real Slate input delivery. Screenshot inspection then found a footer outside the fixed pause background; content sizing corrected it before the final runs. The opt-in test checks pages, repeat protection, Back focus, blocked gameplay key delivery, Resume and child cleanup. It does not certify physical-pad feel or human usability.

```powershell
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunControlsAutomation.ps1 -Resolution 720
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunControlsAutomation.ps1 -Resolution 1440
```

Runner refuses an existing Editor, uses one D3D12 rendered client, fresh evidence labels, timeout, strict report verdict and owned-process cleanup. Logs confirm t.MaxFPS0/r.VSync0. No sustained FPS/stutter/traversal benchmark is claimed from a stationary menu test. Raw launches retain known EditorDataStorage widget warnings, HLOD imports and engine Python ToolsetDefinition/PythonTestRunner startup errors, separate from clean test reports; no crash/ensure/fatal.

Full M11 remains in progress. Latest continuation authorization makes deferred Personal tests nonblocking and UNVERIFIED. Next bounded slice: stable inventory selection when the chosen stack expires/disappears and clearer feedback. No art is needed.
