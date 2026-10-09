# Original progression and biological upgrades — M12/M13 plan

October10 optional recipe access: authoritative FieldTools → BoundTool start/completion gate, baseline survival preserved and actual V1 carried gear retained. Native4+legacy1 and one-/two-client access/restart gates passed; rendered fixture corrections/evidence pending in RECIPE_ACCESS_M12.md. This supersedes non-gating metadata statements below. Purchase UI still future; trusted100XP fixtures do not prove leveling pace. Native selector verdict has already been hardened/published6060557.

Final October10 request technical gate: native3/3, one-client4/4, two-client75ms/1%loss6/6 restart, Editor/Game passed. KNOWLEDGE_REQUESTS_M12.md retains discovery/filter gaps and reviewed raw baseline. Owner knowledge purchase/replication/persistence is verified; useful recipe access and purchase UI remain unimplemented. FullM12/M13/human acceptance stays open; incomplete native-filter verdict is a small tooling prerequisite for further gates.

October10 purchase-boundary checkpoint: KNOWLEDGE_REQUESTS_M12.md adds ownedID-only RPC/server life/relationship/cooldown/catalog point checks and private feedback. Native and one-client restart passed; remaining2client/Game gates recorded there. Existing records persist purchases, but recipe metadata remainsnon-gating and UI stillhasno purchase button. Do not present this as playable technology access; usefuleffect/UI and broader rewards stillneedtheir own gates.

October10 read-only feedback checkpoint: PROGRESSION_FEEDBACK_M12.md documents actual owner XP/level/points and selected first-craft reward states in centered crafting. Native3/3 and rendered720p/1440p passed; Game completion recorded there. Spending explicitly unavailable; no purchaseRPC or recipe restriction. This supersedes earlier statements that XP has no UI. Full progression pacing,technology access and M13 remain incomplete/future.

October9 player integration checkpoint: PLAYER_PROGRESSION_M12.md implements owner-private PlayerState records, actual successful first-craft20XP/dedupe and V2 runtime save/restore with V1zero defaults. Native and one-client restart gates passed; remaining live/build gates are recorded there. This supersedes the temporary archive-onlyV2refusal below. No purchaseUI/recipe restriction or gathering/building/discovery reward yet; eight existing recipes alone earn at most160XP, so full progression pacing/ten levels remain incomplete.

October9 world archive checkpoint: WORLD_PROGRESSION_COMPATIBILITY_M12.md adds opt-in owner-bound V2 metadata and V1 omission/defaults; normal writer staysV1 and actual runtimeV2 load explicitly refuses until component capture/restore integration. No live rewards, points UI or recipe gate yet. The native owner/corruption/atomic-refusal gate passed; live baseline/build completion is recorded in that evidence document.

October9 codec checkpoint: PROGRESSION_CODEC_M12.md verifies separate bounded progression bytes and validated legacy V1 player zero-XP defaults. Main save writers remain unmodified/V1; no runtime rewards, world migration or UI. Owner-bound world compatibility and authoritative PlayerState/event integration are still required before playable progression.

October9 native record checkpoint: PROGRESSION_RECORDS_M12.md introduces bounded XP/point derivation, one-shot craft IDs and optional Tech_FieldTools metadata/transactions with native tests. No live player component, event reward, recipe gate, UI or save field yet. Implemented32knowledge/128craft-record limits are deliberately narrower than the proposed future64 technology entries below. A bounded codec/V1 compatibility gate precedes all gameplay integration. The six-entry technology graph and remaining reward/respec/adaptation plans are still proposals.

October9 protection checkpoint: one original woven guard adds25% nonstacking carried creature-hit reduction through server damage, leaving generic/needs/environmental damage untouched. Exact costs/native/rendered/one-/two-client restore evidence: PROTECTION_M12.md. Carried greybox protection is implemented; equipped armor slots, XP/technology/knowledge ledger, adaptations and the proposed save extensions below remain future work. Existing stable item IDs need no schema migration for gear.

October9 weapon checkpoint: original wooden/stone-bound clubs add useful40/60creature melee damage through existing server-owned carried-item behavior, alongside cord/bound tool. Exact implementation/gates: WEAPON_TIERS_M12.md. This does not implement the proposed XP/technology/knowledge ledger, armor or biomods below, nor establish final combat balance. Ordinary core recipes remain available; deliberate save compatibility is required before introducing persistent progression fields.

