# Milestone 2 — verified 2026-09-25

**Gate passed for the supported greybox scenario:** Editor build, seven regression tests, focused expiry deadline test, rendered solo manual playtest, one-client and two-client NullRHI network checks. No packaged Shipping/Server build or rendered two-client RHI result is claimed. Stop here; Milestone 3 has not begun.

Continuation authorized after the recorded Milestone 1 gate. Baseline Editor build succeeded (up to date, 1.98 seconds); Git was clean at `ee05bfe`. Plan: extend replicated vitals with hunger/thirst and exposure, add deterministic server drain/threshold effects and validated placeholder recovery, extend HUD/PF hooks, then run focused regression, solo manual playtest and low-memory network tests. Use a separate primitive M2 map. Do not start Milestone 3 before this gate passes.

Commits for this work include the milestone number and `Co-authored-by: Codex GPT-6 Astra <codex@openai.com>`. Existing Git identity remains the primary author; unrelated changes are excluded.

## M2 implementation and evidence

Changed gameplay files under `Source/PrimalFrontier/Survival`: `PFPlayerSurvivalComponent.h/.cpp`, `PFSurvivalPlayerController.h/.cpp`, `PFSurvivalHUD.h/.cpp`; added `PFSurvivalHazard.h/.cpp` and `PFRecoveryPickup.h/.cpp`. Changed `Tests/PFSurvivalComponentTests.cpp`; added `PFSurvivalNeedsTests.cpp` and `PFSurvivalEnvironmentTests.cpp`. Tooling changes: `PrimalAgentToolsRuntime/Private/PFCommands.cpp` and new `Private/Tests/PFSurvivalNeedsLiveTests.cpp`. Added `Scripts/SetupSurvivalMilestone2.py` and the single new `Content/PrimalFrontier/Maps/L_M2Survival.umap`. Documentation covers architecture, decisions, milestone evidence, Git attribution, command interface, M2 mechanics, food research and future technology progression. No existing Content asset is modified.

- Editor builds succeeded without compiler warnings: initial needs slice 35.03 s, integration 120.29 s (memory pressure while an Editor reopened), fixture refinement 6.36 s, live test 7.41 s. The reopened All Saved editor was closed normally before linking.
- `Saved/AutomationReports/M2Needs/index.json`: 3 passed, 0 failed/warnings. `M2Regression/index.json`: 7 passed, 0 failed/warnings: `PF.Survival.Component`, `.Needs`, `.Environment`, `.Lifecycle`, `PF.PrimalAgentTools.CommandArguments`, `.MissingSystemsAreBlocked`, `.TeleportAndRuntimeReset`. Logs: `PFM2Needs.log`, `PFM2Regression.log`.
- Computer Use manual play on `L_M2Survival`: observed food/water drain, E consumed the ration and restored reserves to 100, empty reserves reduced Health to zero, death HUD appeared, and respawn reset reserves. Real hazard overlap displayed Exposure 100%, caused damage/death; a later controlled entry/exit retained the same living pawn at Health 87 and restored Exposure 0. The exit observation temporarily used `slomo 0.1` and restored `slomo 1`; no settings or map were saved. Logs: `PFM2Manual.log`, `PFM2ManualExit.log`.
- `PF.Help`, `PF.SetHunger`, `PF.SetThirst`, `PF.SetExposure`, `PF.RecoverNeeds` passed in the actual game. Fixed recovery changed 20/30 to 55/65 before ongoing drain. Reports: `PF_M2ManualExit_20260922T231620_AC98172A40D388872990DBA71048263D.json`, `PF_M2ManualHooks_20260922T231651_6503A18F42786CA5B23CB7A568FAFC89.json` under `Saved/AutomationReports`.
- Engine screenshot artifacts: `Saved/Screenshots/WindowsEditor/ScreenShot00001.png` and `ScreenShot00002.png`. The latter captured the teleport transition before the next exposure update and is motion blurred; it is not evidence of the final zero-exposure HUD. The live Computer Use capture and exposure log at 2026-09-22 23:16:21 UTC confirm exit.
- Setup issue resolved: `SaveMap` created M2 but left M1 active. Restored only this session's unintended M1 write from Git; populated the new M2 map using Editor APIs. Corrected setup to `NewLevelFromTemplate` plus an active-map guard. `PFM2SetupRepair.log` confirms only M2 was saved; Git reports no modifications to existing Content assets.
- Manual gameplay sampled 29–30 FPS; startup/capture stalls were transient. System overlay ranged approximately 13.1–15.4 GB RAM across the two days; resumed rendered process was 2.30 GiB working set / 5.21 GiB private bytes. With two NullRHI network processes starting, available physical RAM fell to about 0.77 GiB. No inference of gameplay failure is made from stutter alone.

