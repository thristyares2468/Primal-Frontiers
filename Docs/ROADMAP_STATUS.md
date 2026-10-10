# Roadmap and open-world direction

## Current delivery state — October 10, 2026

The user's [FULL_PROJECT_ROADMAP.md](FULL_PROJECT_ROADMAP.md) is the M0–M25 delivery sequence. This section supersedes the dated historical statements below. [CURRENT_STATE.md](CURRENT_STATE.md) contains the current resume point; [MILESTONES.md](MILESTONES.md) retains each verification result.

Play on `/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld`: a 400 x 400 m first-person World Partition greybox, using primitives and project simple materials. `L_M7SurvivalArena` remains the smaller integration fixture; M1–M6 maps remain regression fixtures. Neither is a final production island. No third-person gameplay or final art pass is implemented.

| Milestone | Implemented or independently completed | Remaining acceptance / next actor |
| --- | --- | --- |
| M0–M6 | Foundation, replicated survival/needs, finite inventory/food, gathering/crafting, building/storage and two primitive creatures. Existing Trello parents Done. | Later regressions do not replace previous manual results. |
| M7 | Continuous greybox and World Partition open-world map. User reported the personal check done; main/testing cards Done. | No additional FPS, overnight timing or minimum-16GB certification inferred. |
| M8 | Versioned player/world saves, ownership/storage, resources/pickups/creatures, guarded restore, reconnect and independent one-/two-client NullRHI restart tests. Pause Save plus main/world-select/create/load/rename menus have bounded technical evidence. | Personal rendered full-loop replay remains open after the user's reset report. Installed engine does not support packaged Server targets; uncooked dedicated-server tests are a separate result. |
| M9 / M10 | Asset intake/provenance/dependency/memory audit and planning; user imports preserved. | Candidate rights/compatibility confirmation precedes actual art integration. No new asset is needed for current greybox work. Full art milestones remain open. |
| M11 | Original themed Pause/main/settings; selected item pictures/details; centered categorized crafting and knowledge feedback; building/storage guidance; vitals/damage/status/control help; actual Apply/Cancel/display rollback and separate-process settings tests. | Full human first-person usability, physical controller/audio and sustained performance remain open. Technical UI fixtures are not human acceptance. |
| M12 | Stone/bound tool and wooden/stone-bound weapon tiers, carried woven protection, creature damage/yield/cooldown rules, timed recipes and private earned XP/knowledge. FieldTools is level2/2points; FieldWeapons is level3/3points plus FieldTools. | Full early/mid-game pacing, combat feel and overall balance remain open. Current normal route earns optional upgrades; ten-level pacing, repair/durability and broader technology are not implemented. |
| M13 / M14 | Adaptation/ecology and world-expansion directions exist as proposals. | Fresh bounded audits may proceed independently under the user's continuation permission. New ecology/adaptation gameplay and larger-world acceptance are not passed. Taming/companions require explicit approval. |
| M15–M25 | Networking, optimization, audio, save migration and release plans; earlier narrow technical work contributes evidence. | No later full milestone is accepted. Packaging, assets, sustained profiling, playability and release gates remain required. |

### Continue safely

The user permits independent work while Personal playtests are deferred. Inspect fresh Trello, reconcile current source/evidence, keep one bounded AI task Doing, compile/test affected changes, update current state and immediately publish task evidence before starting another. Do not mark manual gates Done or infer that the request to continue is a playtest result.

The current native source checkpoint passed **53/53** Editor-context core/runtime-command tests with an exact source-manifest match and clean raw logs; [NATIVE_CHECKPOINT_M12.md](NATIVE_CHECKPOINT_M12.md) records every identifier and result. This supersedes the earlier35-test checkpoint for those directories. No broad test count proves rendering, private-save replay, gameplay feel or machine performance.

Actual current progression, including legacy gear and bounded active-time gather rewards, is in [PROGRESSION_PLAN.md](PROGRESSION_PLAN.md). [FIELD_WEAPONS_M12.md](FIELD_WEAPONS_M12.md) records the real earned, rendered and private multiplayer restart gates. [SETTINGS_THEME_M11.md](SETTINGS_THEME_M11.md) and [SETTINGS_RESTART_M11.md](SETTINGS_RESTART_M11.md) record the current settings evidence. Adaptations/equipped biological modifiers remain original design proposals in [ADAPTATION_DIRECTION.md](ADAPTATION_DIRECTION.md); refresh primary research before implementation. No research station, scanner, adaptation effect, respec or copied reference-game content is present.

Installed engine identifies itself as **5.8.3**, although the project requests 5.8.2. The actual host has **31.93 GiB physical RAM**; low-memory test precautions remain. Recorded process memory is not a minimum-16GB or FPS pass. Tests use NullRHI where rendering is unnecessary and one rendered window when screenshots are needed. No extra assets, dependencies or production rendering-quality increase is required.

## Historical October 8–9 checkpoint — retained for context

The following original notes describe the earlier planning and manual-gate state. Their absence/next-step statements are historical, not current instructions or acceptance status.

Updated 2026-10-08. [FULL_PROJECT_ROADMAP.md](FULL_PROJECT_ROADMAP.md) preserves the user's full M0-M25 roadmap. Its milestone numbers are the current delivery sequence; the older thematic progression list at the bottom of MILESTONES.md is historical design context.

