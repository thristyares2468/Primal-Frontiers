# Milestone 4 gathering and crafting contract

The M4 greybox gate passed on 2026-09-28; see `MILESTONES.md` for exact tests, evidence and limitations. All visuals use engine primitives and the existing first-person composition.

`DA_CraftingCatalog` holds editable resource definitions and recipes. Resource definitions specify item yield per hit, finite hit count and respawn delay. Recipes specify stable ID, Gameplay Tag category, ingredient quantities, output and duration. Invalid or duplicate definitions fail closed. The item catalog adds a stone gathering tool, cooked food and dried food without changing existing item tuning.

E gathers the node under a server-derived first-person trace within 250 cm. The server checks a living possessed pawn, cooldown, data and inventory capacity before reducing the node. Hand gathering spends one hit; owning the primitive tool spends up to two hits per action, with the same total finite yield. A depleted node rejects further gathering and respawns after its server deadline. Capacity failure consumes no resource. Node state is replicated to clients; private inventory remains owner-only.

Crafting has one timed job per PlayerState. Inputs stay in the bag until completion; the UI explains this. Starting selects exact ingredient stack IDs/quantities, preferring older food. Completion validates the same batches, including unchanged deadlines and current freshness, then atomically removes all ingredients and inserts the complete output. If an input was eaten, dropped, removed or expired, or the output cannot fit, no conversion occurs. This deliberate policy avoids escrow/refund overflow and cancellation exploits. Cancellation consumes nothing and creates nothing; ordinary food decay continues. Death or replacement of the starting pawn cancels the job. There is no client-supplied duration, nutrition, yield, output or deadline.

Initial test recipes:

| Recipe | Ingredients | Time | Result |
| --- | --- | --- | --- |
| Stone gathering tool | 3 wood + 2 stone | 5 s | 1 tool, automatically used while carried |
| Cook food | 1 found food + 1 wood fuel | 6 s | 1 cooked food, 900 s freshness |
| Dry food | 2 found food + 2 wood fuel | 10 s | 1 dried food, 1800 s freshness |

These are original greybox test values, not final balance. Cooking/drying is portable in M4; placeable campfires and station requirements belong to the building milestone. Raw food remains a finite found/gathered item with 300 s freshness. Processed food cannot be fed back into these recipes to refresh its lifetime. Storage preservation, equipment slots/durability, progression unlocks and persistence are deferred.

Expected controls: E gather/pick up, C crafting overlay, 1/2/3 start the displayed recipe while the overlay is open, R cancel, Tab inventory, Q eat selected inventory food. Crafting and inventory remain separate presentation widgets over authoritative C++ components. Screenshots under `References/SurvivalGames` guide readable requirements, quantity/freshness and first-person framing, not copied artwork or interface styling.
