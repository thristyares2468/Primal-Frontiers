# Playing and testing Primal Frontier

Updated 2026-10-09. Use **L_PrimalFrontier_OpenWorld** for current open-world/M8 manual acceptance; **L_M7SurvivalArena** remains the small integration regression map. All 35 intended native regressions passed in 32+3 supported batches on df648eb; this is not one combined run or a claim about every later source change. One-/two-client NullRHI checks passed within their recorded scope. Settings were checked in rendered play; sustained walking, overnight survival and rendered full-loop persistence remain unverified. Exact current evidence and next action: [CURRENT_STATE.md](CURRENT_STATE.md). **L_M6Creatures** is the earlier creature fixture. Installed engine reports **5.8.3**, despite the requested 5.8.2 baseline.

For numbered steps copied into Trello, use [TRELLO_TEST_GUIDES.md](TRELLO_TEST_GUIDES.md). G1 is route/overnight, G2 UI/storage/settings, G3 rendered save/restart, G4 physical controller, G5 distribution, G6 source details (no level), G7 listening. Related cards may cite one actual session rather than ask for repeat playtests. A guide is not a passed test.

Building/storage now shares the original theme: selected piece/preview/storage countdown in an inset, separate last server result, controls below. Existing B/N/T/Click/E/U/O behavior and no-preservation storage remain unchanged. Use the same G2 building/storage checks; [BUILDING_THEME_M11.md](BUILDING_THEME_M11.md) records exact bounded automated/screenshot evidence. No full human shelter/storage acceptance inferred.

## Open the game

M12 tool loop: gather4plant fibre at an existing labelled fibre patch (E), then C → Twist fibre cord → Enter/Craft selected. Four seconds later one cord exists. One Stone gathering tool + one cord +2wood become one Bound stone tool via the selected8-second recipe. Carrying it enables3gather hits/45melee damage using the existing first-person primitives; full-node yield remains finite. Browse appended recipes with Up/Down or the row buttons;1/2/3 remain baseline quick recipes. R cancels without consuming ingredients. [TOOL_TIERS_M12.md](TOOL_TIERS_M12.md) has the exact loop, server rules, automation and limits. This is no XP/armor/equipment-slot/durability implementation or human acceptance claim.

**Normal solo play and saved worlds:** with Editor closed, use the command below without a map/save flag. Single player → Create new world (unused ID) or select an existing world → Load selected world. In gameplay **P → Save world**, require visible success, then End session → Confirm end session to return to the menu. In Single player, select a save, type a friendly name in the field below Refresh worlds and choose **Rename selected world**. Names can contain spaces; gameplay save IDs/files/ownership stay unchanged. Do not recreate an existing world to load it. [WORLD_MENU_M11.md](WORLD_MENU_M11.md) has the short guide/evidence; Trello guide G3 covers human replay after the reported M8 failure. Multiplayer menu is currently information only.

```powershell
& 'C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe' 'C:\UnrealProjects\PrimalFrontier\PrimalFrontier.uproject' -game -windowed
```

The older direct-map console workflow below is an advanced alternative; reusing its create-only launch does not load saved data unless -PFLoadSave is appended. End session does not autosave.

**Inventory selection:** if your selected stack expires/disappears, Up/Down or D-pad selects another stack deliberately. Drop/eat/split/store will not act on the item that moved into its old row. Read the selected name/quantity; closing/reopening the bag retains a lost selection until you choose again. [INVENTORY_SELECTION_M11.md](INVENTORY_SELECTION_M11.md) records the fix and evidence.

**Item details:** Tab then Up/Down shows the selected batch's category, quantity/limit, unit/batch weight, actual remaining freshness and per-portion food/water recovery. These are read-only values; the server still validates consumption and caps the recovery. Removed selection never displays another item's details. [INVENTORY_DETAILS_M11.md](INVENTORY_DETAILS_M11.md) records720p/1440p maximum-scale evidence.

**Crafting menu:** C opens the centered recipe browser with item pictures, output/time/fresh ingredient counts, weight, nutrition and shelf life. Click All/Tool/Food/Material or use PgUp/PgDn (controller LB/RB) to filter. Changing category clears selection; Enter does nothing until you explicitly choose a row with Up/Down (D-pad) or its button. Enter or A/X then crafts the selected recipe. Craft selected, Cancel job and Close buttons provide the same actions. Tab then Space activates a focused button.1/2/3 retain quick tool/cook/dry only when that recipe is visible. Browsing preserves the current job; R deliberately cancels. The world keeps running while movement/look are captured by the menu. Close C/Y/B, or P to Pause; movement/look return on close/resume. Ingredients-present is advisory, and expired food does not count. Pause > Controls & help also lists modal category controls. [CRAFTING_CATEGORIES_M11.md](CRAFTING_CATEGORIES_M11.md) records verification and mouse/hardware limits; the original centered menu evidence is in [CENTERED_CRAFTING_M11.md](CENTERED_CRAFTING_M11.md).

