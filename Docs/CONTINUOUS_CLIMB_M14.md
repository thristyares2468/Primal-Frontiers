# M14 — Continuous wall climbing

October14,2026: replaces the prepaid full-height ascent described in STAMINA_CLIMB_M14.md. The user's `CLIMN_STAMINA PASS` accepts the earlier requested4mF check only. It is not acceptance of this changed behavior. New ordinary gate: G16 / WALL_CLIMB.

## Current behavior

- Approach a solid wall and press **F** to attach. **W/S** climbs up/down; **A/D** moves sideways. Press **F** again to drop. The existing rising **W+Space** paid-jump contact path also attaches; the normal jump still costs20 stamina.
- Moving drains **12 stamina/second**, holding drains **4/second**. There is no recovery while attached, even when the configured recovery delay is zero. No whole-height prepayment or arbitrary height ceiling. At zero, detach and fall under normal gravity; ordinary ground recovery resumes after its delay.
- Default climb speed is140cm/s before carry slowdown. Server checks live/owned pawn, actual nearby Pawn-blocking nonphysics wall, valid finite tuning, at least1 stamina, menus, carry jump threshold and0.5s reattach cooldown. Climbing above the jump weight cutoff is refused/cancelled. Sprint/glide cannot run on the wall.
- Movement sweeps the real capsule. Missing wall contact, death or menu cancellation releases it. Ceilings still block movement and consume stamina; empty stamina drops rather than bypassing collision. Near the top, a short mantle only accepts a walkable surface within65cm of current feet and clear upward/across capsule sweeps.
- First-person camera and existing replicated full body remain. Native `State.Traversal.Climbing` tags attached state. HUD stamina and below-crosshair control/result messages show the actual state. No new art, animation, equipment, skills or dependencies.

## Authority and compatibility

`UPFTraversalMovement` implements one approved custom CharacterMovement mode. The server derives contact normal and authorizes attachment. F sends intent only, not a location, cost, height or completion request. W/S/A/D use the existing saved-move acceleration path. Only the authority spends stamina. A reliable sequenced server-owner acknowledgment enables prediction; UE replicates custom movement/pose for other players. Client direct `TryClimb`/`BeginWallClimb`, forged unapproved custom mode, invalid runtime tuning and late acknowledgment on a corpse are tested/refused.

Old serialized height/cost properties and reflected ballistic acknowledgment remain inert for compatibility; they no longer tune or launch climbs. Tune movement component WallClimbSpeed/WallClimbStaminaPerSecond/WallHoldStaminaPerSecond. Active climbing is transient; save/load retains ordinary player vitals/position but never promises restoring an attached wall mode. No save version or resource-layout migration.

## Reachable saved fixture

Level `/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld`. Turn right/east from camp.4m wall centre `(1850,-1100,200)`, west faceX1700;10m wall centre `(2450,-1100,500)`, west faceX2300. Existing low blocks and second network4m lane `(2250,-3000,200)` remain. Sign `(1550,-850,200)` now states F attach/drop, W/S/A/D and4/12 drain/empty fall.

Supported `AddClimbBudgetFixtures.py` updates only the exact known old sign text, refuses unknown sign/geometry and preserves all83 other actors plus all4 fixtureGUIDs (87total). Setup `Saved/AutomationReports/M14WallSign_20261014/setup.json`:1 text mutation/reload verified. Repeat `repeat.json`:0 mutations/same identities. Only the sign's external package and engine-saved level header changed; no nodes/spawners/wall dimensions/layout changed.

## Verified checkpoints

All report roots below are under `Saved/AutomationReports`; matching build/runtime logs are under `Saved/Logs`. Native tests use real physics and disposable worlds; live tests use real input in the actual saved OpenWorld, disposable profile/slot guards, no bag grants. Runner flags FullHeightClimb/ManualClimb are retained labels; AuthoredLive now means the continuous4m/low-budget10m sequence.

