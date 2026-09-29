# Milestone 5 — verified 2026-09-29

Authorized after M4 from clean `e7ffd71`. Baseline Editor build passed in 27.07 seconds. Implement only greybox building: data-defined foundation/wall/floor/ceiling/door/storage, grid preview and rotation, authoritative placement/cost/collision/support validation, ownership, health/demolition and transactional storage. Use a separate small M5 map and preserve existing assets. Compile and run focused automation, then regression, manual shelter/storage play and one-client before two-client replication. For rendered tests use session-only `t.MaxFPS 0`, VSync off and frame smoothing disabled, recording frame time/FPS and memory without increasing graphics quality. Update evidence and decisions; stop before creatures/M6.

**Gate passed for the supported greybox building scenario.** Editor compilation, 12-test regression, final three-test rerun, rendered first-person shelter/storage playtest, and real one-/two-client NullRHI replication passed. Stop before M6. Packaged Server/Shipping builds, persistence and rendered two-client performance are not claimed.

## M5 changes and verification

- Added `Source/PrimalFrontier/Building/PFBuildingCatalog`, `PFBuildPiece`, `PFBuildingComponent` and `PFBuildingHUD` header/implementation pairs. Added atomic `PFInventoryComponent::TransferTo`, controller input/RPC integration and the HUD build hint. Six data-defined primitive pieces, grid/rotation preview, server-derived placement, collision/support/cost/ownership checks, door interaction, damage/demolition and private storage are described in `BUILDING_M5.md`.
- Added `Source/PrimalFrontier/Tests/PFBuildingTests.cpp`, plugin `Private/Tests/PFBuildingLiveTests.cpp`, real `PF.TestBuildingPlacement` and guarded `PF.ResetBuildings`. Added `Scripts/SetupBuildingMilestone5.py`; supported Editor Python created only new `Content/PrimalFrontier/Building/DA_BuildingCatalog.uasset` and `Content/PrimalFrontier/Maps/L_M5Building.umap`. Setup evidence: `Saved/Logs/PFM5Setup.log`. Existing binary/template assets are unchanged. Architecture, decisions, plugin documentation and test-machine reference are updated.
- Intermediate compilation failed C3535 on `auto*` deduction from TObjectPtr and C4458 on a local Instigator name; corrected explicit pointer access and naming, then rebuilt successfully. The final C++ build succeeded in **5.94 s**, without compiler warnings, after a visual preview/HUD adjustment. UBT evidence: `C:/Users/jackh/AppData/Local/UnrealBuildTool/Log.txt` (rotating log).
- `Saved/AutomationReports/M5Core/index.json`: **1 passed, 0 failed/warnings**, `PF.Building.PlacementAndStorage`. `M5Regression/index.json`: **12 passed, 0 failed/warnings**. Exact tests: `PF.Building.PlacementAndStorage`, `PF.Crafting.Gathering`, `PF.Crafting.Transactions`, `PF.Inventory.Transactions`, `PF.Inventory.WorldTransfers`, `PF.PrimalAgentTools.CommandArguments`, `PF.PrimalAgentTools.MissingSystemsAreBlocked`, `PF.PrimalAgentTools.TeleportAndRuntimeReset`, `PF.Survival.Component`, `PF.Survival.Environment`, `PF.Survival.Lifecycle`, `PF.Survival.Needs`. Logs: `Saved/Logs/PFM5Core.log`, `PFM5Regression.log`.
- The first live attempt (`PFM5Server1.log`, `PFM5Client1.log`) stalled because its transfer immediately followed storage-open and hit the intentional RPC cooldown. It was stopped without an acceptance result. Added a 0.7 s harness wait; gameplay throttling is unchanged. Rebuilt successfully and replayed.
- `PF.Building.Live` then passed once in **each of five processes**, all with zero failures/warnings: `M5Server1Verified/index.json`, `M5Client1Verified/index.json`, followed by `M5TwoServer/index.json`, `M5TwoClient1/index.json`, `M5TwoClient2/index.json` under `Saved/AutomationReports`. Matching logs are `Saved/Logs/PF<report-name>.log`. Real clients used owned RPCs to place foundation/wall/ceiling/storage, checked replicated support/owner/cost, deposited food preserving its exact deadline, rejected occupied demolition, and observed damage to 75 health. Both clients observed the other player's structures while storage contents remained private. Direct client mutation, unknown definitions and client reset were rejected. PF live client exports intentionally contain NOT AUTHORITY results; their command-history aggregates are not the automation verdict.
- Final binary rerun on September 29: `Saved/AutomationReports/M5Final/index.json` reports **3 passed, 0 failed/warnings/not-run**: `PF.Building.PlacementAndStorage`, `PF.Inventory.Transactions`, `PF.Inventory.WorldTransfers`. This invocation wrote to **`Saved/Logs/PrimalFrontier.log`**, not the requested custom log name; the log records three tests and TestExit status 0 at **2026-09-29 03:45:36 UTC**. No warning/error lines were found in that final log. No C++ changes followed this run.

## M5 rendered playtest and performance

