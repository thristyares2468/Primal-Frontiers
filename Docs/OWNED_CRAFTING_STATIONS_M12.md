# M12 — Player-built owned workbench and refinery

Bounded task: https://trello.com/c/fKB7Zgqs. October22 scoped implementation/build/native/rendered/network gates PASS. Full M12, ordinary-input pacing, powered/fueled processing and hosted multiplayer remain incomplete.

## Behavior

The build catalog adds **Workbench** (8 Wood, key9) and **Refinery** (12 Wood, key0). Existing eight IDs, enum values and costs stay unchanged. Both require the owner's foundation/floor/ceiling, snap to the existing100cm platform grid, accept R rotation and use server collision/material/owner validation. A foundation costs2Wood, so the three-piece test costs22Wood. Existing structures/saves require no layout migration. Only engine cubes and code-native item pictures are used.

An owned workbench accelerates supported gathering-tool recipes to125%; an owned refinery accelerates supported metal recipes to150%. Stand within2.5m with a clear line of sight and open C. Unsupported recipes remain portable. Only the structure owner receives its bonus. Leaving range, obstructing or demolishing the station cancels its active job and keeps inputs; completed jobs consume inputs once. No station storage, fuel, unattended processing or power is implied. Public camp stations retain public access and their existing bonuses.

The server derives one hidden/noncolliding replicated helper from each station structure. Save records contain the structure ID, ownership and support; helpers are reconstructed once on restore, then removed with the parent. A client cannot submit its own station multiplier or authorize a helper. The catalog upgrade script refuses custom/partial ID sequences; its actual run reported changed=False because native constructor defaults already expose all ten choices. No binary asset or authored map changed.

## Reachable test and controls

Level: `/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld`. Use one rendered **Standalone Game**, single player, a disposable new world through the main menu. Do not use NullRHI for this visual check or save changes to the editor map. Existing personal worlds need no deletion/conversion. This guide is optional reproduction; automated assisted playtesting does not replace a human pacing/readability gate.

Prerequisites:22Wood for the foundation and stations, plus3Wood+2Stone for a stone gathering tool. For refining, learn **Field tools** through the existing ordinary technology tree and obtain2Copper ore+1Wood fuel. Do not spend or reset a personal save merely for this check. If progression/ingredients are unavailable, only test construction and mark crafting untested.

**Start here:** find the **CAMP CRAFTING STATIONS** sign near spawn at `(350,-650,160)`cm. Public workbench `(700,-500,0)` and refinery `(1100,-500,0)` labels now point to B for owned stations. Use the clear starting-area construction patch around `(-800,-1200,0)`cm, away from starts and normal gathering routes. The isolated rendered test places its disposable examples there without changing the level asset or your world.

1. Press B, choose Foundation (1), aim at flat ground in that patch and left-click a green preview. Expected: one platform,2Wood spent. R rotates the preview. Escape cancels placement.
2. B opens the picker again. Scroll if needed: ten choices include distinct workbench/refinery pictures and costs. Press9 (Workbench), aim at one end of your platform and left-click. B then0 selects Refinery; place at the opposite end without overlap. Expected:8+12Wood spent once; failed/overlapping placements spend nothing. Both remain supported by your platform.
3. Stand within2.5m of your workbench with a clear view. C, Stone gathering tool: detail says **Owned workbench:125%**. Craft with Enter/button and stay nearby. Expected: one tool from3Wood+2Stone;5seconds becomes4seconds before player crafting-speed upgrades. Selection alone must not spend ingredients.
4. With another recipe's ingredients, start again and walk more than2.5m away before completion. Expected: station-unavailable cancellation, ingredients kept, no new tool. Away from any station, portable recipes still work at normal speed.
5. At your refinery, select Copper ingot in C. Expected: **Owned refinery:150%** after its knowledge/ingredient requirements are met; one ingot consumes2Copper ore+1Wood once. Portable cooking is unchanged. Never confuse blocked knowledge or missing ingredients with an authority failure.
6. Pause > Save, require explicit success, end session normally, then load that same world. Expected: exactly one foundation/workbench/refinery, same ownership and inventory, both bonuses work. No duplicate visible helper. To remove a test piece: B, choose any piece to enter building mode, aim at your existing station within2.5m and press H; do not left-click a new placement. Supporting platforms refuse removal while pieces depend on them. Escape cancels building mode afterward.
7. Optional existing local multiplayer setup only: another player can see your structure but receives no private bonus, cannot demolish it or take ownership. Do not start hosted PlayFab/cloud/voice setup for this test.

