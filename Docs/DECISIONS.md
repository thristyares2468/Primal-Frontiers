\# Architecture and Design Decisions

## 2026-10-06 — Continuous world prototype and finite water

The full M0-M25 roadmap is preserved in FULL_PROJECT_ROADMAP.md; ROADMAP_STATUS.md defines the current M7/M8 execution boundary and the final open-world requirement. Keep the M7 prototype continuously traversable across its zones. Its small bounded geometry is an integration fixture, not a final open-world or streaming implementation. Do not confuse automated path connectivity with a completed manual travel gate.

Add fibre and water through existing item/resource data definitions. Water is a finite, nonperishable placeholder inventory portion; collecting it uses server range/aim/cooldown/depletion/capacity checks, and drinking spends exactly one portion through the existing owned inventory RPC. It restores thirst only and is refused at full thirst. Physical vessels, water contamination, swimming and water simulation are future features. Fibre has no new recipe in this increment. No external dependency or imported art is required.

Recover only missing M7 changes after the user/Claude merge at 6d3ee12; preserve centralized asset paths, named request codes, inventory capacity fix, pickup labels and documentation refactors. Rebuild and retest the merged source rather than treating pre-merge evidence as current certification.

## 2026-10-06 — Preserve user asset imports while finishing the greybox gate

The user has added Adventures_Pack, Bike, DynamicFalling, Modular_Rural_Cabin and Polyphoria and authorized sourcing other assets when needed. Keep these user changes outside the current M7 commit and preserve their paths/references. No imported pack is needed for the small arena verification. Before selecting an asset for a concrete feature, inspect suitability, dependency/memory cost and available usage rights; notify the user if a license, purchase or manual import is needed. This does not expand M1-M8 into final-art production or third-person gameplay.

## 2026-10-04 — Interaction alignment, pause and gamepad input

Keep first-person camera origin on the survivor capsule at the same eye height used by server traces; animated head sockets can differ between rendering and dedicated-server simulation. Use a shared bounded visibility query with 12 cm small-target tolerance and an independent obstruction check. E prioritizes nearby resources/items even when building preview is open. Inventory transactions, reach, life state, cooldowns and authority remain validated on the server. Give visible success/refusal and range prompts.

Pause is local UI: freeze simulation only in Standalone. Network menus explicitly say the world continues. Resume restores game input; ending a session requires two activations and warns that persistence is not yet implemented. Use P in PIE because Editor Esc can end Play. Gamepad survival buttons reuse keyboard action methods/RPCs, while the existing Enhanced Input context supplies analog movement/look and jump. Keep overlays mutually exclusive to make contextual buttons unambiguous. No new plugin or template asset edits. Physical device/OS delivery, feel and hot-plugging are separate from automated binding verification.

## 2026-09-30 — Small integrated greybox arena and server time

M7 reuses existing catalogs and mechanics in a separate 60 x 70 m non-streamed arena. Keep safe, resource and danger zones connected by walkable primitive ground with optional stepped height variation. Baked navigation and two capped spawn points bound AI work. A replicated server clock owns day/night time; clients render two simple directional lights and cannot set server time. No World Partition, water simulation, new art or external dependency is justified at this scale. Engine inspection reports installed 5.8.3; record that version instead of claiming these runs used 5.8.2.

## 2026-09-29 — Bounded server creature AI

M6 uses two original data definitions, native Gameplay Tags for replicated states and 10 Hz server decisions. Built-in AIController/NavMesh handles movement, and server distance/line-of-sight queries provide simple perception. Attack windup rechecks target life, sight and range; finite loot uses existing food expiry and pickup transactions. Each spawn point has one resident and a shared eight-creature limit includes corpses. NavigationSystem is a built-in module dependency for gameplay and its live tests. No behavior-tree asset, external AI plugin or art acquisition is needed. The user has existing art assets: notify them before planning their integration, after the greybox milestone boundary.

## 2026-09-29 — Test hardware and performance reporting

