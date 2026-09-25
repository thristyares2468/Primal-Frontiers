# Milestone 3 inventory contract

The small `DA_ItemCatalog` data asset defines stable item IDs, display names, Gameplay Tag categories, stack limits, weight, a soft placeholder icon reference, food recovery and shelf life. Initial items are wood, stone and found food. The world uses existing engine cubes; no external assets are added. The catalog is a tiny explicit runtime load. Missing/invalid definitions fail closed.

`UPFInventoryComponent` lives on `APFInventoryPlayerState`. The server owns its bounded array of stack IDs, item IDs, quantities and absolute expiration deadlines. Contents replicate only to the owning player; other players see dropped items, not private inventory. Default capacity is eight slots and 30 kg. Inserts preflight the complete operation before committing; a failed pickup retains its world actor. Drop spawns a validated actor before removing inventory quantity. World pickup has authority, range, line-of-sight, living/possessed pawn and duplicate-use guards.

Matching nonperishables fill existing stacks before creating new ones. Food stacks merge only when both item ID and batch deadline match. Different food batches occupy separate slots so older food cannot become fresh when combined. Splitting and dropping copy the deadline exactly. Expired batches are removed on the server, and every mutation checks expiry before acting. The placeholder UI shows each batch's quantity and remaining seconds. Player consumption uses a selected valid food stack; there is no free-food gameplay request.

The controller accepts only operations on stack GUIDs in its own inventory. There is no grant RPC, client-defined food deadline or client-defined nutrition. Reliable interaction/actions are rate-limited. Public component mutation APIs reject non-authority roles. Item quantities, slot/weight bounds and definition data are validated. Developer hooks `PF.GiveItem Item_Wood 10` and `PF.RemoveItem Item_Wood 2` remain non-Shipping, server-only and refuse ambiguous multi-player selection.

## Controls

- E: trace and pick up a world item.
- Tab: toggle the keyboard inventory overlay.
- Up/Down: choose a stack.
- X: split half into a free slot.
- G: drop one from the selected stack in front of the camera; obstructed drops fail without loss.
- Q: eat one selected food item if it can restore needs.

The placeholder drops float where placed. Physics, drag-and-drop UI, equipment, crafting, storage and technology unlocks are outside M3. The HUD remains separate from the authoritative component and can be replaced by Blueprint presentation.

## Lifetime and save preparation

Inventory starts empty and survives death/respawn because PlayerState outlives the pawn. M3 does not save or restore it across disconnect/reconnect or process restarts. A new connection starts empty; returning client data is never accepted as restore authority. M8 must add authenticated identity, versioned item/batch save records, validated quantities/capacity, missing-item migration and remaining freshness on server world-time restore. Do not serialize transient actor pointers or use player display names as identity. World drops remain in the running server until collected or spoiled; restarting the server resets the unsaved map.

## Verification

`PF.Inventory.Transactions` tests stacking, split conservation, atomic capacity failure, invalid inputs, role guards and batch expiry. `PF.Inventory.WorldTransfers` uses actual PlayerState, pawn and pickup actors to check data loading, transfers, duplication rejection, ownership, eating and respawn retention. `PF.Inventory.Live` is opt-in with `-PFRunInventoryLiveTests -PFExpectedPlayers=1|2`; it exercises real client RPCs and owner-only replication using the M3 map. Its temporary accelerated food lifetime and test grants exist only in disposable automation sessions. Exact results and limitations are recorded in `MILESTONES.md`.
