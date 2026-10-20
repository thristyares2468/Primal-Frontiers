# Technical Architecture

October21 M15: FPFHostedAuthorityState is an unwired game-thread value guard for future trusted adapters, with monotonic send-time lease budget, nonreused local callback correlation, exact restoration/identity prerequisite and durable-ack/uncertainty handling. Five native tests and Editor/Game/sourceServer builds pass; it performs no auth/HTTP/world mutation or actual gameplay freeze. Both hosted GameMode login paths remain unconditionally closed. Its proof inputs must come from real authenticated adapters and validated world restore; value tests are not integration. Timing drift/reserve and wire nonce mapping remain deployment gates. HOSTED_AUTHORITY_STATE_M15.md records exact scope/evidence.

October20 M15: server-only UPFHostingSubsystem explicitly loads pinned PlayFab GSDK after bounded agent-config validation. Actual agent port/map/catalog readiness precedes Standby; health callback is UObject-free, worker shutdown dispatches weakly to GameThread. Default offline Server is unchanged; clients omit SDK. Hosted PreLogin and InitNewPlayer fail closed even after Active, pending durable remote restoration/authenticated identity/writer fencing. Local-agent/process-boundary evidence is not playable hosted/voice acceptance. See PLAYFAB_GSDK_M15.md; vendor license/provenance/known local messages are retained.

October16 M12: replicated APFCraftingStation offers public, stateless camp speed bonuses with native GameplayTag type and actual authority/range/sight checks. Crafting retains server-owned exact-batch conversion, snapshots the best nonstacking station and earned speed, revalidates the same actor until completion and cancels safely on loss. Owner-only job context drives feedback; client proximity preview never grants authority. Static authored station identities are not new persistent player structures; no save schema or resource-layout change. Dedicated servers skip stripped TextRender presentation. CAMP_CRAFTING_STATIONS_M12.md records actual tests and limits.

October15 M15 follow-up: UPFVoiceAuthority derives actual server-world public ID/connection/pawn position/life and ephemeralspace; no client supplied route inputs. Weak controller consent/preferences reset on reconnect, no tick/capture/provider/RPC/save fields. Native3/3 and actual one2/2/impairedtwo3/3 headless gates passed; PROXIMITY_VOICE_AUTHORITY_M15.md holds failure repairs and limits. This is metadata bridge, not playable voice.

October15 M15: Voice/PFProximityVoicePolicy is a pure metadata helper with bounded public participant/space snapshots and default-off consent/PTT/range/mute/block routing. No audio/provider/plugin/RPC added. Caller authority is an explicit upcoming adapter gate; see PROXIMITY_VOICE_POLICY_M15.md for contract, native proof and limits.

October14 follow-up: inventory now owns Items and embedded Player upgrades pages; PFInventoryInput centralizes balanced open/close and preview key routing, preserving existing stat-ID authority and item selection. Separate Pause upgrade entry and ambiguous U/storage binding removed. Stationary wall input has0cost and no recovery; moving12/s remains. No point/save/schema/resource-layout changes. Current proof and limits: INVENTORY_UPGRADES_WALL_REST.md; older4/s-hold notes below are historical.

October14 continuous wall climbing supersedes prepaid ballistic ascent: one authority-approved custom CharacterMovement mode, real capsule sweeps/contact, local short top mantle, continuous12/s moving or4/s holding and no attached recovery. F intent plus saved acceleration only; sequenced owner acknowledgment enables prediction, normal remote movement replication retains full body. Legacy serialized cost fields/RPC are inactive compatibility shims; no save-version/resource-layout change. CONTINUOUS_CLIMB_M14.md records verification and limits.

October14 Copper slice: classify existing stone/bound carried tools as Stone Pick/Axe, append optional Wood tool/Copper ore/ingot/pick and three native recipes. Server resolves actual compatible bag capabilities and explicit refusal details; existing RPCs and centered pictured crafting UI reused. One exact named/tagged OpenWorld15->16 resource layout edge preserves verified original/UTC before runtime apply, normal Save publishes aged current state. Original85actor identities/layout retained. COPPER_TIERS_M12.md details contracts/tests; larger tiers/stations/charge/dimensions and general migrations remain future work.

M12 harvest capabilities (October 14): trusted item family/grade and resource requirement fields use neutral legacy defaults. PFHarvestCapabilities validates exact native families/bounded mechanical grades; InventoryHarvest resolves real fresh owner bag capabilities read-only, and ResourceNode authorizes before mutations. No new RPC, energy system or save-layout change. HARVEST_CAPABILITIES_M12.md separates the native backend gate from upcoming live/new-content gates.