| Run | Result and scope | Time / sampled peak GiB working, private |
| --- | --- | --- |
| PFM14WallCooldownBuild.log | EditorPASS, source discovery forced |32.64s |
| Automation_M14WallRegression_20261014_015342832_7e8640bf |4/4: Contact, AutomaticClimb, Rules, StatRuntime; raw/test severity0 |39.17s /2.964,2.863 |
| PFM14WallAuthorityBuild.log | EditorPASS after additional negative tests |6.34s |
| Automation_M14WallAuthority_20261014_020254836_c86c1fb5 |Contact+Rules2/2; forged mode, tag, invalid tuning, late corpse ACK; raw/test severity0 |17.21s /3.004,2.864 |
| M14HeightAuthoredClimb1_20261014_015818551_82247959 |2/2 server+one client; F hold/W4m clear top,24-stamina10m partial climb/zero fall, owner authority refusal |45.85s /1.733–1.790,1.628–1.786 |
| M14HeightAuthoredClimb2_20261014_015921746_e185ea00 |3/3 server+two clients,75ms/1% outgoing; same sequence plus each sees remote climb |54.06s /1.736–1.800,1.633–1.792 |
| M1TraverseAutoClimb1_20261014_020600536_bcd68451 |2/2 actual server/client automatic contact + sprint/overweight/jump/glide regression |44.67s /1.728–1.796,1.639–1.784 |
| M14Height720_20261014_020343888_d8b8da5e |Rendered AuthoredLive1/1,720p/HUD1.5, five real HUD/physics PNGs |45.44s /3.186,4.855 |
| PFM14WallFeedbackBuild.log |EditorPASS, fall/end feedback distinguished from attach refusal |43.29s |
| M14Height720_20261014_020937583_f8bf70af |Rendered replay1/1 asserts exact ended feedback; PNG zero-stamina fall inspected |44.92s /3.181,4.545 |
| M14HeightAuthoredClimb2_20261014_021250394_dae302cb |Final feedback-source impaired server+two clients3/3,75ms/1%, owner ended message asserted |57.17s /1.731–1.797,1.627–1.788 |
| M12StatsUI720_20261014_021102531_1b0a9472 |Existing upgraded-stat menu/real10m climb1/1,720p/HUD1.5 |44.63s /3.099,5.307 |
| PFM14WallCaptureBuild.log |EditorPASS, stats screenshot delayed one HUD tick only |6.64s |
| M12StatsUI720_20261014_021517837_3dbcafbc |Capture-only stats replay1/1;10m top screenshot inspected,60/140 and ended feedback |45.45s /3.090,5.209 |
| PFM14ContinuousWallGameBuild.log |Development Win64 GamePASS, new source discovered |46.04s |

Native Contact also checks80/110/400/1000cm real landings,20m partial exhaustion/ground landing,4/s holding without recovery,12/s sideways, descent/F drop, ceiling/contact/death/menu/weight guards. StatRuntime checks upgraded maximum survives allocation/save/respawn and real continuous10m climb. Automated passes do not replace ordinary WALL_CLIMB or a physical controller observation.

Rendered holding/ascending/top/low-budget/fall PNGs live under `Saved/AutomationReports/ControlsUI/<rendered Run>`. First run showed98→85→69/100 and22→0/100, readable wrapped controls atHUD1.5. Final replay shows **Climb ended: Stamina empty; fell off wall** with0/100; a completed climb is also labelled ended instead of a new attachment result. Upgraded-stat actual10m UI test passed; its first capture had a one-frame-old climbing HUD, so delay only the final screenshot after HUD tick. Replay inspected **Climb ended: Reached clear wall top**,60/140 and standing body shadow on top. All rendered runs request uncapped FPS; private/default-settings guards unchanged. Final Editor6.64s/Game46.04s pass without compiler warnings. Four distinct native suites plus2negative replay records,10network role records and4rendered records =20successful final execution records within the listed scope. 1440p remains user-deferred. Packaged Server remains unsupported by the installed distribution. Installed engine reports5.8.3 rather than requested5.8.2; unchanged. Actual host31.93GiB RAM/RTX4060 8GB; no competing rendered clients or quality increase. Sampled memory/short startup FPS are not minimum-spec or sustained stutter certification.

