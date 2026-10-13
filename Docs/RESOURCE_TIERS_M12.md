# Resource and harvesting tiers — M12 technology expansion

Updated October 13, 2026. **Design/backlog only; not implemented or tested.** The user's latest direction supersedes the initial bronze/steel proposal:

**Wood → Stone → Copper → Iron → Diamond → Deepforge Alloy → Resonance Composite (Tech)**

Deepforge Alloy and Resonance Composite are provisional original replacements for the user's final two reference materials. The final tier is explicitly advanced technology. Do not use Netherite, Siftite, Tek, Pal Metal or another reference game's distinctive material names as Primal Frontier item IDs/content. The requested seven-tier structure is authoritative; names, costs and pacing are tunable proposals.

## Current implementation checked

`PFResourceNode::Gather` validates authority, living owned pawn, aim/reach, cooldown and inventory capacity, but has **no resource/tool eligibility gate**. `PFInventoryComponent::GatheringHits` picks the highest valid carried gathering benefit: bare hands one hit, stone tool two, bound stone tool three. Benefits improve action speed without changing finite node yield. The bound stone variant remains in the Stone tier; it is not an extra eighth material tier.

Current five resource categories, two optional learned recipe gates and default recipes are retained. There is no copper/iron/diamond ore, metal processing, mining-grade field, powered harvesting, durability or equipment selection in this planning task. Existing bags/chests still have eight slots, carrying/weight rules still apply and large quantity ceilings do not remove weight. New item IDs do not automatically earn new XP or restore in older catalogs.

## Quality tiers and access requirements

Tool **material quality**, **tool family** and **minimum harvest capability** are distinct. An iron axe is not an iron pick. A faster tool does not automatically gain every harvesting capability. Higher compatible picks can harvest lower-grade nodes; axes/sickles receive their own compatible families. Eligibility comes from trusted data and server-owned actual tools, not a client-supplied tier or merely learned knowledge.

| Tool material | Intended role | Proposed mining access / next materials |
| --- | --- | --- |
| Wood | Cheap starter implements; hand forage and loose stones preserve an accessible start | Ordinary stone; hand-gatherable baseline wood/fibre/food/water remain available. |
| Stone | First useful mining/cutting tools; current stone and bound stone variants belong here | Copper ore, iron ore, coal, flint and basic mineral deposits. |
| Copper | Better early-game speed; future durability benefit, same basic mineral access as Stone | Improves Stone-access harvesting; not a shortcut to diamonds. |
| Iron | First precision/hard-mineral tools | **Diamonds require an Iron pick or a compatible higher pick.** Candidate gold, silver, quartz and obsidian uses/requirements are authored separately. |
| Diamond | High-performance mechanical tools and upgrade base | Deepforge ore and other selected dense late-game deposits. |
| Deepforge Alloy | Original refined late-game alloy; upgrade Diamond gear using rare ingredients at a specialised station | Resonant minerals needed for advanced components; stronger mechanical harvesting, not unlimited output. |
| Resonance Composite — Tech | Original final tier of powered, specialised harvesting equipment | Advanced energy-bearing/dense deposits, precision extraction and bounded assisted harvesting. Needs an actual powered tool and charge; no passive free global gathering. |

Copper is a meaningful quality bridge, not necessarily a mandatory ore-access gate. Stone and Copper may both mine iron; Iron remains the diamond threshold. This follows the requested Minecraft-style structure without forcing every intermediate material into a grind prerequisite. Exact benefits/durability and all candidate minerals require later implementation and balance tests.

Expand resources in tested groups: starter materials first; copper/iron/coal and refined outputs; then diamonds/hard minerals; then original rare ore, alloy ingredients, conductive parts, resonant minerals and energy-cell ingredients. Give every added resource a usable recipe/station or purpose; do not add decorative inventory clutter. Do not add all resources/stations at once or silently enlarge inventory/save limits.

## Progression and crafting rules

- Ordinary tiers use the existing original level/knowledge/prerequisite system for crafting access, plus real ingredients and processing stations when those stations exist. A traded or looted legitimate tool can harvest according to its capability; knowledge gates manufacturing rather than duplicating a second harvest lock unless an explicit specialised operation needs training.
- Do not retroactively delete or downgrade existing stone/bound tools, baseline recipes or saved items. Wood becomes an accessible option, not a reason to invalidate an existing starter save. Exact handling of legacy unrestricted nodes must be explicit and tested before applying requirements.
- Every stage must be reachable without its own output: wood/loose stones bootstrap Stone; Stone mines copper/iron; an accessible early furnace processes iron using obtainable fuel; Iron mines diamonds; Diamond accesses Deepforge ingredients; Deepforge accesses resonant components for Tech. No furnace/tool/energy-cell recipe may require the resource it is supposed to unlock.
- Diamond → Deepforge → Tech can upgrade a lower-tier tool rather than discard it, using an atomic craft transaction. Refusal/cancellation preserves the original item and materials; completion consumes inputs once and creates one new item. Stable item IDs and any future durability/charge state need explicit record validation.
- Tech needs meaningful exploration/research or original challenge discovery plus advanced crafting infrastructure and energy, rather than ordinary levelling alone. Specific challenges, credits, stations and unlock levels are **TBD**, not new implemented boss systems. This adapts the earlier ARK/Palworld references without copying names, story, visual content or exact unlock tables.
- Better compatible tools can improve speed and bounded efficiency, but existing finite yields/respawn, stack/weight/capacity, cooldowns and XP anti-farming rules remain. New resources need an explicit trusted reward-category mapping; no separate unlimited XP budget per ore variant.