**Controls/help:** P → Controls & help. Left/Right or LB/RB selects keyboard/controller; Up/Down or D-pad scrolls; Esc/P/B/Menu returns to Pause. Bindings are read-only and derived from registered controls/current Enhanced Input mappings. Pause alone is safe in standalone; multiplayer continues. Save explicitly before ending. [CONTROLS_HELP_M11.md](CONTROLS_HELP_M11.md) records automated rendered720p/1440p evidence; human/controller acceptance remains unverified and nonblocking for independent development.

The latest world extension adds primitive woodland, rocky ground, a roofed ruin, fibre and freshwater collection nodes to this same continuous level. At a labelled freshwater node, aim and press E to gather finite portions. Open Tab, select Water portion, and press Q to drink one (controller X in inventory). It restores 35 thirst, restores no hunger, and is not spent at full thirst. Food still has expiry timers. Blue surfaces are visual water placeholders; swimming is not implemented. See [ROADMAP_STATUS.md](ROADMAP_STATUS.md) for the final open-world direction and current greybox limitations.

1. Open `C:\UnrealProjects\PrimalFrontier\PrimalFrontier.uproject`.
2. In the Content Drawer, browse **Content > PrimalFrontier > Maps**.
3. Double-click **L_PrimalFrontier_OpenWorld** for current open-world/M8 acceptance; **L_M7SurvivalArena** remains the small integration fixture. The template `Lvl_FirstPerson` is not the survival test map.
4. In the Play dropdown choose **one player**, **Standalone** net mode, and **Selected Viewport**. Press **F11** for immersive view. This avoids the observed slow New Editor Window presentation path; the stationary sample averaged 107.81 FPS at 2560x1392 with 75% render scale.
5. Click Play, then click the game view. Move with WASD, look with the mouse and jump with Space.

**P** is the secondary pause key: press it to open Pause, then press it again to resume. Use P while testing in the Editor because **Esc may stop PIE** before the game receives it. Click the game view first so it receives keyboard input. Shift+F1 releases the mouse to the Editor; click the game to recapture it. The Editor Stop button ends the session. No map save is needed to play.

**P → Settings** opens Game, Graphics, Audio and Accessibility. Apply retains changes; Cancel discards the draft. Motion blur defaults off. Display changes require a 15-second confirmation in standalone; PIE owns its window. See [SETTINGS.md](SETTINGS.md). New Editor Window PIE has an unresolved D3D12 presentation delay. `L_PrimalFrontier_OpenWorld` is the new World Partition development candidate; its full traversal/overnight gate remains pending.

If modules need rebuilding, close the Editor and build **PrimalFrontierEditor / Development Editor / Win64** in Visual Studio. Reflected C++ changes need a full restart rather than Live Coding.

**M8 development persistence is implemented and passes NullRHI automation, but its rendered/manual full-loop playtest is pending. Unsaved changes can still disappear at exit: save explicitly.**

For a repeatable standalone save/restart, launch a Development editor game process on the same map/slot. PowerShell:

```powershell
& 'C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe' 'C:\UnrealProjects\PrimalFrontier\PrimalFrontier.uproject' /Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld -game -PFSaveSlot=ManualSurvival
```

Gather, craft, build and store an item using the controls below. Open the game console and enter **PF.SaveWorld ManualSurvival**. Check for a passed [PrimalAgentTools] result before quitting. Restart with the same command **plus -PFLoadSave**. Compare vitals, inventory, position, structures, ownership and storage. Food ages while closed; expired food should disappear. There is no automatic standalone/server-exit save or timed autosave. A client disconnect checkpoints a configured active server slot; do not rely on that to replace explicit saving before server shutdown.

On multiplayer, PF.SaveWorld/PF.LoadWorld run in the **server console**, not a client. Load at server startup before clients join; use the same host spelling, port and -PFIdentityProfile on reconnect. Private profiles and world files live under Saved/Persistence and must stay out of Git/shares. Corrupt generations are refused for overwrite, with explicit backup recovery on reads; archive separate copies before investigating a failure, never silently delete or replace them.

This step follows the still-pending M7 route/overnight gate. NullRHI tests do not certify these manual controls or visual/performance acceptance. See [PERSISTENCE_M8.md](PERSISTENCE_M8.md).

