# M1/M3 carry weight and traversal - October 12, 2026

Current October14 climbing supersedes the older ledge/prepayment sections: F attach/drop, W/S vertical/A/D sideways,12/s move/4/s hold, no wall recovery, exact-zero fall. [CONTINUOUS_CLIMB_M14.md](CONTINUOUS_CLIMB_M14.md) records current proof and G16 WALL_CLIMB. Sprint/weight/glide accepted earlier remain accepted; old4mF CLIMN_STAMINA PASS does not accept new continuous behavior.

## October13 automatic-climb addition

Hold forward and jump into a clear45–120cm ledge to trigger one server-approved mantle without F. Default20jump+15climb stamina; original-ground height, collision and existing weight/life/modal guards apply. F/right-stick manual alternative retained. Editor/Game/native/networkone/two/render720/manualF regression all pass; [AUTOMATIC_CLIMB_M1.md](AUTOMATIC_CLIMB_M1.md) contains exact10records/fourselectors, failures and limits. User CLIMB remains NOT TESTED because of geometry; older evidence below remains historical.

Requested functions are implemented and technically verified. The supplementary 1440p check remains pending by explicit user choice. Physical-controller, sustained-performance and remaining full-milestone manual gates are separate.

## Playable behavior

| Action | Keyboard | Controller | Default rules |
| --- | --- | --- | --- |
| Sprint | Hold Left Shift | Hold left-stick click | Grounded, living, stamina available, at/below carry limit; 1.5x walking speed, 10 stamina/sec while moving. |
| Short ledge climb | F | Right-stick click | Grounded, living, 15 stamina, below jump cutoff; server finds walkable ledge over45cm and at most120cm high within1m, checks capsule/overhead clearance and supplies bounded impulse. |
| Glide | V toggles while airborne | D-pad Up outside menus | Living, stamina available, at/below carry limit; 8 stamina/sec, reduced gravity, descent capped220cm/sec. Landing clears intent. |

Contextual D-pad menu navigation remains. Menus/death cancel traversal; death clears pending launch forces and refuses late climb acknowledgment. Climb means short ledge assistance, not unrestricted wall climbing. Glide uses existing first-person presentation; final equipment/animation art is outside this scope.

Player bags have eight hard slots and default30kg soft carrying limit. Any excess progressively slows movement and immediately blocks sprint/glide; >=110% of limit (33kg default) blocks jump/climb. Speed fraction is `max(0.1, 1 / max(1, weight / limit)^2)`. Removing weight restores eligibility. CharacterMovement exposes threshold/speed/stamina tuning. HUD shows traversal/overweight state; inventory shows kg.

Published slices established999,000 quantity ceiling, stackable default resources/food/water/cord at that ceiling, nonstackable gear1, per-unit weight, hard60kg chests and strict malformed/duplicate/unknown/overflow/expiry validation. Large stacks do not mean lightweight items/unlimited slots. See [LARGE_STACKS_M3.md](LARGE_STACKS_M3.md) and [CARRY_POLICY_M3.md](CARRY_POLICY_M3.md) for transaction/real-file/compatibility evidence.

## Changed systems and authority

- `PFTraversalMovement.h/.cpp`: CharacterMovement saved-move sprint/glide intent flags, noncombination across different intent, replay, derived carry speed, authority-only stamina and temporary falling physics.
- `PFSurvivorTraversal.cpp`: server-derived geometry, clearances, cooldown and launch; no client climb target/speed/weight authority. Simulated-proxy-only glide flag supplies remote gravity without exposing private contents.
- `PFTraversalInput.cpp`, controller/gamepad input/HUD: actual Shift/F/V bindings, controls help, modal cancellation and state feedback.
- Survivor/template constructor signatures use supported default movement-subobject replacement; owner arms and replicated full body retained. Inventory capacity fields replicate owner-only for matching prediction. Save schema unchanged.
- `PFTraversalTests.cpp`/`PFTraversalLiveTests.cpp`: disposable native/runtime primitive fixtures, actual input delegates, physical movement and real server/client roles. No map saved/modified on disk.
- Existing controls/ecology runners gain opt-in traversal. Headless network transport uses process-only120FPS startup pacing; rendered remains uncapped. Offscreen diagnostic restricted to traversal; personal settings untouched.

