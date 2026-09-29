\# Architecture and Design Decisions

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