## Keyboard controls

M11 bag readability: only four rows are visible at once. The displayed row range changes as Up/Down or D-pad selection reaches later stacks; all eight default slots remain available. After an expired/removed selection, choose a current row explicitly. The footer shows the latest feedback; HUD scaling affects the bag, crafting/building and vitals. See [M11_REVIEW.md](M11_REVIEW.md) for the pending hands-on UI gate.

| Situation | Controls |
| --- | --- |
| Movement | WASD, mouse, Space jump |
| Interaction | E aimed at a nearby labelled resource, item, door or storage |
| Combat | Left click with inventory/crafting/building closed |
| Pause | P opens/resumes; Esc also works in a standalone game window |
| Inventory | Tab open/close; Up/Down select; X split half; G drop one; Q consume one |
| Crafting | C open/close; Up/Down select; Enter craft selected; 1 tool; 2 cook; 3 dry; R cancel; clickable row/action buttons |
| Building | B open/close; N next; T rotate; left click place |
| Owned structures | E door/storage; H demolish in build mode; J debug damage in build mode |
| Storage | Select bag item, close bag, open build mode and storage with E; U stores one selected item, O takes one from first stored stack |
| Console | Backtick/tilde, type command, Enter, close console |

## Controller controls

Xbox button names are used below: A bottom, B right, X left, Y top. Connect before Play, then click the game viewport once. One local player is intended. Existing Enhanced Input supplies sticks/jump; the survival buttons use the same server-validated actions as keyboard input. No external controller plugin was added.

**Physical device compatibility, stick feel and hot-plugging remain a hands-on test. Automated binding tests do not certify these.**

| Situation | Gamepad |
| --- | --- |
| Move / look / jump | Left stick / right stick / A |
| Pause / resume | Menu/Start; B also resumes inside the pause menu |
| Pause choices | D-pad Up/Down selects; A confirms; ending a session needs two confirmations |
| Gather / pickup / door / storage | X with overlays closed or building open |
| Attack / place | RT |
| Inventory | View/Back open/close; D-pad Up/Down select; Left split; Right drop one; X consume |
| Crafting | Y open/close; D-pad Up/Down select recipe; A/X craft selected; Left cancel; B close |
| Building | RB open/close; D-pad Up next, Down rotate; RT place; LB demolish aimed owned piece |
| Storage transfers | In building mode, D-pad Left stores selected bag item, Right takes first stored item |
| Close gameplay overlays | B |

Inventory, crafting and building overlays are mutually exclusive. A remains jump outside modal menus. Crafting captures movement/look but the world continues; Pause remains separate. Console/debug structure damage use keyboard. Look sensitivity and inversion are in P → Settings → Game. Remapping, rumble and device glyphs are deferred. If sticks work but survival buttons do not, check the map and latest compiled module.

## First ten minutes

1. **Gather:** approach a labelled Wood pile until `E / Pad X - Gather resource` appears. Aim at the cube, not its floating text. Press E / X several times, roughly a second apart. Check the bag, then repeat at a Stone outcrop.
2. **Tool:** gather at least **3 wood + 2 stone**. Open crafting and choose tool (1 / pad X). Keep ingredients in the bag for the five-second craft. The primitive tool equips automatically while carried and improves gathering/combat.
3. **Pickup:** select wood in the bag and drop one (G / D-right). Close the bag, aim at the small dropped cube and press E / X. The total should return exactly. Repeat with building mode open.
4. **Food:** gather the Forage patch or collect creature loot. Select food in the bag and consume one (Q / pad X). There is no unlimited eat button or free starting ration.
5. **Cook:** one found food + one wood makes cooked food in six seconds. Two found food + two wood makes dried food in ten seconds. These greybox recipes do not yet need a campfire actor. Cancelling produces no output; ingredients are revalidated at completion.
6. **Build:** collect extra wood. Select Foundation, aim at clear ground ahead, move out of the preview and place when the message says valid. Current pieces cost two wood. Add walls, floor/ceiling, door and storage. Support, overlap, resource cost and ownership are server-validated.
7. **Explore:** leave the safe southern clearing for central resources, the stepped rise and the northern danger area. Exposure harms survival; retreat until Exposure is zero. Foragers flee; prowlers chase and attack.
8. **Fight:** close overlays, approach a creature and left-click / RT in reach. Watch stamina/health. Death automatically respawns at PlayerStart. Inventory survives death within this session; that is not disk persistence.

## Food and survival

