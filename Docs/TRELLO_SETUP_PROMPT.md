# Trello setup prompt

Paste the following into a chat with Trello connected and access to this project.

---

Set up and populate Trello for **Primal Frontier**, my first-person multiplayer open-world survival game.

Project: `C:\UnrealProjects\PrimalFrontier`. Requested engine: Unreal Engine 5.8.2; check project documentation for the installed version and current limitations.

This task authorizes creating and populating the project boards, lists, cards, labels and checklists. It does not authorize implementing gameplay, modifying Unreal assets, deleting unrelated Trello content, changing memberships or making boards public.

## Read project context first

Read:

- `AGENTS.md`
- `Docs/CURRENT_STATE.md`
- `Docs/TRELLO_BOARD_PLAN.md`
- `Docs/FULL_PROJECT_ROADMAP.md`
- `Docs/ROADMAP_STATUS.md`
- `Docs/MILESTONES.md`
- `Docs/DECISIONS.md`
- `Docs/ARCHITECTURE.md`
- `Docs/GAME_VISION.md` and `Docs/CORE_LOOP.md`
- `Docs/NETWORKING.md`
- `Docs/PERSISTENCE_M8.md`
- `Docs/PLAYTEST.md`
- `Docs/ADAPTATION_DIRECTION.md`
- `Plugins/PrimalAgentTools/README.md` and `DEVELOPER_COMMANDS.md`

Use other milestone-specific Docs files to supply accurate feature objectives. Where a claim depends on a test result, inspect the relevant existing report/log if accessible. Do not launch Unreal, build or run tests for this organization task.

Treat older milestone entries as historical checkpoints. Recent verified evidence and explicit current-state limitations take precedence over old statements such as “M8 not implemented.” If documents disagree, preserve the uncertainty on the affected card rather than inventing a pass.

## Trello structure

Inspect accessible workspaces and boards first. Reuse an existing Primal Frontier workspace and matching boards where possible. Do not duplicate cards from an earlier setup. If the destination workspace is genuinely ambiguous, ask only which workspace to use.

Create or reuse these six boards:

1. **Primal Frontier — Main Board**
2. **Primal Frontier — Programming**
3. **Primal Frontier — Greyboxing**
4. **Primal Frontier — HUD & UI**
5. **Primal Frontier — Sound & Audio**
6. **Primal Frontier — Testing**

Every board must have these workflow lists, in this order:

**To Do → Doing → Done**

Do not create Level Design or Game Design Document boards. Open-world/geometry work belongs on Greyboxing; roadmap coordination belongs on Main Board. Keep source documentation in the project Docs directory.

Use distinct simple board colours where supported: Main Board purple, Programming blue, Greyboxing grey, HUD & UI teal, Sound & Audio green, Testing orange. Appearance is secondary to correct content. Do not acquire external background art or purchase a Trello upgrade.

Respect existing privacy and membership. Do not make boards public, invite people, assign other people, or invent deadlines. If a workspace board limit prevents six boards, report the exact limit and ask how to proceed; do not silently create public boards or pay for anything.

## Main Board: milestone cards

Create one card for each milestone, in numeric order:

- M0 — Project Foundation
- M1 — Player Survival Foundation
- M2 — Hunger, Thirst, and Environmental Survival
- M3 — Inventory and Item Data
- M4 — Gathering and Crafting
- M5 — Greybox Building System
- M6 — Greybox Creatures and Combat
- M7 — Greybox World
- M8 — Persistence and Multiplayer Stability
- M9 — Asset Pipeline Planning
- M10 — First Art Pass
- M11 — UI and UX Pass
- M12 — Combat, Tools, and Progression
- M13 — Expanded Creatures and Ecology
- M14 — Expanded World
- M15 — Multiplayer Hardening
- M16 — Optimization Pass
- M17 — Audio and Feedback
- M18 — Save Compatibility and Migration
- M19 — Content Pass
- M20 — Alpha Slice
- M21 — Beta Preparation
- M22 — Beta
- M23 — Release Candidate
- M24 — Release
- M25 — Post-Release Support

Use the corresponding FULL_PROJECT_ROADMAP section for each goal, implementation objectives, playtests and acceptance criteria. Preserve substantive requirements; group related details into useful checklists rather than pasting an unreadable wall of text.

Each milestone card needs:

- A concise goal and scope.
- Current implementation and verification status.
- Dependencies on prior milestones.
- An **Implementation objectives** checklist.
- An **Acceptance and verification** checklist.
- Evidence/source document paths and relevant report paths.
- The next concrete action and any blocker.
- Links to relevant discipline cards once created.

Initial placement, subject to newer verified context:

- **Done:** M0–M6, based on their documented supported checkpoints. Explain that later regressions and unverified production/package scenarios are separate.
- **Doing:** M7 and M8. Do not mark either complete just because code exists or automation passes.
- **To Do:** M9–M25, with a Future label. Creating these cards does not authorize starting their gameplay work.