- Computer Use controlled one rendered first-person `L_M5Building` session at 960x540. Developer fixture setup supplied wood/food and positioned the player; actual B/N/T/click/E/U/O/H/J controls performed the building interactions. Placed a foundation, three rotated walls, a floor used as a roof, a door and storage. Duplicate/unsupported placement showed invalid preview/refusal. Opened the door, deposited and withdrew wood, observed occupied-storage demolition refusal, damaged empty storage, demolished it and rebuilt it. Ceiling placement and unauthorized ownership actions were separately exercised by automation/network tests. Controls remained responsive; this is a small shelter test, not a large construction stress test.
- `Saved/Logs/PFM5ManualVerified.log` records placement, accepted door/storage interactions, successful deposit/withdraw, rejected occupied demolition, owner damage, demolition and rebuild. `PF.TestBuildingPlacement` passed over **7 structures**, `PF.Help` passed and report export passed. Export: `Saved/AutomationReports/PF_M5Manual_20260928T221751_B0FA584B467FC7257F4009A99422F6DC.json`. Opened and visually inspected `Saved/Screenshots/WindowsEditor/ScreenShot00008.png`: visible door opening/storage, building overlay, needs HUD and FPS/unit stats. Health 42 reflects needs drain during the extended test; hunger/thirst were reset before capture. The session exited normally at 22:29:46 UTC. An earlier overnight `PFM5Manual.log` session was incomplete and is not counted as acceptance evidence.
- Console queries confirm **t.MaxFPS=0** and **r.VSync=0**; startup disabled frame smoothing only for this session. Engine-generated `Saved/Profiling/CSV/Profile(20260929_081751).csv` contains **6,000 frames**, stationary facing the seven-piece shelter with build HUD/preview and stats enabled. All frames retained: mean **6.0123 ms / 166.33 effective FPS**, median **5.7462 ms**, p95 **7.5312 ms**, p99 **8.8820 ms**, maximum **439.7173 ms**. Exactly one frame exceeded 33.33 ms, the first captured frame immediately after the screenshot/capture start; that association is not a root-cause diagnosis. Remaining live observations were generally about 146–197 FPS. This short sample does not establish native 1440p or open-world performance.
- CSV physical memory spans **3,467.95–3,583.34 MiB** (~3.39–3.50 GiB); virtual-used peak **5,506.54 MiB** (~5.38 GiB). External process sample was **3.50 GiB working set / 5.38 GiB private bytes**. Two-client NullRHI samples were server **1.63/1.48 GiB**, clients **1.71/1.63** and **1.71/1.62 GiB** working set/private. Windows reports **33,477,984 KiB visible physical memory (~31.9 GiB)**. User's updated 32 GB hardware reference is in `TEST_MACHINE.md`; observed GPU adapter name differs from the supplied 4060 Ti model. Earlier 16 GB assumptions are superseded for this machine, but 16 GB minimum-spec performance is unverified.
- Known existing game/server startup diagnostics remain: experimental Toolsets Python missing `ToolsetDefinition`/`PythonTestRunner`, TEDS widget registration warnings, rendered `r.MotionVectorSimulation` thread-safety warning and occasional Unreal Trace Server 0x000020b7 warning in network clients. No new gameplay errors, ensures or RHI crashes were found in the passing M5 logs. Unavailable non-Windows SDK notices from final startup do not affect the Win64 tests.
- Limits: cooperative owner-only damage/access; no group permissions/PvP tuning, save/reconnect restoration or preservation equipment. `PF.ResetBuildings` client rejection is network-tested; its successful populated-world reset branch has not been manually exercised. Generated evidence stays local/untracked. Recommended next milestone is the separately authorized M6 primitive creatures; a distinct 1440p benchmark can establish native-display performance before visual expansion.

# Milestone 4 — verified 2026-09-28

**Gate passed for the supported greybox gathering/crafting loop.** Editor build, 11-test regression, focused final tests, rendered first-person playtest, and real one-/two-client NullRHI authority/replication runs pass. Stop after M4; do not begin M5 automatically. Packaged Server/Shipping builds and rendered two-client performance are not verified.

Authorized after M3's completed gate, starting from clean `fe30fb6`. Baseline Editor build passed, up to date in 1.61 s. Implement only a small gathering/crafting loop: data-driven primitive wood/stone/food nodes with depletion and respawn; first-person server traces; one primitive gathering tool; editable tool/cook/dry recipes with duration and cancellation; atomic inventory conversion and stale-ingredient rejection; placeholder keyboard crafting UI and real PF validation hooks. Use `L_M4Gathering`, preserve template assets and keep the technology tree deferred.

Gate plan: compile Editor after each meaningful C++ slice; run focused transaction/gathering/crafting tests plus regression; create only the project-owned M4 fixtures through supported Editor APIs; rendered solo first-person gathering/tool/cooking/cancel playtest; one-client then two-client NullRHI authority/replication checks; inspect logs/reports/screenshots and record memory/stutter; update architecture, decisions and evidence. Stop after M4; do not begin building or M5.

## M4 implementation and evidence

