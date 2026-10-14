# Automatic jump-contact climb — October 13, 2026

**Historical short/prepaid mantle, now superseded by CONTINUOUS_CLIMB_M14.md.** Rising paid W+Space wall contact now attaches to the continuous custom mode; hold W to ascend, zero stamina falls. The ordinary20-stamina jump and automatic contact guards remain. New current gate G16, not the old cost/height instructions below.

The requested automatic short-ledge climb is implemented and technically verified. A real paid ground jump contacting a wall while rising can mantle a clear walkable ledge 45–120 cm above the original ground feet, within the existing 1 m reach. Hold forward and jump (W + Space, or left stick + A); F/right-stick remains the manual alternative. This does not implement continuous tall-wall scaling.

The server selects geometry and checks life, controller, modal UI, original height, capsule clearance, cooldown, weight and stamina. One automatic attempt is armed for 0.75 seconds by the first actual ground jump, not an airborne extra jump or walking off an edge. Default successful cost is 20 jump + 15 climb stamina. At 33 kg or more, jump/climb is refused; sprint/glide already stop above 30 kg. The acknowledged horizontal drive continues through normal swept falling physics after initial wall sliding, and clears on landing, cancellation, death or a new jump. Clients supply no target, origin or climb approval. Idle players incur no new recurring trace.

No source map, binary asset, personal save/profile or rendering setting changed. Native tests use disposable boxes; live tests use raised runtime primitive lanes in `/Game/PrimalFrontier/Maps/L_M7SurvivalArena`. The normal player level remains `/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld`.

## Final verification

All paths are project-relative. Reports are under `Saved/AutomationReports/`; matching unique logs are under `Saved/Logs/`. Successful records have exact selectors, exit zero, zero test-level warnings/errors and no timeout/assertion/fatal/ensure.

| Check | Result | Evidence |
| --- | --- | --- |
| Final production Editor build | PASS, 6.20 s, no compiler warnings | `PFAutoClimbLiveEditorBuild.log` |
| Final fixture-only Editor build | PASS, 6.09 s, no compiler warnings | `PFAutoClimbFloorEditorBuild.log` |
| Development Win64 game build | PASS, 32.58 s, no compiler warnings | `PFAutoClimbGameBuild.log` |
| Native `PF.Traversal.AutomaticClimb` + `PF.Traversal.Rules` | PASS 2/2, 17.03 s | `Automation_AutoClimbFinal_20261012_231728727_ea3c9df3/index.json`, `run-summary.json` |
| Uncooked dedicated server + one NullRHI client, `PF.Traversal.AutomaticLive` | PASS 2/2 role records, 45.03 s | `M1TraverseAutoClimb1_20261012_232143591_bb8cfd35/run-summary.json` |
| Server + two NullRHI clients, same selector, confirmed 75 ms outgoing lag / 1% loss on every role | PASS 3/3 role records, 53.01 s | `M1TraverseAutoClimb2_20261012_232323069_1f8a0250/run-summary.json` |
| Rendered standalone 1280×720, HUD 1.5, automatic live course | PASS 1/1, 44.56 s | `M11AutoClimb720_20261012_232428010_f4043e3f/run-summary.json` |
| Shared manual F path, server + one NullRHI client, `PF.Traversal.Live` | PASS 2/2 role records, 45.00 s | `M1TraverseTraversal1_20261012_232556093_05db5b1c/run-summary.json` |

These are **10 reported executions across four unique selectors**, including repeated network roles. Native coverage exercises actual physical landing/cost, too-tall original-ground height, obstruction, post-jump stamina, modal/weight/client refusal, airborne extra jumps, cancellation, old drive clearing and late dead-client acknowledgments. Live coverage drives real jump + forward without F, checks server/owner landing and cost, then existing sprint, weight, glide and peer replication. Manual F regression retains its shared physics path.

