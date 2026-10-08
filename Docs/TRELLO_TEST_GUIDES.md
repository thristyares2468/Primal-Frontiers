# Trello test guides

Updated October 9, 2026. These guides are the current next-action instructions, not evidence of completion. Related cards use the same guide so one actual session can supply evidence to multiple cards. Manual gates remain unverified until observations arrive. Future features require actual implemented controls and a named test fixture before asking the user to test them.

## route

Guide G1 — open-world route and survival (Personal)
Level: /Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld
Content Drawer: Content > PrimalFrontier > Maps > L_PrimalFrontier_OpenWorld.
Use this open-world map, not Lvl_FirstPerson or the older L_M7SurvivalArena regression map.

Start here: open PrimalFrontier.uproject and double-click that level. No map/assets need saving.

Launch: Play dropdown > Number of Players = 1 > Net Mode = Play Standalone > Selected Viewport. Click Play, click the game view, then F11. "Play Standalone" here is the net mode; Selected Viewport is the presentation mode. Do not select New Editor Window for this first check. WASD moves, mouse looks, Space jumps. P opens Pause; use P to resume. Esc can stop PIE. Shift+F1 releases the mouse; the Editor Stop button ends PIE.

Test in order:
1. Walk/look/jump around the safe southern camp. Expected: first-person control, solid ground, readable Health/Stamina/Hunger/Thirst; jump changes stamina and it recovers.
2. Aim at a nearby physical Wood pile cube (not its floating label), press E about a second apart, then do the same at Stone outcrop. Tab opens the bag. Collect 3 wood + 2 stone; C then 1 crafts a tool in five seconds. Expected: ingredients consumed once, one tool appears. R cancels a pending craft; cancelled craft gives no output.
3. Gather found food from a Forage patch. Tab, Up/Down selects, Q consumes one. At a labelled freshwater node, E gathers a water portion; select it and Q drinks. Expected: quantity falls by one and the appropriate hunger/thirst rises; food shows a decreasing expiry time. There is no unlimited starting food.
4. Walk from camp to woodland/grassland, rocky ground/stepped rise, roofed ruin, freshwater edge and a distant FRONTIER LANDMARK; return south. Expected: no falling through ground, persistent missing collision or broken interactions after travel. Water surfaces are placeholders: swimming is not implemented. Record any confusing route/resource spacing.
5. On clear ground gather extra wood. B opens building; N cycles pieces, T rotates, left click places a valid preview after you move clear of it. Each current piece costs two wood. Place a foundation and supported wall/door/storage. Expected: valid placement succeeds once; overlapping or unsupported placement is refused without consuming wood.
6. Close overlays. Observe passive creatures flee and avoid or engage a hostile with left click in reach. Enter then leave a hazard briefly: Exposure should rise then return to zero outside. Expected: threats/hazards affect server-owned health; no need to die deliberately. If death occurs, record automatic respawn and retained session inventory.
7. Remain in ordinary gameplay through dusk/night and return to a safe shelter until morning. Gather/eat/drink when needed. Do not use time skips, grants, teleports or stat resets for this acceptance. Expected: a usable complete gather/craft/build/survive loop through a real night. If time is unavailable, report overnight NOT TESTED rather than assume it passed.
8. For uncapped measurement open console (~), run "t.MaxFPS 0", "r.VSync 0", "stat fps", "stat unit" one at a time; close console. Walk the same route and record typical FPS and hitches, not just a paused/stationary value.

Pass when the route and overnight steps actually work with no new serious gameplay errors. A partial session is useful; list unfinished steps. M8 save/restart is separate G3.
What to report here: PASS / FAIL / NOT TESTED for each numbered step; exact map and launch mode; failed step + input + on-screen message + expected/actual quantity; approximate time and screenshot if useful. For performance include actual resolution, quality/render scale, FPS range while walking and noticeable hitches. Stuttering alone is not a gameplay failure, but record it.

Logs/screenshots: project-local Saved/Logs (PrimalFrontier.log for ordinary PIE; use the matching unique log for custom launches), Saved/Screenshots/WindowsEditor. Console "shot showui" captures the HUD. Do not upload private saves, identity profiles or full connection/login URLs.

Keep this card open until actual observations are recorded. Automation and this guide are not a manual pass. Related cards may cite the same playtest report; no need to repeat the same route for every board.

## ui

Guide G2 — first-person UI, pickup, storage and settings (Personal)
Level: /Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld
Content Drawer: Content > PrimalFrontier > Maps > L_PrimalFrontier_OpenWorld.
Use this open-world map, not Lvl_FirstPerson or the older L_M7SurvivalArena regression map.