- Added `Source/PrimalFrontier/Crafting/PFCraftingCatalog`, `PFCraftingComponent`, `PFResourceNode` and `PFCraftingHUD` (.h/.cpp pairs): editable resource/recipe data, finite primitive nodes, server-derived camera interaction, a timed single-job queue, cancellation and placeholder UI. Added atomic inventory conversion, tool/cooked/dried item definitions, owned controller RPCs, replicated primitive tool presentation and a native aim marker. Detailed contract: `GATHERING_CRAFTING_M4.md`.
- Added `Tests/PFCraftingTests.cpp` and plugin `Private/Tests/PFCraftingLiveTests.cpp`; updated `PFCommands.cpp` with real `PF.TestGathering`, `PF.TestCrafting`, `PF.Craft` and `PF.CancelCraft` hooks. Updated architecture, decisions, food and plugin documentation. `Scripts/SetupGatheringMilestone4.py` creates only `Content/PrimalFrontier/Crafting/DA_CraftingCatalog.uasset` and `Content/PrimalFrontier/Maps/L_M4Gathering.umap` through supported Editor APIs. No existing Content asset or template asset changed; no external art or new plugin dependency.
- Editor builds passed after each slice. One intermediate C4458 failure (local `Slot` hiding `UWidget::Slot`) was fixed by renaming the local to `Placement`; corrected build passed in 6.12 s. Native aim-marker compile passed in 6.35 s. Final `PrimalFrontierEditor Win64 Development` build on September 28 succeeded, up to date in 2.03 s, with no compiler warnings. UBT log: `C:/Users/jackh/AppData/Local/UnrealBuildTool/Log.txt` (rotating file).
- `Saved/AutomationReports/M4Core/index.json`: 2 passed, 0 failed/warnings. `M4Regression/index.json`: 11 passed, 0 failed/warnings. Exact regression tests: `PF.Crafting.Gathering`, `PF.Crafting.Transactions`, `PF.Inventory.Transactions`, `PF.Inventory.WorldTransfers`, `PF.PrimalAgentTools.CommandArguments`, `PF.PrimalAgentTools.MissingSystemsAreBlocked`, `PF.PrimalAgentTools.TeleportAndRuntimeReset`, `PF.Survival.Component`, `PF.Survival.Environment`, `PF.Survival.Lifecycle`, `PF.Survival.Needs`. Logs: `Saved/Logs/PFM4Core.log`, `PFM4Regression.log`.
- Final binary focused rerun: `Saved/AutomationReports/M4Final/index.json`, `PF.Crafting.Gathering` and `PF.Crafting.Transactions`, 2 passed, 0 failed/warnings. `Saved/Logs/PFM4Final.log` records two tests performed and `RequestExitWithStatus(1, 0, ...)` at 2026-09-28 08:48:04 UTC. Coverage includes capacity, insufficient/moved/expired ingredients, cancellation, duplicate completion, unknown recipes, forged deadlines, death cancellation, resource depletion/respawn, range/aim and authority rejection.
- Real network test `PF.Crafting.Live` passed once in each process: `Saved/AutomationReports/M4Server1/index.json`, `M4Client1/index.json`, then `M4TwoServer/index.json`, `M4TwoClient1/index.json`, `M4TwoClient2/index.json`; every report has 1 success, 0 failed/warnings. Logs use matching `Saved/Logs/PF<report-name>.log` names. Each client gathered through its owned RPC, crafted a tool, depleted and observed respawn of its node, cancelled/restarted cooking, and verified output quantities. Direct client mutations and unknown recipe requests were rejected. The server checked resulting inventories and PF integrity hooks. The live harness has a completed stage before disconnect-sensitive lookups and allows server linger for client shutdown.
- Network command exports: `Saved/AutomationReports/PF_M4LiveClient_20260927T063444_*.json`, `PF_M4LiveServer_20260927T063444_*.json`, and corresponding `20260927T063614` client/server files. Client command histories include intentional NOT AUTHORITY failures; their aggregate status is not the automation verdict.
- Rendered manual first-person session `Saved/Logs/PFM4Manual.log`: insufficient-resource recipe refusal; hand gathering wood 2/4/6; depleted-node rejection and respawn; stone gathering; tool crafting with exact costs; visible primitive first-person tool; tool harvesting two remaining stone hits in one action. Final aim check revealed that an external overlay crosshair was offset from the viewport center. Added a native centered marker and replayed the food interaction. `PFM4ManualFinal.log` records four gathered food and cancellation without consuming ingredients.
- September 28 rendered replay `Saved/Logs/PFM4FoodFinal.log`: restored already-tested tool/fuel using developer setup, but gathered the food through E. Four gathered food + three wood became one raw food, one cooked food and one dried food, with no wood remaining. Cook took 6 s and dry took 10 s; the UI showed independent freshness deadlines. Q consumed the selected cooked item, reduced occupied slots from four to three, restored Food to 100 and recovered Water. This replay used one rendered standalone client at 960x540/30 FPS cap and closed normally at 08:47:26 UTC.
- `PF.TestGathering`, `PF.TestCrafting`, `PF.Help` and export passed in that rendered process. Evidence: `Saved/AutomationReports/PF_M4FoodFinal_20260928T084634_E51FEE714A7293F84AEC0FB94782C788.json`, aggregate `Passed`. Inspected engine screenshot `Saved/Screenshots/WindowsEditor/ScreenShot00007.png`: readable recipes, tool, native aim marker, cooked/dried output quantities and freshness. `ScreenShot00006.png` is an earlier tool/UI capture with a blurred teleport transition, not the final food evidence.
- Known engine startup issues: game/server launches emit experimental StateTreeToolset/ToolsetRegistry Python errors about missing `ToolsetDefinition`/`PythonTestRunner`, and TEDS widget registration warnings. Rendered replay also emitted a render-thread safety warning for `r.MotionVectorSimulation`. These do not fail the gameplay tests; no new gameplay error, ensure or RHI crash was found in the reviewed runs. Baseline `Saved/Logs/PrimalFrontier.log` remains dated September 23 with normal exit; current launches use the explicit log paths above. Existing starter-content reference findings remain outside M4.
- Performance: two-client NullRHI samples were about 1.63/1.72/1.72 GiB working set for server/client/client (1.48/1.61/1.64 GiB private). Final rendered process sampled 3.36 GiB working set / 5.23 GiB private; Windows free physical memory was 15,068,460 KiB and visible total 33,477,984 KiB. This host reports about 31.9 GiB, so these samples do not certify 16 GB operation. Startup stalled briefly; active play settled at the requested 30 FPS cap. Stutter alone was not treated as gameplay failure; no sustained benchmark is claimed.
- Limitations: cooking/drying is portable and consumes wood fuel; placeable stations belong to future building work. Tool equipping is automatic while carried, with no durability or equipment slots. Storage preservation, technology unlocks, persistence and world expansion remain deferred. All evidence under `Saved` stays local and untracked. Next milestone is greybox building only after explicit authorization.

# Milestone 3 — verified 2026-09-26

**Gate passed for the supported greybox scenario.** Editor build, regression, real one-/two-client NullRHI automation, rendered first-person pickup/split/drop/recovery, food consumption/freshness, developer hooks and actual reconnect observation pass. Reconnect deliberately starts empty until M8 persistence. No packaged Shipping/Server target or rendered two-client result is claimed. All disposable test processes are closed. Stop here; do not begin M4 automatically.

User authorized M3 after M2 passed. Begin from clean `8be2a14`. Implement a small data-asset catalog, owner-replicated inventory on PlayerState, capacity/weight limits, transactional insert/remove/split/drop/pickup, finite batch food freshness and consumption, keyboard placeholder UI and non-Shipping PF grant/remove hooks. Inventory survives pawn respawn; reconnect starts empty until M8 persistence, with no client-supplied restore state. Use a separate primitive M3 map and preserve existing assets. Gate: Editor build, focused regression/duplication/authority tests, rendered first-person inventory playtest, one then two NullRHI clients, reconnect observation, logs/reports/memory and documentation. Do not begin M4 or technology progression in this step.

Implementation and checkpoint evidence (2026-09-25; pending items below are resolved by the final gate):