Use the user's 32 GB Ryzen 9 5900X machine as the current reference; record the full supplied configuration and observed GPU-name discrepancy in `TEST_MACHINE.md`. Keep historical 16 GB constraints as historical evidence, not a current measurement. M5 uses an uncapped 960x540 sample with all frames retained, including the capture-start hitch. Do not extrapolate this stationary small-arena sample to native 1440p, large worlds or a different GPU SKU. No persistent rendering settings change is needed.

## 2026-09-28 — Milestone 5 building authority and safe removal

Use a fixed 400 cm structural grid and data-defined piece IDs/costs/health, with view-derived server placement rather than trusting client transforms. Replicate structure ownership/support and reuse the existing inventory for private storage. Transfers retain exact freshness deadlines. Refuse removal or lethal owner damage while a piece supports children or contains items; no automatic cascading loss or refunds. M5 is a cooperative placeholder rule; group access, preservation equipment and persistence require later work. Test uncapped rendering with session overrides and report frame times alongside memory instead of treating a capped FPS value as optimization evidence.

## 2026-09-27 — Milestone 4 atomic completion and finite resource nodes

Crafting keeps ingredients in inventory and snapshots exact stack IDs, quantities and deadlines at start. Completion revalidates and performs one atomic conversion. Moving/consuming/spoiling an input can invalidate the job, clearly reported; cancellation requires no refund space and cannot reset food freshness. Death or pawn replacement cancels work. One active job bounds replication and UI cost. Resource nodes validate first-person aim, reach, cooldown and capacity before spending finite hit counts. The carried primitive tool reduces actions required without increasing total node yield. Recipes and resources are editable catalog entries; portable cooking/drying consumes wood fuel and time, while placeable stations wait for M5. No new plugin or external dependency is added.

## 2026-09-26 — Keep supplied visual references outside runtime content

Preserve the user's eight Palworld/ARK screenshots unchanged under [References/SurvivalGames](References/SurvivalGames/README.md), with an index of relevant readability, first-person framing and progression principles. These are documentation references, not imported assets or a copied art/UI specification. M3 keeps its lightweight readable inventory overlay. Technology progression, creature/world visuals and final presentation remain behind their respective plans and gates.

## 2026-09-25 — Milestone 3 inventory ownership and food batches

Keep inventory on PlayerState, independent of the pawn, so death/respawn does not destroy item state. Replicate bounded stack contents owner-only; replicate world pickups to relevant clients. Use a small editable data-asset catalog and stable item/stack identifiers. Preflight insertion capacity, preserve the world actor when pickup fails, and spawn a drop before removing its quantity. Food from different deadlines occupies separate slots; split/drop/pickup never refreshes it. Consume the selected usable stack through validated server requests. No inventory grant RPC exists. Save/reconnect restoration remains M8; M3 reconnect deliberately starts empty. See `INVENTORY_M3.md`.

## 2026-09-25 — Retain technology-tree references for later planning

The user's Palworld and ARK progression lists inform accessible ordinary unlock points and a separate challenge-earned path, with early essentials, prerequisite/station requirements and later preservation upgrades. [TECH_TREE_DIRECTION.md](TECH_TREE_DIRECTION.md) records the original-game design direction and scope boundary. Do not implement a tech tree while finishing M2 or assume the pasted item roster is an approved feature list.

## 2026-09-23 — Food research and preservation direction

Use Palworld and ARK: Survival Evolved's cooking and preservation progression as mechanical research, documented with sources in [FOOD_AND_PRESERVATION.md](FOOD_AND_PRESERVATION.md). Primal Frontier uses original tuning: per-food lifetimes, found/gathered inputs, later cooking/drying and maintained storage. Five-minute M2 rations are test tuning. Moving food cannot reset freshness, spoiled ingredients cannot create edible meals, and preservation slows remaining decay rather than rejuvenating food. Future inventory tracks freshness by acquisition batch; its expiry UX must be playtested because it differs from sequential stack spoilage in the researched games. These later systems remain planned, not implemented or verified.

## 2026-09-22 — Milestone 2 survival needs and placeholder interaction

2026-09-23 refinement requested by the user: world rations are finite and perishable, never a permanent player ability. Use a replicated server-time expiration deadline and server-side consumption rejection/cleanup. Default shelf life is 300 simulation seconds. Carry expiration into future inventory and cooking; do not add those systems ahead of their milestones. Developer recovery remains a non-Shipping testing hook.