Start here: open PrimalFrontier.uproject and double-click that level. No map/assets need saving.

Launch: Play dropdown > Number of Players = 1 > Net Mode = Play Standalone > Selected Viewport. Click Play, click the game view, then F11. "Play Standalone" here is the net mode; Selected Viewport is the presentation mode. Do not select New Editor Window for this first check. WASD moves, mouse looks, Space jumps. P opens Pause; use P to resume. Esc can stop PIE. Shift+F1 releases the mouse; the Editor Stop button ends PIE.

Test in order:
1. Gather a few wood/stone batches normally (E). Tab opens inventory; Up/Down deliberately selects a named stack. X splits half, G drops one. Close Tab, aim at the dropped cube within about 2.5 m, E picks it up. Expected: split preserves total; drop/pickup restores exact total, not duplicated items. Repeat pickup with B building mode open.
2. Scroll through bag rows if you have more than four stacks. Let an expendable food batch expire or remove its selected stack. Expected: rows remain readable; actions do not silently switch to the item that slid into its old row. Up/Down explicitly chooses a new stack. Report if the footer/messages cover the crosshair or exceed the screen.
3. C opens craft: 1 tool costs 3 wood + 2 stone, takes five seconds; 2 cook costs 1 found food + 1 wood, six seconds; 3 dry costs 2 food + 2 wood, ten seconds. R cancels. Expected: clear progress/refusal feedback and exact single output. Current recipes need no campfire actor. Do not test nonexistent technology unlocks.
4. B opens build: N next piece, T rotate, left click valid placement. Gather enough wood; two wood per current piece. Foundation supports walls/door/storage. Expected: preview and valid/refused message readable while aiming; invalid placement spends no resources.
5. Tab select a wood stack, close Tab; in B mode aim at your storage, E opens it, U stores one, O retrieves one from the first stored stack. E toggles your door. Expected: bag + storage total conserved; quantities and ownership messages readable. A closed bag retains the selected GUID for storage.
6. P opens Pause. Open Controls & help; Left/Right switches keyboard/controller, Up/Down scrolls; return to Pause and Resume. Expected: no accidental jump/action while browsing; mouse/look/movement return. Solo world pauses; multiplayer world does not.
7. P > Settings. Browse Game/Graphics/Audio/Accessibility. Change a draft value, choose Cancel, reopen: expected original value retained. Then choose a small intentional HUD-size or look-sensitivity change and Apply: expected visible effect and retained preference. Restore your preferred value afterward. Motion blur may stay off. Record exact applied setting; Preferences live on this machine separately from world saves.
8. Test graphical/display confirmation only in a separate standalone -game window (G3 launch), not PIE which owns its display. Apply a supported resolution/window mode, leave confirmation unanswered: expected return to prior display within 15 real-time seconds. Do not choose unsupported modes or treat missing audio as a slider pass.
9. While moving and gathering, inspect vitals, urgent/damage messages, crafting/build feedback, bag readability and centre aim. Expected: understandable feedback without losing first-person control. Pause reconnect-profile message is local device storage, not proof of a world save. Server setup acknowledgment is separate: fresh connection says new survivor; after actual save/restart it says restored. Compare the real label; neither label proves world saving or all actors have replicated.

Physical controller feel is G4; rendered persistence is G3. These steps can be done during G1 instead of repeating a route.
What to report here: PASS / FAIL / NOT TESTED for each numbered step; exact map and launch mode; failed step + input + on-screen message + expected/actual quantity; approximate time and screenshot if useful. For performance include actual resolution, quality/render scale, FPS range while walking and noticeable hitches. Stuttering alone is not a gameplay failure, but record it.

Logs/screenshots: project-local Saved/Logs (PrimalFrontier.log for ordinary PIE; use the matching unique log for custom launches), Saved/Screenshots/WindowsEditor. Console "shot showui" captures the HUD. Do not upload private saves, identity profiles or full connection/login URLs.

Keep this card open until actual observations are recorded. Automation and this guide are not a manual pass. Related cards may cite the same playtest report; no need to repeat the same route for every board.

## persistence

Guide G3 — rendered save, close, restart and reconnect (Personal)
Level: /Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld
Mode: one rendered standalone Development editor game process first. This is a separate -game window, not Selected Viewport PIE and not NullRHI. M7 route/overnight acceptance is still open; independent AI work remains allowed.

