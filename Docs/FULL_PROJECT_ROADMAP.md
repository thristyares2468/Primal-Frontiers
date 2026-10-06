# Primal Frontier Full Project Milestones

Project: Primal Frontier
Engine: Unreal Engine 5.8.2
Style: First-person multiplayer open-world survival game
Primary rule: Build systems first using greybox/placeholders. Do not add final art/assets until the greybox game works.

Hardware note: Developer machine has 16 GB RAM. Unreal may stutter. Keep tests lightweight, prefer small maps, use NullRHI for non-visual automated checks, and avoid assuming two rendered clients will always be stable.

## Global Rules

- First person is the primary gameplay experience.
- Do not implement third person unless explicitly requested later.
- Gameplay authority must live in C++.
- Blueprints may be used for presentation, tuning, UI, and asset composition.
- Multiplayer must be server-authoritative.
- Dedicated server support matters.
- Use placeholder assets, primitives, simple materials, and greybox levels first.
- Do not download or import external assets during milestones 1-8.
- Store project-owned assets under `Content/PrimalFrontier`.
- Run a build and playtest before moving to the next milestone.
- Stop and fix failures before continuing.
- Update docs after major system changes.
- Do not stage, commit, reset, or delete unrelated files unless explicitly asked.

---

## Milestone 0: Project Foundation

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

---

## Milestone 1: Player Survival Foundation

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

---

## Milestone 2: Hunger, Thirst, And Environmental Survival

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

---

## Milestone 3: Inventory And Item Data

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

---

## Milestone 4: Gathering And Crafting

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

---

## Milestone 5: Greybox Building System

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

---

## Milestone 6: Greybox Creatures And Combat

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

---

## Milestone 7: Greybox World

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

---

## Milestone 8: Persistence And Multiplayer Stability

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

---

## Milestone 9: Asset Pipeline Planning

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

---

## Milestone 10: First Art Pass

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

---

## Milestone 11: UI And UX Pass

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

---

## Milestone 12: Combat, Tools, And Progression

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

---

## Milestone 13: Expanded Creatures And Ecology

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

---

## Milestone 14: Expanded World

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

---

## Milestone 15: Multiplayer Hardening

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

---

## Milestone 16: Optimization Pass

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

---

## Milestone 17: Audio And Feedback

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

---

## Milestone 18: Save Compatibility And Migration

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

---

## Milestone 19: Content Pass

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

---

## Milestone 20: Alpha Slice

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

---

## Milestone 21: Beta Preparation

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

---

## Milestone 22: Beta

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

---

## Milestone 23: Release Candidate

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

---

## Milestone 24: Release

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

---

## Milestone 25: Post-Release Support

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