Extend the existing replicated vitals snapshot with food/water reserves and exposure. Integrate threshold damage using time spent below the threshold; keep server authority in C++ and use existing death/respawn handling. Pause stamina recovery while starving, dehydrated or exposed. Health regeneration is optional and off by default to preserve the M1 damage contract.

Use a no-tick primitive hazard actor and the survival component's existing 10 Hz server tick to resolve the strongest overlapping exposure. Add a single-use ration with a validated server view trace and bounded fixed recovery, rather than introducing inventory early. Replicated actor destruction prevents duplicate consumption. A small separate M2 map preserves the M1/template assets. No new engine plugin, external art, third-person mode or world expansion is required.



\## 2026-09-12 — First-person primary experience



Decision: First person is the primary camera and gameplay experience.



Reason: It best supports the intended survival, exploration, combat, and building feel.



Consequence: The game uses first-person arms and held-item meshes locally, while retaining a replicated full-body mesh for other players and future third-person support.



\## 2026-09-12 — Third-person is optional later



Decision: Third person is not an early-development requirement.



Reason: The game should establish a strong first-person experience before taking on the extra animation, camera, UI, collision, and balancing work of a second perspective.



Consequence: Camera architecture remains extensible, but early gameplay is tested primarily in first person.



\## 2026-09-12 — Server-authoritative architecture



Decision: Important gameplay state is owned and validated by the server.



Reason: Cooperative multiplayer, eventual PvP, persistence, and anti-cheat all need a trustworthy authority.



Consequence: Inventory, damage, building, crafting, creature ownership, and progression are validated on the server.



\## 2026-09-12 — C++ core, Blueprint presentation



Decision: Core gameplay systems use C++; Blueprints focus on presentation, tuning, UI, and asset composition.



Reason: C++ improves long-term maintainability, testability, networking control, and performance.



Consequence: Security-sensitive or complex gameplay rules are not implemented solely in Blueprints.



\## 2026-09-12 — Survival-to-Frontier progression



Decision: The main progression transforms players from vulnerable survivors into dimensional explorers.



Reason: This progression provides clear goals while allowing survival, settlement, creatures, technology, and exploration to reinforce one another.



Consequence: New systems, resources, and regions should map to a meaningful progression tier.



\## 2026-09-12 — Original setting and content



Decision: Primal Frontier is an original world, setting, creature roster, technology system, and visual identity.



Reason: It can be inspired by broad survival-game genres without copying another game’s protected expression.



Consequence: Avoid copying names, creatures, lore, structures, maps, assets, UI, sounds, or story content from other games.

# 2026-09-21 — Isolated first-person survival foundation

Use a reusable replicated C++ vitals component and a separate survivor subclass, controller and GameMode. Preserve the template camera, owner-only first-person mesh and remote full-body mesh through new project-owned Blueprint compositions. Keep template assets and the project's default-map settings unchanged.

Health and Stamina mutate only on authority; damage uses Unreal's damage pipeline. Respawn creates a fresh pawn at a PlayerStart. Placeholder UMG presentation reads the current possession and exposes a Blueprint event for future layouts. Hunger/Thirst remain tag interfaces only.

PrimalAgentToolsRuntime now depends on the project game module to call the same validated survival APIs. The game module does not depend on tooling, and command registration remains excluded from Shipping. No MCP server, gameplay RPC for arbitrary developer mutations, external assets or new plugin dependency is introduced. One-player command targeting deliberately rejects ambiguity.

Use component/lifecycle automation, an opt-in live replication test, and a manual first-person test as distinct gates. Use one rendered client and a NullRHI server on the 16 GB machine. Do not infer multiplayer success from role simulation or a previous tooling test.
# 2026-09-28 — Native first-person aim marker

The M4 rendered playtest exposed an offset external crosshair overlay. Add a small centered native HUD marker so the player can aim the server-derived gathering trace reliably. This is presentation only; trace reach and server validation remain unchanged. The repeated forage/cooking/drying playtest passed with this marker. Portable cooking/drying remains the M4 scope; placeable stations, equipment durability and technology unlocks are future work.
