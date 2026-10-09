# Primal Frontier — Trello board plan

Prepared October 8, 2026. Trello publication verified October 8, 2026.

Verified boards:
- [Main Board](https://trello.com/b/XqitZ1Fg/primal-frontier-main-board)
- [Programming](https://trello.com/b/YZIEdyWN/primal-frontier-programming)
- [Greyboxing](https://trello.com/b/JA3pN8Ix/primal-frontier-greyboxing)
- [HUD & UI](https://trello.com/b/dhjVbNNj/primal-frontier-hud-ui)
- [Sound & Audio](https://trello.com/b/Hbm7xohf/primal-frontier-sound-audio)
- [Testing](https://trello.com/b/WwUL7l6L/primal-frontier-testing)

All six boards were created in the existing Primal Frontier workspace with workspace visibility, no invited members, no deadlines or assignees, and workflow lists ordered To Do, Doing, Done. Main Board contains M0-M25; M0-M6 are Done, M7-M8 are Doing, and M9-M25 are To Do. Discipline boards contain seven actionable cards each in To Do. Trello verification confirmed publication; checklist and label enrichment remains a follow-up gap if the connection is rate-limited.

Create or reuse six Primal Frontier boards: Main Board, Programming, Greyboxing, HUD & UI, Sound & Audio, Testing. Each has exactly three workflow lists, ordered To Do, Doing, Done. Do not create Level Design or Game Design Document boards. Preserve unrelated existing boards/cards.

Use milestone labels M0–M25, plus Needs playtest, Blocked and Future where relevant. Main Board contains milestone cards with objective and acceptance checklists; discipline boards contain actionable feature/test cards. Cross-link each discipline card to its milestone. Do not invent deadlines or assign other people.

Done means the documented supported checkpoint passed, not a guarantee against future regressions. M7 and M8 remain Doing. M9–M25 are planned To Do; creating roadmap cards does not authorize implementation beyond the current M8 boundary.

## Main Board

### [Done] M0 — Project Foundation
Status: historical supported checkpoint recorded in Docs/MILESTONES.md. See that milestone for exact evidence and limits.

Goal: Make the project safe for long-term AI-assisted development.

Build:

- Confirm Unreal 5.8.2 project opens.
- Confirm first-person template is the base.
- Set up Git ignore rules for generated folders.
- Confirm `.vs`, `Binaries`, `Intermediate`, `Saved`, and machine-specific files are not tracked.
- Add or verify `AGENTS.md`.
- Add architecture and planning docs.
- Confirm PrimalAgentTools plugin is present and working.
- Confirm basic editor command/test workflow works.

Continue only if:

- Project opens.
- Editor target builds.
- Git status is understandable.
- Generated files are ignored.
- Plugin command `PF.Help` works.
- Docs describe the project direction.

### [Done] M1 — Player Survival Foundation
Status: historical supported checkpoint recorded in Docs/MILESTONES.md. See that milestone for exact evidence and limits.

Goal: Create the basic first-person survival player.

Build:

- Health system.
- Stamina system.
- Damage handling.
- Death state.
- Respawn flow.
- Basic first-person HUD.
- Server-authoritative stat changes.
- Replication for remote players.
- Developer commands for testing health/stamina where appropriate.

Playtest:

- Walk, sprint, take damage, die, respawn.
- Test in single player.
- Test with server/client if practical.

Continue only if:

- Health and stamina work.
- Death and respawn work.
- Client cannot cheat authority.
- Logs are clean enough to proceed.
- Tests or manual evidence are recorded.

### [Done] M2 — Hunger, Thirst, And Environmental Survival
Status: historical supported checkpoint recorded in Docs/MILESTONES.md. See that milestone for exact evidence and limits.

Goal: Make the player need to survive over time.

Build:

- Hunger stat.
- Thirst stat.
- Drain over time.
- Damage or penalties when starving/dehydrated.
- Basic food/water recovery.
- Placeholder environmental exposure system.
- HUD display for hunger/thirst/exposure.
- Developer commands for hunger/thirst testing.

Playtest:

- Hunger and thirst decrease.
- Player can recover them.
- Bad states cause real consequences.
- Values replicate correctly.

Continue only if:

- Hunger/thirst loop works.
- Server controls stat changes.
- HUD updates correctly.
- No major log errors.
- Docs updated.

### [Done] M3 — Inventory And Item Data
Status: historical supported checkpoint recorded in Docs/MILESTONES.md. See that milestone for exact evidence and limits.

Goal: Add the item foundation for survival gameplay.

Build:

- Item data assets or data tables.
- Item IDs using stable names/tags.
- Inventory component.
- Stack sizes.
- Add/remove item logic.
- Pickup items.
- Drop items.
- Basic inventory UI.
- Weight or slot limit.
- Server-authoritative inventory changes.
- Developer commands for granting items.

Playtest:

- Pick up items.
- Drop items.
- Stack items.
- Fill inventory.
- Test reconnect or respawn if practical.

Continue only if:

- No duplication bug found.
- Client cannot add items without authority.
- Inventory replicates.
- UI matches actual inventory state.
- Docs updated.

### [Done] M4 — Gathering And Crafting
Status: historical supported checkpoint recorded in Docs/MILESTONES.md. See that milestone for exact evidence and limits.

Goal: Let players collect resources and craft basic items.

Build:

- Gatherable resource actors.
- First-person interaction trace.
- Resource yields.
- Resource depletion.
- Optional resource respawn.
- Recipe data.
- Crafting validation.
- Crafting queue or instant crafting.
- Basic crafting UI.
- Example recipes:
  - wood
  - stone
  - fibre
  - basic tool
  - basic wall/foundation placeholder

Playtest:

- Gather resources.
- Craft item.
- Fail crafting when missing resources.
- Confirm inventory changes correctly.

Continue only if:

- Full gather-to-craft loop works.
- No duplication during crafting.
- Server validates crafting.
- UI clearly shows craftable/uncraftable recipes.
- Docs updated.

### [Done] M5 — Greybox Building System
Status: historical supported checkpoint recorded in Docs/MILESTONES.md. See that milestone for exact evidence and limits.

Goal: Let players place simple survival structures.

Build:

- Placement preview.
- First-person placement flow.
- Collision checks.
- Range checks.
- Snap points or grid rules.
- Server-authoritative placement.
- Structure ownership.
- Basic structure health.
- Demolish/remove flow.
- Placeholder pieces:
  - foundation
  - wall
  - floor
  - ceiling
  - doorway
  - door
  - storage box

Playtest:

- Place structures.
- Snap pieces together.
- Block invalid placement.
- Damage or demolish a structure.
- Confirm another client sees the structure.

Continue only if:

- Placement is predictable.
- Invalid placements are rejected.
- Structures replicate.
- Player can build a tiny shelter.
- No severe performance issue.
- Docs updated.

### [Done] M6 — Greybox Creatures And Combat
Status: historical supported checkpoint recorded in Docs/MILESTONES.md. See that milestone for exact evidence and limits.

Goal: Add basic wildlife/threat systems using placeholder shapes.

Build:

- Creature data.
- Passive creature.
- Hostile creature.
- Spawn points.
- Simple AI states:
  - idle
  - wander
  - flee
  - chase
  - attack
  - death
- Creature health.
- Player melee or basic weapon damage.
- Creature loot drops.
- Server-authoritative AI/combat.
- Basic spawn limits.

Playtest:

- Creature spawns.
- Creature moves.
- Creature attacks or flees.
- Player can kill creature.
- Loot can be collected.
- Multiplayer replication works where practical.

Continue only if:

- AI does not break navigation.
- Combat works in first person.
- Loot enters inventory correctly.
- Spawn limits prevent overload.
- Docs updated.

### [Doing] M7 — Greybox World
Status: open-world candidate, settings and network/streaming checks exist; sustained manual walking and overnight survival remain unverified. New Editor Window PIE presentation slowdown remains unresolved.

Goal: Put the systems into a playable survival space.

Build:

- Small greybox island or contained survival map.
- Spawn beach.
- Resource zones.
- Creature zones.
- Buildable flat areas.
- Landmark areas.
- Water source area.
- Hazard area.
- Basic cave/ruin/test landmark.
- Simple day/night or world time if not already present.
- Full survival route from spawn to shelter.

Playtest:

- Start with nothing.
- Gather.
- Craft.
- Build.
- Fight or avoid creatures.
- Survive hunger/thirst.
- Die and respawn.
- Recover or continue.

Continue only if:

- Core loop works inside the world.
- Map loads reliably.
- Performance is acceptable on 16 GB RAM.
- Single-client playtest passes.
- Multiplayer is attempted where practical.
- Docs updated.

### [Doing] M8 — Persistence And Multiplayer Stability
Status: native world/save tests and one-client separate-process restart passed; two-client create/save/load retry passed; separate-process two-client restart/reconnect also passed. Manual persistence gate remains unverified. Packaged Server target is blocked by installed-engine distribution.

Goal: Make the greybox survival game save, load, and survive basic multiplayer use.

Build:

- Save player position.
- Save health/hunger/thirst/stamina.
- Save inventory.
- Save placed structures.
- Save storage contents.
- Save depleted resources where practical.
- Save creature state where practical.
- Versioned save data.
- Corrupt save handling.
- Manual save/load developer commands.
- Dedicated server compatibility checks.
- Reconnect flow.

Playtest:

- Gather items.
- Build structure.
- Store items.
- Save.
- Close game/server.
- Reload.
- Confirm state returns.
- Test reconnect.
- Test no item duplication.

Continue only if:

- Important state persists.
- Server owns saving/loading.
- No obvious duplication bugs.
- Logs are acceptable.
- Two-client test attempted if practical.
- Known 16 GB RAM/D3D12 limits documented.

### [To Do] M9 — Asset Pipeline Planning
Status: future roadmap, not started. Depends on prior milestone acceptance.

Goal: Prepare for real assets without breaking the working greybox game.

Build:

- Asset folder rules.
- Naming conventions.
- Import rules.
- Material rules.
- LOD/collision rules.
- Texture size limits.
- Marketplace/external asset approval process.
- Placeholder-to-final replacement plan.
- Redirector cleanup process.
- Performance budget.

Continue only if:

- Asset rules are documented.
- No random assets are imported.
- Replacement process is clear.
- Git/LFS plan is clear if large assets will be used.

### [To Do] M10 — First Art Pass
Status: future roadmap, not started. Depends on prior milestone acceptance.

Goal: Replace key greybox visuals with controlled prototype art.

Build:

- One visual biome pass.
- Basic terrain material pass.
- First-pass resource meshes.
- First-pass structure meshes.
- First-pass creature placeholders or simple models.
- First-pass item icons.
- Keep scope small.
- Avoid huge marketplace packs unless approved.

Playtest:

- Confirm gameplay still works.
- Confirm collision still works.
- Confirm build placement still works.
- Check memory use.

Continue only if:

- Game still plays the same or better.
- No broken references.
- No huge performance drop.
- Assets follow naming/folder rules.

### [To Do] M11 — UI And UX Pass
Status: future roadmap, not started. Depends on prior milestone acceptance.

Goal: Make the game readable and usable.

Build:

- Improved HUD.
- Inventory UI polish.
- Crafting UI polish.
- Building placement feedback.
- Damage indicators.
- Status warnings.
- Death/respawn screen.
- Basic settings menu.
- Keybind display or controls help.

Continue only if:

- Player can understand what is happening.
- UI works in first person.
- UI does not hide important combat/building information.
- Multiplayer UI state is accurate.

### [To Do] M12 — Combat, Tools, And Progression
Status: future roadmap, not started. Depends on prior milestone acceptance.

Goal: Make survival progression feel meaningful.

Build:

- Tool tiers.
- Weapon tiers.
- Armor or protection.
- Creature damage tuning.
- Resource yield tuning.
- Crafting progression.
- Unlocks or learned recipes.
- Repair system if desired.
- Balance pass.

Continue only if:

- Progression has a reason to exist.
- Early game is playable.
- Mid-game has clear goals.
- Combat is understandable.
- No major exploits found.

### [To Do] M13 — Expanded Creatures And Ecology
Status: future roadmap, not started. Depends on prior milestone acceptance.

Goal: Make the world feel more alive.

Build:

- More creature types.
- Spawn tables by biome.
- Predator/prey behavior if practical.
- Day/night spawn differences.
- Creature loot variety.
- Basic taming or companion system only if explicitly approved.
- Creature performance limits.

Continue only if:

- Creatures add gameplay instead of noise.
- Performance remains acceptable.
- Server handles creature logic.
- Multiplayer remains stable.

### [To Do] M14 — Expanded World
Status: future roadmap, not started. Depends on prior milestone acceptance.

Goal: Grow the playable world carefully.

Build:

- Larger map sections.
- More biomes.
- More landmarks.
- Better traversal routes.
- More resource distribution.
- More buildable locations.
- Streaming or World Partition strategy if needed.
- Performance profiling.

Continue only if:

- Larger world does not break the core loop.
- Streaming/loading is acceptable.
- Memory use is documented.
- Player can navigate without confusion.

### [To Do] M15 — Multiplayer Hardening
Status: future roadmap, not started. Depends on prior milestone acceptance.

Goal: Make multiplayer less fragile.

Build:

- Dedicated server testing.
- Join/leave/reconnect handling.
- Replication review.
- Anti-duplication checks.
- Authority checks.
- Lag tolerance for interaction/building/combat.
- Server logs for important events.
- Basic admin/dev commands.

Continue only if:

- Server remains stable.
- Two-client tests pass where hardware allows.
- NullRHI tests pass for non-rendered clients.
- Known D3D12/rendering issues are separated from gameplay bugs.

### [To Do] M16 — Optimization Pass
Status: future roadmap, not started. Depends on prior milestone acceptance.

Goal: Make the game run better.

Build:

- CPU profiling.
- Memory profiling.
- Network profiling.
- Creature spawn limits.
- Structure replication limits.
- Asset size review.
- Texture/LOD/collision review.
- Rendering scalability settings.
- Low-memory test profile for 16 GB systems.

Continue only if:

- Main bottlenecks are known.
- Easy wins are fixed.
- Performance budget is documented.
- The game remains playable on the target machine.

### [To Do] M17 — Audio And Feedback
Status: future roadmap, not started. Depends on prior milestone acceptance.

Goal: Add feedback that makes actions feel real.

Build:

- Footsteps.
- Hit sounds.
- Gathering sounds.
- Crafting sounds.
- Building placement sounds.
- Creature sounds.
- UI sounds.
- Ambient loops.
- Basic mix settings.

Continue only if:

- Audio improves clarity.
- Sounds do not spam.
- Multiplayer sound events behave correctly.
- No copyrighted audio is used without approval.

### [To Do] M18 — Save Compatibility And Migration
Status: future roadmap, not started. Depends on prior milestone acceptance.

Goal: Protect existing saves as development continues.

Build:

- Save version migration.
- Missing data fallbacks.
- Removed item/recipe handling.
- Structure migration.
- Test saves from older versions.
- Error reporting for failed loads.

Continue only if:

- Old saves either load or fail safely.
- No silent inventory/world corruption.
- Migration behavior is documented.

### [To Do] M19 — Content Pass
Status: future roadmap, not started. Depends on prior milestone acceptance.

Goal: Add enough content for a real playable slice.

Build:

- More recipes.
- More items.
- More structures.
- More resources.
- More creatures.
- More world points of interest.
- Better progression pacing.
- Basic goals for 1-3 hours of play.

Continue only if:

- Content supports the survival loop.
- No major new systems are half-finished.
- Balance is playable.
- Docs updated.

### [To Do] M20 — Alpha Slice
Status: future roadmap, not started. Depends on prior milestone acceptance.

Goal: Produce a stable internal alpha.

Build:

- One polished playable region.
- Stable multiplayer baseline.
- Stable persistence.
- First-pass art/audio/UI.
- Known issue list.
- Test checklist.
- Build/package process.

Continue only if:

- Fresh player can understand the game.
- Core loop works without developer help.
- No critical crash in normal single-player flow.
- Multiplayer limitations are documented.

### [To Do] M21 — Beta Preparation
Status: future roadmap, not started. Depends on prior milestone acceptance.

Goal: Prepare for broader testing.

Build:

- Bug fixing.
- Balance pass.
- Onboarding/tutorial improvements.
- Settings menu.
- Keybinds.
- Save backup handling.
- Crash/log collection process.
- Build distribution plan.

Continue only if:

- Testers can install/run the game.
- Feedback can be collected.
- Known issues are tracked.
- No obvious data-loss bugs remain.

### [To Do] M22 — Beta
Status: future roadmap, not started. Depends on prior milestone acceptance.

Goal: Run external or semi-external testing.

Build:

- Tester build.
- Feedback process.
- Crash triage.
- Balance iteration.
- Multiplayer stress testing where possible.
- Save compatibility checks.
- Performance review across hardware.

Continue only if:

- Critical issues are fixed.
- Feedback themes are understood.
- Scope for release is locked.

### [To Do] M23 — Release Candidate
Status: future roadmap, not started. Depends on prior milestone acceptance.

Goal: Make a build that could ship.

Build:

- Final bug fixes.
- Final content lock.
- Final performance pass.
- Final save migration pass.
- Final packaging.
- Final legal/content review.
- Credits/license documentation.
- Release notes.

Continue only if:

- No known blocker bugs.
- Build is reproducible.
- Assets/licenses are clean.
- Game can be installed and played from a clean machine.

### [To Do] M24 — Release
Status: future roadmap, not started. Depends on prior milestone acceptance.

Goal: Ship the game or demo.

Build:

- Release build.
- Store/package setup if applicable.
- Public known issue list.
- Support process.
- Backup branch/tag.
- Post-release monitoring plan.

Done when:

- Release build is published or delivered.
- Source state is tagged.
- Known issues are documented.
- Next patch plan exists.

### [To Do] M25 — Post-Release Support
Status: future roadmap, not started. Depends on prior milestone acceptance.

Goal: Keep the game healthy after release.

Build:

- Hotfix process.
- Bug triage.
- Save migration support.
- Performance patches.
- Balance updates.
- Content updates.
- Community feedback review.

Continue ongoing:

- Fix critical bugs first.
- Avoid breaking saves.
- Keep changes small and testable.

## Programming

### Done
- M0 — PrimalAgentTools editor/development commands and test workflow.
- M1 — Authoritative health/stamina, damage, death/respawn and replication.
- M2 — Hunger/thirst, exposure, finite food batches and spoilage.
- M3 — Inventory, stacking/splitting, pickup/drop and capacity validation.
- M4 — Resource gathering, depletion/respawn, timed crafting/cancellation and food preparation.
- M5 — Placement validation, supported pieces, ownership, damage/demolition and storage.
- M6 — Passive/hostile creatures, navigation/perception, combat, loot and spawn limits.
- M7 — World clock, water/fibre resources and simulation streaming policy (implementation checkpoint; full world gate is still Doing).

### Doing
- M8 — Whole-world persistence integration and reconnect: retain player/batch IDs, stats/location, structure/support/owner IDs, storage, resource depletion, creature/spawner state and pickups. Validate before restore; preserve backup generations and explicit refusals. Finish two-client retry and final regression.

### To Do
- M8 — Packaged dedicated-server validation: installed engine explicitly refuses Server targets; requires a source-built/server-capable engine before a package can be certified.
- M12 — Data-driven tool/weapon/protection tiers, learned recipes and research/technology points. Use Palworld/ARK as structural references only; create original names, costs, prerequisites and balance.
- M12 — Permanent adaptations and limited active/passive biological modifiers: server-validated discoveries, station/range checks, costs, cooldowns, independent per-player gating, save migration. Research direction is in ADAPTATION_DIRECTION.md; gameplay is not implemented.
- M13 — Biome spawn tables, original creature variety and ecology/discovery integration; companions/taming require explicit approval.
- M15 — Multiplayer hardening, join/leave/reconnect, lag tolerance, authority and duplication review.
- M16 — CPU/memory/network profiling and low-memory budgets.
- M18 — Versioned save migrations and removed/missing catalog handling.
- M19 — Data-driven content/progression expansion.
- M20–M25 — Packaging, alpha/beta stabilization, release and patch support; follow Main Board gates.

## Greyboxing

### Done
- M1–M4 — Small primitive player, inventory and gathering/crafting fixtures.
- M5 — Primitive shelter pieces, storage and placement test area.
- M6 — Passive/hostile debug creature shapes and bounded spawn fixtures.
- M7 — 400 × 400 m World Partition candidate L_PrimalFrontier_OpenWorld, camp/zones, hazards, resources, ruins and landmarks. Geometry/asset checks passed; this is an implementation card, not completed M7 acceptance.

### Doing
- M7 — Verify continuous first-person route and overnight loop on L_PrimalFrontier_OpenWorld; keep M7 Doing until human evidence is available.

### To Do
- M9 — Audit already-imported assets for provenance, memory, dependencies, collision/LODs and first-person suitability; request specific gaps when needed.
- M10 — Controlled single-biome art replacement only after greybox gates pass.
- M14 — Expand connected open world, landmarks, biomes and streaming within measured budgets.
- M19 — Additional resource areas and original points of interest.
- M20 — Prepare one polished playable alpha region.

## HUD & UI

### Done
- M1/M2 — First-person vitals HUD and needs/exposure feedback.
- M3 — Placeholder inventory, stack split/drop/consume and freshness display.
- M4 — Crafting recipe/queue/cancel UI.
- M5 — Build selection, placement feedback and storage UI.
- M7 — Pause menu and game/graphics/audio/accessibility settings with display rollback; motion blur defaults off.
- M7 — Keyboard/controller action mappings and PLAYTEST.md guide (physical controller feel remains a separate test).

### Doing
- M7 — Resolve remaining playtest feedback and New Editor Window PIE performance diagnosis without declaring traversal verified.

### To Do
- M8 — Confirm readable user save/reload/reconnect flow in a rendered playtest; developer commands currently provide the interface.
- M11 — HUD/inventory/crafting/building polish, warnings, damage feedback, death screen and controls help.
- M12 — Original technology/research tree and adaptation/modifier station UI; keep permanent unlocks distinct from equipped active/passive effects.
- M21 — Onboarding/tutorial and keybinding usability for external testers.

## Sound & Audio

### Done
- M7 — Audio preference controls implemented (this does not certify authored sound coverage or audibility).

### To Do
- M7 — Verify actual audio preference audibility with existing authored sounds.
- M17 — Audit approved sound sources; implement footsteps, hits, gathering, crafting, building, creatures, UI and ambient feedback.
- M17 — Bounded multiplayer sound events and master/category mix; avoid event spam and unlicensed audio.
- M20 — Audio integration for the alpha region.
- M23 — Final audio credits/license check and mix verification.

## Testing

### Done
- Phase 9/10 — PrimalAgentTools editor/live command checks; preserve rendered Client 2 D3D12 crash as historical limitation, not a multiplayer pass.
- M1–M6 — Recorded build, narrow automation, supported manual and network checkpoints (see MILESTONES.md).
- M7 — Settings regression: 19 passes; two-client NullRHI world checks and open-world streaming probe passed.
- M8 — WorldRecords/WorldRuntime and 12-test hook regression passed; disconnect checkpoint fixture passed.
- M8 — One-client actual server create/save/load and separate-process restart/reconnect: four PF.Persistence.Live process reports passed. This is server-driven automation, not a manual route test.

### Doing
- M7 — Human first-person route and overnight survival gate: not tested/passed yet. Selected Viewport + F11 is the documented presentation workaround.
- M8 — Two-client persistence create/restart retry: first attempt timed out at stage 0 because its stone-node fixture filter required two nodes where only one exists. Test-only filter removed; rebuild passed (7.47 s), M8CreateTwoVerifiedServer/Client1/Client2 each passed without test errors/warnings. M8RestartTwoVerifiedServer/Client1/Client2 also each passed without test errors/warnings. These are automated NullRHI checks, not a manual acceptance pass.

### To Do
- M7 — Physical controller feel and sustained open-world streaming/performance test.
- M8 — Final relevant regression, clean-working-set Editor/Game build and log/report review.
- M8 — Rendered manual gather → craft → shelter/storage → save → close/restart → reconnect → compare values/items/ownership; test corrupt-save refusal and repeat load without duplication.
- M8 — Packaged Server build/run after source-engine setup; current Launcher distribution reports unsupported target.
- M9/M10 — Asset reference/license/collision/LOD/memory and art-replacement regressions.
- M11 — UI readability/controller/accessibility checks.
- M12/M13 — Progression/adaptation authority, per-player gating, costs/cooldowns and persistence; ecology/navigation/spawn budget tests.
- M14/M16 — Streaming, uncapped traversal FPS, CPU/GPU/network and minimum-memory profiling.
- M15 — Network/reconnect/duplication and hostile-request tests.
- M18 — Old/corrupt/incomplete save migration and data-loss tests.
- M19/M20 — Full content loop and fresh-player alpha acceptance.
- M21/M22 — Clean-machine tester installation, feedback/crash triage and beta tests.
- M23/M24 — Reproducible release/package/license/save checks, release tag and delivery verification.
- M25 — Hotfix regression, save compatibility and post-release monitoring.

## Publication status

No Trello board IDs or URLs exist for this draft. Once the connection is available, inspect existing workspace/boards first, create or reuse the six boards, add the three lists, populate cards/checklists and verify them. Record the returned board URLs here. Do not replace this status with a success until Trello confirms creation.