- Food-expiration refinement builds passed (26.34 s, 7.05 s and final fixture build 6.22 s). `M2ExpiryRegression/index.json` repeats the seven regression tests with 7 passed, 0 failed/warnings. `M2ExpiryDeadline/index.json` passes the focused environment test with cleanup deliberately disabled until after the expired-consumption rejection check.
- `PFM2FoodManual.log`: observed the ration's visible freshness countdown in the rendered game; accelerated the disposable session with `slomo 10` and observed all three pickups expire and disappear (server expiration logs 2026-09-22 23:25:31 UTC). `ScreenShot00003.png` shows the countdown. This is accelerated expiration verification, not a normal-speed performance benchmark.
- Original one-client NullRHI network gate passed: `M2Server1/index.json` and `M2Client1/index.json`, each 1 success, 0 failures/warnings for `PF.Survival.NeedsLive`. Both reached TestExit with status 0. This run predates the replicated food-expiration assertion; final results follow below. A later `PFM2ExpiryServer1` setup was intentionally stopped before connecting a client while the user used the rendered game; it has no acceptance result.
- User-requested Palworld/ARK food research and original proposed spoilage/preservation rules are in [FOOD_AND_PRESERVATION.md](FOOD_AND_PRESERVATION.md). Only M2 world-food expiration is implemented; cooking, inventory and storage remain behind their later gates.

## Final gate, 2026-09-25

- `PrimalFrontierEditor Win64 Development` build succeeded, up to date in 2.11 s, no compiler warnings. Rotating evidence: `C:/Users/jackh/AppData/Local/UnrealBuildTool/Log.txt`.
- Final one-client expiry/needs run: `Saved/AutomationReports/M2ExpiryServer1Verified/index.json` and `M2ExpiryClient1Verified/index.json`, each **1 passed, 0 failed, 0 warnings**. Corresponding logs: `PFM2ExpiryServer1Verified.log`, `PFM2ExpiryClient1Verified.log`; both exit status 0 at 05:19:10 UTC.
- Two-client run on localhost port 7783: `Saved/AutomationReports/M2TwoClientServer/index.json`, `M2TwoClient1/index.json`, `M2TwoClient2/index.json`, each **1 passed, 0 failed, 0 warnings** for `PF.Survival.NeedsLive`; all exit status 0 at 05:21:54 UTC. Corresponding logs have prefix `PF` and suffix `.log` under `Saved/Logs`. Both clients observed PlayerIds 256 and 257 at needs 40/50/exposure .25, recovery 75/85/0, empty needs with Health 90, death Health 0, and replacement pawns at 100/100/100. Fresh food and subsequent replicated removal assertions passed on every process. Direct and PF command client mutations were rejected.
- PrimalAgentTools final exports: `PF_M2LiveServer_20260925T052144_E4015694434DDB4B0ECAD0A2BDE72692.json`, `PF_M2LiveClient_20260925T052144_808CE59F4A097DEA5DC73A8F7DA2EA42.json`, `PF_M2LiveClient_20260925T052144_E789E26E4B8F957379E39788CC019AA4.json`. Client command-history aggregate failures are the deliberate `NotAuthority` probes; the automation independently verifies rejection and passes.
- Known pre-existing startup issues remain in engine Toolsets Python (`ToolsetDefinition` and `PythonTestRunner` missing in game/server mode) and TEDS widget registration. These are not new gameplay failures. No new gameplay errors, ensures or RHI crashes were found in the successful final network runs. The ordinary `PrimalFrontier.log` was also checked; its last recorded editor session exited normally on September 23.
- On this resumed host Windows reports 33,477,984 KiB visible physical memory (about 31.9 GiB), unlike the earlier 16 GB runs. The three NullRHI processes sampled 1.63/1.73/1.73 GiB working set (1.46/1.62/1.62 GiB private bytes), with 12,327,252 KiB physical memory free. Thus the final two-client result is not a certification of the earlier 16 GB environment. Headless runs do not measure rendering stutter; prior rendered solo observations remain above.
- The user's technology references are retained in `TECH_TREE_DIRECTION.md`: accessible ordinary unlock points, a separate challenge-earned path, early essentials and later preservation. No technology tree was implemented.