October9 implementation checkpoint: user explicitly authorized independent M11/M12 work while Personal checks remain unverified. The first bounded tool slice adds fibre cord and a bound stone tool with validated carried performance through the existing atomic recipe path; TOOL_TIERS_M12.md records its exact gates. This is separate from the proposed XP/technology progression below: no points, knowledge, adaptations, armor or save-schema extension is implemented yet. Baseline recipes stay available; numbers below remain provisional. Older prerequisite/planning statements describe the original October8 checkpoint, not an instruction to undo later authorized work.

October 8, 2026. **Planning complete; gameplay not implemented or playtested.** This is an independently actionable Trello planning task while M7/M8 manual acceptance waits. M12/M13 implementation still requires the preceding milestone gates. All numbers below are provisional playtest settings, not measured pacing or final balance. No assets, catalog entries, RPCs or save versions change in this task.

Use [TECH_TREE_DIRECTION.md](TECH_TREE_DIRECTION.md) and [ADAPTATION_DIRECTION.md](ADAPTATION_DIRECTION.md) for reference provenance. This plan defines original land-survival rules; it does not reproduce another game's unlock roster, costs, organisms, UI or fiction. Refresh primary-source adaptation research when implementation becomes eligible.

## Starting knowledge and two reward paths

Keep finding food/water, gathering by hand, Recipe_Tool, Recipe_Cook, Recipe_Dry and the six existing Build_ definitions available from the start. These are the current C++ constructor defaults, not a new inspection of serialized catalog assets. Their ingredients, durations, capacity, support and ownership checks remain mandatory. Unlocks grant knowledge, never free inventory, fuel, freshness or structures. Basic cooking stays portable; do not retroactively require a station for existing recipes.

Ordinary XP awards technology points for useful verified survival actions. A separate discovery-credit path rewards unique original exploration objectives, not repetitive creature kills. XP, credits, learned technologies, lasting adaptations and equipped modifiers are distinct player-owned records. Avoid one currency purchasing everything or a long copied level table.

### Provisional first slice: ten levels

Start at level 1 with zero XP and the baseline knowledge above. The next-level cost at level L is `100 + 50 * (L - 1)` XP for L = 1..9. Clamp this slice at level 10 and 2,700 cumulative XP. Award three technology points per attained level after level 1: 27 total. Grant levels/points once in a server transaction, even when one award crosses several thresholds; do not grant points again on load or respec.

| Level | Cumulative XP | Total earned technology points |
| --- | ---: | ---: |
| 1 | 0 | 0 |
| 2 | 100 | 3 |
| 3 | 250 | 6 |
| 4 | 450 | 9 |
| 5 | 700 | 12 |
| 6 | 1,000 | 15 |
| 7 | 1,350 | 18 |
| 8 | 1,750 | 21 |
| 9 | 2,200 | 24 |
| 10 | 2,700 | 27 |

### XP sources and repeat rules

| Server-confirmed event | Proposed XP | Repeat policy |
| --- | ---: | --- |
| Gather succeeds into owned inventory | 5 | 25 XP per resource category per 30 active-server minutes, five categories maximum |
| First completed craft of a recipe | 20 | Once per player/stable recipe ID; cancellation/failure grants zero |
| First supported placement of a building kind | 20 | Once per player/stable kind ID; removal/rebuild grants no further first reward |
| First reached authored discovery objective | 40 | Once per player/discovery ID after server location/objective validation |

Only successful inventory/placement/craft outcomes produce events. Pickup/drop, split/merge, consuming food, taking damage, passive survival ticks and developer grants award no XP. Do not encourage intentional starvation or repeated death. Kill XP is outside this slice; a future combat reward needs an anti-farming/contribution policy before implementation.

Gather repetition is intentionally a bounded source of ordinary XP, not claimed cheat-proof activity detection. Use one bounded counter per server-defined resource category, driven by monotonic active-server time, with remaining window/counters persisted across restart. PF.SetTimeOfDay cannot reset these windows. Do not create a timer or ledger entry for every gathered item. One-shot ledgers are bounded by the validated catalog; do not drop old ledger entries to make room and accidentally allow awards again.

No claim that these values reach level 10 in a particular number of minutes: node availability and player behavior must be measured. Test that a fresh player can eat, drink, craft a tool and shelter before earning any points, and that varied play is useful without forcing every activity.

