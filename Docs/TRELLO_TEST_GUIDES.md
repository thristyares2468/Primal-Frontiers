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

October10 crafting feedback: successful gathering now earns bounded5XP/action (five/category/30active minutes), persisted remaining budget. No grant/drop/pickup reward. Level/XP/next threshold/knowledge points and selected first-completion reward are visible. First completion grants20XP; repeats still make items with no extraXP; cancelled/failed jobs grantzero. Legacy saves beginzeroXP/no retrospective carried-item rewards. Optional Bind stone tool now requires level2 FieldTools knowledge costing2points; its Learn button/K/D-right and Craft availability show actual server state. KNOWLEDGE_MENU_M12.md records technical native/rendered evidence; human acceptance remains open. Use step3a during this same UI session, not another assigned task.
Level: /Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld
Content Drawer: Content > PrimalFrontier > Maps > L_PrimalFrontier_OpenWorld.
Use this open-world map, not Lvl_FirstPerson or the older L_M7SurvivalArena regression map.

Start here: open PrimalFrontier.uproject and double-click that level. No map/assets need saving.

Launch: Play dropdown > Number of Players = 1 > Net Mode = Play Standalone > Selected Viewport. Click Play, click the game view, then F11. "Play Standalone" here is the net mode; Selected Viewport is the presentation mode. Do not select New Editor Window for this first check. WASD moves, mouse looks, Space jumps. P opens Pause; use P to resume. Esc can stop PIE. Shift+F1 releases the mouse; the Editor Stop button ends PIE.