Start here: close the Editor, then start the exact game command below in PowerShell. ManualGuide20261009 is an example UNUSED test slot. If it already contains a save you want to preserve, pick a new letters/digits/underscore slot and use that SAME slot in every command below. Do not delete/reset an existing save. Keep PFIdentityProfile=ManualGuide and the same endpoint/map across both launches.

First launch (fresh chosen slot; no -PFLoadSave):
& 'C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe' 'C:\UnrealProjects\PrimalFrontier\PrimalFrontier.uproject' /Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld -game -windowed -PFSaveSlot=ManualGuide20261009 -PFIdentityProfile=ManualGuide

1. Use normal E gathering; craft one tool (C,1 after 3 wood + 2 stone). Build foundation/door/storage with B,N,T,left click on valid preview (two wood each). Store one wood using selected bag stack > close Tab > B > E storage > U. Expected: playable interactions, one tool and a shelter/storage with known contents.
2. Before saving note location/landmark, health/stamina/hunger/thirst, all bag item names/counts, structures/door/owner and storage contents. Screenshot bag/storage/vitals. Food expiry ages while closed; write its remaining time.
3. Open console (~), run "PF.SaveWorld ManualGuide20261009". Expected: explicit PASS with [PrimalAgentTools]. If it fails, STOP this test and report the refusal; do not quit assuming a save exists. Save changes nothing in the editor .umap.
4. Close the game normally AFTER successful save. There is no timed autosave or automatic standalone/server-exit save. Restart the same command with -PFLoadSave appended:
& 'C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe' 'C:\UnrealProjects\PrimalFrontier\PrimalFrontier.uproject' /Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld -game -windowed -PFSaveSlot=ManualGuide20261009 -PFIdentityProfile=ManualGuide -PFLoadSave
5. Compare against step2 before doing more gathering. Expected: saved position/vitals/tool/nonperishable counts, structure ownership/door/storage restored once. Hunger/thirst can drain during live comparison; food loses offline age and expired food disappears. Storage currently gives no freshness bonus. Crafting jobs cancel on restore. Do not count expected expiry as lost-save failure.
6. Open your door/storage, retrieve one item (B,E,O), drop/recover one (Tab,G > close > E). Expected: ownership usable and exact totals conserved. Save again, close/restart again and compare: no additional duplicate tools, structures or storage.
7. Multiplayer/reconnect is a separate extension after solo passes: use uncooked Editor server (packaged Server unavailable), one client first, same server slot/map + same client profile/host spelling/port on reconnect. PF.SaveWorld runs in SERVER console; a client request must be refused, not forwarded. AI can prepare exact disposable server/client commands when this extension is ready; do not guess production startup/authentication. A second rendered client is optional if memory/RHI permits; if it crashes report INCOMPLETE with matching crash log, not a gameplay pass. Existing two-client NullRHI automation does not replace this rendered check.

What to report: each step PASS/FAIL/NOT TESTED; before/after table for location/vitals/items/structure/storage/owner, time closed, exact save PASS/refusal and any screenshot. No need to corrupt a save manually. Keep private .pfs/profile/login credentials local under Saved/Persistence, out of Trello/Git. Logs: Saved/Logs, including PrimalFrontier.log for this ordinary game launch; save separate before/after copies outside shared uploads if needed and redact sensitive connection text. Related M8 cards may reference this one session.


## controller

Guide G4 — physical controller feel and hot-plugging (Personal)
Level: /Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld
Content Drawer: Content > PrimalFrontier > Maps > L_PrimalFrontier_OpenWorld.
Use this open-world map, not Lvl_FirstPerson or the older L_M7SurvivalArena regression map.

Start here: open PrimalFrontier.uproject and double-click that level. No map/assets need saving.

Launch: Play dropdown > Number of Players = 1 > Net Mode = Play Standalone > Selected Viewport. Click Play, click the game view, then F11. "Play Standalone" here is the net mode; Selected Viewport is the presentation mode. Do not select New Editor Window for this first check. WASD moves, mouse looks, Space jumps. P opens Pause; use P to resume. Esc can stop PIE. Shift+F1 releases the mouse; the Editor Stop button ends PIE.

Start here: connect your controller before Play; record model and wired/Bluetooth connection. Xbox names: A bottom, B right, X left, Y top. This requires a real device/user observation, not generated key events.