## Small technology catalog proposal

IDs below are future identifiers only; the referenced new content does not exist. Use stable IDs plus registered Gameplay Tags for categories/requirements. A display-name change must not change identity.

| Proposed knowledge ID | Minimum level | Point cost | Knowledge prerequisites | Purpose |
| --- | ---: | ---: | --- | --- |
| Tech_FieldWorkbench | 2 | 2 | Baseline | Optional placeable station; preserves portable baseline recipes |
| Tech_ImprovedTool | 3 | 3 | Tech_FieldWorkbench | First distinct tool improvement, not a free tool |
| Tech_InsulatedStorage | 3 | 3 | Tech_FieldWorkbench | First preservation step with separate container-decay implementation |
| Tech_WeatherGear | 4 | 4 | Tech_FieldWorkbench | Equipment route through environmental danger |
| Tech_FuelPreservation | 5 | 4 | Tech_InsulatedStorage | Maintained preservation; cannot renew a batch |
| Tech_FieldResearch | 6 | 4 | Tech_FieldWorkbench | Station for original discoveries/adaptation profiles |

All six cost 20 of the available 27 points, leaving seven deliberately unallocated rather than inventing later-tier content. Graph is acyclic and every prerequisite is affordable before the child. Station availability and materials are additional crafting requirements, not ingredients consumed by learning. No industrial power system, new weapon roster, bosses or 80-level progression is implied.

Later recipe/build definitions reference a knowledge requirement and optional station category/range. Server rechecks owner knowledge, life, station availability, exact ingredients/freshness and capacity at start and completion. Loss of a station or respec during a pending job safely cancels/invalidates it without partial consumption or a refund duplication. Existing craft transactions remain the authority boundary.

### Co-op, challenges and respec

Technology and adaptations belong to each player; no global shared unlock automatically grants them to others. Crafted physical items can be shared through validated existing transfers. A player can use a shared ordinary tool without owning its recipe, but cannot craft it without knowledge. Structures remain after a builder respecs; ownership/access rules remain independent. Do not introduce group storage permissions through progression.

A qualifying authored exploration challenge grants one discovery credit once per player/objective ID. Credit requires the player's own server-confirmed participation/location and condition; another player's adaptation cannot satisfy an individual hazard-capability check. Eligible nearby contributors receive their own bounded reward; avoid exclusive last-hit credit. Later world objectives supply a safe equipment/consumable alternative so an unadapted co-op player is not permanently locked out. No challenge objectives or bosses are added now.

Propose a full technology respec at an owned safe research station, with a 30 active-server-minute cooldown and a data-defined material cost (not a new invented current item). Refund spent ordinary points exactly once, clear all purchased technologies together and preserve XP, discovery records and baseline knowledge. Refuse while crafting, dead, out of range or under the station's server-defined danger condition. Never refund challenge credits or erase/reaward discoveries through respec. Cooldown remaining time persists and pauses offline. A failure leaves knowledge, currency and items unchanged. Final cost and pacing require playtesting.

## Lasting adaptations and equipped modifiers

Lasting adaptation research is a player-specific environmental capability unlock. Optional biological modifiers are an equipped active/passive loadout. Keep their UI and save records separate: learning a trait does not equip every effect. No free starter biomod is inherited from the reference game.

For the first implementation slice, propose one adaptation and two alternative passive modifiers. Current Exposure is a generic 0..1 hazard intensity, not a typed heat/cold model. Before a heat trait can apply, introduce and test server-defined hazard-category Gameplay Tags so it never reduces unrelated danger. Then proposed Heat acclimation reduces the heat contribution by 20% rather than granting immunity. It requires a unique heat-region discovery, Tech_FieldResearch, one discovery credit and data-defined materials; never earn it simply by taking repeated damage. Cold protection remains equipment-based in this slice.

Start with one passive slot and one loadout-budget point. Proposed Heat endurance lowers typed heat-exposure burden by 15% while increasing thirst drain by 15%; proposed Efficient exertion lowers existing jump stamina costs by 10% while increasing hunger drain by 10%. Sprint is not an implemented cost hook in the inspected survivor; extending this trait to sprint needs a separately tested action. Each costs one budget point; the single slot makes them alternatives. No food recovery bonus, spoilage pause or water creation. Numbers/names are proposals. Combine adaptation/modifier reduction factors once (0.8 * 0.85 = 0.68 when both heat effects apply), clamp total heat mitigation at 35%, and apply increased need rates independently. Do not mutate persistent base tuning/vitals to bake effects into saved values.