- Baseline build succeeded in 24.43 s. First inventory compile failed C2065 because Unreal replication macros require `OutLifetimeProps` as the parameter name; corrected both new classes, rebuilt successfully in 13.87 s. HUD float-literal C4305 warnings were corrected with float suffixes. Subsequent builds passed without compiler warnings. UBT initially skipped newly added test translation units; regenerating its makefile picked them up, and compiled test registration was verified in the actual reports.
- `Saved/AutomationReports/M3Transactions/index.json` initially failed three expiry assertions because one large fixture tick was clamped. Replaced it with thirty 0.1 s ticks and explicitly asserted that world time crossed the deadline. `M3TransactionsVerified/index.json`: 1 passed, 0 failed/warnings. Engine exit status alone is insufficient: the initial failed run still exited with 0.
- `M3Regression/index.json`: 9 passed, 0 failed/warnings. Exact tests: `PF.Inventory.Transactions`, `PF.Inventory.WorldTransfers`, `PF.Survival.Component`, `.Needs`, `.Environment`, `.Lifecycle`, `PF.PrimalAgentTools.CommandArguments`, `.MissingSystemsAreBlocked`, `.TeleportAndRuntimeReset`. Log: `Saved/Logs/PFM3Regression.log`.
- `PFM3Setup.log` confirms only the new catalog and M3 map were saved through supported Editor APIs; existing Content files remain unchanged.
- First live run (`PFM3Server1.log`, `PFM3Client1.log`) was stopped after identifying a harness timing issue: its drop RPC immediately followed split and hit the intentional 0.15 s rate limit. Added a 0.5 s client-test wait; gameplay throttling is unchanged. The stopped run has no acceptance result.
- Corrected real-network runs passed `PF.Inventory.Live`: `M3Server1Verified/index.json` and `M3Client1Verified/index.json`, each 1 success, 0 failures/warnings, exit status 0 at 05:50:53 UTC. Two-client reports `M3TwoServer/index.json`, `M3TwoClient1/index.json`, `M3TwoClient2/index.json` also each have 1 success, 0 failures/warnings. Every client split through its owned controller RPC, dropped food, picked it up through server interaction, retained the original food deadline, observed batch expiry, and retained nonperishable inventory through death/respawn. Both clients verified the other PlayerState exists but its private inventory is not replicated.
- Engine-generated command exports are `Saved/AutomationReports/PF_M3Live_20260925T055043_*.json` and `PF_M3Live_20260925T055347_*.json`. The client exports contain intentionally rejected grant probes; their aggregate command-history failures do not mean the automation failed.
- Final HUD build succeeded in 6.69 s without compiler warnings. Initial rendered check found text too small at 960x540; enlarged the panel/font and enabled wrapping. The repeated visual check shows readable controls, Wood x5 after E, Wood x3 plus x2 after X, one world drop and inventory quantity four after G, then quantity five after E recovery. `PFM3ManualVerified.log` records these server mutations. Food UI and reconnect observations are still pending.
- Two-client NullRHI processes sampled 1.63/1.72/1.73 GiB working set and 1.47/1.63/1.64 GiB private bytes, with 11,822,960 KiB physical memory free. Windows reports about 31.9 GiB visible physical memory. Rendered solo frames observed the requested 30 FPS cap; this is a short sample, not a sustained performance certification.
- Rendered process sampled 3.18 GiB working set / 5.02 GiB private bytes, with 12,343,224 KiB free physical memory. Known engine startup Python `ToolsetDefinition`/`PythonTestRunner` errors and TEDS registration warnings remain. No new gameplay errors, ensures or RHI crashes were found in the passing tests. No persistent M3 screenshot has been captured yet; Computer Use captures supplied the partial manual observations.

Changed files: new `Source/PrimalFrontier/Inventory/PFItemCatalog`, `PFInventoryComponent`, `PFInventoryPlayerState`, `PFItemPickup`, `PFInventoryHUD` header/implementation pairs; new `Tests/PFInventoryTests.cpp`, `PFInventoryWorldTests.cpp`; updated survival GameMode, controller and HUD; updated PrimalAgentTools `PFCommands.cpp`, `PFCommandTests.cpp`, new `PFInventoryLiveTests.cpp`; new `Scripts/SetupInventoryMilestone3.py`, `Content/PrimalFrontier/Items/DA_ItemCatalog.uasset` and `Maps/L_M3Inventory.umap`; documentation under `Docs/` and the plugin. Existing binary assets are unchanged. See `INVENTORY_M3.md` for controls and persistence limits.

## M3 final gate, 2026-09-26 (local time; log timestamps are UTC)

- Resumed from clean committed checkpoint `340c538`. No C++ or Content assets changed in this continuation. `PrimalFrontierEditor Win64 Development` rebuilt successfully in **23.35 seconds**, with no compiler warnings. UBT rebuilt the module unity units after its working-set change. Rotating build evidence: `C:/Users/jackh/AppData/Local/UnrealBuildTool/Log.txt`.
- Focused final run: `Saved/AutomationReports/M3FinalInventory/index.json` reports **2 passed, 0 failed, 0 warnings, 0 not run**: `PF.Inventory.Transactions` and `PF.Inventory.WorldTransfers`. `Saved/Logs/PFM3FinalInventory.log` records TestExit status 0 at 2026-09-25 20:17:15 UTC. No warning/error lines were found in this run. The prior nine-test regression and five successful `PF.Inventory.Live` process reports were inspected again; their results remain as recorded above. No gameplay code changed after those network tests.
- Computer Use rendered solo test on `L_M3Inventory`: teleported to the existing food fixture, lowered needs through server-only PF hooks, pressed E to collect food x4, and observed its freshness counter in the inventory. Pressing Q consumed exactly one, reduced weight from 0.8 to 0.6 kg, restored food/water reserves and left the same batch countdown running (260 s before, 253 s after). HUD food/water rose from approximately 17/15 to 51/23 despite ongoing drain. Log: `Saved/Logs/PFM3FoodFinal.log`, accepted consume request at 20:13:44 UTC.
- `PF.GiveItem Item_Wood 25` and `PF.RemoveItem Item_Wood 5` passed; final wood quantity was 20. `PF.GiveItem Item_Stone 100` correctly failed capacity validation without inserting stone: final inventory remained food x3 plus wood x20, 2/8 slots and 10.6/30 kg. `PF.ExportTestReport M3FoodFinal` passed. Export: `Saved/AutomationReports/PF_M3FoodFinal_20260925T201414_223D5306403E2B0666641A8806198DC2.json`. Its aggregate **Failed** and `InventoryRejected` error are the deliberately oversized grant probe, not an unexpected gameplay failure.
- Real reconnect: a single rendered client joined a NullRHI dedicated Editor server at `127.0.0.1:7786`, collected wood x5 (1 slot, 2.5 kg), then issued `open 127.0.0.1:7786`. It successfully rejoined with 0/8 slots and 0 kg. The collected world pickup stayed absent, demonstrating the server world was not reset. `Saved/Logs/PFM3ReconnectServer.log` records first join at 20:15:02 UTC, inventory insertion at 20:15:12, connection teardown and second successful join at 20:15:42. Client evidence: `Saved/Logs/PFM3ReconnectClient.log`. This verifies the intentionally nonpersistent M3 behavior, not item restoration.
- Engine screenshots were opened and visually inspected: `Saved/Screenshots/WindowsEditor/ScreenShot00004.png` shows food x3 with its countdown, wood x20 and needs HUD; `ScreenShot00005.png` shows empty inventory after reconnect. The earlier manual pickup/split/drop/recovery remains recorded above. Both rendered clients exited normally. The owned headless server was stopped after testing; automation exited with status 0.
- Rendered solo sampled **3.34 GiB working set / 5.25 GiB private bytes**. Multiplayer client sampled **3.28 / 5.30 GiB**, headless server **1.64 / 1.48 GiB**, with **11,680,232 KiB free physical memory**. Current Windows visible memory is **33,477,984 KiB (~31.9 GiB)**, so these observations do not certify a 16 GB configuration. Playtest samples were generally 28–31 FPS at the requested 30 FPS cap; screenshot captures briefly showed 20 FPS. No sustained benchmark or rendered two-client RHI result is claimed.
- Known startup Toolsets Python missing `ToolsetDefinition`/`PythonTestRunner` errors and TEDS registration warnings remain in game/server logs. Apart from the deliberate capacity rejection, no new gameplay errors, ensures or RHI crashes were found. The ordinary `Saved/Logs/PrimalFrontier.log` still ends with the September 23 normal editor exit; this continuation used the specifically named logs above.
- Saved the user's eight reference PNGs unchanged under `Docs/References/SurvivalGames`; source/copy SHA-256 comparisons all matched. Added a reference index and linked technology direction; recorded the documentation-only boundary in `DECISIONS.md`. They guide readable quantities/freshness, first-person framing and future original progression without importing runtime art or starting a tech tree.

