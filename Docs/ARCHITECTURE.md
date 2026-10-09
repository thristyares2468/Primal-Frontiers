# Technical Architecture

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