No new plugin/dependency, external art, MCP server, third person or binary asset change. Related help/docs/test guides updated. Live runs hash-guard private saves/default rendering preferences.

## Final verification

Paths below are project-relative. Each report is under `Saved/AutomationReports/`. Passing runs require exact selectors, successful exits, zero test warnings/errors, no timeout/fatal/ensure/assertion.

| Check | Result | Evidence |
| --- | --- | --- |
| PrimalFrontierEditor Development Win64 | PASS34.00s; no compiler warnings | `Saved/Logs/PFTraversalRemoteEditorBuild.log` |
| PrimalFrontier Development Win64 | PASS65.98s; no compiler warnings | `Saved/Logs/PFTraversalGameBuild.log` |
| Native regression | PASS5/5,25.27s | `Automation_TraversalFinal_20261012_062106873_9ff10b42/index.json`, `run-summary.json` |
| Rendered1280x720/HUD1.5, uncapped | PASS PF.Traversal.Live1/1,55.13s | `M1Traverse720_20261012_061327199_4d42383d/run-summary.json` |
| Uncooked dedicated server + one NullRHI client | PASS2/2 role records,47.99s | `M1TraverseTraversal1_20261012_062302574_4d90762b/run-summary.json` |
| Dedicated server + two NullRHI clients, actual75ms outgoing lag/1%loss every role | PASS3/3 role records,59.96s | `M1TraverseTraversal2_20261012_062553693_2cee6314/run-summary.json` |
| Windowed2560x1440 | FAILED to reach readiness; no gameplay verdict | `M1Traverse1440_20261012_062834334_d6d55622/run-summary.json` |
| Offscreen2560x1440 | FAILED to reach readiness; no gameplay verdict | `M1Traverse1440_20261012_063359730_fb6b08b3/run-summary.json` |

Five exact native passes: **PF.Input.Gamepad, PF.Inventory.CarryPolicy, PF.Inventory.WorldTransfers, PF.Survival.Lifecycle, PF.Traversal.Rules**. Rules cover actual sprint/stamina, first excess slowdown/refusal,110%jump cutoff, unload recovery, saved-move capture/replay/combine, glide descent/cost/depletion/landing, real low-ledge landing, obstruction/modal/client-authority refusal, death pending forces and late dead-client acknowledgment.

Live tests exercise bound Shift/F/V, real owned movement requests and server stamina/velocity, overweight sprint/jump refusal, unloading, ledge landing and glide. Final two-client test additionally requires both clients to observe the other simulated proxy's glide/gravity. This is bounded transport/movement verification, not restart/extended soak/full rendered multiplayer certification. NullRHI process-only120FPS pacing avoids startup saved-move backlog; rendering requests `t.MaxFPS 0`/`r.VSync 0`.

Three corrected PNGs inspected at `Saved/AutomationReports/ControlsUI/M1Traverse720_20261012_061327199_4d42383d/`: **overweight.png, ledge_top.png, gliding.png** show readable feedback and actual ledge top. Fixture Health99-94 synchronizes phases; it is not traversal damage. This rendering revision precedes final remote/death safeguards; unchanged UI/input plus later native/network results verify those additions within their scope.

Logs: `Saved/Logs/M1Traverse720_20261012_061327199_4d42383d.log`; `Saved/Logs/PFM1TraverseTraversal1_20261012_062302574_4d90762b{Server,Client1}.log`; `Saved/Logs/PFM1TraverseTraversal2_20261012_062553693_2cee6314{Server,Client1,Client2}.log`; `Saved/Logs/PFAutomation_TraversalFinal_20261012_062106873_9ff10b42.log`. Conventional `PrimalFrontier.log` checked but remains prior editor exit; these isolated launches deliberately use unique absolute logs.

Native raw severity0. Rendered logs have26warnings/14known experimental-Python errors matching established rendered baseline with zero new normalized categories. Final network logs each have21warnings/14known startup/Python errors; all five match clean network baseline with zero new categories. These raw messages are recorded, not hidden. No new gameplay fatal/ensure/assertion.

