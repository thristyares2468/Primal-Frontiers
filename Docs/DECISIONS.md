\# Architecture and Design Decisions



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
