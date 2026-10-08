# Roadmap and open-world direction

Updated 2026-10-08. [FULL_PROJECT_ROADMAP.md](FULL_PROJECT_ROADMAP.md) preserves the user's full M0-M25 roadmap. Its milestone numbers are the current delivery sequence; the older thematic progression list at the bottom of MILESTONES.md is historical design context.

Primal Frontier is an open-world first-person survival game. The intended world is a connected, freely explored space with resource, settlement, danger and discovery areas, not a sequence of disconnected challenge rooms. M1-M6 maps remain isolated regression fixtures. M7 brings those mechanics together in one continuous map, `L_M7SurvivalArena`, without loading screens between its zones.

The retained 60 x 70 m map is a small integration prototype. The October 8 follow-up adds a separate 400 x 400 m World Partition candidate, `L_PrimalFrontier_OpenWorld`, with runtime streaming and the same primitive survival camp. Core simulation stays loaded while distant landmarks stream. Asset/network checks are partial evidence; full walking/overnight play and streaming-transition performance remain gated. Neither map is a final island or production-scale world. Landscape-scale terrain and final art remain deferred.

## Current execution boundary

- M7 remains in progress until sustained walking, the complete survival route and overnight play pass. Fixture teleports and navigation assertions are partial evidence only.
- On October 8 the user deferred the unpassed M7 manual gate and then authorized continuing until an immediate user action is needed. M8 whole-world saves, server lifecycle/commands and development reconnect now pass 26 focused regressions plus ten one-/two-client NullRHI create/restart process reports. Remaining: rendered/manual persistence acceptance and packaged Server setup (installed engine rejects Server targets). Keep M7/M8 Doing in Trello; do not mark M7 complete or certify the complete playable loop without its missing evidence.
- The user's later October 8 instruction permits continuing independent eligible planning/audit tasks without further prompts. M9 intake rules and read-only inventory are now documented/tested; full acceptance/candidate approval stays open. No M10 art/gameplay integration begins while M7/M8 manual gates and candidate rights/compatibility remain unverified. Later roadmap cards are still plans, not completed milestones.

## Future adaptation request (October 8)

Independent M12/M13 planning is complete in [PROGRESSION_PLAN.md](PROGRESSION_PLAN.md): XP/point economy, baseline access, original unlock graph, respec/co-op/challenges, adaptation/loadout and future authority/save/test gates. This is a completed subtask only; full M12/M13 remain planned, with no new gameplay or passed acceptance.

The user requested a Subnautica 2-inspired adaptation system, with independent research and modifications appropriate to Primal Frontier. [ADAPTATION_DIRECTION.md](ADAPTATION_DIRECTION.md) records primary-source observations, an original land-survival proposal and server/save/test requirements. Candidate window: M12 progression with M13 ecology integration. No implementation is added to M8; refresh research before that future milestone.

## Asset handling

Independent audio feedback planning is complete in [AUDIO_FEEDBACK_PLAN.md](AUDIO_FEEDBACK_PLAN.md), following the read-only coverage audit. M17 stays planned; no sound implementation, sourcing/import, audio acceptance or new performance result is implied. The open manual/provenance gates still govern integration.

The user/Claude imports and source refactors are preserved. Existing Adventures_Pack, Bike, DynamicFalling, Modular_Rural_Cabin and Polyphoria assets are now committed by the user; older checkpoint notes describing them as untracked are historical. Current M7 uses engine primitives and project-owned simple materials only.

No additional asset is needed for this greybox step. Before M9 integration, audit the actual imported assets against required first-person hands/tools, remote player mesh, original creature visuals/animations, modular structures, terrain/foliage, UI and sound. Check provenance, usage rights, dependencies, collision, LODs and memory cost. The asset-source list is a shortlist, not a completed license or suitability audit. Notify the user of specific gaps before custom asset creation, purchases or manual imports. Do not substitute another game's protected creature designs, maps or UI.
