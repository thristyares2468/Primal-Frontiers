# Primal Frontier — current state

Updated October9,2026. This is the current handoff; historical evidence remains in MILESTONES.md and CURRENT_STATE_HISTORY_2026-10-08.md. Implemented does not mean human accepted.

## Current game

First-person, server-authoritative greybox survival: Health/Stamina/Hunger/Thirst, exposure/damage/death/respawn; finite expiring food; inventory/pickup/drop; gathering/crafting; primitive building/storage/ownership; passive/hostile creatures; time of day; versioned world/player saves and reconnect. Remote players retain full-body presentation. No third-person gameplay.

Current open-world map: /Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld, a400x400m World Partition development candidate. L_M7SurvivalArena and earlier maps remain regression fixtures. Existing user-imported assets are preserved. No assets/dependencies/MCP server were added in this task.

**New verified bounded M8/M11 menu slice:** original texture-free Pause/main-menu theme, host-local Pause Save with actual acknowledgment, confirmed return to menu, Single player New/Load/Refresh/Rename and Main Settings. Runtime starts at existing /Engine/Maps/Entry; Editor startup/template maps remain unchanged. Server default is the open-world map. Multiplayer menu is informational only; hosting/join/browser/invite UI remains future work. Launch and use guide: [WORLD_MENU_M11.md](WORLD_MENU_M11.md), [PLAYTEST.md](PLAYTEST.md).

**Rename:** choose a saved world in Single player, type a friendly name below Refresh worlds and press Rename selected world.1–64 characters, spaces/Unicode accepted, invalid/control/duplicate names refused. The display label is stored separately in a version1 checksummed Metadata_WorldNames registry. Stable console save ID, gameplay files/generations and ownership are unchanged. Corrupt metadata refuses writes without replacement; Identity_/Metadata_ cannot be gameplay slots. No personal world was renamed by automation.

**Save observation:** user-reported M8 rendered check failed: world appeared fresh after PF.SaveWorld ManualGuide20261009. Retained log shows timed resource respawns immediately before Save Passed and no second map load/death; it does not establish the full cause. Native/rendered conservation tests preserve live pawn/actors/attributes/inventory/location and exact captured gameplay state. A separate solo reopen bug was reproduced and fixed: CheckLogin now matches existing sole standalone-owner adoption when another world's endpoint credential replaced the profile; network checks remain strict. Manual acceptance needs a replay, not an assumed pass. ManualGuide saves were not loaded/mutated/removed by these tests.

## Latest verified evidence

Final Editor19.22s/Game50.32s passed without compiler warnings. Logs: Saved/Logs/PFM11MenuLayoutFinalEditorBuild.log and PFM11RenameFinalGameBuild.log. Exact test/build/failure history: [WORLD_MENU_M11.md](WORLD_MENU_M11.md).

| Check | Latest report under Saved/AutomationReports | Result | Sampled working/private GiB |
| --- | --- | --- | --- |
| PF.Input.Gamepad + PF.Persistence.WorldRuntime/StartupFailurePreservesSave/RejectedLoadPreservesWorld | Automation_M11WorldMenuFinal_20261009_031014929_6efba8a2 |4/4 passed, raw/test severity0|3.003/2.907|
| PF.UI.WorldMenuLive720p | M11WorldMenu720_20261009_030843603_e443e451 |1/1 passed; persisted Rename, invalid/duplicate/corrupt-metadata refusal, unchanged gameplay bytes, actual New/Save/End/Load|3.284/4.652|
| PF.UI.WorldMenuLive1440p, final labels | M11WorldMenu1440_20261009_031331936_5ef2e6f8 |1/1 passed|3.448/5.714|
| PF.UI.ControlsLive720p | M11Controls720_20261009_031456406_59dda1f9 |1/1 passed|3.129/5.152|
| PF.UI.ReconnectProfileLive720p | M11Reconnect720_20261009_031549752_c11eef2b |1/1 passed|2.369/5.122|
| One-client NullRHI Create/Restart server+client | M8Live1_20261009_031704133_94663d4f |4/4 passed|1.723–1.817/1.631–1.787 per process|
| Two-client NullRHI Create/Restart server+clients | M8Live2_20261009_031919719_d6a38e7e |6/6 passed; Client2 no crash|1.720–1.825/1.621–1.790 per process|

