# Earned gathering timer lifecycle — M12

October 10, 2026. Bounded task: https://trello.com/c/uoZQxtUs. This is additional native Unreal verification, not a completed M12 or a human playtest.

## What changed

Added `PF.Progression.GatherLifecycle` in `Source/PrimalFrontier/Tests/PFProgressionGatherLifecycleTests.cpp`. Production gathering, progression, rendering, networking, assets and settings are unchanged.

The disposable Game world starts with zero XP and an empty inventory. It uses the loaded wood definition, normal bag limits, barehand gathering, the normal 0.5-second cooldown, three hits with two wood per hit, 20-second node regrowth and the original 1,800-active-second XP window. Movement and hunger/thirst drain are disabled only to isolate the timer test; no XP/item grants, seeded progression, direct clock writes, shortened timers or private save edits are used.

The test checks:

1. Three owned trace/gather actions yield six wood and 15 XP, then deplete the node. Immediate cooldown and depleted-node requests cannot duplicate either.
2. Forty paused Unreal world ticks preserve game time, the depleted node deadline, exact earned credit duration and inventory. Unpausing and ticking normally refills three hits after the default regrowth deadline, without creating items or XP.
3. Two further gathers exhaust the five-credit wood budget at 25 XP. Later valid gathers still yield finite wood. Natural node refill never resets the reward window.
4. Ordinary world ticks reach the final second of the default reward window, then pass its expiry. Capturing an expired window removes its credits without adding XP or changing knowledge/craft history.
5. The next accepted normal gather earns one new five-XP credit. Immediate cooldown refuses duplication; a second accepted gather ages the new window rather than extending it. Final state is 35 XP and exactly 18 normally gathered wood in one nonperishable stack.

## Exact evidence

All paths below are relative to the project-local `Saved` directory. Both successful runs reconcile the exact requested selectors with engine reports, have zero test warnings/errors, engine and strict runner exit codes 0, and no timeout. Actual raw logs were inspected: zero warning/error lines and no assertion, fatal or ensure failures.

| Gate | Evidence | Result |
| --- | --- | --- |
| Initial Editor build | Logs/PFM12GatherLifecycleEditorBuild.log | Succeeded, 15.95 seconds; no compiler warnings |
| Corrected Editor build | Logs/PFM12GatherLifecycleFixEditorBuild.log | Succeeded, 5.61 seconds; no compiler warnings |
| Failed first native attempt | AutomationReports/Automation_M12GatherLifecycle_20261010_045058614_eb9d5bf1/index.json | One failed fixture assertion; retained, 17.08 seconds |
| Failed-only replay | AutomationReports/Automation_M12GatherLifecycleRetry_20261010_045229915_e0a53fb1/index.json | GatherLifecycle 1/1 passed, 18.13 seconds; working/private peaks 3.028/2.865 GiB |
| Focused regression | AutomationReports/Automation_M12GatherLifeRegression_20261010_045331946_8d82a1c5/index.json | GatherEvents and GatherClock 2/2 passed, 16.25 seconds; working/private peaks 3.023/2.905 GiB |
| Game Development Win64 | Logs/PFM12GatherLifecycleGameBuild.log | Succeeded, 23.59 seconds; no compiler warnings |

Each automation directory also contains its generated `run-summary.json`. Matching current raw logs are `Logs/PFAutomation_<run suffix>.log`, with the same complete run name after `PF`. Ordinary `Logs/PrimalFrontier.log` still has its older October 9, 04:53 UTC timestamp and is not evidence for these launches.

The first attempt failed only `Ordinary ticks reached deadline`: its loop allowed 2,000 requested one-second frames, while installed Unreal `AWorldSettings::FixupDeltaSeconds` clamps large frame deltas. Earlier real gathering, pause and regrowth assertions passed. The fix is exclusively in the test helper: use ordinary maximum 0.2-second frames and a 20,000-iteration safety bound. Engine/world settings and default gameplay timers were not modified. The failed engine process exited 0, but the strict report runner correctly exited 1; do not reinterpret that run as passed.

## Replay

Level: NONE required. The native test creates a disposable `EWorldType::Game` world, using `/Engine/Maps/Entry?game=/Script/PrimalFrontier.PFSurvivalGameMode` as the game-mode URL. It does not load or save a project map. Mode: one `UnrealEditor-Cmd` process with NullRHI, no rendered clients.

Start here: close competing Unreal processes, then build `PrimalFrontierEditor Win64 Development` for `PrimalFrontier.uproject`.

```powershell
./Scripts/RunNativeAutomation.ps1 -TestFilter PF.Progression.GatherLifecycle -Label M12GatherLifecycle
./Scripts/RunNativeAutomation.ps1 -TestFilter 'PF.Progression.GatherEvents+PF.Progression.GatherClock' -Label M12GatherLifeRegression
```

Run the focused regression only after the exact lifecycle test passes. Inspect actual generated index/summary/raw logs, then build the Game Development Win64 target. A clean report is required; failures stop this slice for diagnosis and replay.

## Limits and next step

Installed engine reports 5.8.3; the requested baseline is 5.8.2. The host has 31.93 GiB RAM. Memory values are sampled individual process peaks, not whole-machine totals. This native fixture accelerates simulated game time and cannot certify 30 minutes of wall-clock play, human pacing, sustained FPS/stuttering, controller feel or minimum-16-GB behavior. No rendered feature changed, so no new screenshot or rendered/network certificate is claimed. Existing private one-/two-client restart evidence remains separately recorded in `GATHER_EVENTS_M12.md`.

Full M11/M12 and the existing Personal usability, combat/pacing, controller and rendered persistence gates remain open. Immediately update this bounded Trello card with the completed technical evidence before choosing the next independently actionable objective. No new human task is necessary for this test-only slice.