All three rendered PNGs were inspected: `ControlsUI/M11AutoClimb720_20261012_232428010_f4043e3f/ledge_top.png`, `overweight.png`, `gliding.png`. They show the first-person disposable course and readable HUD, not a full open-world or human usability certificate. Live fixture health markers are coordination signals, not climb damage.

Peak native process: 2.935 GiB working / 2.786 GiB private; network peak: 1.800 / 1.788 GiB; rendered peak: 3.126 / 4.924 GiB. Private-file hash guards and default settings remained unchanged (427, 428, 430 and 431 files across final live runs). NullRHI transport is process-paced at 120 FPS; rendering requests `t.MaxFPS 0` and `r.VSync 0`. No sustained FPS result was measured. The actual host reports 31.93 GiB RAM; this is not minimum-16GB certification. The user explicitly deferred 1440p.

Native raw/test severity is zero. Final network roles each have 21 raw warnings and 14 known startup Python errors, matching the established M6 baseline with no new normalized warning/error lines. Rendered has 26/14, matching the prior rendered traversal baseline. Test-level severity remains zero. Ordinary `PrimalFrontier.log` was inspected; these isolated runs write unique absolute logs instead. Installed engine identifies as 5.8.3 although the project requests 5.8.2. Packaged Server remains unsupported by this installed distribution; uncooked dedicated-server evidence is separate.

## Failures and recovery retained

1. Initial Editor build (`PFAutoClimbEditorBuild.log`, 26.04 s) failed C3535/C2440: the test fixture attempted `auto*` deduction from `TObjectPtr`. `.Get()` fixed the fixture; `PFAutoClimbEditorRetry.log` passed in 5.21 s.
2. `Automation_AutoClimbNative_20261012_230814735_6570d398` passed Rules but failed the new physical landing assertion. Failed-only first-landing replay `Automation_AutoClimbLandingRetry_20261012_230950331_2c3f43f5` still failed. A proposed overshoot explanation was not confirmed.
3. Bounded frame diagnostics in `Automation_AutoClimbDiagnostic_20261012_231151353_58d76f8b` showed upward launch but horizontal velocity stripped by the initial wall slide and not recovered under the climb air-control lock. Automatic-only server-approved horizontal drive fixed this through normal collision sweeps. `Automation_AutoClimbDriveRetry_20261012_231429693_757bf126` passed 2/2; the final native run also includes additional lifecycle/airborne guards.
4. Initial one-client run `M1TraverseAutoClimb1_20261012_231820438_2a0f690c` passed 2/2 but emitted one new pre-test unresolved relative movement-base warning. The disposable raised floor had not reached the client before teleport/correction. A fixture-only floor receipt barrier uses existing saved-move sprint intent before raised placement. Final one-client replay eliminates the warning, followed by the final two-client/rendered/manual runs above. No production network settings or RPC were added for this fixture.

## Reproduce / next actor

With no other Unreal instance open and an available UDP port:

```powershell
./Scripts/RunNativeAutomation.ps1 -TestFilter 'PF.Traversal.AutomaticClimb+PF.Traversal.Rules' -Label AutoClimbFinal
./Scripts/RunEcologyAutomation.ps1 -TestCase AutoClimb -Players 1 -Port 17998
./Scripts/RunEcologyAutomation.ps1 -TestCase AutoClimb -Players 2 -Port 17998 -SimulateLagLoss
./Scripts/RunControlsAutomation.ps1 -TestCase AutoClimb -Resolution 720 -HUDScale 1.5
./Scripts/RunEcologyAutomation.ps1 -TestCase Traversal -Players 1 -Port 17998
```

The user reported **CLIMB NOT TESTED — geometry unavailable** in the normal open world. Do not repeat the other accepted tests or count this as a human climb pass. A suitable existing clear ledge and at least 35 stamina allow W + Space without F; expect a single bounded mantle and normal movement afterward. F/right-stick costs 15 without the jump. No map editing is required from the user. Physical controller/audio stay untested; 1440p stays deferred. Providing accessible greybox traversal geometry is a possible next bounded AI world task, preserving save layout and existing assets.