October13 player stats: owner-only PlayerState record gains four bounded allocation ranks, independent Level-1 stat budget versus normal3-per-level knowledge points. Authoritative owned ID-only request/cooldown derives configured native gains for real damage/new-job time/carry threshold/max stamina; death retains record and respawn reapplies capacity. PFXPV3 onlyfor nonzero ranks, oldV1/V2 bytes/defaults preserved; outerworldV2 decodes validated ranks before player max-stamina validation/restore. Centered U/Paused-entry native presentation cannot grant XP or mix budgets. Shared Game-config tuning validated, default values unchanged/no newdependency. PLAYER_STATS_M12.md records proof andlimits. Full-height climb nowfinishes its swept rounded-capsule edge onto a server-approved landing target, detailed STAMINA_CLIMB_M14.md.


October10 gather-window foundation: FPFProgressionRecord adds at most five exact native GameplayTag windows with count/remaining active seconds; pure validated5XP/5credits/1800s transactions only, no live gather hook or running clock yet. InnerPFXP V2 appends bounded windows, V1 defaults empty and empty-window writes stay exact old bytes; outer owner-bound worldV2 unchanged. Current gameplay therefore writes original innerV1. Unknown tag strings never interned; all invalid reads/writes/transactions preserve outputs. GATHER_WINDOWS_M12.md records native and actual temporary-file restore gates. Separate actual server success/elapsed/private replication/restart gate is required before playable gathering rewards.

October10 crafting knowledge UI: PFProgressionDetails describes validated level/cost/points/prerequisites/learned state without mutation; PFCraftingHUD Learn and K/D-right call existing owned ServerLearnKnowledge. No optimistic grants. Optional Craft controls require read-only owner access, with server reservation/conversion checks unchanged. Controls sit inside scrollable selected-item details; summary/feedback remain owner-private. Native3+presentation replay1, rendered720p/1440p and Editor/Game passed; exact evidence KNOWLEDGE_MENU_M12.md. This supersedes historical pending-purchase-UI statements below. Earned pacing and human/fullM12 acceptance remain open.

October10 optional recipe access implementation: FieldTools metadata now gates only BoundTool through a validated read-only owner-record query; authoritative crafting checks before reservation and again before conversion/reward. Baseline tool/cook/dry are forbidden as knowledge assignments; cord/clubs/guard remain unrestricted in current catalog. Missing progression fails closed for optional recipes. Existing/legacy owned gear is retained with zero defaults, not confiscated or retrospectively rewarded. Request feedback now reports recipe access available, while purchase UI remains a separate pending task. RECIPE_ACCESS_M12.md records actual gates; supersedes older non-gating statements below.

October10 knowledge request boundary: owned controller ServerLearnKnowledge sends only lookup ID. UPFProgressionComponent validates own living pawn/controller/PlayerState/server world,0.25active-world cooldown and pure complete-record purchase before atomic commit. Record/feedback owner-only; no XP/price/target claim,no item generation. ExistingV2 saves preserve knowledge/derived spentpoints. No UI button/recipe restriction yet; metadata remainsnon-gating and crafting says spendingunavailable. KNOWLEDGE_REQUESTS_M12.md records native/live gates; this supersedes earlier no-purchase-RPC statements below.

Latest M12 runtime integration supersedes the earlier temporary archive-only V2 refusal below. UPFProgressionComponent lives on PlayerState, replicates records owner-only and earns first-craft XP only after the existing server inventory conversion. Runtime world capture/restore is V2 with explicit V1 zero defaults; logout publishes schema/records together so pre-first-save reconnect works. All progression preflights join the existing inventory/location checks. Native knowledge metadata remains non-gating; no purchase RPC/UI or gathering/building reward window yet. PLAYER_PROGRESSION_M12.md records exact current gates and retained failures.

FPFWorldSaveFormat now supports explicit owner-bound progression in opt-in world V2 metadata with atomic semantic DecodeValidated. Normal runtime writer remainsV1, omitting the new reflected field; legacy reads use empty defaults. Current runtime refuses V2 before mutation until a server PlayerState component can preserve its records. WORLD_PROGRESSION_COMPATIBILITY_M12.md separates these archive contracts from unimplemented gameplay integration.

Progression now has a separate bounded native byte codec and validated V1 player default adapter. It is not called by existing world/player writers; their schema and runtime access remain unchanged. Known-name resolution/atomic output refusal and zero retrospective XP are tested. PROGRESSION_CODEC_M12.md defines the upcoming owner-bound world archive compatibility gate before player integration.

