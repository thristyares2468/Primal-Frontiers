# Adaptation system — research and original design direction

Requested October 8, 2026. Planning only: no adaptation gameplay is implemented in M8. Candidate implementation window is M12 progression, with M13 ecology supplying discovery sources. Revisit after greybox persistence, the environmental model and progression foundations are verified. The current development pass still ends at M8.

## Verified reference observations

Unknown Worlds' official [Adaptive Measures patch notes](https://unknownworlds.com/en/news/subnautica-2-adaptive-measures-update) describe discoverable biological upgrades at world locations, additional passive slots earned through creature scanning, visible scan requirements/progress and save compatibility for prior unlocks. They also report a multiplayer bug where one player's environmental adaptation let an unadapted player bypass a progression check. The [Buddy System notes](https://unknownworlds.com/en/news/subnautica-2-buddy-system-update) report further fixes to joining/progression checks and upgrade feedback. Research checked October 8; Early Access mechanics may change.

Useful inference: discovery can reward environmental capability as well as equipment, while limited passive slots create choices. Each player's capability must be checked independently, including join-in-progress and restored saves. The official notes do not establish a complete current upgrade roster or all underlying formulas; do not invent those details or treat community speculation as verified design.

## Proposed Primal Frontier loop

Explore an ecosystem -> observe or safely collect an original biological sample -> study it at a crafted field-research station -> spend research/materials -> select a bounded adaptation profile -> test it against a specific environmental challenge. Discovery rewards exploration and knowledge, rather than repeatedly standing in danger or killing the same creature. Ordinary crafting/technology unlocks remain a separate progression path.

Start later with two original placeholder traits, such as heat endurance and efficient exertion. Heat endurance reduces one environmental burden but increases water demand; efficient exertion reduces a defined stamina cost but increases food demand. Numbers and trait names are proposals, not final balance. Do not grant universal immunity, free food, effortless underwater survival or bonuses that erase shelters/equipment/preservation. Primal Frontier is a land-focused first-person survival game; pressure-depth gating is not its default progression model.

Use a small active profile with mutually exclusive traits and a clearly explained adaptation budget. Research unlocks persist on the player; switching an equipped profile requires a safe research station, bounded time and server validation. Environmental protection and available consumables offer alternate routes so co-op players can help without granting each other permanent traits. Presentation can remain readable first-person HUD indicators and a simple station menu; no copied icons, UI, organisms, names, lore or art.

## Implementation contract for the later milestone

- C++ authority on PlayerState for stable unlock/profile state; Gameplay Tags and data assets for requirements/effects. Do not serialize arbitrary ability classes or transient actor pointers.
- Server validates station ownership/range, unique discoveries, prerequisites, consumed research materials, budget, exclusions and cooldowns. Clients request IDs only. Attribute modifiers use bounded additive/multiplicative rules and never change persisted base vitals twice on reload.
- Version the future player-save extension deliberately. Preserve unlocks/profile across death, reconnect and restart; reject unknown or mutually incompatible traits. M8's V1 player record must not be silently extended without a migration/test plan.
- Tests: unlock once, duplicate-sample refusal, invalid client request, trait conflict/budget, environmental thresholds, costs/tradeoffs, death/respawn, join-in-progress, two-player independent gating, save migration and no stacking exploit after repeated load.
- Playtest whether discovery is understandable, choices matter, baseline survival still works and co-op never softlocks a less-progressed player. Use existing assets/primitives; ask for specific asset gaps only when this feature reaches implementation.

Before implementation, refresh primary-source research and inspect the then-current progression/ecology systems. This direction schedules the requested feature without expanding the active M8 persistence checkpoint.