Test in order:
1. Gather a few wood/stone batches normally (E). Tab opens inventory; Up/Down deliberately selects a named stack. X splits half, G drops one. Close Tab, aim at the dropped cube within about 2.5 m, E picks it up. Expected: split preserves total; drop/pickup restores exact total, not duplicated items. Repeat pickup with B building mode open.
2. Scroll through bag rows if you have more than four stacks. Let an expendable food batch expire or remove its selected stack. Expected: rows remain readable; actions do not silently switch to the item that slid into its old row. Up/Down explicitly chooses a new stack. Report if the footer/messages cover the crosshair or exceed the screen.
3. C opens the centered crafting menu with pictures and selected-item details. Click All/Tool/Food/Material/Weapon/Protection or use PgUp/PgDn (controller LB/RB) to filter. Changing category clears selection: Enter does nothing until you choose a row. Click a row or Up/Down selects without crafting; Enter/Craft selected starts it. Changing category during a job preserves that job and its inputs; only R/Cancel deliberately cancels.1/2/3 work only when their baseline recipe is visible. Controller Y opens, D-pad Up/Down selects, A/X crafts, D-left cancels, B/Y closes.1 tool costs3wood+2stone/five seconds;2cook costs1found food+1wood/six seconds;3dry costs2food+2wood/ten seconds. New appended recipes: gather4fibre from a labelled fibre patch (E), select Twist fibre cord → one cord after4seconds; one stone tool+one cord+2wood → Bound stone tool after8seconds. Keep inputs until completion; bound tool gathers3hits/45melee damage, finite node yield unchanged. R/Cancel job cancels without output/cost. Close restores mouse-look/movement/cursor; P transfers to Pause then resumes without stuck input. Expected: readable pictures/nutrition/fresh cost/time/tool stats, actual progress/refusal and exact single output/ingredient consumption. World continues behind this modal menu. Test actual mouse row/buttons here: background-window automation verifies Slate keyboard activation only. Weapon category: Shape wooden club costs3wood+1cord/6seconds (40melee); Bind stone club costs1club+1cord+2stone+2wood/8seconds (60melee). From nothing the upgrade totals5wood+2stone+8fibre, including two cord crafts. Best fresh carried melee item applies automatically; a45damage bound tool supersedes the40damage club. Weapons add no gathering bonus. Close C/Tab/B, aim at a living creature within2.5m, left click; each accepted swing costs5stamina with0.5s cooldown. No PvP/equip slots. Expected exact costs, readable Weapon pictures/stats, one output and normal movement/attack input after closing. WEAPON_TIERS_M12.md records bounded AI evidence, not your combat-feel acceptance. Recipes currently need no campfire. Learn FieldTools for the optional BoundTool using step3a; equipment slots/durability remain absent. Protection category: gather12fibre+2wood total; first twist one cord (4fibre/4seconds), then Weave protective guard (8fibre+1cord+2wood/6seconds). Tab shows1kg woven guard and25% creature hit reduction while carried. Two guards do not stack; generic damage/needs/exposure are unaffected. Dropping removes the benefit. Expected readable picture/cost/stat, one output, server-owned health and ordinary bag/craft closure. PROTECTION_M12.md records AI damage/restart evidence, not manual attack-feel acceptance. Existing session suffices; TOOL_TIERS_M12.md contains exact evidence and limits.
3a. Optional earned upgrade check, within this same UI session: use an unused fresh world for exact totals, or record your existing XP/knowledge/craft history and compare only new rewards. Do not overwrite a personal save. In a fresh0XP/empty bag with bare hands, gather BEFORE crafting: six successful Wood actions (two nodes or normal regrowth), one Stone action, two Food actions and four Fibre actions; E about a second apart while aimed at each physical node. Expected12wood/2stone/4food/8fibre and60XP: Wood caps at25XP after five actions, Stone5, Food10, Fibre20. Different prior history, tools or extra actions change XP; the rule is5XP per successful action, at most five credits/category in30active minutes, never per item/hit. Drop/pickup, PF grants, failures/cancel and time-of-day changes grantzero/resetnothing. Complete Stone gathering tool, Cook food, Dry food, Twist fibre cord and Shape wooden club once each, waiting for each job. Exact fresh route reaches160XP/level2/3points; second Cord consumes4fibre but adds noXP. Select Bind stone tool and scroll details to the top: FieldTools locked,level2,cost2,available3,Learn enabled/Craft disabled. Click Learn or K/controller D-right; wait for server feedback. Expected1point remaining and Craft enabled with inputs; repeat Learn costs nothing. Craft one Bound stone tool (tool+cord+2wood,eight seconds): one bound tool,180XP/1point and exact single costs. Close C, gather one intact Fibre node with it:6fibre,one fifth Fibre credit,185XP, not15XP for its three hits. Further Fibre/wood actions in their exhausted windows still yield finite items but noXP; budget renews after30active minutes and keeps remaining duration/counters over save/reconnect, with no offline renewal. At level1 Learn/Craft remain disabled. Existing learned saves skip spending and use their recorded baseline. Fail on wrong costs/rewards, duplicate outputs, false success, unreadable controls or stuck movement. Technical fixtures are not human pacing/controller acceptance.
4. B opens build: N next piece, T rotate, left click valid placement. Gather enough wood; two wood per current piece. Foundation supports walls/door/storage. Expected: preview and valid/refused message readable while aiming; invalid placement spends no resources.
5. Tab select a wood stack, close Tab; in B mode aim at your storage, E opens it, U stores one, O retrieves one from the first stored stack. E toggles your door. Expected: bag + storage total conserved; quantities and ownership messages readable. A closed bag retains the selected GUID for storage.
6. P opens Pause. Open Controls & help; Left/Right switches keyboard/controller, Up/Down scrolls; return to Pause and Resume. Expected: no accidental jump/action while browsing; mouse/look/movement return. Solo world pauses; multiplayer world does not.
7. P > Settings. Browse Game/Graphics/Audio/Accessibility. Change a draft value, choose Cancel, reopen: expected original value retained. Then choose a small intentional HUD-size or look-sensitivity change and Apply: expected visible effect and retained preference. Restore your preferred value afterward. Motion blur may stay off. Record exact applied setting; Preferences live on this machine separately from world saves.
8. Test graphical/display confirmation only in a separate standalone -game window (G3 launch), not PIE which owns its display. Apply a supported resolution/window mode, leave confirmation unanswered: expected return to prior display within 15 real-time seconds. Do not choose unsupported modes or treat missing audio as a slider pass.
9. While moving and gathering, inspect vitals, urgent/damage messages, build feedback, bag readability and centre aim. Expected: understandable feedback during first-person play. Crafting is intentionally modal; closing it restores control, while the world keeps running. Pause reconnect-profile message is local device storage, not proof of a world save. Server setup acknowledgment is separate: fresh connection says new survivor; after actual save/restart it says restored. Compare the real label; neither label proves world saving or all actors have replicated.

