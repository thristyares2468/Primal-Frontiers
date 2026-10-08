# M11 local reconnect-profile feedback

October9,2026. Bounded local UI feature; full milestone/manual acceptance remains unverified.

The server-issued credential callback records Unknown/Saved/Failed locally. Pause shows the actual successful write or sanitized failure and explains that this does not save world state. Unknown shows no outcome. Blueprint presentation can read the typed status/text; it cannot grant a credential through a new setter. Feedback changes update only their label, preserving modal focus, selected button and quit confirmation. No added RPC, replicated gameplay field, save schema, asset or dependency.

The isolated rendered fixture uses a unique14-character UI-prefixed identity profile supplied by RunControlsAutomation.ps1. It reads the real deferred server-issued profile from disk, rejects an invalid credential without changing prior disk-backed login options, then rewrites the valid credential. It checks feedback, bounds, credential omission, Controls focus, End-session confirmation and unchanged inventory. The deliberately rejected invalid credential registers exactly one sanitized expected warning; Unreal automation demotes that matching log to Verbose. Unexpected test warnings/errors still fail. This tests rejection, not disk-full/permission failure, and does not prove production account identity or network reconnection.

## Evidence

Installed engine5.8.3 (project requested5.8.2). Editor build22.04s and Game26.82s passed without compiler warnings: Saved/Logs/M11ReconnectEditorBuild_20261009.log and M11ReconnectGameBuild_20261009.log. Adding the network-fixture assertions required another Editor/Game build10.42/17.00s, also clean (M11ReconnectNetworkEditorBuild_20261009.log and M11ReconnectNetworkGameBuild_20261009.log).

| Test | Exact report directory under Saved/AutomationReports | Result | Seconds | Sampled working/private GiB |
| --- | --- | --- | --- | --- |
| PF.UI.ReconnectProfileLive,1280x720 | M11Reconnect720_20261008_221603465_ecfe6e7d | 1/1 passed | 31.00 | 2.989/3.974 |
| PF.UI.ReconnectProfileLive,2560x1440 | M11Reconnect1440_20261008_221725512_e4471903 | 1/1 passed | 30.57 | 3.252/4.426 |
| PF.Input.Gamepad,PF.Persistence.FileGenerations | Automation_M11Reconnect_20261008_221846103_6f726773 | 2/2 passed | 16.06 | 2.943/2.808 |

Each report contains index.json and run-summary.json. Engine/strict exits0; test warnings/errors0. Native raw severity0. Rendered raw logs have21 known EditorDataStorageUI warnings,3 Linker/HLOD warnings and14 engine Python startup error lines each;0 VSM overflow/ensure/fatal/crash. The expected injected rejection is Verbose, not a hidden unexpected warning.

Six screenshots inspected: Saved/AutomationReports/ControlsUI/<rendered-run>/saved.png,failed.png,retry.png. Raw rendered logs: Saved/Logs/<rendered-run>.log; native log: Saved/Logs/PFAutomation_M11Reconnect_20261008_221846103_6f726773.log. Rendered runs use normal settings,uncapped FPS/VSync0 and requested HUDScale1.5. Pause's existing fixed readable typography remains unchanged; no uniform scaled-menu/accessibility certification. Short stationary menu tests cannot establish traversal FPS or sustained stutter; timings/memory are sampled process observations.

One-client NullRHI PF.Persistence.Live creation/restart PASSED4/4 process reports: M8Live1_20261008_222001753_9a0d3db4{CreateServer,CreateClient1,RestartServer,RestartClient1}. Engine/strict exits0; test errors/warnings0; actual server credential RPC leaves Saved status and readable disk-backed reconnect details in both client phases. Working/private peaks: server1.710–1.716/1.608–1.612GiB; client1.794–1.806/1.724–1.771GiB. Raw24 known startup warnings/14 Python error lines per process; no ensure/fatal. Exact aggregate summary: Saved/AutomationReports/M8Live1_20261008_222001753_9a0d3db4/run-summary.json. Per-process reports/logs use the full run plus phase/role names, index.json and Saved/Logs/PF<run>.log.

Two-client NullRHI PF.Persistence.Live creation/restart PASSED6/6: M8Live2_20261008_222210203_173cb094{CreateServer,CreateClient1,CreateClient2,RestartServer,RestartClient1,RestartClient2}. Confirmed75ms outgoing lag/1% loss; phase durations51.55/40.06s. New local Saved/readable-profile assertions passed for both clients on initial connection and separate-process restart. Existing invalid RPC rejection, restored identity/tool/health/structure/storage and owner/foreign privacy checks passed. Server working/private1.717–1.719/1.620–1.628GiB; clients1.802–1.810/1.732–1.776GiB. Engine/strict0,test errors/warnings0; raw24 known startup warnings/14 Python lines per process,0ensure/fatal. Aggregate: Saved/AutomationReports/M8Live2_20261008_222210203_173cb094/run-summary.json. Client2 did not crash headlessly; no new rendered multiplayer/RHI or human gameplay pass follows.

Trello: https://trello.com/c/0JTT4fKx (bounded task Done). Next independent M11 check: real settings draft/cancel/modal focus in the rendered pause UI. Full manual usability remains open.

Repeat the rendered fixture:

```powershell
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunControlsAutomation.ps1 -TestCase Reconnect -Resolution 720 -HUDScale 1.5
```

The runner refuses an existing Unreal process and retains generated private test profiles locally. Do not publish identity files/world saves or full login URLs. Physical-controller feel, human open-world usability, M7 overnight and M8 rendered save/restart remain Personal gates.

## Rendered launcher profile-isolation follow-up

Source review found that ordinary standalone PostLogin writes a credential for every rendered test, not only Reconnect. The runner now supplies a fresh UI-prefixed profile for every case. The previous runner could overwrite default Local details via login; this fix preserves existing files and does not attempt recovery or replacement of earlier credentials/world saves.

PF.UI.ControlsLive PASSED1/1 with the fixed launcher: M11Controls720_20261008_222930630_3f0e3575,32.09s,3.081/4.100GiB,engine/strict0,test warnings/errors0. Read-only SHA256 comparisons surrounding that whole launch found two default Local profile files before/two after,zero changed/added files. No hashes or credential contents were published; the comparison result is recorded here from the shell verification. Three PNGs inspected; raw24 known startup warnings/14 Python lines,zero other warning/error/ensure/fatal. No C++ changes/new build required for this script-only fix. Trello https://trello.com/c/rCmZYZFy.