Pass name **OWNED_STATIONS**. Report PASS only for performed steps; report the first failure, step number and displayed feedback, or list untested progression/multiplayer portions. Normal survival draining/time continues outside pause. Existing earned-pacing/controller/audio/1440p checks remain separate.

## Technical reproduction

Close existing Unreal processes before each isolated run. From `C:\UnrealProjects\PrimalFrontier`, build PrimalFrontierEditor Win64 Development using the installed Engine Build.bat. Run:

```powershell
./Scripts/RunNativeAutomation.ps1 -TestFilter 'PF.Crafting.OwnedStations+PF.Crafting.OwnedStationsRestore+PF.Crafting.Stations+PF.Building.PlacementAndStorage' -Label M12OwnedFinal -TimeoutSeconds 180
./Scripts/RunNativeAutomation.ps1 -TestFilter 'PF.Persistence.WorldRecords+PF.Persistence.WorldRuntime+PF.Persistence.StructureCollisionRestore' -Label M12OwnedSaveFinal -TimeoutSeconds 180
./Scripts/RunPersistenceAutomation.ps1 -Players 1 -OwnedStations -Port 18081
./Scripts/RunPersistenceAutomation.ps1 -Players 2 -OwnedStations -SimulateLagLoss -Port 18081
./Scripts/RunControlsAutomation.ps1 -Resolution 720 -TestCase OwnedStations
./Scripts/RunControlsAutomation.ps1 -Resolution 720 -TestCase BuildingMenus
```

Native tests use synthetic temporary worlds and uniquely owned save slots. Network tests use the actual open-world map/uncooked dedicated server, owned RPC gathering/crafting/cancel, replicated bonuses and input/output conservation. Their explicitly spawned station fixture bypasses material construction; actual placement/cost is tested separately in native/rendered tests. Headless station runs are limited to120FPS to prevent NullRHI's1,400FPS saved-move overflow; this is not a production optimization. Rendered runs request uncapped FPS/VSync off and use assisted travel/material/700XP seeds, not earned pacing. Runners hash unrelated settings/world/profile files and clean only their owned processes/profiles. Build Game separately; no updated cooked Server package is claimed by these runs.

Reports: `Saved/AutomationReports/<run>/index.json`, `run-summary.json`; rendered PNGs `Saved/AutomationReports/ControlsUI/<run>/`; logs `Saved/Logs/<run>.log` or `PF<run>.log`. Build transcripts under `C:/UnrealSource/BuildLogs/M12OwnedStations_*_20261022.log`. Inspect screenshots and raw severities before acceptance.

## Evidence and retained failures