All test processes finished. Remaining limits: no inventory, player cooking/drying, preservation containers or persistent freshness yet; the world ration is a finite fixture with a 300 simulation-second test lifetime. Recommended next milestone is M3's authoritative item/inventory data, carrying freshness through pickup/drop/stack operations. Do not start it automatically in this step.

# Milestone 1 — verified 2026-09-21

**Gate passed for the supported one-client greybox scenario. Stop here; Milestone 2 has not begun.** The final Editor build, five focused regression tests, actual server/client replication test, and manual first-person playtest passed. This is not a packaged Shipping or two-client acceptance result.

## Changes

- Added separate `PFSurvivorCharacter`, `PFSurvivalGameMode`, `PFSurvivalPlayerController` and `PFSurvivalHUD` headers/implementations under `Source/PrimalFrontier/Survival`. Integrated the existing replicated Health/Stamina component; server damage, jump cost, delayed recovery, death and PlayerStart respawn work through the same authoritative APIs.
- Exported the existing C++ character class for cross-module use; its camera/input implementation and template assets remain unchanged.
- Updated PrimalAgentToolsRuntime's build dependency and `PFCommands.cpp`; added `PFSurvivalLiveTests.cpp` and `Source/PrimalFrontier/Tests/PFSurvivalLifecycleTests.cpp`. `PF.SetHealth`, `PF.SetStamina`, `PF.Damage`, `PF.Kill` and `PF.Respawn` use real gameplay APIs. Commands are discoverable, logged, reported and non-Shipping. Mutations reject clients and ambiguous multiple-player targeting.
- Added `Scripts/SetupSurvivalMilestone1.py`, three project-owned Blueprint compositions in `Content/PrimalFrontier/Survival`, and `Content/PrimalFrontier/Maps/L_M1Survival.umap`. The map contains four cube actors, one PlayerStart and one directional light. It uses existing template presentation/input assets and engine primitives only.
- Updated architecture, decisions, this milestone log and developer command documentation; added `Docs/SURVIVAL_M1.md`. Hunger and Thirst remain interfaces only.

## Build and automated evidence

`Build.bat PrimalFrontierEditor Win64 Development -Project=C:/UnrealProjects/PrimalFrontier/PrimalFrontier.uproject -WaitMutex -NoHotReloadFromIDE -MaxParallelActions=2` succeeded. Final compilation/link: **12.56 seconds, no compiler warnings**. Evidence: `C:/Users/jackh/AppData/Local/UnrealBuildTool/Log.txt` (rotating build log).

`Saved/AutomationReports/M1Regression/index.json`: **5 succeeded, 0 failed, 0 warnings, 0 not run**:

1. `PF.Survival.Component`: initial values, damage, stamina/recovery, invalid input, authority guards, death and reserved interfaces.
2. `PF.Survival.Lifecycle`: actual native GameMode possession, grounded PlayerStart, damage/jump cost, client mutation rejection, three timed death/respawn cycles.
3. `PF.PrimalAgentTools.CommandArguments`: command registration, concise help, arguments, PF.Help execution and recording.
4. `PF.PrimalAgentTools.MissingSystemsAreBlocked`: unimplemented integrations remain explicit blockers.
5. `PF.PrimalAgentTools.TeleportAndRuntimeReset`: existing authority/collision behavior unchanged.

Log: `Saved/Logs/PFM1Regression.log`. The focused lifecycle retry also passed with NullRHI in `M1LifecycleVerified/index.json` and `PFM1LifecycleVerified.log`.

`PF.Survival.Live` passed independently on a NullRHI dedicated server process and one rendered client over localhost port 7781. Each report has **1 succeeded, 0 failed, 0 warnings**:

- `Saved/AutomationReports/M1Server/index.json`; `Saved/Logs/PFM1Server.log` (success at 03:45:31.916 UTC).
- `Saved/AutomationReports/M1Client/index.json`; `Saved/Logs/PFM1Client.log` (success at 03:45:31.906 UTC).

The client observed initial 100/100, authoritative Health 75/Stamina 20, death at 0/0, replacement possession and 100/100 after respawn. All five client mutation commands and direct component mutation probes were rejected. Both processes reached TestExit without a crash. This uses the Editor executable's `-server` mode; a packaged Server target is not claimed.

PrimalAgentTools exports:

- `Saved/AutomationReports/PF_M1Manual_20260921T034349_02F73ADD47C7FCA616EC5C939AF0E44F.json`
- `Saved/AutomationReports/PF_M1LiveAuthority_20260921T034521_E400187743BCB85B32A8FEADFFA05A6B.json`
- `Saved/AutomationReports/PF_M1LiveClient_20260921T034521_F9B92423418C1B381676A38C87F1D028.json`

The client command-history export deliberately has aggregate status **Failed**, because its five forbidden mutation probes are recorded as failed commands with `NotAuthority`. The automation report separately verifies those rejections and passes. Do not interpret the history export as a successful mutation or silently discard its failures.

## Manual playtest and assets

Computer Use inspected the actual standalone and connected-client windows at 960x540. The standalone test showed the first-person template mesh/camera, movement (debug X changed from -400.00 to -397.72 on W), mouse look, and jump reducing Stamina from 100 to 80. `PF.Damage 25` changed the visible Health bar to 75. `PF.SetStamina 0` emptied the bar and it recovered to 100. `PF.Kill` showed the death message and automatic respawn restored Health/Stamina and the initial view. Connected-client play showed server-driven 75/20, death, and respawn on the HUD; jump input also responded.

Persistent screenshot, generated by Unreal's `shot showui` and inspected: `Saved/Screenshots/WindowsEditor/ScreenShot00000.png`. Manual log: `Saved/Logs/PFM1Manual.log`. Asset creation log: `Saved/Logs/PFM1Setup.log`.

All **609 pre-existing Content files** retain their pre-task SHA256 aggregate `AC7D247B10EF3AB88337210954C410EF2C779CEEE96FB7E1FE3D12CC11AD9B16`. Only the four new M1 assets were created. Existing map relocation/deletions and unrelated Phase 10 work were preserved.

## Failures resolved, warnings and limits

