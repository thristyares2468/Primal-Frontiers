# Playing and testing Primal Frontier

Updated 2026-10-06. The integrated development level is **L_M7SurvivalArena**. Latest merged-source build, 17-test regression, one-/two-client checks and rendered water/fibre checks pass. Prior rendered interaction/hazard/day-night evidence is in MILESTONES.md; sustained manual walking and overnight survival are still pending. **L_M6Creatures** is the previously verified creature level. The installed engine reports **5.8.3**, despite the original 5.8.2 requirement.

## Open the game

The latest world extension adds primitive woodland, rocky ground, a roofed ruin, fibre and freshwater collection nodes to this same continuous level. At a labelled freshwater node, aim and press E to gather finite portions. Open Tab, select Water portion, and press Q to drink one (controller X in inventory). It restores 35 thirst, restores no hunger, and is not spent at full thirst. Food still has expiry timers. Blue surfaces are visual water placeholders; swimming is not implemented. See [ROADMAP_STATUS.md](ROADMAP_STATUS.md) for the final open-world direction and current greybox limitations.

1. Open `C:\UnrealProjects\PrimalFrontier\PrimalFrontier.uproject`.
2. In the Content Drawer, browse **Content > PrimalFrontier > Maps**.
3. Double-click **L_M7SurvivalArena**. The template `Lvl_FirstPerson` is not the survival test map.
4. In the Play dropdown choose **one player**, **Standalone** net mode, and **Selected Viewport** or **New Editor Window**.
5. Click Play, then click the game view. Move with WASD, look with the mouse and jump with Space.

**P** opens the pause menu. **Esc may stop PIE** before the game receives it. Shift+F1 releases the mouse to the Editor; click the game to recapture it. The Editor Stop button ends the session. No map save is needed to play.

If modules need rebuilding, close the Editor and build **PrimalFrontierEditor / Development Editor / Win64** in Visual Studio. Reflected C++ changes need a full restart rather than Live Coding.

**World/inventory changes currently disappear when the session ends. M8 persistence is not implemented.**

## Keyboard controls

| Situation | Controls |
| --- | --- |
| Movement | WASD, mouse, Space jump |
| Interaction | E aimed at a nearby labelled resource, item, door or storage |
| Combat | Left click with inventory/crafting/building closed |
| Pause | P opens/resumes; Esc also works in a standalone game window |
| Inventory | Tab open/close; Up/Down select; X split half; G drop one; Q consume one |
| Crafting | C open/close; 1 tool; 2 cook; 3 dry; R cancel |
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
| Crafting | Y open/close; X tool; D-pad Up cook, Down dry, Left cancel |
| Building | RB open/close; D-pad Up next, Down rotate; RT place; LB demolish aimed owned piece |
| Storage transfers | In building mode, D-pad Left stores selected bag item, Right takes first stored item |
| Close gameplay overlays | B |

Inventory, crafting and building overlays are mutually exclusive. A remains jump outside the pause menu. Console/debug structure damage use keyboard. Remapping, rumble, device glyphs and a sensitivity screen are deferred. If sticks work but survival buttons do not, check the map and latest compiled module.

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

Limits: placeholder visuals/UI, small arena, no tech tree, third person, completed saves/reconnect persistence, final art or large-world streaming. Verification status is in `MILESTONES.md`.
