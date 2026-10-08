# Technology progression reference and direction

The user's eight visual references are indexed in [References/SurvivalGames](References/SurvivalGames/README.md). Images 05 and 08 illustrate tier grouping, point costs, search/filtering and distinct progression paths. Use these readability principles with original Primal Frontier presentation; the images are documentation only.

Recorded 2026-09-25 from the user's pasted Palworld technology/ancient-technology lists and ARK: Survival Evolved engram/boss-unlock lists. This is a future design reference, not implemented gameplay. The pasted entries include version-specific, DLC, mod and internal-looking names; they are not a verified catalog or a requirement to recreate every entry.

## Principles to carry forward

- Ordinary progression should award points readily through leveling and useful survival activity. Basic necessities should be available early, without grind that prevents eating, drinking or obtaining shelter.
- Use a separate challenge-earned path for exceptional discoveries, inspired by the distinction between ordinary points and boss-earned unlocks. Specific challenges and rewards require an original design; bosses are not part of the current milestone implementation.
- Organize unlocks by readable tiers, prerequisites, point costs and required crafting stations. Make blocked requirements clear in the UI.
- Start with primitive tools, fire, basic cooking and simple shelter/storage. Follow with better processing, durable construction, food preservation and later powered equipment where explicitly planned.
- Bundle related building pieces to avoid spending a point on every cosmetic variation. Keep meaningful equipment choices distinct.
- Unlocking a recipe grants knowledge, not free items. Crafting still requires acquired ingredients, the correct station and server validation. Food still expires; preservation and cooking provide earned ways to manage it.
- Keep Primal Frontier's names, creatures, recipes, fiction, progression layout and balance original. The references inform structure and pacing, not a copied roster or numerical level/cost table.

## Fit with the existing milestones

M2 remains focused on survival needs and perishable world food. M3 item definitions and M4 recipe definitions should leave room for stable unlock identifiers and Gameplay Tags without implementing a full tech tree early. M5 building definitions can use the same recipe/unlock contract. A dedicated progression plan and test gate must define XP sources, level curve, point economy, prerequisites, respec policy, co-op sharing and challenge rewards before implementation. The current eight-milestone plan does not silently expand to an 80-level tree or an endgame boss system.

The independent October 8 planning task is now specified in [PROGRESSION_PLAN.md](PROGRESSION_PLAN.md): provisional XP/point economy, prerequisites, respec/co-op/challenge policy and biological loadout/save/test contracts. The user's later roadmap and continuation instructions allow this planning while M7/M8 manual gates wait; they do not pass those gates or start M12 gameplay.

When implementation is eligible, use these plans alongside [FOOD_AND_PRESERVATION.md](FOOD_AND_PRESERVATION.md), and revisit the supplied lists rather than assuming all entries are approved scope. Validate unlock spending, duplicate requests, prerequisite bypass, authority and save/reconnect behavior. Preserve the existing portable cooking and baseline shelter loop.