- Editor native build replay PASS5.91s after explicit test-pointer/TObjectPtr compile fixes; restoration-test build6.09s, clock-fixture rebuild6.25s, network build16.35s, UI build6.73s and ten-key menu build6.94s all PASS.
- Supported catalog script PASS with ten IDs, changed=False; no asset diff.
- Native placement `Automation_M12OwnedStations_20261021_233707981_15b58bcf`:1/1 PASS17.48s, zero raw/test severity,2.987working/2.850privateGiB. Actual owner placement/cost, authority/refusal, timed tool transaction, demolition cancellation and refinery capability.
- Initial restoration group `Automation_M12OwnedRestore_20261021_233904181_0e13a0ca`:2/3PASS, missing synthetic mandatory world clock caused two assertion errors. Clock fixture repaired only. Failed-selector replay `Automation_M12OwnedRestoreReplay_20261021_234111862_6d4a5074`:1/1cleanPASS16.64s,3.027working/2.875privateGiB; actual saved structure IDs/support/owners and helper uniqueness across two loads.
- First network `M12OwnedStationLive1_20261021_234418146_c513264d`: gameplay contracts pass, client319test warnings/363raw CreateSavedMove warnings fail gate before expansion. Began before automation at roughly1,400FPS. No warnings suppressed. Headless120FPS replay `M12OwnedStationLive1_20261021_234936933_753fdcc5`:2/2server/clientPASS61.03s, exits0/test warnings/errors0,1.729–1.797working/1.630–1.781privateGiB, protected files unchanged.
- Two-client `M12OwnedStationLive2_20261021_235132056_59eb9f4f`:3/3PASS63.31s with75ms outgoing lag/1%loss, exits0/test warnings/errors0/no timeout/fatal/ensure;1.738–1.797working/1.635–1.785privateGiB, protected files unchanged. Each raw24warnings/14errors matches prior uncooked OpenWorld HLOD/experimental Python startup baseline; final normalized review follows.
- Rendered owned construction/craft `M11OwnedStations720_20261021_235313215_456794f1`:1/1cleanPASS43.11s, inspected placed-scene/workbench/refinery PNGs,3.440working/5.819privateGiB,635protected files/defaults unchanged. Actual validated22Wood construction plus Slate tool/ingot transactions and range cancellation; assisted travel/seeded materials, not manual pacing.
- First ten-choice picker `M11BuildingMenus720_20261021_235445086_e6e9d973`: FAIL2assertions because gameplay-key helper did not route modal9 input. Fixture changed to Slate key events, no production relaxation. Replay `M11BuildingMenus720_20261021_235639304_e5b3687a`:1/1cleanPASS38.53s with all ten choices, actual9/0 selection, wall rotation, chest/sort/filter/ownership/freshness and physical stairs; inspected picker PNG shows every new choice/cost/picture,3.052working/4.840privateGiB,637protected files/defaults unchanged.
- Large-text picker `M11BuildingMenus720_20261022_000056734_2b69aea8`:1/1cleanPASS38.58s,720p/HUD1.5,3.057working/4.456privateGiB,638protected files/defaults unchanged. Inspected PNG: readable wrapping, scrollable rows and Close remain within panel; third row requires scrolling at this scale. Keyboard9/0/actual selections still pass. Physical controller untested.
- Final native initial group `Automation_M12OwnedFinal_20261021_235729181_51e1cd12`: all7actual tests passed17.05s/raw severity0, but strict runner refused unlisted prefix match OwnedStationsRestore. Explicit combined208-character selector rejected before launch by200-character guard (transcript NativeFinalReplay); split without changing guards. Final `Automation_M12OwnedFinalReplay_20261021_235829810_7eff6b49`:4/4cleanPASS17.03s (OwnedStations, OwnedStationsRestore, Stations, PlacementAndStorage). `Automation_M12OwnedSaveFinal_20261021_235914441_cd54a252`:3/3cleanPASS16.75s (WorldRecords, WorldRuntime, StructureCollisionRestore). All exits0/raw/test severities0, sampled peak3.101working/3.036privateGiB.
- Final Editor menu-fixture replay build6.74s and Game29.60s PASS, compiler warning/error count0. No post-test production C++ edits. No sourceServer/cook/stage update for this slice.
- Raw log comparison: all three final network roles have33unique normalized warning/error messages, exactly equal corresponding successful public-station run M12StationLive2_20261015_223600421_218b4499; both normal-scale rendered runs35unique messages exactly equal public render M11Stations720_20261015_222232140_d255043a. HLOD/experimental Python/editor widget warnings remain; no new gameplay severity/fatal/ensure. Existing Saved/Logs/PrimalFrontier.log is historical October14 GPU crash; this slice uses the unique abslogs above, not that stale default log.

No sustained1440p/FPS/stutter/manual/physical-controller/cloud/voice/cooked-client/full-M12 acceptance is claimed.