Files changed in this continuation: `Docs/MILESTONES.md`, `Docs/DECISIONS.md`, `Docs/TECH_TREE_DIRECTION.md`, and `Docs/References/SurvivalGames/README.md` plus eight PNGs. Generated logs/reports/screenshots remain untracked. No unrelated template assets changed. Recommended next step is a separately authorized M4 gathering/crafting slice; M3 retains placeholder keyboard UI and no persistence, equipment or cooking.

# Milestone 2 — verified 2026-09-25

**Gate passed for the supported greybox scenario:** Editor build, seven regression tests, focused expiry deadline test, rendered solo manual playtest, one-client and two-client NullRHI network checks. No packaged Shipping/Server build or rendered two-client RHI result is claimed. Stop here; Milestone 3 has not begun.

Continuation authorized after the recorded Milestone 1 gate. Baseline Editor build succeeded (up to date, 1.98 seconds); Git was clean at `ee05bfe`. Plan: extend replicated vitals with hunger/thirst and exposure, add deterministic server drain/threshold effects and validated placeholder recovery, extend HUD/PF hooks, then run focused regression, solo manual playtest and low-memory network tests. Use a separate primitive M2 map. Do not start Milestone 3 before this gate passes.

Commits for this work include the milestone number and `Co-authored-by: Codex GPT-6 Astra <codex@openai.com>`. Existing Git identity remains the primary author; unrelated changes are excluded.

## M2 implementation and evidence

Changed gameplay files under `Source/PrimalFrontier/Survival`: `PFPlayerSurvivalComponent.h/.cpp`, `PFSurvivalPlayerController.h/.cpp`, `PFSurvivalHUD.h/.cpp`; added `PFSurvivalHazard.h/.cpp` and `PFRecoveryPickup.h/.cpp`. Changed `Tests/PFSurvivalComponentTests.cpp`; added `PFSurvivalNeedsTests.cpp` and `PFSurvivalEnvironmentTests.cpp`. Tooling changes: `PrimalAgentToolsRuntime/Private/PFCommands.cpp` and new `Private/Tests/PFSurvivalNeedsLiveTests.cpp`. Added `Scripts/SetupSurvivalMilestone2.py` and the single new `Content/PrimalFrontier/Maps/L_M2Survival.umap`. Documentation covers architecture, decisions, milestone evidence, Git attribution, command interface, M2 mechanics, food research and future technology progression. No existing Content asset is modified.

- Editor builds succeeded without compiler warnings: initial needs slice 35.03 s, integration 120.29 s (memory pressure while an Editor reopened), fixture refinement 6.36 s, live test 7.41 s. The reopened All Saved editor was closed normally before linking.
- `Saved/AutomationReports/M2Needs/index.json`: 3 passed, 0 failed/warnings. `M2Regression/index.json`: 7 passed, 0 failed/warnings: `PF.Survival.Component`, `.Needs`, `.Environment`, `.Lifecycle`, `PF.PrimalAgentTools.CommandArguments`, `.MissingSystemsAreBlocked`, `.TeleportAndRuntimeReset`. Logs: `PFM2Needs.log`, `PFM2Regression.log`.
- Computer Use manual play on `L_M2Survival`: observed food/water drain, E consumed the ration and restored reserves to 100, empty reserves reduced Health to zero, death HUD appeared, and respawn reset reserves. Real hazard overlap displayed Exposure 100%, caused damage/death; a later controlled entry/exit retained the same living pawn at Health 87 and restored Exposure 0. The exit observation temporarily used `slomo 0.1` and restored `slomo 1`; no settings or map were saved. Logs: `PFM2Manual.log`, `PFM2ManualExit.log`.
- `PF.Help`, `PF.SetHunger`, `PF.SetThirst`, `PF.SetExposure`, `PF.RecoverNeeds` passed in the actual game. Fixed recovery changed 20/30 to 55/65 before ongoing drain. Reports: `PF_M2ManualExit_20260922T231620_AC98172A40D388872990DBA71048263D.json`, `PF_M2ManualHooks_20260922T231651_6503A18F42786CA5B23CB7A568FAFC89.json` under `Saved/AutomationReports`.
- Engine screenshot artifacts: `Saved/Screenshots/WindowsEditor/ScreenShot00001.png` and `ScreenShot00002.png`. The latter captured the teleport transition before the next exposure update and is motion blurred; it is not evidence of the final zero-exposure HUD. The live Computer Use capture and exposure log at 2026-09-22 23:16:21 UTC confirm exit.
- Setup issue resolved: `SaveMap` created M2 but left M1 active. Restored only this session's unintended M1 write from Git; populated the new M2 map using Editor APIs. Corrected setup to `NewLevelFromTemplate` plus an active-map guard. `PFM2SetupRepair.log` confirms only M2 was saved; Git reports no modifications to existing Content assets.
- Manual gameplay sampled 29–30 FPS; startup/capture stalls were transient. System overlay ranged approximately 13.1–15.4 GB RAM across the two days; resumed rendered process was 2.30 GiB working set / 5.21 GiB private bytes. With two NullRHI network processes starting, available physical RAM fell to about 0.77 GiB. No inference of gameplay failure is made from stutter alone.

