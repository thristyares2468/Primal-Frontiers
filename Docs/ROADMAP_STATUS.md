# Roadmap and open-world direction

Updated 2026-10-06. [FULL_PROJECT_ROADMAP.md](FULL_PROJECT_ROADMAP.md) preserves the user's full M0-M25 roadmap. Its milestone numbers are the current delivery sequence; the older thematic progression list at the bottom of MILESTONES.md is historical design context.

Primal Frontier is an open-world first-person survival game. The intended world is a connected, freely explored space with resource, settlement, danger and discovery areas, not a sequence of disconnected challenge rooms. M1-M6 maps remain isolated regression fixtures. M7 brings those mechanics together in one continuous map, `L_M7SurvivalArena`, without loading screens between its zones.

The retained 60 x 70 m map is a small integration prototype. The October 8 follow-up adds a separate 400 x 400 m World Partition candidate, `L_PrimalFrontier_OpenWorld`, with runtime streaming and the same primitive survival camp. Core simulation stays loaded while distant landmarks stream. Asset/network checks are partial evidence; full walking/overnight play and streaming-transition performance remain gated. Neither map is a final island or production-scale world. Landscape-scale terrain and final art remain deferred.

## Current execution boundary

- M7 remains in progress until sustained walking, the complete survival route and overnight play pass. Fixture teleports and navigation assertions are partial evidence only.
- M8 follows that gate: versioned authoritative player/world saves, validation, corrupt-save handling, restart/reconnect and multiplayer duplication checks.
- Stop after M8 for this development pass. The supplied later milestones are a plan, not evidence of completion or permission to skip gates.

## Asset handling

The user/Claude imports and source refactors are preserved. Existing Adventures_Pack, Bike, DynamicFalling, Modular_Rural_Cabin and Polyphoria assets are now committed by the user; older checkpoint notes describing them as untracked are historical. Current M7 uses engine primitives and project-owned simple materials only.

No additional asset is needed for this greybox step. Before M9 integration, audit the actual imported assets against required first-person hands/tools, remote player mesh, original creature visuals/animations, modular structures, terrain/foliage, UI and sound. Check provenance, usage rights, dependencies, collision, LODs and memory cost. The asset-source list is a shortlist, not a completed license or suitability audit. Notify the user of specific gaps before custom asset creation, purchases or manual imports. Do not substitute another game's protected creature designs, maps or UI.
