# Codebase guide

A map of the C++ and tooling code: where each system lives, how authority works, how to add content, and how to test it. Every source file also starts with a header note giving its purpose, the milestone commit that added it, and the design doc to read. This guide ties those notes together.

Design intent lives in the other docs (`GAME_VISION.md`, `CORE_LOOP.md`, `ARCHITECTURE.md`, `NETWORKING.md`). Each milestone's scope and test status lives in its `*_M<n>.md` file.

---

## 1. Layout

| Folder | Files | Added in | Read with |
|---|---|---|---|
| `Source/PrimalFrontier/` (root) | `PrimalFrontier*.h/.cpp` | Template `cb67f77` | Unmodified First Person template. The project subclasses or replaces it. |
| `Survival/` | `PFSurvivorCharacter`, `PFPlayerSurvivalComponent`, `PFSurvivalGameMode`, `PFSurvivalPlayerController`, `PFSurvivalHUD` | M1 `d32fbaf`/`f7ed11d` | `SURVIVAL_M1.md` |
| | `PFSurvivalHazard`, `PFRecoveryPickup` (and hunger, thirst and exposure on the component) | M2 `8be2a14` | `SURVIVAL_M2.md` |
| | `PFInteraction`, `PFPauseMenu`, `PFGamepadInput.cpp` | M7 `b5a3165` | `WORLD_M7.md`, `PLAYTEST.md` |
| `Inventory/` | `PFItemCatalog`, `PFInventoryComponent`, `PFInventoryPlayerState`, `PFItemPickup`, `PFInventoryHUD` | M3 `340c538`/`fe30fb6` | `INVENTORY_M3.md` |
| `Crafting/` | `PFCraftingCatalog`, `PFCraftingComponent`, `PFResourceNode`, `PFCraftingHUD` | M4 `e7ffd71` | `GATHERING_CRAFTING_M4.md` |
| `Building/` | `PFBuildingCatalog`, `PFBuildPiece`, `PFBuildingComponent`, `PFBuildingHUD` | M5 `5702d4b` | `BUILDING_M5.md` |
| `Creatures/` | `PFCreatureCatalog`, `PFCreature`, `PFCreatureSpawner`, `PFNavigationBounds` | M6 `0c2d935` (+ `822092f` unity-build tag fix) | `CREATURES_M6.md` |
| `World/` | `PFWorldClock` | M7 `b5a3165` | `WORLD_M7.md` |
| `Tests/` | One file per system: `PF.<System>.<Case>` automation tests | Same commit as the system | Section 6 |
| `Variant_*/` | Epic sample variants (Horror, Shooter) | Template | Not used by the game. See section 7. |
| `Plugins/PrimalAgentTools/` | `PrimalAgentToolsRuntime` (PF.* commands and live network tests) and `PrimalAgentTools` (editor asset checks, fixtures, screenshots) | `ccbad56`, extended each milestone | `PRIMAL_AGENT_TOOLS.md`, `Plugins/PrimalAgentTools/DEVELOPER_COMMANDS.md` |
| `Scripts/` | `Setup<System>Milestone<n>.py` editor scripts that create each milestone's data assets and test map | Same commit as the milestone | Each script's docstring |

### Where state lives

| Object | Owns | Why there |
|---|---|---|
| `APFSurvivorCharacter` (pawn) | `UPFPlayerSurvivalComponent`: health, stamina, hunger, thirst, exposure | Vitals reset with the body on respawn. |
| `APFInventoryPlayerState` | `UPFInventoryComponent` (owner-only replication) and `UPFCraftingComponent` | Items and the craft queue survive death and pawn replacement. |
| `APFSurvivalPlayerController` | `UPFBuildingComponent`, local input, HUD and menu state | Per-player request handling. The controller exists only on the server and its own client. |
| `APFBuildPiece` | Its own `UPFInventoryComponent` used as storage, plus `Builder` (a PlayerState) and `Support` | Structures outlive their builder's pawn. |
| World actors | `APFResourceNode`, `APFItemPickup`, `APFCreature`, `APFWorldClock`, `APFSurvivalHazard` | Replicated to everyone; mutated only on the server. |

---

## 2. The authority pattern

Every gameplay change follows the same route:

```
client input -> controller/component Server RPC (IDs only) -> rate limit -> server derives
the target itself -> validates -> calls the component API (which checks HasAuthority)
-> replication updates clients -> Client RPC sends feedback text
```

For example, pressing **E** on an item:

1. `APFSurvivalPlayerController::Interact()` runs locally and calls `ServerInteract()`.
2. On the server, `ServerInteract` applies the 0.25 s rate limit. `PFInteraction::FindTarget(Pawn)` then re-traces from the server's own eye point. The client never says *what* it is looking at.
3. The target's `TryPickup(Pawn)` calls `UPFInventoryComponent::Grant`, which is atomic and checks capacity and weight.
4. The inventory replicates to the owner only. `ClientInventoryFeedback("Picked up …")` sends the message.

Rules that hold everywhere (keep them when you extend the code):

- **Clients send only IDs, stack GUIDs, quarter turns and small action codes.** They never send positions, targets, amounts of damage or item counts the server would trust.
- **Every mutating API returns `false` when called without authority.** Tests check this by calling it on a client or with `ROLE_SimulatedProxy`.
- **Rate limits.** Interaction, crafting and building requests are limited to one per 0.25 s. Creature attacks have their own cooldown.
- **Fail closed.** Catalog `Find`/`Recipe`/`Resource` lookups return `nullptr` for a missing ID, a duplicate ID or out-of-range tuning values. Callers treat `nullptr` as "refuse".
- **Atomic inventory transactions.** `UPFInventoryComponent` builds a proposed copy of the stacks, applies the whole change to that copy, and commits only if every step succeeds. A partial grant, split or transfer can never happen.
- **Freshness travels with the batch.** Each perishable stack keeps its own `ExpiresAt` deadline through split, drop, pickup and storage.
- **Owner-only privacy.** Inventory and storage contents replicate only to their owner (`COND_OwnerOnly`). Other players see the structure, not its contents.

### Action codes (uint8 in RPCs)

| RPC | 0 | 1 | 2 |
|---|---|---|---|
| `ServerInventoryAction(StackId, Action, Quantity)` | split | drop | consume (quantity must be 1) |
| `UPFBuildingComponent::ServerTargetAction(Action)` | demolish | interact (open storage, toggle door) | owner hammer (25 damage) |

Any other code is ignored. The live tests send code 255 on purpose to prove this.

---

## 3. Data: catalogs and IDs

All content is defined in `UDataAsset` catalogs. Their default entries are set in the C++ constructor and saved as data assets by the setup scripts.

| Catalog (asset) | Class | IDs |
|---|---|---|
| `/Game/PrimalFrontier/Items/DA_ItemCatalog` | `UPFItemCatalog` | `Item_Wood`, `Item_Stone`, `Item_Food`, `Item_Tool`, `Item_CookedFood`, `Item_DriedFood` |
| `/Game/PrimalFrontier/Crafting/DA_CraftingCatalog` | `UPFCraftingCatalog` | `Recipe_Tool`, `Recipe_Cook`, `Recipe_Dry`; resources `Node_Wood`, `Node_Stone`, `Node_Food` |
| `/Game/PrimalFrontier/Building/DA_BuildingCatalog` | `UPFBuildingCatalog` | `Build_Foundation`, `Build_Wall`, `Build_Floor`, `Build_Ceiling`, `Build_Door`, `Build_Storage` |
| `/Game/PrimalFrontier/Creatures/DA_CreatureCatalog` | `UPFCreatureCatalog` | `Creature_Forager`, `Creature_Prowler` |

Components load these catalogs with `LoadObject` from the fixed paths above when no catalog is assigned. Don't move or rename the assets without updating those paths (see section 7).

### Adding content

- **Item:** add an `Add(...)` line in the `UPFItemCatalog` constructor (`PFItemCatalog.cpp`). Give it a weight and a Gameplay Tag, and set a stack limit and shelf life if needed.
- **Recipe:** add an `AddRecipe(Id, Name, OutputItem, Seconds, Tag, {{Input, Count}, …})` line in `PFCraftingCatalog.cpp`. Every input and the output must be valid item IDs, or the recipe fails closed.
- **Resource node type:** add an `AddResource(Id, Name, YieldItem)` line in the same file, then place an `APFResourceNode` with that `ResourceId`.
- **Build piece:** pieces are parallel arrays in `EPFBuildKind` order. A new piece needs a new enum value, a new `Ids`/`Names` entry, the `uint8(D.Kind)>5` bound in `UPFBuildingCatalog::Find` raised, and placement and support rules in `UPFBuildingComponent`.
- **Creature:** add an `FPFCreatureDefinition` in the `UPFCreatureCatalog` constructor and keep it inside the `Find` limits (health up to 1000, speed up to 500, sight 100–2000, damage up to 100, loot 1–10). Then place an `APFCreatureSpawner` with that `CreatureId`.