- Food-expiration refinement builds passed (26.34 s, 7.05 s and final fixture build 6.22 s). `M2ExpiryRegression/index.json` repeats the seven regression tests with 7 passed, 0 failed/warnings. `M2ExpiryDeadline/index.json` passes the focused environment test with cleanup deliberately disabled until after the expired-consumption rejection check.
- `PFM2FoodManual.log`: observed the ration's visible freshness countdown in the rendered game; accelerated the disposable session with `slomo 10` and observed all three pickups expire and disappear (server expiration logs 2026-09-22 23:25:31 UTC). `ScreenShot00003.png` shows the countdown. This is accelerated expiration verification, not a normal-speed performance benchmark.
- Original one-client NullRHI network gate passed: `M2Server1/index.json` and `M2Client1/index.json`, each 1 success, 0 failures/warnings for `PF.Survival.NeedsLive`. Both reached TestExit with status 0. This run predates the replicated food-expiration assertion; final results follow below. A later `PFM2ExpiryServer1` setup was intentionally stopped before connecting a client while the user used the rendered game; it has no acceptance result.
- User-requested Palworld/ARK food research and original proposed spoilage/preservation rules are in [FOOD_AND_PRESERVATION.md](FOOD_AND_PRESERVATION.md). Only M2 world-food expiration is implemented; cooking, inventory and storage remain behind their later gates.

## Final gate, 2026-09-25

- `PrimalFrontierEditor Win64 Development` build succeeded, up to date in 2.11 s, no compiler warnings. Rotating evidence: `C:/Users/jackh/AppData/Local/UnrealBuildTool/Log.txt`.
- Final one-client expiry/needs run: `Saved/AutomationReports/M2ExpiryServer1Verified/index.json` and `M2ExpiryClient1Verified/index.json`, each **1 passed, 0 failed, 0 warnings**. Corresponding logs: `PFM2ExpiryServer1Verified.log`, `PFM2ExpiryClient1Verified.log`; both exit status 0 at 05:19:10 UTC.
- Two-client run on localhost port 7783: `Saved/AutomationReports/M2TwoClientServer/index.json`, `M2TwoClient1/index.json`, `M2TwoClient2/index.json`, each **1 passed, 0 failed, 0 warnings** for `PF.Survival.NeedsLive`; all exit status 0 at 05:21:54 UTC. Corresponding logs have prefix `PF` and suffix `.log` under `Saved/Logs`. Both clients observed PlayerIds 256 and 257 at needs 40/50/exposure .25, recovery 75/85/0, empty needs with Health 90, death Health 0, and replacement pawns at 100/100/100. Fresh food and subsequent replicated removal assertions passed on every process. Direct and PF command client mutations were rejected.
- PrimalAgentTools final exports: `PF_M2LiveServer_20260925T052144_E4015694434DDB4B0ECAD0A2BDE72692.json`, `PF_M2LiveClient_20260925T052144_808CE59F4A097DEA5DC73A8F7DA2EA42.json`, `PF_M2LiveClient_20260925T052144_E789E26E4B8F957379E39788CC019AA4.json`. Client command-history aggregate failures are the deliberate `NotAuthority` probes; the automation independently verifies rejection and passes.
- Known pre-existing startup issues remain in engine Toolsets Python (`ToolsetDefinition` and `PythonTestRunner` missing in game/server mode) and TEDS widget registration. These are not new gameplay failures. No new gameplay errors, ensures or RHI crashes were found in the successful final network runs. The ordinary `PrimalFrontier.log` was also checked; its last recorded editor session exited normally on September 23.
- On this resumed host Windows reports 33,477,984 KiB visible physical memory (about 31.9 GiB), unlike the earlier 16 GB runs. The three NullRHI processes sampled 1.63/1.73/1.73 GiB working set (1.46/1.62/1.62 GiB private bytes), with 12,327,252 KiB physical memory free. Thus the final two-client result is not a certification of the earlier 16 GB environment. Headless runs do not measure rendering stutter; prior rendered solo observations remain above.
- The user's technology references are retained in `TECH_TREE_DIRECTION.md`: accessible ordinary unlock points, a separate challenge-earned path, early essentials and later preservation. No technology tree was implemented.

All test processes finished. Remaining limits: no inventory, player cooking/drying, preservation containers or persistent freshness yet; the world ration is a finite fixture with a 300 simulation-second test lifetime. Recommended next milestone is M3's authoritative item/inventory data, carrying freshness through pickup/drop/stack operations. Do not start it automatically in this step.

# Milestone 1 — verified 2026-09-21

**Gate passed for the supported one-client greybox scenario. Stop here; Milestone 2 has not begun.** The final Editor build, five focused regression tests, actual server/client replication test, and manual first-person playtest passed. This is not a packaged Shipping or two-client acceptance result.

## Changes

- Added separate `PFSurvivorCharacter`, `PFSurvivalGameMode`, `PFSurvivalPlayerController` and `PFSurvivalHUD` headers/implementations under `Source/PrimalFrontier/Survival`. Integrated the existing replicated Health/Stamina component; server damage, jump cost, delayed recovery, death and PlayerStart respawn work through the same authoritative APIs.
- Exported the existing C++ character class for cross-module use; its camera/input implementation and template assets remain unchanged.
- Updated PrimalAgentToolsRuntime's build dependency and `PFCommands.cpp`; added `PFSurvivalLiveTests.cpp` and `Source/PrimalFrontier/Tests/PFSurvivalLifecycleTests.cpp`. `PF.SetHealth`, `PF.SetStamina`, `PF.Damage`, `PF.Kill` and `PF.Respawn` use real gameplay APIs. Commands are discoverable, logged, reported and non-Shipping. Mutations reject clients and ambiguous multiple-player targeting.
- Added `Scripts/SetupSurvivalMilestone1.py`, three project-owned Blueprint compositions in `Content/PrimalFrontier/Survival`, and `Content/PrimalFrontier/Maps/L_M1Survival.umap`. The map contains four cube actors, one PlayerStart and one directional light. It uses existing template presentation/input assets and engine primitives only.
- Updated architecture, decisions, this milestone log and developer command documentation; added `Docs/SURVIVAL_M1.md`. Hunger and Thirst remain interfaces only.