## Server, UI and save requirements

Use Gameplay Tags for harvesting families/capabilities and validated data assets for tool grade, node requirements and recipes. Initially derive the best eligible **compatible** tool from actual server inventory to match current automatic tool use; do not invent an equipped-tool UI as already present. Invalid/unknown data refuses safely.

An under-tier/wrong-family/unpowered request must return a readable reason such as **“Requires Iron pick”**, without changing node hits, inventory, stamina, XP, cooldown or energy. Validate ownership, life, current tool, range/aim, power and capacity at commit; atomic failure cannot destroy a resource or partially grant items. A multiplayer client cannot claim to own a Diamond/Tech tool or supply yield/tier/power values.

Prompts show required tool and current eligibility. Item/crafting views show material tier, family, harvest capability, cost/station/knowledge requirement and actual charge/durability only when implemented. Use original placeholder pictures/primitives; no new art is needed. Details must explain Copper's speed benefit even when its mining access matches Stone.

**Compatibility prerequisite:** live restore currently requires exact authored resource/spawner names/counts/definitions. New permanent ore nodes change world layout. Implement and test the bounded content-layout migration first, or keep the first prototype in a disposable runtime fixture. Do not delete old world records, silently bypass validation or overwrite a failed restore. Preserve existing private saves and resource identities; general M18 migration is not already implemented.

## Smallest implementation sequence and future tests

1. Add the minimum trusted tool-family/grade and resource-requirement contract, preserving old defaults. Compile Editor; native data/authority/refusal tests in disposable worlds. No normal-map ore expansion yet.
2. Prove one new gated mineral/tool pair with actual gathered ingredients and an acyclic craft path, then Iron-pick-to-diamonds. Native tests cover bare/under-tier/wrong-family/exact-tier/higher-tier, swapped/dropped tools, full inventory, malformed definitions, unchanged refusal state and finite yield.
3. One server/client first, then two NullRHI clients: actual owning-client gather requests, server-selected capabilities, competing harvesting, privacy and no duplicate output/XP. Save/new-server restart preserves inventory and node depletion without creating/free-upgrading tools. Use disposable save profiles only.
4. One rendered player confirms target prompts, pictures, rejected low-tier harvesting and successful crafted-upgrade harvesting. Existing baseline `/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld`; small regression map `/Game/PrimalFrontier/Maps/L_M7SurvivalArena`. A tier-specific authored fixture/runner is **TBD**, not available yet. Use one 720p window, uncapped when profiling; 1440p remains explicitly deferred.
5. Expand Copper/Diamond/Deepforge/Tech in separate bounded gates only after the first access/craft/save loop passes. Powered Tech additionally tests missing/depleted charge, finite range/yield, cancellation, join/reconnect and restart. Actual manual pacing is separate from automated assertions.

No new user test is assigned until the feature and exact fixture/controls exist. Update this plan, CURRENT_STATE, decisions, milestone evidence and Trello after each real gate; never mark this design as shipped gameplay.

## Reference research and limits

Checked October 13, 2026. Minecraft's official [Pickaxe overview](https://www.minecraft.net/en-us/article/taking-inventory-pickaxe) describes the iron-pick-to-diamonds progression. Its official [Copper Age release notes](https://www.minecraft.net/en-us/article/minecraft-java-edition-1-21-9) introduce Copper tools, with Stone-level damage and higher durability; the [Copper Age overview](https://www.minecraft.net/en-us/updates/copper-age-drop) places their speed/durability between Stone and Iron. The older official [Nether Update notes](https://www.minecraft.net/en-us/article/nether-update-java) describe upgrading Diamond equipment into the late-game material; they are not current recipe specifications. Primal Frontier's names, gates, ingredient quantities and Tech endpoint above are an original design, not a claim every reference-game tier has a distinct minimum mining level.

The earlier ARK/Palworld references inform separate ordinary knowledge and special advanced research, plus increasingly capable/powered harvesting. Indexed [ARK Engrams/Tekgrams](https://ark.wiki.gg/wiki/Tekgram), [Excavation Rig](https://ark.wiki.gg/wiki/Excavation_Rig) and [Palworld Tools](https://palworld.wiki.gg/wiki/Tools) support those broad patterns; direct wiki fetches returned403, so no unverified exact current costs/level tables are adopted. No claim is made that the user-supplied “Siftite” is a vanilla Minecraft material. Refresh primary/current documentation before any reference-specific implementation decision.