1. Left stick move, right stick look, A jump. Expected: stable direction, usable sensitivity/inversion, no drifting when released. P > Settings > Game can tune sensitivity; record the values you actually tested.
2. With overlays closed, X gathers/picks up aimed nearby cubes; RT attacks. View/Back opens bag; D-pad Up/Down selects, Left splits, Right drops one, X consumes selected food/water. Close overlay with B before X pickup. Expected: correct context action and conserved counts.
3. Y opens craft; X tool, D-pad Up cook/Down dry/Left cancel. RB opens build; Up next, Down rotate, RT place, LB demolishes your aimed owned piece. Building mode D-pad Left stores selected bag item, Right retrieves first storage item. Expected: no unrequested simultaneous action.
4. Menu/Start Pause, D-pad selects, A activates. Controls & help LB/RB switches tabs; D-pad scrolls. Settings LB/RB tabs, D-pad rows, B cancels. Resume and move/look again. Expected: focus visible and restored, no accidental session end from holding A. Do NOT double-confirm End session unless you intend to exit.
5. In a safe solo spot unplug/reconnect once; test sticks/buttons again, then switch to keyboard/mouse. Expected: no crash, stuck input or permanent loss of control. Report what happens instead of assuming hot-plug works. No remapping, rumble, glyph auto-switch or split-screen feature is promised.
6. Optional repeat on one network client after AI prepares server session: Menu must not freeze the server/world. Do not require two rendered clients for this device check.
What to report here: PASS / FAIL / NOT TESTED for each numbered step; exact map and launch mode; failed step + input + on-screen message + expected/actual quantity; approximate time and screenshot if useful. For performance include actual resolution, quality/render scale, FPS range while walking and noticeable hitches. Stuttering alone is not a gameplay failure, but record it.

Logs/screenshots: project-local Saved/Logs (PrimalFrontier.log for ordinary PIE; use the matching unique log for custom launches), Saved/Screenshots/WindowsEditor. Console "shot showui" captures the HUD. Do not upload private saves, identity profiles or full connection/login URLs.

Keep this card open until actual observations are recorded. Automation and this guide are not a manual pass. Related cards may cite the same playtest report; no need to repeat the same route for every board.

## distribution

Guide G5 — resource/hazard/spawn spacing (Personal)
Level: /Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld
Content Drawer: Content > PrimalFrontier > Maps > L_PrimalFrontier_OpenWorld.
Use this open-world map, not Lvl_FirstPerson or the older L_M7SurvivalArena regression map.

Start here: open PrimalFrontier.uproject and double-click that level. No map/assets need saving.

Launch: Play dropdown > Number of Players = 1 > Net Mode = Play Standalone > Selected Viewport. Click Play, click the game view, then F11. "Play Standalone" here is the net mode; Selected Viewport is the presentation mode. Do not select New Editor Window for this first check. WASD moves, mouse looks, Space jumps. P opens Pause; use P to resume. Esc can stop PIE. Shift+F1 releases the mouse; the Editor Stop button ends PIE.

Reuse the G1 route; no separate route required.
1. From southern camp find wood, stone, fibre, food and freshwater by ordinary walking and labels. Record approximate travel time and any missing/unreachable node. Expected: early tool/food/water obtainable without grants or console teleports.
2. Deplete one expendable node, then return after roughly20 simulation seconds. Expected: unavailable while depleted, regrows afterward without duplicate yield.
3. Walk woodland, rocky rise, roofed ruin, water edge and a distant landmark; find a clear buildable patch. Expected: legible routes, usable shelter supports, no terrain/collision trap. Placeholder water is not swim support.
4. Approach a passive and hostile cautiously. Expected: passive reacts/flees; hostile detects/chases/attacks; spawn density does not overwhelm camp/host memory. Note repeated/spam spawns and encounters you could not test.
5. Briefly enter and leave hazard. Expected: Exposure affected inside, returns to zero outside. Record visual cue/readability and whether escape is reasonable; do not need a console-induced death.
6. Repeat part of the route at night as part of G1. Expected: resources/landmarks/escape remain identifiable. Report overly long routes, empty zones or hard-to-read labels as usability findings.
What to report here: PASS / FAIL / NOT TESTED for each numbered step; exact map and launch mode; failed step + input + on-screen message + expected/actual quantity; approximate time and screenshot if useful. For performance include actual resolution, quality/render scale, FPS range while walking and noticeable hitches. Stuttering alone is not a gameplay failure, but record it.

Logs/screenshots: project-local Saved/Logs (PrimalFrontier.log for ordinary PIE; use the matching unique log for custom launches), Saved/Screenshots/WindowsEditor. Console "shot showui" captures the HUD. Do not upload private saves, identity profiles or full connection/login URLs.