After changing constructor defaults, check the saved data asset in the editor. A saved asset can keep its old array. `Scripts/SetupGatheringMilestone4.py` shows how to append missing defaults without overwriting edited entries.

Use Gameplay Tags (`UE_DEFINE_GAMEPLAY_TAG_STATIC`) for categories and states, not strings. Give file-local tags a unique name across the module: unity builds merge `.cpp` files, which is why M6 renamed `TAG_PF_CreatureDead` (`822092f`).

---

## 4. System notes

- **Survival** (`PFPlayerSurvivalComponent`): ticks on the server only. Hunger and thirst drain, and reaching zero deals starvation or dehydration damage. Exposure comes from overlapping hazards (or a test override). Stamina is spent by jumping and creature attacks (5 per attack), and recovers over time. On death, `APFSurvivalGameMode` respawns the player after `RespawnDelay`. The inventory stays on the PlayerState.
- **Interaction** (`PFInteraction`): a single sphere-swept eye trace with 12 cm tolerance, 2.5 m gameplay reach and 5 m advisory reach for the "move closer" prompt. It is used for prompts, pickups, gathering, storage and attacks so that client and server agree on the target. The first-person camera is pinned to the same eye point.
- **Gathering** (`APFResourceNode`): one hit per gather with bare hands and two with `Item_Tool`. A node yields `YieldPerHit` items per hit and respawns `RespawnSeconds` after it is depleted.
- **Crafting** (`UPFCraftingComponent`): one timed job per player. Inputs are re-checked and consumed atomically when the job *completes*. Cancelling consumes nothing.
- **Building** (`UPFBuildingComponent`): placement uses a server trace, a 4 m grid snap for foundations, a quarter-turn rotation, support checks, and a wood cost charged only on success. Demolition, and lethal damage, are refused while a piece supports others or its storage is occupied. Only the builder's own controller can damage a piece. This is a cooperative rule; PvP is future work.
- **Creatures** (`APFCreature`): server AI with simple perception. The forager flees and the prowler chases with an attack windup. Movement uses the navmesh built by `APFNavigationBounds`. Loot is a one-time perishable food pickup. Spawners respect a world cap of eight creatures.
- **World clock** (`APFWorldClock`): the server advances the hour. A replicated hour and a `World.Time.Day/Night` tag drive client lighting. `PF.SetTimeOfDay` and `SetHour` are server-only.
- **Input:** `APFSurvivalPlayerController::SetupInputComponent` uses legacy `BindKey` for gameplay keys. `PFGamepadInput.cpp` maps the gamepad to the same functions. Enhanced Input (template `IMC_Default`) handles move, look and jump. Controls are listed in `PLAYTEST.md`.

---

## 5. Developer commands

Type `PF.Help` in a non-Shipping console. Commands run in the process whose console you use; clients cannot run mutating commands. `Plugins/PrimalAgentTools/DEVELOPER_COMMANDS.md` has the full list. `PFCommands.cpp` explains the dispatch flow and how to add a command.

---

## 6. Tests

**Editor and unit tests:** Session Frontend → Automation, filter `PF.`. You can also run them headless with `-ExecCmds="Automation RunTests PF.<Filter>"`.

