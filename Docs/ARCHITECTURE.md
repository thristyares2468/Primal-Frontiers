
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