Keep this card open until actual observations are recorded. Automation and this guide are not a manual pass. Related cards may cite the same playtest report; no need to repeat the same route for every board.

## provenance

Guide G6 — asset-source details (Personal; no level launch)
Level required: NONE. This is a provenance/licensing information task, not gameplay or permission to import/download assets.

Start here: choose ONE existing intended local pack, for example Modular_Rural_Cabin, and locate its original store/source page.

1. Provide the public original listing/source link and pack name/author.
2. Identify the license type and version/date if known; say UNKNOWN if unavailable. Folder names and a free public price do not establish your license/acquisition.
3. Say which local Content folder corresponds to that pack, which version you imported if known, and which asset you would like used first.
4. Keep account details/receipts/payment info/private download links private. Do not import, rename or move more assets to satisfy this card.
Expected: AI can compare the supplied source/license against the existing read-only asset inventory and document allowed use; unknown details remain unresolved. This card stays open until required source details are supplied and assessed. G1/G3 acceptance is separate. Source: Docs/ASSET_PIPELINE_M9.md. No art integration pass is implied.

## audio

Guide G7 — current audio preferences (Personal)
Level: /Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld
Content Drawer: Content > PrimalFrontier > Maps > L_PrimalFrontier_OpenWorld.
Use this open-world map, not Lvl_FirstPerson or the older L_M7SurvivalArena regression map.

Start here: open PrimalFrontier.uproject and double-click that level. No map/assets need saving.

Launch: Play dropdown > Number of Players = 1 > Net Mode = Play Standalone > Selected Viewport. Click Play, click the game view, then F11. "Play Standalone" here is the net mode; Selected Viewport is the presentation mode. Do not select New Editor Window for this first check. WASD moves, mouse looks, Space jumps. P opens Pause; use P to resume. Esc can stop PIE. Shift+F1 releases the mouse; the Editor Stop button ends PIE.

Start here: choose your normal headphones/speakers and record the device. You can do this during G1/G2 rather than launching a second game.

1. Listen while moving/interacting. Record any ACTUALLY audible existing cue and the action which produced it. There is no promised authored survival music/footstep/creature/UI set yet; silence means MISSING COVERAGE, not passed audio.
2. P > Settings > Audio, note current Master, change to0 and Apply, then repeat that exact audible cue. Expected: Master mutes it. Restore prior Master, Apply and repeat: expected sound returns. A draft change without Apply is not a volume test.
3. For Music/Effects/UI test ONLY an existing cue whose SoundClass route AI has confirmed for that category. If none is available, mark that category NOT TESTABLE/MISSING COVERAGE; do not judge a slider by silence or assume template cues use that category.
4. Enable mute when unfocused, Apply; while an actual recurring cue is playing switch focus away/back. Expected: mute/recovery behavior matches setting. If no reproducible cue, mark NOT TESTED. Restore preferred settings afterward.
5. Report clipping, painful volume, unclear feedback and actions which need future sound. No assets need downloading/importing for this check.
Expected: honest listening observations separated from future M17 emitter work. Hearing/output-device checks require you; NullRHI/nosound fixtures cannot provide them.
What to report here: PASS / FAIL / NOT TESTED for each numbered step; exact map and launch mode; failed step + input + on-screen message + expected/actual quantity; approximate time and screenshot if useful. For performance include actual resolution, quality/render scale, FPS range while walking and noticeable hitches. Stuttering alone is not a gameplay failure, but record it.

Logs/screenshots: project-local Saved/Logs (PrimalFrontier.log for ordinary PIE; use the matching unique log for custom launches), Saved/Screenshots/WindowsEditor. Console "shot showui" captures the HUD. Do not upload private saves, identity profiles or full connection/login URLs.

Keep this card open until actual observations are recorded. Automation and this guide are not a manual pass. Related cards may cite the same playtest report; no need to repeat the same route for every board.

## Card-description contract

Every new/updated test card starts with exact map package path (or NONE for non-gameplay work), launch/net mode, prerequisites and one Start here action. Follow with numbered steps, inputs, expected state/counters, pass/refusal conditions, evidence paths and remaining limits. Name future/TBD maps honestly. Copy the appropriate guide into the card rather than only linking this document; retain earlier evidence below it. Personal priorities/members/deadlines/status stay unchanged unless actual observations resolve them. Do not mark current generated guides as manual test passes.