M12 progression currently consists only of bounded native records/catalog and atomic validated transactions. XP/knowledge/credited-craft IDs have no owner component, RPC/event/UI hook or save integration yet. Levels/points are derived; baseline recipes remain unrestricted. PROGRESSION_RECORDS_M12.md defines the separate codec/V1 compatibility gate before authoritative PlayerState integration.

M12 protection derives the best fresh validated carried guard from owner-only inventory. Server survivor damage recognizes actual creature causers; generic/needs/exposure bypass mitigation. Public health replication shows resulting damage; no protection cache, extra replicated modifier or save field. Existing timed recipe transaction and V1 stable-ID inventory restoration remain authority boundaries. PROTECTION_M12.md records25% nonstacking behavior, data bounds and exact verification; no armor slot/worn-mesh system implemented.

M12 weapons derive melee strength and held item from validated fresh owned inventory independently from gathering benefit. Public character replication carries only held item ID; existing local/remote engine primitives show a club shape. Timed recipe component, server trace/cooldown/life/stamina and V1 stable-ID saves remain the authority boundaries. See WEAPON_TIERS_M12.md; explicit equip slots, PvP,XP,armor and save extensions are separate future slices.

## Technology

- Unreal Engine 5.8.2
- C++ for core gameplay and networking
- Blueprints for presentation, UI, effects, and tuning
- Unreal Gameplay Tags for shared gameplay state
- Unreal Data Assets or Data Tables for configurable content
- First-person primary camera
- Dedicated-server-compatible architecture

## Gameplay Systems

M12 bounded tool tier (2026-10-09): validated item-catalog GatheringHits/MeleeDamage fields replace hardcoded Item_Tool benefits. Server inventory derives the best valid fresh carried benefit per action; resource gathering bounds hit spending by remaining hits and retains fixed total yield, while melee retains server trace, range, cooldown and stamina. Existing replicated held-tool boolean now follows actual gathering benefit so upgrades retain first-/remote primitive presentation. Cord and bound-tool recipes use existing exact-batch atomic crafting. Stable item IDs fit V1 saves; no XP/knowledge/equipment-slot/durability/armor system or schema migration is introduced. TOOL_TIERS_M12.md distinguishes implementation and verification limits.

M8/M11 world menu (2026-10-09): PFSessionGameInstance owns explicit one-shot solo New/Load travel intent. PFMainMenu uses existing Engine Entry and PFWorldMenuModel validates bounded slots, catalog records and an allowlist of existing gameplay maps before selection/loading. Stable world save IDs stay independent of cosmetic display names in a version1 checksummed Metadata_WorldNames registry. Pause Save routes directly to the existing server-only WorldSubsystem from local authority, with visible success/refusal; no client save/load RPC or new service. Standalone sole-owner credential adoption is separate from network authentication. Original texture-free PFUITheme is presentation only. Game default is Entry; server default is L_PrimalFrontier_OpenWorld. No EditorStartupMap/template assets changed. See WORLD_MENU_M11.md for verified scope and missing multiplayer-menu features.

Milestone 8 integration (2026-10-08): `Persistence/PFWorldPersistence` is a server-only WorldSubsystem using bounded versioned catalog/value records (`PFWorldSaveData` / `PFWorldSaveFormat`) and built-in Json/JsonUtilities. It persists player vitals/location/batches, structure ownership/support/storage, resource depletion, creature/spawner identities/state, pickups and clock. `PFPlayerSaveAdapter` validates server identity/collision and replaces inventory/vitals/location; `PFSaveFileStore` preserves an active checksummed A/B generation. Food ages offline; other world timers pause; crafting cancels. GameMode startup/deferred PostLogin restores; controller destruction captures logout before pawn removal. `PFLocalPlayer` stores a private development reconnect capability by endpoint/profile; the replicated PlayerState GUID is only a public ownership ID. LocalPlayerClassName is the only new project config. Tooling commands remain non-Shipping adapters, not core authority. One-/two-client NullRHI restart tests pass; manual gates remain pending. This is not production authentication or globally rollback-atomic restoration. See `PERSISTENCE_M8.md`.

Milestone 7 input and interaction (2026-10-04): `PFInteraction` shares a server-compatible eye trace between prompts, pickup and resource gathering. The survivor camera is aligned to that stable eye. `PFPauseMenu` is a replaceable placeholder UMG presentation; `PFSurvivalPlayerController` owns local menu/input state and never pauses network simulation. `PFGamepadInput.cpp` maps contextual gamepad buttons to existing validated action paths. Existing Enhanced Input assets retain analog movement/look/jump. No template binary asset changes or new dependency are required. See `PLAYTEST.md` for keyboard/gamepad controls and verification limits.