- A C++ local name initially shadowed UWidget::Slot; renamed and rebuilt successfully.
- The resumed build initially failed LNK1104 because the user-opened Editor locked the DLL. Closed the All Saved editor normally, then the build passed.
- The first lifecycle run incorrectly required exact PlayerStart Z despite capsule grounding. It now checks start XY and bounded vertical adjustment. A subsequent expected-warning regex needed correction. The final rerun passes; the narrowly matched native fixture warnings reflect missing presentation meshes, not a failure in the actual Blueprint character.
- Asset setup reported a hierarchy-change warning when reparenting the new GameMode copy. The new composition loaded and passed the real possession/respawn tests; templates were not rewritten.
- Existing engine startup issues remain: StateTreeToolset/ToolsetRegistry Python scripts expect editor-only `ToolsetDefinition`/`PythonTestRunner` in game/server launches, TEDS widget-factory registration warnings, and the `r.MotionVectorSimulation` render-thread warning. The server also recovered a stale Zen lock. No new survival gameplay errors, ensures or RHI crashes occurred in the passing runs. `Saved/Logs/PrimalFrontier.log` was inspected; the M1 launches intentionally use separate named logs.
- Observed rendered gameplay held the requested **30 FPS** cap in sampled frames. Screenshot capture briefly displayed 1 FPS before returning to 30; no sustained gameplay stall was observed in this short test. This is not a long-duration performance certification.
- Standalone process: about **2.82 GB working set / 5.10 GB private bytes**. During the network test, rendered client: **2.37 / 5.11 GB**; NullRHI server: **1.64 / 1.47 GB**. Whole-system overlay ranged about **14.5–15.7 GB RAM**; OS available physical RAM reached **0.95 GB**. Process private bytes are not physical residency. Two rendered clients were not practical; no new two-client result is claimed.
- Existing historical generated files exceed GitHub's file-size limit. Do not rewrite history or stage unrelated changes as part of this milestone.

Recommended next step: user review of this first-person foundation. Start Milestone 2 only after explicit approval. All test sessions were closed.

---

# Greybox milestone plan — 2026-09-21

Current authorization: implement and verify **Milestone 1 only**, then stop and report. Milestones 2–8 below are a roadmap, not permission to bypass a gate.

1. Player foundation: integrate the existing Health/Stamina component with an isolated survivor character, first-person input, server damage, death, PlayerStart respawn, placeholder HUD and PF developer hooks. Build, automated tests, manual solo playtest, then one client/server test are required.
2. Hunger/thirst and environmental survival, after an explicit continuation.
3. Item data and authoritative inventory.
4. Gathering and crafting.
5. Greybox building and ownership.
6. Passive/hostile greybox creatures.
7. Small greybox survival arena.
8. Versioned persistence and multiplayer/reconnect acceptance; stop permanently after this milestone.

Each milestone must compile, pass focused automated tests, pass a manual playable test, validate server authority, and record logs, useful screenshots, memory/stutter observations and decisions before progression. Use NullRHI for nonvisual checks and one rendered client at a time. Use existing template assets and primitives only. Preserve unrelated Git and asset changes.

Audit: Phase 9 reports contain three passing tests, Phase 10 editor reports six passing tests, and the Phase 10 standalone live test passed. Client 2's rendered D3D12 residency crash remains an incomplete multiplayer result, not a gameplay pass. The current baseline build now succeeds after normally closing the saved editor (12.10 seconds, two build workers); no new gameplay integration has been verified yet.

## Earlier checkpoint

## First-person survival foundation: in progress, blocked at first build

Scope for this milestone is Health, Stamina, server-authoritative changes, death/respawn, a minimal first-person HUD and focused tests. Hunger and Thirst remain planned interfaces. Inventory, crafting, creatures, building, persistence and world expansion are not authorized by this milestone.

The initial source slice adds `UPFPlayerSurvivalComponent` with replicated Health/Stamina, server-only mutation guards, clamping, stamina recovery, death-state Gameplay Tags and notifications. `PF.Survival.Component` contains assertions for initial values, damage, stamina, authority rejection and death. Hunger/Thirst tags reserve names only; `SupportsStat` explicitly returns false for them. GameplayTags and SlateCore were added to the game module dependencies (SlateCore is for the planned placeholder HUD).

This component is **not yet attached to a player**. Character integration, respawn, HUD, PrimalAgentTools integration, project-owned Blueprint copies and live replication tests remain unimplemented. Existing first-person/template assets have not been changed by this work.

Verification attempted:

- `Build.bat PrimalFrontierEditor Win64 Development -Project=C:/UnrealProjects/PrimalFrontier/PrimalFrontier.uproject -WaitMutex -NoHotReloadFromIDE -MaxParallelActions=2`
- UHT, `PFPlayerSurvivalComponent.cpp` and `PFSurvivalComponentTests.cpp` compiled; no compiler warnings appeared in this attempt.
- Target build **FAILED** with `LNK1104` because running UnrealEditor PID 35724 held `UnrealEditor-PrimalFrontier.dll`, `UnrealEditor-PrimalAgentToolsRuntime.dll` and `UnrealEditor-PrimalAgentTools.dll` open. UBA first reported the file-in-use errors and then retried linking without UBA; the same lock remained.
- Build evidence: `C:/Users/jackh/AppData/Local/UnrealBuildTool/Log.txt`, result `Failed (OtherCompilationError)`, elapsed 78.57 seconds. This generated log may be replaced by the next build.
- New tests run: **none**. The failed link prevents testing current source; previous Phase 10 passes do not verify this new component.
- Existing `Saved/Logs/PrimalFrontier.log` contains the `r.MotionVectorSimulation` render-thread warning. No new editor or multiplayer process was launched.
- Resource observation: UBT reported 15.93 GB physical RAM and 32.01 GB committed at build start. Compilation was limited to two workers. No gameplay performance or stutter assessment was made.

Stopped after the build failure without expanding implementation. Next step: close the existing editor after handling any unsaved work, retry the same incremental build, and run only `PF.Survival.Component` with NullRHI before continuing this milestone. No Git staging or commits were performed.

---

\# Milestones



\## Milestone 0 — Toolchain



\- \[ ] Unreal project opens

\- \[ ] Editor target compiles

\- \[ ] Git and Git LFS are configured

\- \[ ] Logs can be collected

\- \[ ] Development test map exists

\- \[ ] Automated test workflow exists



\## Milestone 1 — Vulnerable Survivor



\- \[ ] First-person character

\- \[ ] Health

\- \[ ] Stamina

\- \[ ] Hunger

\- \[ ] Thirst

\- \[ ] Death and respawn

\- \[ ] Two-player multiplayer test



\## Milestone 2 — Hunter



\- \[ ] Resource gathering

\- \[ ] Inventory

\- \[ ] Primitive tools

\- \[ ] Basic weapon

\- \[ ] Passive creature

\- \[ ] Hostile creature

\- \[ ] Basic combat



\## Milestone 3 — Builder



\- \[ ] Crafting

\- \[ ] Foundation placement

\- \[ ] Walls and doors

\- \[ ] Storage

\- \[ ] Building persistence

\- \[ ] Server validation



\## Milestone 4 — Creature Tamer



\- \[ ] Companion creature prototype

\- \[ ] Ownership

\- \[ ] Follow and stay commands

\- \[ ] Creature persistence

\- \[ ] Basic creature utility



\## Milestone 5 — Settlement Leader



\- \[ ] Cooperative permissions

\- \[ ] Improved building pieces

\- \[ ] Crafting stations

\- \[ ] Settlement storage

\- \[ ] Resource and creature management



\## Milestone 6 — Industrial Survivor



\- \[ ] Advanced resources

\- \[ ] Processing systems

\- \[ ] Industrial crafting tier

\- \[ ] Advanced structures

\- \[ ] Dangerous Eryndor regions



\## Milestone 7 — Ancient Researcher



\- \[ ] Ancient sites

\- \[ ] Research progression

\- \[ ] Ancient technology

\- \[ ] First Frontier preparation



\## Milestone 8 — Dimensional Explorer



\- \[ ] First Frontier

\- \[ ] Frontier hazards

\- \[ ] Frontier creatures

\- \[ ] Frontier resources

\- \[ ] Dimensional progression



\## Milestone 9 — Master of the Frontiers



\- \[ ] Multiple Frontiers

\- \[ ] Endgame systems

\- \[ ] High-tier technology

\- \[ ] Optional third-person mode

\- \[ ] Optional PvP evaluation