Default freshness: found food **300 simulation seconds**, cooked **900**, dried **1,800**. The bag shows each batch's remaining time. Splitting, dropping, pickup and ordinary storage keep its deadline. Storage has **no preservation bonus**. Expired food is removed and cannot be eaten. Cooking/drying is the available preservation progression; refrigeration and technology unlocks are deferred.

Found/cooked food restores some water as a greybox recovery mechanic. A complete drinking/container system is not present. Hunger/thirst drain and empty reserves damage health. Solo pause stops simulation/expiry. Multiplayer menus do not pause the world, enemies or food expiry: find safety first.

## If pickup seems broken

- Check the map: ordinary scenery/floor/wall cubes are not inventory items.
- Aim the **native white centre crosshair** at the physical item within about **2.5 m**. External crosshair overlays may be offset.
- `Move closer` means out of range. Walls block interaction.
- Close bag/crafting before pad X pickup: X consumes/crafts in those panels. Building mode now allows nearby pickup.
- Depleted nodes regrow (default 20 seconds). Leave time between presses. Check free slots and weight; expired items cannot be recovered.
- Read the new success/refusal message. Record map, target label, input, message and screenshot if it still fails.

## Self-test checklist

Run one step at a time in a fresh solo session.

- [ ] Walk, look and jump in first person.
- [ ] Gather wood/stone; quantities increase.
- [ ] Split a stack; total stays unchanged.
- [ ] Drop one and recover it; total returns exactly, including with building open.
- [ ] Craft one tool; ingredients decrease once and one tool appears.
- [ ] Cancel a craft; no duplicate output.
- [ ] Consume food; quantity drops once and needs recover.
- [ ] Freshness does not reset through split/drop/pickup.
- [ ] Place a foundation, wall, door and storage; invalid overlap is rejected.
- [ ] Store/retrieve one item without duplication; open/close door.
- [ ] Passive flees; hostile chases/attacks; killed creature yields finite loot.
- [ ] Enter/leave hazard; Exposure rises then returns to zero.
- [ ] Open P menu; solo simulation pauses; Resume restores movement/look.
- [ ] Die and automatically respawn.
- [ ] With a controller, repeat gameplay and pause navigation; record device/connection type.

For a small multiplayer check, use two players, **Play As Listen Server**, **New Editor Window** in the Play dropdown. Start with one player first. Gather/drop/pick up, place a structure and observe it from the other window. Opening one menu must leave the other player/world running. Another player cannot demolish or access owned storage. Two rendered windows are not a clean FPS benchmark; automated dedicated-server runs use NullRHI to reduce memory.

## Optional developer checks

Use one command at a time in solo or on the authoritative server; connected clients cannot grant themselves items or set vitals.

| Command | Use |
| --- | --- |
| `PF.Help` | Commands and implementation status |
| `PF.Damage 25` | Apply server damage |
| `PF.SetStamina 0` | Depletion/recovery |
| `PF.Kill` | Death/automatic respawn |
| `PF.SetHealth 100` | Restore living player |
| `PF.SetHunger 100`, `PF.SetThirst 100` | Restore needs for isolated tests |
| `PF.GiveItem Item_Wood 10` | Fixture grant, bypasses gathering |
| `PF.GiveItem Item_Tool 1` | Fixture grant, bypasses crafting |
| `PF.SetTimeOfDay 22`, `PF.SetTimeOfDay 9` | Night/day in M7 with its clock |
| `PF.ExportTestReport MyPlaytest` | Export command history/results locally |
| `shot showui` | Save game view/HUD screenshot |

Record fixture grants: a cheated bag does not prove gathering. Exported PF command reports do not automatically prove manual checklist completion. Avoid reset commands in a test session you want to keep.

## Performance and bug reports

For uncapped testing enter `t.MaxFPS 0`, `r.VSync 0`, `stat fps`, `stat unit`. Frame smoothing and external caps can still matter; record their state. Record actual game resolution. Prior 960x540 samples are not 2560x1440 benchmarks. Measure one rendered client. Note startup versus persistent stutter and whether an action failed.

Evidence lives under `C:\UnrealProjects\PrimalFrontier\Saved`: `Logs`, `AutomationReports`, `Screenshots\WindowsEditor`. Custom launches use logs such as `PFM7...` instead of `PrimalFrontier.log`.

A useful bug report includes map, solo/client/server, steps, expected versus actual result, displayed message/count, keyboard/controller, approximate time, screenshot and matching log. Exclude credentials/unrelated logs.

Limits: placeholder visuals/UI, small arena, no tech tree, third person, production authentication, save migration, final art or production-scale world. Verification status is in `MILESTONES.md`.