October9 current override: user reported “M7 personal check done”; main/manual M7 Trello cards are now Done based on that human report. Older unverified M7 statements below describe the earlier checkpoint; no detailed FPS, route/night timings or minimum-spec certification is inferred. M8/M11 Personal gates remain separate. Requested substantial UI improvement is the current M11 overhaul, starting with a themed Pause screen before inventory/crafting. Additional gatherable resources/useful recipes are explicitly planned in M12; wider biome distribution remains M14. No new resources/recipes are implemented by the UI slice.

Primal Frontier is an open-world first-person survival game. The intended world is a connected, freely explored space with resource, settlement, danger and discovery areas, not a sequence of disconnected challenge rooms. M1-M6 maps remain isolated regression fixtures. M7 brings those mechanics together in one continuous map, `L_M7SurvivalArena`, without loading screens between its zones.

The retained 60 x 70 m map is a small integration prototype. The October 8 follow-up adds a separate 400 x 400 m World Partition candidate, `L_PrimalFrontier_OpenWorld`, with runtime streaming and the same primitive survival camp. Core simulation stays loaded while distant landmarks stream. Asset/network checks are partial evidence; full walking/overnight play and streaming-transition performance remain gated. Neither map is a final island or production-scale world. Landscape-scale terrain and final art remain deferred.

## Current execution boundary

Latest October 8 instruction supersedes the earlier planning-only boundary below: independent implementation may proceed while Personal playtests are deferred. M11 controls/help starts with existing placeholder UI, without M10 art dependencies. No manual result is assumed; M7/M8/controller checks remain UNVERIFIED. Asset provenance and technical build/test failures still govern affected tasks.

Independent M11–M13 controls/reconnect/research UI planning is complete in [UI_CONTROLS_PLAN.md](UI_CONTROLS_PLAN.md). Read-only help precedes future remapping; progression views wait for real backends. No UI implementation or acceptance was added; preceding gates remain required.

Independent M18 compatibility planning is complete in [SAVE_COMPATIBILITY_PLAN.md](SAVE_COMPATIBILITY_PLAN.md). Full migration implementation/acceptance remains To Do behind preceding gates. No source/save/schema or art change; M7/M8 remain unverified manually.

- M7 remains in progress until sustained walking, the complete survival route and overnight play pass. Fixture teleports and navigation assertions are partial evidence only.
- On October 8 the user deferred the unpassed M7 manual gate and authorized independent continuation. M8 whole-world saves, server lifecycle/commands and development reconnect have historical native and separate one-/two-client NullRHI Create/Restart evidence. The October9 [current35-test native checkpoint](NATIVE_CHECKPOINT_M11.md) includes all later native additions and passed32+3 sequential batches with no missing/extra records. [CURRENT_STATE.md](CURRENT_STATE.md) identifies exact source/build/evidence; older counts remain historical. Rendered/manual persistence acceptance and packaged Server setup (installed engine rejects Server targets) remain open. Keep M7/M8 Doing in Trello; no native checkpoint certifies the complete playable loop.
- The user's later October 8 instruction permits continuing independent eligible planning/audit tasks without further prompts. M9 intake rules and read-only inventory are now documented/tested; full acceptance/candidate approval stays open. No M10 art/gameplay integration begins while M7/M8 manual gates and candidate rights/compatibility remain unverified. Later roadmap cards are still plans, not completed milestones.

## Future adaptation request (October 8)

Independent M12/M13 planning is complete in [PROGRESSION_PLAN.md](PROGRESSION_PLAN.md): XP/point economy, baseline access, original unlock graph, respec/co-op/challenges, adaptation/loadout and future authority/save/test gates. This is a completed subtask only; full M12/M13 remain planned, with no new gameplay or passed acceptance.

The user requested a Subnautica 2-inspired adaptation system, with independent research and modifications appropriate to Primal Frontier. [ADAPTATION_DIRECTION.md](ADAPTATION_DIRECTION.md) records primary-source observations, an original land-survival proposal and server/save/test requirements. Candidate window: M12 progression with M13 ecology integration. No implementation is added to M8; refresh research before that future milestone.

## Asset handling

Independent audio feedback planning is complete in [AUDIO_FEEDBACK_PLAN.md](AUDIO_FEEDBACK_PLAN.md), following the read-only coverage audit. M17 stays planned; no sound implementation, sourcing/import, audio acceptance or new performance result is implied. The open manual/provenance gates still govern integration.

The user/Claude imports and source refactors are preserved. Existing Adventures_Pack, Bike, DynamicFalling, Modular_Rural_Cabin and Polyphoria assets are now committed by the user; older checkpoint notes describing them as untracked are historical. Current M7 uses engine primitives and project-owned simple materials only.

No additional asset is needed for this greybox step. Before M9 integration, audit the actual imported assets against required first-person hands/tools, remote player mesh, original creature visuals/animations, modular structures, terrain/foliage, UI and sound. Check provenance, usage rights, dependencies, collision, LODs and memory cost. The asset-source list is a shortlist, not a completed license or suitability audit. Notify the user of specific gaps before custom asset creation, purchases or manual imports. Do not substitute another game's protected creature designs, maps or UI.
