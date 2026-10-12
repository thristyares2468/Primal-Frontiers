# M1/M3 extension — Weight, large stacks and traversal

User-authorized October12,2026; parent Trello https://trello.com/c/DMKjODqr. B/Esc M11 repair fb2b403 is published/checked off first. This request does not complete remaining manual gates or authorize unrelated art/world expansion.

## Audit and intended behavior

Per-unit item weights and bag kg HUD exist. Current bag is hard30kg/8slots, storage60kg/8slots; overweight pickup/craft/withdrawal refuses, so no playable encumbrance. Catalog ceiling1000, inserts/craft outputs10000, inventory/chest UI requests1000; archives validate per-definition quantities/weight, world wrappers hardcode player30/storage60/pickup100000. Existing character has stamina-gated jumping/death/respawn, no dedicated sprint/climb/glide. Stamina recovers only after spending delay, with needs and exposure checks. Remote full-body/owner arms remain.

Treat999k as999,000 maximum per stack. Preserve smaller per-item tuning/nonstackable gear initially; actual baseline stackable defaults will be reviewed in its own slice. No per-item actors/arrays: keep bounded stacks, stable GUIDs, counts, batch deadlines, exact atomic conservation and owner-only contents. Trusted quantities must remain finite/bounded and sum safely in int32; real byte codec/transfer tests precede live defaults.

Player carry weight becomes a soft movement limit while slots remain hard; chests retain hard capacity. Immediately over carrying limit no sprint; increasing excess progressively reduces walking speed, with a tunable10%-over default jump cutoff. Save/load must preserve legal overweight players and large pickups without weakening malformed/duplicate/unknown/deadline/finite checks or hard storage validation. Never mutate personal saves to migrate them.

Sprint is grounded, stamina-consuming and server-owned. Climb initially means a short low-ledge climb derived from server geometry/capsule clearance, not unlimited free-wall climbing. Glide is a greybox falling-state ability with bounded descent/steering/stamina; no final animation/art/equipment promise. Both obey life/weight/stamina and restore normal physics/input on cancellation/landing/death. Exact controls/geometry ranges remain TBD until implementation. Important movement must remain Unreal server validated/predicted or explicitly server acknowledged with tested correction limits; do not accept client transforms or speed claims.

## Bounded verification sequence

1. Shared999k ceiling and transaction/RPC/archive boundary, PF.Inventory.LargeStacks native fixture plus existing transactions/codec. No baseline defaults/carry policy changed yet.
2. Carry-policy/value tests, actual stackable catalog audit/defaults and server save compatibility. Explicit hard-limit fixtures continue validating atomic capacity refusal; do not erase failure/conservation tests just because player policy becomes soft.
3. Sprint/encumbrance and actual movement/owner state tests, low-memory one-client play, then server/one client before two NullRHI where relevant.
4. Bounded climb/glide and obstruction/stamina/death/modal/landing tests, real rendered greybox traversal exercise and relevant network verification.
5. HUD/help/current guides, exact logs/screenshots/memory evidence, updated CURRENT_STATE/MILESTONES/DECISIONS/Trello and scoped Sol commits. Keep human feel/physical devices/sustainedFPS/minimum hardware separate.

Existing /Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld is the rendered gameplay map; no level required for native temporary test worlds. New live fixtures/selectors are TBD until authored; do not instruct tests for absent features. No external art, assets, Blender, MCP, third-person or unrelated Editor settings. Failures stop expansion until the failed slice is fixed/replayed. Full request stays open until all requested functions are implemented and tested; a protocol-only pass is not playable completion.