Milestone 7 adds a bounded separate survival arena and `APFWorldClock`. Authority advances and validates time; the replicated hour/Gameplay Tag phase drives lightweight client sun/night-fill presentation. Existing catalogs, interactions, hazards and capped AI are reused. No streaming or extra runtime dependency is introduced. See `WORLD_M7.md`.

Milestone 6 adds catalog-driven primitive creature characters, capped spawn points and bounded navigation. Server AI owns perception, path requests, attack windup/damage and one-time perishable loot. Clients receive movement, health, target and Gameplay Tag state; first-person attacks use an owning-controller RPC with server-derived aim/range, stamina cost and cooldown. See `CREATURES_M6.md` for scope and gate status.

Milestone 5 adds a building component on the owning controller, a structure catalog, replicated primitive structure actors and a separate placeholder HUD. Placement requests contain only an ID and quarter turn; authority derives the view trace, grid and support and charges validated wood costs. Runtime structure ownership references PlayerState. Storage reuses owner-only inventory replication and preserves batch deadlines through atomic server transfers. See `BUILDING_M5.md` for support, demolition and current scope limits.

Milestone 4 adds a data-asset resource/recipe catalog, replicated primitive resource nodes and an owner-only timed crafting component on PlayerState. Completion atomically converts exact input batches; cancellation consumes nothing, and stale inputs or insufficient output capacity fail without partial conversion. The controller derives gathering targets from server traces and accepts only recipe IDs/cancel requests for its own queue. Carrying the tool enables a replicated equipped indicator with local first-person and remote primitive presentation. See `GATHERING_CRAFTING_M4.md` for the contract and current test status.

Milestone 3 adds a small item-catalog data asset and an owner-only replicated inventory component on `APFInventoryPlayerState`. Stable stack GUIDs and per-batch server deadlines survive pawn replacement. The player controller validates split/drop/eat RPCs against its own inventory; pickup traces and world-actor transfers run on the server. A separate native placeholder inventory widget handles keyboard presentation. See `INVENTORY_M3.md` for transaction, privacy and save-preparation limits.

Milestone 2 extends the same survival snapshot with food, water and exposure. A primitive hazard region is sampled by the authoritative survival tick, and a single-use ration is consumed through a validated server view trace on the survival controller. The client supplies no recovery quantities. See `SURVIVAL_M2.md` for threshold timing, UI and test contracts.

- Player character
- Camera and interaction
- Attributes and survival needs
- Inventory and equipment
- Items and resources
- Gathering
- Crafting
- Building
- Creature AI
- Creature companionship
- Combat
- Technology progression
- Frontiers and world progression
- Saving and persistence
- Multiplayer networking

## Authority Rules

- The server owns important game state.
- Clients request actions from the server.
- The server validates requests.
- The server replicates approved results.
- Clients must never directly grant items, damage, structures, resources, or progression.

## Project Layout

Milestone 1 isolates survival under `Source/PrimalFrontier/Survival`: a replicated vitals component, first-person survivor subclass, authoritative respawn GameMode and local presentation controller/HUD. Project-owned Blueprint compositions and `L_M1Survival` live under `Content/PrimalFrontier`. Templates retain their existing assets and behavior. PrimalAgentToolsRuntime depends on the game module for development command adapters; the game module has no reverse dependency. See `SURVIVAL_M1.md` for the authority and respawn contracts.

```text
Source/PrimalFrontier/          Core C++ game code
Content/PrimalFrontier/         Game assets
Plugins/PrimalAgentTools/       Editor and test automation
Docs/                           Design and technical documentation
Config/                         Unreal project configuration
```

For a file-by-file map of the C++ code, the authority pattern and how to add content, see `CODEBASE_GUIDE.md`.

October18 M15 transport: `PFCheckpointClient` is an opt-in dedicated-server HTTP adapter, with process-environment credentials read only after authority checks, an allowlisted HTTPS endpoint,30s timeout and6MiB streamed response bound. `PFCheckpointProtocol` preserves the exact fileV1 .pfs envelope as Base64, originalUTC/generation, SHA256 (engine-bundled OpenSSL) and existing CRC. Public memory envelope APIs share the ordinary file reader's validation. Developer adapters asynchronously upload inspected existing saves or verify fetched bytes without publishing/restoring them; pending requests never report successful saves. Catalog/map/layout restore, production identity, writer leases and automatic startup/admission remain follow-up work. [Contract and evidence](CHECKPOINT_TRANSPORT_M15.md).
