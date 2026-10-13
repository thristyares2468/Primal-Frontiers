# M14 — climb contact repair and visible refusals, October 13, 2026

The user confirmed testing taller walls elsewhere. Fresh `Saved/Logs/PrimalFrontier_2.log` shows W/Space attempts near `(1500,4691)` at the 4m DangerLandmark, refused for height; some F presses occurred mid-air. The implemented ability mantles clear ledges45–120cm high. It does not scale tall vertical walls. This clarification is not a human pass of the nearby test blocks.

A separate real F bug was reproduced: after walking into an80cm or110cm wall, the request was accepted and charged15stamina, but the character rose and returned to the ground without reaching the top. Earlier automation approached from a favourable gap and did not cover grounded wall contact with actual keyboard routing.

## Repair

Both manual and jump-contact mantles now retain the same bounded server-derived horizontal drive through touching-wall collision. Unreal processes the queued LaunchCharacter after UpdateCharacterStateBeforeMovement; preserve the approved drive through that grounded callback only while a launch is pending and the1.5s window is valid. Landing, ordinary jumps, cancellation and death still clear it. Falling uses normal swept collision; original reach, ground-anchored height, top/overhead clearance, weight, stamina, cooldown and server authority remain.

The owning player sees explicit acceptance/refusal beneath the crosshair through the controller's existing feedback RPC. Refusals distinguish height, missing ledge, overhead clearance, stamina, weight, menus and needing grounded F input. No new movement-target RPC, client authority or inventory action was added. Tall-wall penetration by the top probe is explicitly rejected with the supported height range.

Built-in private ApplicationCore links the input-device mapper used by real-key automation. No external library, plugin, new art or asset edit. Existing saved80/110cm blocks beside spawn are retained; no resource/spawner IDs, private saves, settings or schema changed.

Changed gameplay files: PFSurvivorTraversal.cpp, PFTraversalMovement.cpp, PFTraversalInput.cpp, PFSurvivalPlayerController.h and the module Build.cs. Tests: new PFClimbContactTests.cpp and extended PFAuthoredClimbLiveTests.cpp; isolated controls/ecology runners gain -ManualClimb and explicit summary mode. Documentation/G10/Trello record the limits and retest.

## Tests and evidence

Paths are relative to the project. Native test is `PF.Traversal.Contact`: ordinary forward movement first stops against each wall; then manual or jump-contact climbing must physically land and charge exactly once. The saved-map test remains `PF.Traversal.AuthoredLive`, with manual or automatic mode. It now routes actual W/F/Space events through PlayerInput/Enhanced Input, walks2m into contact and validates server cost, owner landing, input release, no inventory grants and direct client refusal. It creates no substitute geometry.

The initial Shift readiness cue waits for server-confirmed setup/open game input and can be re-armed if startup clears its intent; actual climb actions are single attempts, never retried or replaced by direct calls. Rendered runs also press F at the actual saved4m DangerLandmark and assert owner-visible refusal, no stamina spend and grounded position.

| Check | Result | Evidence |
| --- | --- | --- |
| Contact fixture build after protected-handler correction | PASS5.35s | Saved/Logs/PFM14ClimbContactFixtureBuild.log |
| Pending-launch gameplay ordering build | PASS13.89s | Saved/Logs/PFM14ClimbContactPendingBuild.log |
| Real-input module dependency build | PASS29.11s | Saved/Logs/PFM14ClimbInputDependencyBuild.log |
| Final Editor build with readiness guard | PASS6.03s, no compiler warnings | Saved/Logs/PFM14ClimbReadinessBuild.log |
| Development Win64 game build | PASS39.67s, no compiler warnings | Saved/Logs/PFM14ClimbContactGameBuild.log |
| Native Contact / AutomaticClimb / Rules | PASS3/3,17.09s, raw severity0 | Saved/AutomationReports/Automation_M14ClimbFinalNative_20261013_043242869_a79e478d/index.json and run-summary.json |
| Actual W/F, server + one NullRHI client | PASS2/2,45.15s | Saved/AutomationReports/M14ManualAuthoredClimb1_20261013_043040380_8064358b/run-summary.json |
| Actual W/Space, server + one NullRHI client | PASS2/2,45.13s | Saved/AutomationReports/M14AuthoredClimb1_20261013_043326786_a468592e/run-summary.json |
| Guarded W/F, server + two NullRHI clients,75ms outgoing lag/1% loss confirmed | PASS3/3,54.03s | Saved/AutomationReports/M14ManualAuthoredClimb2_20261013_044212764_3d5b0e5f/run-summary.json |
| Same W/Space | PASS3/3,53.17s | Saved/AutomationReports/M14AuthoredClimb2_20261013_044457457_f7c822fc/run-summary.json |
| Rendered W/F,1280×720, uncapped requested, HUD1.5; tall refusal | PASS1/1,47.84s | Saved/AutomationReports/M14ClimbF720_20261013_044338683_72d84c28/run-summary.json |
| Rendered W/Space with same tall refusal | PASS1/1,46.47s | Saved/AutomationReports/M11AuthoredClimb720_20261013_044630969_93565aeb/run-summary.json |