Physical controller feel is G4; rendered persistence is G3. These steps can be done during G1 instead of repeating a route.
What to report here: PASS / FAIL / NOT TESTED for each numbered step; exact map and launch mode; failed step + input + on-screen message + expected/actual quantity; approximate time and screenshot if useful. For performance include actual resolution, quality/render scale, FPS range while walking and noticeable hitches. Stuttering alone is not a gameplay failure, but record it.

Logs/screenshots: project-local Saved/Logs (PrimalFrontier.log for ordinary PIE; use the matching unique log for custom launches), Saved/Screenshots/WindowsEditor. Console "shot showui" captures the HUD. Do not upload private saves, identity profiles or full connection/login URLs.

Keep this card open until actual observations are recorded. Automation and this guide are not a manual pass. Related cards may cite the same playtest report; no need to repeat the same route for every board.

## persistence

Guide G3 — rendered save, rename, close and reload (Personal)
Level: /Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld; main menu uses /Engine/Maps/Entry.
Mode: one rendered standalone Development editor game process. This is a separate -game window, not PIE or NullRHI. M7 Personal check was reported completed October9. This M8 check previously failed and remains replay pending.

Start here: close Unreal Editor, open PowerShell, and run:
& 'C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe' 'C:\UnrealProjects\PrimalFrontier\PrimalFrontier.uproject' -game -windowed

1. In the main menu choose Single player. For a new test enter an unused letters/digits/underscore world ID and Create new world. To recover an existing saved world, select it and Load selected world instead; do not recreate or overwrite ManualGuide20261009. Gameplay should use the open-world level above; a new/restored survivor message appears in Pause. No map needs saving in Editor.
2. Gather with E; obtain3 wood +2 stone, then C,1 to craft a tool. Build foundation/door/storage with B,N,T,left click on valid preview (two wood per piece). Select wood in Tab, close the bag, open storage with B,E and U to deposit. Note location/landmark, vitals, bag counts/tool, structure ownership and storage contents; capture screenshots.
3. Press P and select Save world. Require the visible 'Saved world:' success message. If it says refusal, STOP and report it; do not quit assuming a save exists. Saving must leave your current location, bag and buildings unchanged. Timed resource respawn is normal; a fresh player or missing structures/items is a failure.
4. Select End session, then Confirm end session. In Single player select the saved world. Optionally type a friendly name below Refresh worlds and press Rename selected world; expect success and the updated selection. The original internal console save ID remains unchanged. Empty/duplicate/overlong names must be refused without data loss.
5. Load selected world. Compare the saved location/vitals/tool/counts/structures/storage/ownership before gathering further. Then close the game after another acknowledged save. Relaunch the SAME menu command from Start here, choose the SAME world (using its new name if renamed), and Load selected world again. Require restored survivor and the same nonperishable totals/ownership; no duplicate actors/items.
6. Check a door/storage and retrieve one item (B,E,O). Drop/recover one (Tab,G, close bag, E). Save, close/reload once more only if you want a repeated-load check. Expected totals remain conserved. Food ages while closed; expired food disappears. Hunger/thirst can drain during live comparison, crafting jobs cancel on restore, and storage has no freshness bonus. There is no timed autosave or automatic standalone/server-exit save.
7. Multiplayer/reconnect is a separate extension after solo passes. The main-menu Multiplayer screen is INFORMATION ONLY; it does not host/join. Use documented uncooked dedicated-server commands, one client first, same server slot/map and client profile/host/port. Save/Load runs on the SERVER and client requests must be refused. Packaged Server is blocked by the installed engine distribution. A second rendered client is optional if memory/RHI permits; a crash is INCOMPLETE, never a gameplay pass. NullRHI restart evidence is separate.

Send back: PASS/FAIL/NOT TESTED for Save, Rename (if tried), Load and close/relaunch Load; describe any missing/duplicated item, structure or ownership. One before/after screenshot is enough to start. Logs: Saved/Logs/PrimalFrontier.log for ordinary launches. Keep private .pfs/profile credentials local under Saved/Persistence and out of Trello/Git. Do not manually corrupt or edit saves. AI evidence/replay and normal launch guide: Docs/WORLD_MENU_M11.md. This card requires your actual rendered interaction/state comparison; automated Slate and headless tests cannot complete it.

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