## Build and automated evidence

`Build.bat PrimalFrontierEditor Win64 Development -Project=C:/UnrealProjects/PrimalFrontier/PrimalFrontier.uproject -WaitMutex -NoHotReloadFromIDE -MaxParallelActions=2` succeeded. Final compilation/link: **12.56 seconds, no compiler warnings**. Evidence: `C:/Users/jackh/AppData/Local/UnrealBuildTool/Log.txt` (rotating build log).

`Saved/AutomationReports/M1Regression/index.json`: **5 succeeded, 0 failed, 0 warnings, 0 not run**:

1. `PF.Survival.Component`: initial values, damage, stamina/recovery, invalid input, authority guards, death and reserved interfaces.
2. `PF.Survival.Lifecycle`: actual native GameMode possession, grounded PlayerStart, damage/jump cost, client mutation rejection, three timed death/respawn cycles.
3. `PF.PrimalAgentTools.CommandArguments`: command registration, concise help, arguments, PF.Help execution and recording.
4. `PF.PrimalAgentTools.MissingSystemsAreBlocked`: unimplemented integrations remain explicit blockers.
5. `PF.PrimalAgentTools.TeleportAndRuntimeReset`: existing authority/collision behavior unchanged.

Log: `Saved/Logs/PFM1Regression.log`. The focused lifecycle retry also passed with NullRHI in `M1LifecycleVerified/index.json` and `PFM1LifecycleVerified.log`.

`PF.Survival.Live` passed independently on a NullRHI dedicated server process and one rendered client over localhost port 7781. Each report has **1 succeeded, 0 failed, 0 warnings**:

- `Saved/AutomationReports/M1Server/index.json`; `Saved/Logs/PFM1Server.log` (success at 03:45:31.916 UTC).
- `Saved/AutomationReports/M1Client/index.json`; `Saved/Logs/PFM1Client.log` (success at 03:45:31.906 UTC).

The client observed initial 100/100, authoritative Health 75/Stamina 20, death at 0/0, replacement possession and 100/100 after respawn. All five client mutation commands and direct component mutation probes were rejected. Both processes reached TestExit without a crash. This uses the Editor executable's `-server` mode; a packaged Server target is not claimed.

PrimalAgentTools exports:

- `Saved/AutomationReports/PF_M1Manual_20260921T034349_02F73ADD47C7FCA616EC5C939AF0E44F.json`
- `Saved/AutomationReports/PF_M1LiveAuthority_20260921T034521_E400187743BCB85B32A8FEADFFA05A6B.json`
- `Saved/AutomationReports/PF_M1LiveClient_20260921T034521_F9B92423418C1B381676A38C87F1D028.json`

The client command-history export deliberately has aggregate status **Failed**, because its five forbidden mutation probes are recorded as failed commands with `NotAuthority`. The automation report separately verifies those rejections and passes. Do not interpret the history export as a successful mutation or silently discard its failures.

## Manual playtest and assets

Computer Use inspected the actual standalone and connected-client windows at 960x540. The standalone test showed the first-person template mesh/camera, movement (debug X changed from -400.00 to -397.72 on W), mouse look, and jump reducing Stamina from 100 to 80. `PF.Damage 25` changed the visible Health bar to 75. `PF.SetStamina 0` emptied the bar and it recovered to 100. `PF.Kill` showed the death message and automatic respawn restored Health/Stamina and the initial view. Connected-client play showed server-driven 75/20, death, and respawn on the HUD; jump input also responded.

Persistent screenshot, generated by Unreal's `shot showui` and inspected: `Saved/Screenshots/WindowsEditor/ScreenShot00000.png`. Manual log: `Saved/Logs/PFM1Manual.log`. Asset creation log: `Saved/Logs/PFM1Setup.log`.

All **609 pre-existing Content files** retain their pre-task SHA256 aggregate `AC7D247B10EF3AB88337210954C410EF2C779CEEE96FB7E1FE3D12CC11AD9B16`. Only the four new M1 assets were created. Existing map relocation/deletions and unrelated Phase 10 work were preserved.

## Failures resolved, warnings and limits

- A C++ local name initially shadowed UWidget::Slot; renamed and rebuilt successfully.
- The resumed build initially failed LNK1104 because the user-opened Editor locked the DLL. Closed the All Saved editor normally, then the build passed.
- The first lifecycle run incorrectly required exact PlayerStart Z despite capsule grounding. It now checks start XY and bounded vertical adjustment. A subsequent expected-warning regex needed correction. The final rerun passes; the narrowly matched native fixture warnings reflect missing presentation meshes, not a failure in the actual Blueprint character.
- Asset setup reported a hierarchy-change warning when reparenting the new GameMode copy. The new composition loaded and passed the real possession/respawn tests; templates were not rewritten.
- Existing engine startup issues remain: StateTreeToolset/ToolsetRegistry Python scripts expect editor-only `ToolsetDefinition`/`PythonTestRunner` in game/server launches, TEDS widget-factory registration warnings, and the `r.MotionVectorSimulation` render-thread warning. The server also recovered a stale Zen lock. No new survival gameplay errors, ensures or RHI crashes occurred in the passing runs. `Saved/Logs/PrimalFrontier.log` was inspected; the M1 launches intentionally use separate named logs.
- Observed rendered gameplay held the requested **30 FPS** cap in sampled frames. Screenshot capture briefly displayed 1 FPS before returning to 30; no sustained gameplay stall was observed in this short test. This is not a long-duration performance certification.
- Standalone process: about **2.82 GB working set / 5.10 GB private bytes**. During the network test, rendered client: **2.37 / 5.11 GB**; NullRHI server: **1.64 / 1.47 GB**. Whole-system overlay ranged about **14.5–15.7 GB RAM**; OS available physical RAM reached **0.95 GB**. Process private bytes are not physical residency. Two rendered clients were not practical; no new two-client result is claimed.
- Existing historical generated files exceed GitHub's file-size limit. Do not rewrite history or stage unrelated changes as part of this milestone.

Recommended next step: user review of this first-person foundation. Start Milestone 2 only after explicit approval. All test sessions were closed.

---

# Greybox milestone plan — 2026-09-21