M7 has a 400 × 400 m World Partition candidate, `L_PrimalFrontier_OpenWorld`; sustained manual walking between zones and overnight survival remain unverified. Retain the New Editor Window PIE presentation slowdown as unresolved, with Selected Viewport + F11 as the documented workaround.

M8 now has native world-record/runtime checks, server-only PF save/load, one-client save/restart/reconnect and two-client NullRHI create/restart evidence. Read CURRENT_STATE for the latest exact reports. The rendered/manual full-loop persistence gate remains unverified. The packaged Server target is blocked by the installed engine distribution; uncooked Editor `-server` testing is a separate capability. Do not represent NullRHI as a manual or rendered FPS pass.

## Discipline boards: actionable objectives

Use `TRELLO_BOARD_PLAN.md` as the prepared breakdown, then reconcile it with the latest project state. Create manageable feature/test cards, with checklists for closely related substeps; avoid one giant card per board or hundreds of trivial cards.

**Programming:** survival attributes/authority, inventory/pickup/drop, food spoilage, gathering/crafting, building/storage/ownership, creature AI/combat, world clock, persistence/reconnect, dedicated-server setup, progression, adaptations, multiplayer hardening, optimization and save migration.

**Greyboxing:** small regression fixtures, primitive structures/creatures, current open-world camp and zones, resource/hazard/spawn distribution, landmarks, traversal and streaming. Later cards cover approved asset audits, a controlled biome replacement and measured world expansion. Preserve existing imported assets; no new asset work is authorized by this task.

**HUD & UI:** first-person vitals, inventory/freshness, crafting, build preview/storage, pause and settings, controls/controller feedback, save/reconnect feedback, later UI polish and original technology/research/adaptation interfaces.

**Sound & Audio:** distinguish implemented audio preference controls from actual authored sound coverage. Track audibility verification, approved sound-source audit, footsteps/action/creature/UI/ambient feedback, bounded replicated events, mix and licenses. Do not call an audio pass complete because sliders exist.

**Testing:** build/automation checkpoints, authority/refusal checks, item duplication, death/respawn, crafting cancellation, ownership/storage, creature navigation/combat, streaming, manual routes, overnight survival, physical controller feel, persistence/restart/reconnect, corrupt saves, dedicated-server packaging, uncapped rendered FPS and memory profiling, later migration/alpha/beta/release gates.

Keep implementation completion distinct from milestone acceptance. For example, “M7 — World Partition candidate created” can be Done while “M7 — Sustained walking and overnight playtest” remains Doing/Needs playtest. Future components remain To Do. A blocked action stays in To Do or Doing with a Blocked label and an explicit unblock condition; do not add a fourth workflow list.

## Progression references and adaptations

Track an original technology/research tree under M12, informed by the supplied Palworld and ARK structural references: progression points, prerequisites and a separate exploration/challenge research path. Do not copy protected creatures, item names, art, UI, lore or an entire balance table. Exact design and implementation are future work.

Track both requested biological systems provisionally under M12, with M13 ecology integration:

- Lasting, player-specific adaptations unlocked through original discoveries/research.
- Optional equipped biological modifiers, with separate active and passive slots.
- Active input, server-validated costs/cooldowns, bounded passive conditions and meaningful survival tradeoffs.
- Independent per-player progression checks in multiplayer.
- Persist unlocks separately from equipped effects, with explicit save-version migration tests.
- Refresh research before implementation.

These are planning cards, not implemented features. Avoid free nutrition or effects that bypass the established find/gather/cook/consume/spoilage loop.

## Card conventions

- Title: `M<number> — Clear actionable objective`.
- Labels: milestone plus only relevant labels such as Needs playtest, Blocked, Future, Bug or Performance. Lists already communicate ordinary workflow status.
- Description: purpose, scope, current status, acceptance, dependencies, evidence and next action.
- Checklist marks: check only objectives with evidence; keep manual gates unchecked until actually performed.
- Include the local project-relative evidence paths. Do not upload private save files, reconnect credentials, full logs containing login tokens, or memory files.
- Cross-link to Main Board milestone cards. Avoid duplicating the same task across multiple discipline boards.
- Do not add fabricated dates, estimates, assignees or proof.

## Verify and report

After creation, read each board back and verify all three lists, milestone coverage, cards, checklist states and cross-links. Confirm no accidental duplicates or omitted milestone numbers. Preserve unrelated content.

Return the six board links, card counts by list, the active M7/M8 cards, blockers needing my input and anything the Trello connection could not perform. Do not claim success unless Trello confirms the changes.

Record verified board links/publication status in `Docs/TRELLO_BOARD_PLAN.md` and update `Docs/CURRENT_STATE.md`. Do not modify unrelated project files or gameplay. Do not stage or commit unrelated in-progress work as part of this Trello task.
