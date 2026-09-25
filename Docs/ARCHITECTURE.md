
---

## `ARCHITECTURE.md`

```md
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
