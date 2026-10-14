# M12/M14 — Inventory upgrades and stationary wall rest

October14,2026. User WALL_CLIMB PASS accepts the previous continuous-climbing check. This bounded follow-up implements the requested inventory location and neutral holding; prior acceptance and fresh technical verification remain separate.

## Controls and authority

**Tab / gamepad View** opens the centered inventory Items page, with a mouse cursor and balanced movement/look lock. Select **Player upgrades**, press **U**, or use **RB** to switch to its embedded upgrade page. U from gameplay directly opens that same inventory page. **Esc / U / gamepad B / LB** from upgrades returns to Items; **Tab / View** closes the whole inventory. Esc/B on Items closes the inventory. P/Menu switches to Pause, C/Y to crafting; the world keeps running while inventory is open. Pause no longer contains a separate Player upgrades entry. U has exactly one binding, so it cannot also transfer to an aimed chest.

The existing inventory selection GUID survives page changes. Item navigation/split/drop/consume remain on Items; upgrade navigation cannot accidentally drop/consume a bag item. The existing stat widget is a child of inventory, not another viewport layer or a copied progression system. Each upgrade still sends only the owned stat ID to the existing validated server request. Stat/normal-tech pools, earned XP, maximum stamina without refill, carry/craft/attack effects, save format and budgets are unchanged. Headless controllers exercise flags without viewport/input freeze; physical controller feel remains unverified.

**Stationary wall holding spends zero stamina and regenerates none.** Moving up/down/sideways still drains12/s, exhaustion drops, F attaches/drops, W/S/A/D and automatic rising paid W+Space contact remain. Do not interpret neutral holding as regeneration or infinite powered ascent. Recovery resumes normally off the wall. No arbitrary height ceiling, collision bypass, client stamina authority or new asset dependency. Old editable hold-drain property is removed; holding is always neutral. Server validates movement speed/drain tuning as before.

## Existing reachable fixture

`/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld`: camp4m wall `(1850,-1100,200)`, west faceX1700;10m wall `(2450,-1100,500)`, west faceX2300; sign `(1550,-850,200)`. Turn right/east around the low blocks. **Still: no drain/recovery; move:12/s; empty=fall** now appears on the saved sign. Supported AddClimbBudgetFixtures.py setup/reload1 text mutation, repeat0; all4fixtureGUIDs and83other actors preserved (87total). No resource/spawner/save-layout migration.

Inventory can be checked at the safe camp without spawning any extra asset. For a normal UI observation, press Tab, select Player upgrades, inspect separate point counts, Esc to Items, Tab to close. If one stat point is already earned, choose Stamina once and check +20 maximum with no refill/no tech-point spend. No immediate XP grind needed: no points means report no-points/disabled buttons.

For rest: F at the4m face, W up about1second, release movement and note stamina for3seconds. It should remain unchanged while attached, then resume draining only when moving. F releases. Existing G16 WALL_CLIMB acceptance is retained; this tiny rate change has its own native/live/visual checks, not a request to repeat all accepted encounters/build/save/UI tests.

## Verification checkpoints

Reports under Saved/AutomationReports; corresponding unique logs under Saved/Logs. Backend suites use real disposable worlds, live fixtures use the actual saved level and real F/W input. All automated profile/save/settings sources are isolated; no personal progress should be modified.

| Run | Result | Seconds / peak working, private GiB |
| --- | --- | --- |
| PFInventoryStatsRestFirstBuild.log |EditorPASS |41.74s |
| PFInventoryStatsRestTestsBuild.log |EditorPASS after updated UI/native fixtures |11.91s |
| Automation_InventoryStatsWallRest_20261014_024802883_b0130b43 |6/6: Contact, AutomaticClimb, Rules, Gamepad, InventorySelection, StatRuntime; raw/test severity0 |17.76s /2.975,2.839 |
| M12StatsUI720_20261014_025055443_f349e47d |PlayerStatsLive1/1: actual embedded child, tab/U, budgets, Back/View, no Pause entry, real10m top;8PNG generated, relevant pages inspected |48.84s /3.109,5.320 |
| PFInventoryBoundsContractBuild.log |EditorPASS, fixture-only centered bounds contract |6.17s |
| M11Inventory720_20261014_025614809_0b016e69 |InventorySelectionLive1/1: actual expiry/full bag/selected-GUID/reordering/freshness/quantities and new modal bounds |36.21s /3.113,4.883 |
| M14HeightAuthoredClimb1_20261014_025800973_c3bd3afd |AuthoredLive2/2 server+one client; idle stamina/position unchanged,4m top/24budget10m0fall |45.65s /1.728–1.797,1.634–1.782 |
| M14HeightAuthoredClimb2_20261014_025936766_fcde88a7 |AuthoredLive3/3 server+two clients,75ms/1%; idle/remote climbing/authority/moving/exhaustion pass |55.02s /1.724–1.794,1.633–1.784 |
| PFInventoryWallRestFeedbackBuild.log |EditorPASS, clarify neutral-rest help/feedback only |7.23s |
| M14Height720_20261014_030255622_8feeebdc |Rendered AuthoredLive1/1: actual neutral holding24/100, moving/top/zero fall and rest HUD |45.45s /3.106,5.320 |
| M11MenuNav720_20261014_030432366_3b2027bb |MenuNavigationLive1/1: inventory/craft/build/chest/settings/controls Back/Pause and balanced input |31.12s /3.136,5.296 |
| PFInventoryStatsWallRestGameBuild.log |Development Win64 GamePASS/no compiler warnings |46.73s |