All final test errors/warnings0; engine/report/runner exits0. Rendered runners verified actual default settings and131/133/135/136 unrelated save files unchanged. Menu/screenshots use actual Slate keyboard activation, never direct callbacks. Screenshots inspected under Saved/AutomationReports/ControlsUI/<report>/. Pause label wrap fixed after inspection; button labels and long save feedback fit final layouts. These are not mouse/physical controller or human traversal passes.

Known raw startup findings:21EditorDataStorageUI warnings, three uncooked HLOD import warnings and14engine Python error lines in network/ordinary UI processes; blur/DOF priority warnings in ordinary UI. Six specific menu travel baseline messages individually expected, no blanket suppression. No new gameplay error/ensure/fatal/crash in final runs. Native logs clean. Installed engine5.8.3 differs from requested5.8.2. Packaged Server unavailable in this installed distribution; uncooked dedicated-server support is separately tested. Actual host31.93GiB physical RAM;16GB minimum-spec certification remains unverified. Uncapped stationary UI tests do not establish sustained FPS/stutter.

Prior broader regression:35distinct native cases passed in the October9 pre-menu checkpoint; this task reran only the four related cases above. [NATIVE_CHECKPOINT_M11.md](NATIVE_CHECKPOINT_M11.md), [AUTOMATION_VERIFICATION.md](AUTOMATION_VERIFICATION.md) and [M8_NETWORK_DIAGNOSTICS.md](M8_NETWORK_DIAGNOSTICS.md) preserve exact historical evidence. Earlier M11 Controls, stable inventory selection, survival feedback, shadow-budget, action overlays, reconnect, settings/cancel/apply/display and P-key slices have bounded passing evidence in MILESTONES.md.

## Milestones and next step

| Milestone | Status | Next action |
| --- | --- | --- |
| M0–M6 | Historical supported greybox checkpoints passed | Retain evidence; no later acceptance implied |
| M7 | User reported Personal check completed October9 | No route/night/FPS measurements invented; broader production world remains future |
| M8 | Native/live persistence implemented; latest network checks pass; **human rendered check failed/replay pending** | [Personal2 replay](https://trello.com/c/UZVTTPLE), GuideG3: actual gather/craft/build/store/Pause Save/close/relaunch/Load comparison |
| M9 | Independent intake/provenance audit completed; candidate approval open | Exact local source/license mapping before integration |
| M11 | Bounded UI/menu slices passed; full usability/physical controller/audio acceptance unverified | Human first-person usability remains separate; next independent slice is inventory/item details and data-driven crafting UI |
| M10,M12–M25 | Future plans, not accepted | More resources/recipes tracked M12, world distribution M14; adaptation/biomod and tech-tree references remain design inputs |

**Queued user request:** lower only this machine's editor scalability to Medium after current menu/save task finishes. Use supported Unreal Scalability1 in Editor context, verify persisted EditorSettings and unchanged game preferences. Do not raw-edit generated INIs or commit machine settings. Not applied yet.

Trello: bounded Pause theme Done; save/menu/rename task updated/read back Done. Personal M8 retains failed/replay-pending status and current menu GuideG3; parent/manual M8/M11 stay open. Latest authorization allows independent implementation while Personal observations remain unverified. Assignments/deadlines/unrelated cards preserved. Source/doc changes ready for scoped commit/push with Codex GPT-6.1 Sol coauthor; no unrelated work is included. No next gameplay milestone begun during this menu task.
