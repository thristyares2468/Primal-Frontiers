# Food freshness and preservation

Design direction recorded 2026-09-23 at the user's request. M2 world-ration expiration, M3 inventory batch freshness and M4 finite gathering/cooking/drying are implemented; see `INVENTORY_M3.md`, `GATHERING_CRAFTING_M4.md` and the milestone evidence. Storage preservation remains deferred. Current M4 recipes consume fresh gathered food and wood fuel after a server-validated duration; cooked food lasts 900 seconds and dried food 1800 seconds. These are greybox test values, not final balance.

## Research

Palworld generally makes cooked food more nutritious and longer lasting than raw ingredients. It exposes spoil timers and consumes one item at a time from a stack. Cooling storage extends freshness while a suitable worker maintains cooling. Exact effects depend on the current game version and cooling capability; no single universal Palworld multiplier is assumed here. Sources: [Food](https://palworld.wiki.gg/wiki/Food), [Cooling](https://palworld.wiki.gg/wiki/Cooling), community-maintained documentation consulted 2026-09-23.

ARK: Survival Evolved gives perishables individual base lifetimes and applies storage multipliers. Its established progression includes fueled preserving bins (10x) and powered refrigerators (100x); cooking and drying provide additional food choices with longer shelf lives. Stacked items spoil sequentially. These observations concern the Survival Evolved mechanics, not newer Ascended-only structures. Sources: [Spoilage](https://ark.wiki.gg/wiki/Spoilage), [Preserving Bin](https://ark.wiki.gg/wiki/Preserving_Bin), [Cooking](https://ark.wiki.gg/wiki/Cooking), community-maintained documentation consulted 2026-09-23.

Takeaway: collecting food, processing it and maintaining preservation should be a useful survival loop. The following rules and tuning are original Primal Frontier proposals, not claimed values from either game. No names, art, creatures or recipes are imported.

## Intended player experience

- Find or gather food, then cook or preserve it when those systems exist. No unlimited ration supply or free recovery button in ordinary gameplay.
- Show time remaining and a low-freshness warning. Prefer the oldest usable food when consuming equivalent items.
- Raw food lasts less time than cooked meals; dried food is intended for longer expeditions. Cooking costs ingredients, fuel and time.
- Passive insulated storage, fueled preservation and eventually powered refrigeration slow decay. Their upkeep and capacity are visible. When upkeep stops, decay resumes at its ordinary rate without destroying the remaining freshness instantly.
- Expired food cannot restore needs or be made fresh by cooking. Later it may become a non-edible waste/compost item; M2 simply removes the world pickup.
- Moving, sorting, splitting, combining, dropping and reconnecting must never create freshness. Cooking is a server-validated, one-way recipe transformation of still-usable ingredients; no repeated reheating loop.

## Initial balance proposals, not acceptance values

| Food or method | Proposed starting point | Purpose |
| --- | --- | --- |
| Raw food | 15 minutes unpreserved | Encourage prompt use or cooking |
| Cooked meal | 45 minutes unpreserved | Supply a short expedition |
| Dried food | 6 hours unpreserved | Longer trips with processing cost |
| Insulated container | 2x remaining lifetime | Modest early storage benefit |
| Fueled preservation | 5x remaining lifetime while supplied | Resource upkeep and stockpiling |
| Powered refrigeration | 20x remaining lifetime while powered | Later reliable bulk storage |

These are simulation-time targets to playtest against acquisition frequency, hunger drain, session length and storage capacity. They are deliberately configurable. M2's 300-second ration exists to make expiration observable in a short test; it is not the raw/cooked/dried item balance.

## Authoritative freshness contract

M2 assigns each single-use world pickup an immutable server deadline and replicates it. The server rejects expired consumption immediately, including before its cleanup tick. Clients display synchronized remaining time.

M3 inventory keeps freshness provenance per acquisition batch and quantity. Splitting inherits it; merging never refreshes older batches. Unlike the researched sequential stack approach, this batch policy expires each batch at its own deadline, regardless of stack rearrangement. Different deadlines occupy separate slots, and the UI shows each batch's quantity and remaining seconds. The policy remains subject to later balance playtesting.

For preservation, evolve the representation to remaining freshness plus last server evaluation time and decay rate. At a container, fuel or power transition, first account for elapsed decay at the previous rate, then apply the new rate. A 5x preservation effect consumes freshness at one fifth the ordinary rate; it never adds freshness. Do not blindly restart a full deadline when food changes containers.

Cooking consumes validated fresh inputs and creates the recipe's output with its defined lifetime. Ingredients continue aging while queued; validate again at completion, and cancellation cannot refresh returned ingredients. Expired input cannot produce edible output.

M8 must persist freshness with versioned save data and authoritative world time. Proposed policy: decay continues while the server runs, including disconnected players; it pauses while the whole server is stopped. A reconnect or server restart resumes saved remaining freshness, never a full lifetime. Offline real-time decay is a separate future policy, not currently implemented.

## Milestone placement and tests

- M2: found world rations, replicated countdown, expiration removal and expired-use rejection. No inventory, cooking, refrigeration or save claims.
- M3: item definitions, batch freshness, transfer/split/merge/drop tests and oldest-first consumption.
- M4: gatherable ingredients, simple cooking/drying recipes, fuel/time costs, cancellation and stale-input checks. This is the first player-made food loop.
- M5: storage preservation interfaces and a small fueled placeholder where scope permits. Powered refrigeration requires an explicit power-system plan; do not introduce it silently.
- M8: save/load/reconnect freshness and corrupt-state validation.

Required future tests include no timer reset on any transfer, simultaneous batch expiry, preservation upkeep loss/restoration, invalid client freshness requests, expired ingredient rejection, cancellation without duplication, and restart without rejuvenation. Only claim each after its milestone's automated and playable gates pass.