Reserve one active slot but leave it empty for this first slice. A later deliberate exertion action or short resource-sense pulse needs defined range/duration, server cost, cooldown, movement prediction or visibility rules and input accessibility before implementation. Do not promise a dash or build a general ability framework just for this plan. Switching passives requires the owned safe research station, life/range checks and a five-second server operation; interruption makes no change. Active actions/cooldowns cannot be reset by profile switching.

## Authority, persistence and presentation contract

- Use a focused C++ progression component on the player-owned PlayerState, matching inventory/crafting lifetime across pawn replacement. Client requests contain catalog IDs only; server resolves rewards, prices, prerequisites, station, ownership and material quantities. No grant-XP RPC or client-supplied modifier magnitudes.
- Replicate private XP/points/knowledge/discovery/loadout state owner-only. Replicate only gameplay-relevant public presentation/derived attributes to observers. The server validates each player independently, including join-in-progress; full-body remote presentation remains first person compatible.
- Proposed data assets define stable IDs, categories/requirement tags, costs and bounded effects; Blueprint handles presentation/tuning, not granting. Validate duplicate IDs, dependency cycles, unknown references, non-finite/out-of-range values and incompatible effects before use. No plugin dependency or arbitrary class-path serialization.
- Proposed record limits: 64 technology IDs, 64 discovery IDs, 16 adaptation IDs, one passive and one active ID per player. Bounds are a future catalog design constraint, not a change to current save limits. Persist cumulative XP, spent/unspent points, one-shot award ledger, reward-window state and remaining cooldowns. Derive level/total-earned points from the validated curve; reject impossible spends or conflicts.
- Current PFPlayerSaveFormat and world wrapper are V1 with no progression fields. Add a deliberate versioned extension/migration only in the future implementation, including envelope-size checks. V1 migration initializes zero XP/credits, baseline knowledge and empty loadout; never grant retrospective XP from existing inventory/structures. Preserve vitals, ownership, batches and their offline spoilage policy. Future versions/unknown IDs fail safely; content-removal migration needs an explicit mapping, not silent defaults.
- Restore unlocks/loadout across death/reconnect/restart and recompute effects once from base rules. Do not serialize active effect execution, replay actions or double-apply derived bonuses after repeated load. Paused-offline progression cooldowns and existing offline food aging are distinct policies.
- Placeholder UI must show available points, requirement/reason, recipe/station needs, discovery progress, effect tradeoff and current loadout. Include search/category filtering later without copying supplied reference layouts. No new art is required to prove this system.

## Future implementation and acceptance sequence

1. After prerequisite milestone acceptance, implement the smallest XP/knowledge component and catalog with one optional tool unlock; Editor build and native transaction tests first. Confirm baseline crafting remains available.
2. One rendered player earns/spends points and crafts with actual ingredients; reject duplicate purchases, client grants and prerequisite bypass. Save/restart restores progress without replaying rewards. Then run one server/client and two NullRHI clients for private replication and independent gating.
3. Add one original discovery/adaptation and the two alternative passives only after that gate. Test station switching, thresholds/tradeoffs and death/respawn with existing hazards. No ecology/content expansion is required just to prove the transaction path.
4. Native tests must cover multi-level awards, cap/overflow, reward dedupe/window persistence, failed craft rewards, point accounting/refund, dependency cycles, unknown IDs, full-transaction refusal, passive budget/conflicts, effect clamping, no repeated-load stacking and explicit V1 migration/truncated-save refusal.
5. Live checks must include a less-progressed second player, join-in-progress, respec during queued craft, logout/restart, ownership and invalid station/ID requests. Rendered manual play must confirm useful choices, discoverability and survival without progression. Record uncapped frame times/memory and do not infer rendered performance from NullRHI.

Planning verification: compared with PFCraftingCatalog.cpp, PFItemCatalog.cpp, PFBuildingCatalog.cpp and the M8 save contract. The ten-level curve totals 2,700 XP/27 points; six purchases total 20 points and have valid ordering. These are document consistency checks, **not implemented automation or M12/M13 acceptance**. This documentation-only increment does not require a new Editor build; the existing M8 evidence remains the latest gameplay verification.