Native Contact rests at60/100 with zero recovery delay and80/s recovery configured, proving neither loss nor recovery is hidden by a full bar. It also checks moving12/s, partial exhaustion, real ceilings/contact/top, authority/death/weight. Gamepad native routing proves one U binding, embedded-state flags, Back→Items and selected-stack/item conservation. Rendered UI must prove the actual child/tab/focus/budgets/Back/close, not only flags. Existing save/respawn/max-stamina native integration remains covered.

Final Editor7.23s/Game46.73s pass with no compiler warnings. Inventory/upgrades/Items screenshots at720p/HUD1.5 show actual tabs, zero-point disabled choices, separate15technology/0stat after seeded allocations, scrollable crafting row, Items return and full-bag8/8 Rows5–8 with unchanged selected freshness/weight. Rendered fixture seeds1000XP only for UI boundary checks; this is not earned pacing or a normal free-point grant. Neutral wall PNG shows24/100 during rest and0/100 after moving/exhaustion; Pause PNG shows five remaining actions with no upgrades. Private/default guards unchanged. Fifteen successful final records (6native+5network+4rendered); inventory failed report retained below. No new gameplay error/fatal/ensure, successful native raw severity0. All successful live/render logs add0unique severity versus prior `M12StatsUI720_20261013_073212143_3c179b6c.log` after timestamp/frame stripping:24headless/26rendered known warnings,14experimental engine Python error lines per role. HLOD/factory/ToolsetDefinition/PythonTestRunner startup issues remain, not fixed or warning-free. Ordinary PrimalFrontier.log is the user's earlier session, inspected without fatal/ensure; use unique test logs for these verdicts.

Actual host31.93GiB RAM/RTX4060 8GB; one720p rendered process at a time, requested uncapped, working peaks3.106–3.136GiB/private4.883–5.320. No sustained FPS/stutter/minimum-spec benchmark. Installedengine5.8.3 versus requested5.8.2 unchanged.1440p remains deferred; no two rendered clients, packaged-Server claim, final art or later tier expansion. Backend stat/save data unchanged; broader Copper/stat-earned/device/audio/world-roadmap acceptance remains separate.

## Retained failure

M11Inventory720_20261014_025311408_d72156f7 FAIL1/1,8errors,36.03s/3.175working5.278privateGiB. All8 were the obsolete right-side/aim-clear overlay placement assertion. Actual item expiry, selection/reordering/conservation and text heights passed. Inventory is intentionally now a centered modal with locked gameplay, so update only the fixture to require page bounds plus move/look lock/cursor and running world. No production relaxation, no removed item/font assertion; failed selector replay above passes. Build failures/ensures/RHI crashes: none in this follow-up so far.

## Changed files and handoff

- Inventory: PFInventoryHUD.cpp/.h and new PFInventoryInput.cpp.
- Progression: PFStatInput.cpp and PFPlayerStatsMenu.cpp; point authority/record formats unchanged.
- Survival: PFSurvivalPlayerController.cpp/.h, PFGamepadInput.cpp, PFPauseMenu.cpp/.h, PFSurvivorCharacter.h, PFSurvivorTraversal.cpp, PFTraversalMovement.h and PFWallClimbMovement.cpp.
- Tests: PFClimbContactTests.cpp, PFAuthoredClimbLiveTests.cpp, PFGamepadTests.cpp, PFPlayerStatsMenuTests.cpp, PFInventorySelectionLiveTests.cpp (bounds-only fix).
- Supported authoring: AddClimbBudgetFixtures.py, OpenWorld.umap header and existing sign external actor C/UB/MJPOCWBOF45G23LTPORY94.uasset. No geometry, node/spawner, template asset or default configuration change.
- Docs: CURRENT_STATE, MILESTONES, DECISIONS, ARCHITECTURE, NETWORKING, PLAYER_STATS_M12, PLAYTEST, PLAYTEST_RESULTS_2026_10_14, CONTINUOUS_CLIMB_M14, TRELLO_SYNC, TRELLO_TEST_GUIDES and this evidence file.

Trello YQ9xtojr technical Done/complete and GsgHT0o0 user WALL_CLIMB Done/complete confirmed. MainM14 remainsAI Doing/incomplete for original broader world objectives, not a pending climb repeat. STATS_UPGRADE keeps Personal2ToDo with current G14 inventory steps/history retained, no mandatory grind/newduplicate card. Controller current optional controls corrected only; Personal3/due/membership/unverified unchanged. Copper/audio/1440 and earlier accepted checks preserved. Scoped Sol commit/push follows final Git review.