Current authorization: implement and verify **Milestone 1 only**, then stop and report. Milestones 2–8 below are a roadmap, not permission to bypass a gate.

1. Player foundation: integrate the existing Health/Stamina component with an isolated survivor character, first-person input, server damage, death, PlayerStart respawn, placeholder HUD and PF developer hooks. Build, automated tests, manual solo playtest, then one client/server test are required.
2. Hunger/thirst and environmental survival, after an explicit continuation.
3. Item data and authoritative inventory.
4. Gathering and crafting.
5. Greybox building and ownership.
6. Passive/hostile greybox creatures.
7. Small greybox survival arena.
8. Versioned persistence and multiplayer/reconnect acceptance; stop permanently after this milestone.

Each milestone must compile, pass focused automated tests, pass a manual playable test, validate server authority, and record logs, useful screenshots, memory/stutter observations and decisions before progression. Use NullRHI for nonvisual checks and one rendered client at a time. Use existing template assets and primitives only. Preserve unrelated Git and asset changes.

Audit: Phase 9 reports contain three passing tests, Phase 10 editor reports six passing tests, and the Phase 10 standalone live test passed. Client 2's rendered D3D12 residency crash remains an incomplete multiplayer result, not a gameplay pass. The current baseline build now succeeds after normally closing the saved editor (12.10 seconds, two build workers); no new gameplay integration has been verified yet.

## Earlier checkpoint

## First-person survival foundation: in progress, blocked at first build

Scope for this milestone is Health, Stamina, server-authoritative changes, death/respawn, a minimal first-person HUD and focused tests. Hunger and Thirst remain planned interfaces. Inventory, crafting, creatures, building, persistence and world expansion are not authorized by this milestone.

The initial source slice adds `UPFPlayerSurvivalComponent` with replicated Health/Stamina, server-only mutation guards, clamping, stamina recovery, death-state Gameplay Tags and notifications. `PF.Survival.Component` contains assertions for initial values, damage, stamina, authority rejection and death. Hunger/Thirst tags reserve names only; `SupportsStat` explicitly returns false for them. GameplayTags and SlateCore were added to the game module dependencies (SlateCore is for the planned placeholder HUD).

This component is **not yet attached to a player**. Character integration, respawn, HUD, PrimalAgentTools integration, project-owned Blueprint copies and live replication tests remain unimplemented. Existing first-person/template assets have not been changed by this work.

Verification attempted:

- `Build.bat PrimalFrontierEditor Win64 Development -Project=C:/UnrealProjects/PrimalFrontier/PrimalFrontier.uproject -WaitMutex -NoHotReloadFromIDE -MaxParallelActions=2`
- UHT, `PFPlayerSurvivalComponent.cpp` and `PFSurvivalComponentTests.cpp` compiled; no compiler warnings appeared in this attempt.
- Target build **FAILED** with `LNK1104` because running UnrealEditor PID 35724 held `UnrealEditor-PrimalFrontier.dll`, `UnrealEditor-PrimalAgentToolsRuntime.dll` and `UnrealEditor-PrimalAgentTools.dll` open. UBA first reported the file-in-use errors and then retried linking without UBA; the same lock remained.
- Build evidence: `C:/Users/jackh/AppData/Local/UnrealBuildTool/Log.txt`, result `Failed (OtherCompilationError)`, elapsed 78.57 seconds. This generated log may be replaced by the next build.
- New tests run: **none**. The failed link prevents testing current source; previous Phase 10 passes do not verify this new component.
- Existing `Saved/Logs/PrimalFrontier.log` contains the `r.MotionVectorSimulation` render-thread warning. No new editor or multiplayer process was launched.
- Resource observation: UBT reported 15.93 GB physical RAM and 32.01 GB committed at build start. Compilation was limited to two workers. No gameplay performance or stutter assessment was made.

Stopped after the build failure without expanding implementation. Next step: close the existing editor after handling any unsaved work, retry the same incremental build, and run only `PF.Survival.Component` with NullRHI before continuing this milestone. No Git staging or commits were performed.

---

\# Milestones



\## Milestone 0 — Toolchain



\- \[ ] Unreal project opens

\- \[ ] Editor target compiles

\- \[ ] Git and Git LFS are configured

\- \[ ] Logs can be collected

\- \[ ] Development test map exists

\- \[ ] Automated test workflow exists



\## Milestone 1 — Vulnerable Survivor



\- \[ ] First-person character

\- \[ ] Health

\- \[ ] Stamina

\- \[ ] Hunger

\- \[ ] Thirst

\- \[ ] Death and respawn

\- \[ ] Two-player multiplayer test



\## Milestone 2 — Hunter



\- \[ ] Resource gathering

\- \[ ] Inventory

\- \[ ] Primitive tools

\- \[ ] Basic weapon

\- \[ ] Passive creature

\- \[ ] Hostile creature

\- \[ ] Basic combat



\## Milestone 3 — Builder



\- \[ ] Crafting

\- \[ ] Foundation placement

\- \[ ] Walls and doors

\- \[ ] Storage

\- \[ ] Building persistence

\- \[ ] Server validation



\## Milestone 4 — Creature Tamer



\- \[ ] Companion creature prototype

\- \[ ] Ownership

\- \[ ] Follow and stay commands

\- \[ ] Creature persistence

\- \[ ] Basic creature utility



\## Milestone 5 — Settlement Leader



\- \[ ] Cooperative permissions

\- \[ ] Improved building pieces

\- \[ ] Crafting stations

\- \[ ] Settlement storage

\- \[ ] Resource and creature management



\## Milestone 6 — Industrial Survivor



\- \[ ] Advanced resources

\- \[ ] Processing systems

\- \[ ] Industrial crafting tier

\- \[ ] Advanced structures

\- \[ ] Dangerous Eryndor regions



\## Milestone 7 — Ancient Researcher



\- \[ ] Ancient sites

\- \[ ] Research progression

\- \[ ] Ancient technology

\- \[ ] First Frontier preparation



\## Milestone 8 — Dimensional Explorer



\- \[ ] First Frontier

\- \[ ] Frontier hazards

\- \[ ] Frontier creatures

\- \[ ] Frontier resources

\- \[ ] Dimensional progression



\## Milestone 9 — Master of the Frontiers



\- \[ ] Multiple Frontiers

\- \[ ] Endgame systems

\- \[ ] High-tier technology

\- \[ ] Optional third-person mode

\- \[ ] Optional PvP evaluation