This checkpoint has15 reported execution records across4 distinct selectors, not15 unique tests. Earlier one-client/native evidence precedes only fixture diagnostics/readiness and the rendered-only tall-wall branch; gameplay code and those headless climb actions stayed unchanged. The latest guarded two-client and rendered runs verify the final fixture. An extra successful diagnostic manual-two replay is retained but not included in the15-record checkpoint.

Six inspected PNGs are climb_80_top.png, climb_110_top.png and tall_wall_refusal.png in each of the two current `Saved/AutomationReports/ControlsUI/<render-run>/` directories. F shows stamina100→85→70; W/Space100→65→30. Tall refusals show readable text and unchanged stamina. Three further images per run capture spawn, approach and sign; they were generated but not re-inspected in this repair because geometry was unchanged. Health97/96/95 and stage sprint cues are trusted fixture coordination, not normal climb damage or a human test.

All checkpoint tests have zero test warnings/errors, process exit0 and no timeout/fatal/ensure/assertion. Network raw logs retain24 known warnings/14 experimental Python startup error lines per role; rendered26/14. Timestamp-normalized severity sets contain no new entries versus the prior authored headless/rendered checkpoints. Existing HLOD/editor-widget warnings and ToolsetDefinition/PythonTestRunner startup errors remain known issues; they are not silently counted as clean logs. Ordinary PrimalFrontier.log and PrimalFrontier_2.log were inspected for the reported session; fresh tests use unique absolute logs from their summaries.

Peak native working/private memory2.964/2.802GiB; final network per-process max1.801/1.791GiB; render max3.116/5.281GiB. Private-file/default-setting hash guards passed for451/452/457/459/460/462 files. No sustained FPS/stutter measurement or minimum16GB certificate.1440p remains deferred; actual host31.93GiB RAM and installed engine5.8.3 differ from the requested baseline. Installed packaged Server target limitation remains; these are uncooked server tests.

## Retained failures

- Initial fixture build failed C2248 for protected DoMove/DoJumpStart/DoJumpEnd calls. Use public movement/jump APIs in the native physics fixture; real keys are verified separately.
- Baseline Contact failed2 physical-landing assertions: both manual heights accepted but failed to land; automatic cases passed. `Automation_M14ClimbContactBaseline_20261013_042013562_15cfb751`.
- First shared-drive fix still failed2 assertions because grounded pre-movement cleared the queued drive. `Automation_M14ClimbContactFixed_20261013_042248097_076c3af4`. Pending-launch ordering repair then passed Contact, `Automation_M14ClimbContactPending_20261013_042505349_a74cae4b`,16.62s/raw0.
- Real-input build and immediate approach build failed LNK2019 for IPlatformInputDeviceMapper::Get. Explicit built-in ApplicationCore linkage resolved it; failed build logs remain.
- Initial impaired-network manual-two run failed server/Client1 pre-request timeouts; Client2 passed both heights. `M14ManualAuthoredClimb2_20261013_043436262_085bcee1`,123.76s. Client1 had3 saved-move warnings after server exit. Diagnostic replay passed3/3, `M14ManualAuthoredClimb2_20261013_043907610_10415a48`,53.99s. The original timing cause was not captured/proven. Readiness checks were hardened and the failed case replayed successfully; do not claim a diagnosed network-gameplay fix from this intermittent setup event. Final logs have no new warnings; both failed and successful reports are preserved.

## Replay and one human next action

Use no competing Unreal process, fresh isolated profiles/reports and an available port:

```powershell
./Scripts/RunNativeAutomation.ps1 -TestFilter 'PF.Traversal.Contact+PF.Traversal.AutomaticClimb+PF.Traversal.Rules' -Label M14ClimbFinalNative
./Scripts/RunEcologyAutomation.ps1 -TestCase AuthoredClimb -Players 1 -ManualClimb -Port 17998
./Scripts/RunEcologyAutomation.ps1 -TestCase AuthoredClimb -Players 2 -ManualClimb -SimulateLagLoss -Port 17998
./Scripts/RunEcologyAutomation.ps1 -TestCase AuthoredClimb -Players 1 -Port 17998
./Scripts/RunEcologyAutomation.ps1 -TestCase AuthoredClimb -Players 2 -SimulateLagLoss -Port 17998
./Scripts/RunControlsAutomation.ps1 -TestCase AuthoredClimb -ManualClimb -Resolution 720 -HUDScale 1.5
./Scripts/RunControlsAutomation.ps1 -TestCase AuthoredClimb -Resolution 720 -HUDScale 1.5
```

Immediate user action: in normal OpenWorld turn right at spawn, walk against the80cm CLIMB TEST block from flat ground, release W and press F with at least15stamina. Reply **CLIMB_F80 PASS** or **CLIMB_F80 FAIL + displayed refusal**. Current G10 contains the full remaining guide; this partial result does not accept other inputs/heights or fullM14. If an old save restores you elsewhere, return to camp or follow G10's optional standalone PF.Teleport250/-1100/100 reposition with explicit PASS. No editor/map save or repeat of twelve accepted checks is required.
