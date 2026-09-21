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