Raw runtime severity reviewed against `Saved/Logs/M12StatsUI720_20261013_073212143_3c179b6c.log` after stripping timestamp/frame prefixes:0 added unique lines in both one-client roles, both3-role impaired runs, all4rendered runs and both AutoClimb roles. Runtime warning counts24OpenWorld headless,26rendered,21arena;14 Python errors each. Existing warnings concern EditorDataStorage factories/HLOD classes; Python errors concern experimental engine ToolsetDefinition/PythonTestRunner startup. Commandlet sign updates summarize0errors/1known missing Recast warning. These are baseline findings, not clean logs or repaired engine issues. Ordinary PrimalFrontier.log is the older user-editor session, with shutdown HTTP telemetry warning/no fatal/ensure; current test evidence uses unique logs, not that older session as a pass.

## Failures retained and narrow repairs

First Editor31.19s passed. Native/HUD rebuild failedC2027 missing movement include; include repair7.07s passed. Automation_M14WallPhysics_20261013_232655748_2ce7c7fc passedAutomaticClimb/Rules but Contact exact-zero assertion failed; clamp final epsilon residual only. PFM14WallZeroBuild5.26s and Automation_M14WallZeroReplay_20261013_232951463_7daee569 Contact1/1 passed/raw0 (17.08s,2.915/2.798GiB).

Live fixture build errorsC1075 parentheses thenC2280 weak-pointer boolean were repaired as syntax/IsValid only; PFM14WallWeakPtrBuild6.17s passed. First one-client M14HeightAuthoredClimb1_20261013_233550168_de6686af failed2/2 at second phase:4m climb passed, fixture requested reattach0.04s after landing, correctly refused by production0.5s cooldown. Keep failed reports/three saved-move warnings at subsequent stalled disconnect. Add0.75s fixture phase wait; do not weaken gameplay. Final one-/two-client replays above pass. No RHI crash or gameplay ensure/fatal in successful runs. Known24OpenWorld warning/14 experimental-Python error lines per live role remain baseline findings, not warning-free results.

## Changed files and publication

36 scoped files published as `9a8128ca634dfae466de50542dd8f1ad93de0f96` to origin/master,2LFSobjects/18KB uploaded; remote equality/clean tree verified. Coauthor Codex GPT-6.1 Sol. Subsequent receipt edits are documentation only.

- Survival: PFPlayerSurvivalComponent.cpp; PFSurvivalPlayerController.cpp/.h; PFSurvivorCharacter.cpp/.h; PFSurvivorTraversal.cpp; PFTraversalInput.cpp; PFTraversalMovement.cpp/.h; new PFWallClimbMovement.cpp.
- Tests: PFAuthoredClimbLiveTests.cpp; PFAutomaticClimbTests.cpp; PFClimbContactTests.cpp; PFPlayerStatsMenuTests.cpp; PFPlayerStatsTests.cpp; PFTraversalLiveTests.cpp; PFTraversalTests.cpp; new PFWallClimbFixture.h.
- Scripts: AddClimbBudgetFixtures.py; RunEcologyAutomation.ps1.
- Content: L_PrimalFrontier_OpenWorld.umap and existing sign external actor C/UB/MJPOCWBOF45G23LTPORY94.uasset only.
- Docs: ARCHITECTURE, AUTOMATIC_CLIMB_M1, CONTINUOUS_CLIMB_M14, CURRENT_STATE, DECISIONS, MILESTONES, NETWORKING, PLAYER_STATS_M12, PLAYTEST, PLAYTEST_RESULTS_2026_10_14, STAMINA_CLIMB_M14, TRAVERSAL_M1, TRELLO_SYNC, TRELLO_TEST_GUIDES.

Trello technical ru1gvQJ5 Done/complete confirmed; [new Personal1 WALL_CLIMB](https://trello.com/c/GsgHT0o0) ToDo/incomplete and parent1tI62BmQ Personal1Doing/incomplete contain the fullG16 guide. Parent history retained verbatim; old4mF gate7zjNOKJJ Done/complete. No new manual/fullM14 acceptance; prior unrelated assignments/deadlines/acceptances preserved.

## Remaining limits and next action

This is greybox wall movement, not complete climbing animation, curved corner traversal, ceiling crawling, climbing jumps or a full traversal art system. A/D can leave the finite wall and fall. Empty stamina does not auto-glide. Controller mappings retain right-stick click attach/drop and movement stick, but physical device usability is unverified. Do one normal rendered G16 check; leave Copper/stat/controller/audio/performance gates separate and do not expand later playable tiers before their acceptance.