## Retained failures and corrections

- Initial Rules fixture remote controller prevented physical movement:7assertions failed `Automation_TraversalRules_20261012_054610762_fad76978`. Local fixture controller fixed it; failed-only replay `Automation_TraversalRulesRetry_20261012_054858232_6b2f8927` passed, followed by final5-test regression. Three C4305 test float literals corrected/rebuilt clean.
- Initial720p test passed but screenshots were stale. Frame settling/requested HUD-scale correction produced the three inspected captures above.
- Initial network timed out after held-jump/teleport/view issues and startup backlog (`M1TraverseTraversal1_20261012_055522343_4a1b36f3`). Follow-up fixture conditional prevented F (`..._060059641_8c71a820`); repaired independent condition. Next gameplay pass had41startup movement warnings (`..._060538938_2297408c`). Engine source showed startup GameUserSettings resets t.MaxFPS; process-only startup FrameRateLimit120/local pacing eliminated them. Clean replays and final hardening runs passed.
- Both1440p attempts stayed1-2FPS before readiness. Runner stopped only its owned process at180.94s/windowed and180.82s/offscreen; neither completed an automation test report. Offscreen did not isolate cause. This is unresolved rendering/performance failure, not demonstrated gameplay crash. User chose **Leave the1440p check pending**; no repeat/application closure is inferred.

## Memory, hardware and gaps

Native peak3.002GiBworking/2.911private. Corrected720p3.230/5.413GiB;380private files/preferences unchanged. Final one-client maximum1.785/1.779GiB; finaltwo-client1.788/1.774GiB perprocess,381/382private files/preferences unchanged. These are sampled per-process peaks, not summed host use.

Windowed1440p4.640/5.832GiB; offscreen4.415/5.757GiB,384/385private files/preferences unchanged. Host31.93GiBRAM/Ryzen5900X. Read-only nvidia-smi reports **RTX4060/8188MiBVRAM**, differing from supplied4060Ti16GB reference. After Unreal exit, totalGPUmemory remained about6.5GiB used/42%utilization; Apex remained open at about7.03GiB working memory. Contention plausible, not proven sole cause; settings/other apps untouched. Installed engine is5.8.3 in UE_5.8, differing from requested5.8.2.

No sustained uncapped FPS,16GBminimum-spec, physical controller/hot-plug or full human traversal certificate. Installed engine packaged Server target remains unavailable; uncooked dedicated server verified. Existing accepted personal checks retained; remaining gates open. Next diagnostic later, when user confirms available GPU, is same isolated windowed1440p run. No next milestone started.

## Repeatable commands

Close Unreal before isolated runs. Do not retry pending1440p until GPU availability confirmed. Repository PowerShell:

```powershell
./Scripts/RunNativeAutomation.ps1 -TestFilter 'PF.Traversal.Rules+PF.Input.Gamepad+PF.Survival.Lifecycle+PF.Inventory.CarryPolicy+PF.Inventory.WorldTransfers' -Label TraversalFinal
./Scripts/RunControlsAutomation.ps1 -TestCase Traversal -Resolution 720 -HUDScale 1.5
./Scripts/RunEcologyAutomation.ps1 -TestCase Traversal -Players 1 -Port 17995
./Scripts/RunEcologyAutomation.ps1 -TestCase Traversal -Players 2 -SimulateLagLoss -Port 17995
# Pending, only after GPU availability confirmed:
./Scripts/RunControlsAutomation.ps1 -TestCase Traversal -Resolution 1440 -HUDScale 1.5
```

Native: no saved level required. Rendered: `/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld`, one-player standalone. Network: `/Game/PrimalFrontier/Maps/L_M7SurvivalArena`, dedicated/NullRHI. Runners create isolated profiles/runtime primitives/reports, enforce clean exact verdicts/protect private files. Timeout/missing report/test warning or error/unexpected selector/private guard change is failure. G4 in `Docs/TRELLO_TEST_GUIDES.md` is the separately pending physical-controller guide.
