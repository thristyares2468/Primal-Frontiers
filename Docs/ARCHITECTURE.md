# Technical Architecture

## Technology

- Unreal Engine 5.8.2
- C++ for core gameplay and networking
- Blueprints for presentation, UI, effects, and tuning
- Unreal Gameplay Tags for shared gameplay state
- Unreal Data Assets or Data Tables for configurable content
- First-person primary camera
- Dedicated-server-compatible architecture

## Gameplay Systems

Milestone 8 preparation (2026-10-08): `Persistence/PFPlayerSaveFormat` is a bounded versioned player/batch codec. `PFPlayerSaveAdapter` validates server identity/collision and replaces inventory/vitals/location; `PFSaveFileStore` preserves an active checksummed generation while publishing a verified inactive file. Food uses remaining lifetime minus offline age. PlayerState carries a public ownership GUID, not a reconnect credential. Whole-world saves and reconnect remain future M8 work. See `PERSISTENCE_M8.md`; the user's M7 gate deferral does not certify that milestone.

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