| Test | File | Covers |
|---|---|---|
| `PF.Survival.Component` / `.Needs` / `.Environment` / `.Lifecycle` | `Tests/PFSurvival*Tests.cpp` | Vitals, needs, hazards, death and respawn |
| `PF.Inventory.Transactions` / `.WorldTransfers` | `Tests/PFInventory*Tests.cpp` | Stacking, split, capacity, expiry, pickups and drops |
| `PF.Crafting.Gathering` / `.Transactions` | `Tests/PFCraftingTests.cpp` | Nodes, recipes, cancel, atomicity |
| `PF.Building.PlacementAndStorage` | `Tests/PFBuildingTests.cpp` | Placement, support, storage, demolition |
| `PF.Creatures.Lifecycle` | `Tests/PFCreatureTests.cpp` | Spawn validation, AI, combat, loot |
| `PF.Interaction.TargetAndPickup` | `Tests/PFInteractionTests.cpp` | Camera/eye alignment, aim tolerance, occlusion, reach |
| `PF.Input.Gamepad` | `Tests/PFGamepadTests.cpp` | Gamepad bindings |
| `PF.World.Clock` | `Tests/PFWorldTests.cpp` | Hour wrap, phase tags, authority |
| `PF.PrimalAgentTools.*` | Plugin tests | Command parsing, blockers, teleport, reports, fixtures, screenshots |

**Live network tests** (plugin `Private/Tests/*Live*.cpp`) run in a real dedicated server plus one or two client processes. Each file's header lists its stages. All of them are opt-in through a command-line flag so they can never run in a normal session.

| Test | Map | Flags |
|---|---|---|
| `PF.Survival.Live` | M1 | `-PFRunSurvivalLiveTests` |
| `PF.Survival.NeedsLive` | M2 | `-PFRunNeedsLiveTests -PFExpectedPlayers=1\|2` |
| `PF.Inventory.Live` | M3 | `-PFRunInventoryLiveTests -PFExpectedPlayers=1\|2` |
| `PF.Crafting.Live` | M4 | `-PFRunCraftingLiveTests -PFExpectedPlayers=1\|2` |
| `PF.Building.Live` | M5 | `-PFRunBuildingLiveTests -PFExpectedPlayers=1\|2` |
| `PF.Creatures.Live` | `L_M6Creatures` | `-PFRunCreatureLiveTests -PFExpectedPlayers=1\|2` |
| `PF.World.Live` | `L_M7SurvivalArena` | `-PFRunWorldLiveTests -PFExpectedPlayers=1\|2` |
| `PF.LiveDeveloperCommands` | any | `-PFRunLiveTests` |

Launch pattern (based on `DEVELOPER_COMMANDS.md`): start the server with `UnrealEditor-Cmd.exe <abs path>\PrimalFrontier.uproject <map> -server <flags> -ExecCmds="Automation RunTests <test>" -unattended -NoSaveConfig`. Then start each client with the same flags plus `-game 127.0.0.1 -RenderOffscreen`. Every process exports a JSON report to `Saved/AutomationReports`. Afterwards, check `Saved/Logs/PrimalFrontier.log` (per `AGENTS.md`).

---

## 7. Known issues and recommendations

These were found while annotating the code. None of them have been changed in behaviour.

1. **Hard-coded catalog paths.** Nine `LoadObject` calls repeat four asset paths. Collect them in one header, or move to soft references or the Asset Manager as `AGENTS.md` prefers, so a rename can't silently break lookups.
2. **Magic action codes.** Inventory and building actions are raw `uint8` values (table above). Named constants would keep the RPC signature and make call sites self-explaining.
3. **Lowered stack limits.** The stack-merge loops in `UPFInventoryComponent` compute `StackLimit - Quantity`. If a designer lowers a stack limit below an existing stack, the result is negative. Clamp it with `FMath::Max(0, …)`.
4. **Item pickup label shows the raw ID.** `APFItemPickup` shows "E: Item_Wood x5" instead of the catalog display name.
5. **Storage transfer keys only work in build mode.** E opens storage anywhere, but U and O (deposit and take) only work while the build overlay is on.
6. **No server target.** `AGENTS.md` names `PrimalFrontierServer`, but there is no `Source/PrimalFrontierServer.Target.cs`, so dedicated-server builds can't be produced yet.
7. **Large controller.** `APFSurvivalPlayerController` holds input, HUD, menu, inventory, crafting and attack request code. Split it into small input and request components when it next grows. Moving gameplay keys from `BindKey` to Enhanced Input actions would also let players rebind them.
8. **Unused template code and plugins.** `Variant_Horror`/`Variant_Shooter` are still compiled (StateTree is needed only for them). The `.uproject` also enables many experimental PCG, Water and Toolset plugins that the game does not use. Removing them would cut build time and editor startup noise. Check for references first.
9. **Docs drift.** `ARCHITECTURE.md` contains a pasted copy of itself inside a code block. The milestone docs are the accurate per-system record.
