# M12 - Level-earned player upgrades

**Current UI:** Tab→Player upgrades or U opens the embedded inventory page; Esc/U/B/LB returns to Items, Tab/View closes inventory. No separate Pause entry. Existing point-ID authority, stat/technology budgets and no-refill capacity rules are unchanged. Holding still on a wall is now neutral, moving12/s. INVENTORY_UPGRADES_WALL_REST.md supersedes the historical presentation/hold-cost notes below; G14 uses the current inventory flow.

October14 traversal update: CONTINUOUS_CLIMB_M14.md supersedes the historical prepaid costs below. Stamina rank increases capacity without refill; wall attachment does not prepay height, movement/holding drains12/4 per second and zero makes you fall. Native StatRuntime and actual rendered PlayerStatsLive10m continuous climb passed; current technical evidence is in that M14 doc. G14 stat gate remains separate/unverified; old4m user CLIMN_STAMINA PASS accepted only its prior scope. Current wall guide G16.

One player stat point is earned per level after level1, using the existing server-owned gathering and first-craft XP. Level2 starts at100XP. The current greybox supports levels1-10, so there are9 total stat points. Existing technology earns3 points per level and spends them on learned recipes; the ledgers are independent. Ancient technology rewards/unlocks are still planned and are labelled unavailable in the menu. No boss rewards, free XP or item grants were added to ordinary gameplay.

Press **U**, or Pause > **Player upgrades**, to open the centered native placeholder screen. Click an upgrade or use arrows/D-pad and Enter/A. U/Esc/B returns to gameplay; P/Menu opens Pause. The stats screen does not pause the world. Disabled buttons explain the no-points state; the screen shows current level/XP, next threshold, separate budgets and each allocation. Keyboard/gamepad selection scrolls the chosen row into view. Physical controller feel remains a separate untested gate.

| Upgrade | Default per point | Runtime effect |
| --- | --- | --- |
| Attack | +10% of base damage | Server melee trace damage multiplies the carried weapon/bare-hand damage. It does not grant gathering tiers or change structure-demolition rules. |
| Stamina | +20 maximum | Increases capacity without refilling it. Normal recovery fills the new capacity. A fresh respawn fills the upgraded maximum. |
| Carry weight | +10 kg | Raises the player bag threshold from30kg; existing slowdown/sprint/jump rules use that threshold. Chest capacity remains60kg. |
| Crafting speed | +10% speed | New job duration is recipe duration divided by the speed multiplier. Allocating mid-job does not shorten its existing deadline. The crafting detail shows the actual new-job duration. |

Allocations survive death because progression lives on PlayerState. Respawn applies validated ranks to the fresh pawn. Inventory remains the existing owner-only bag; no new inventory UI or capacity bypass is introduced.

## Authority and data

`FPFPlayerStatRanks` holds four bounded integer ranks, not client-supplied bonuses or cached points. Sum of ranks cannot exceed Level-1. `PurchaseStat` validates a complete candidate, rejects unknown enum values and overspending atomically. `ServerUpgradeStat` accepts only an upgrade ID; the owning controller/PlayerState/pawn must be authoritative, alive, in the same game world and rate-limited. Direct client or foreign-pawn mutations refuse. Records and result feedback replicate only to their owner; public vitals/carry behavior use the normal replicated systems.

Stat gain tuning is exposed on the existing native `UPFProgressionCatalog` as bounded Game-config properties. The runtime uses its shared native defaults, not an arbitrary newly created catalog asset. Project tuning may be configured under `[/Script/PrimalFrontier.PFProgressionCatalog]` in DefaultGame.ini with `AttackBonusPerPoint`, `StaminaPerPoint`, `CarryKgPerPoint`, and `CraftingBonusPerPoint`. No INI was changed in this implementation. Values must be finite: attack/crafting0-1, stamina1-200, carry1-100. Defaults remain0.1/20/10/0.1. Server/client builds should share this project balance; changed tuning needs its own verification and save-compatibility decision. Base capacity is the current greybox100stamina/30kg policy. Do not lower capacities and assume existing saves are compatible.

The progression envelope is now **PFXP V3** when allocations exist. Four ranks follow the bounded gather-window payload. Zero allocations keep byte-compatible V1/V2 writers; both old versions read with zero ranks, retaining old XP/knowledge/windows and derived unspent points for already-earned levels. Outer world format staysV2; player inventory/vitals format staysV1. Validation decodes and validates owner-bound progression before accepting stamina above100; it derives the legitimate maximum from ranks and trusted tuning. Restore preflights that capacity before applying saved vitals. No maximum or point budget is trusted from a client or save field. Corrupt/unbounded records preserve outputs; private worlds/profiles are not migrated in place automatically.

## Climb integration

`CONTINUOUS_CLIMB_M14.md` describes the current duration-based stamina drain. Primitive4m and10m walls are east/right of camp in `/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld`, at `(1850,-1100,200)` and `(2450,-1100,500)`. F attaches with at least1 stamina; W then moves up, S down, A/D sideways, F drops. Hold4/s/move12/s with no recovery on wall; at zero fall. Ordinary W+Space still pays20 jump stamina before attachment. More capacity lets you climb longer; no whole-height affordability check. Recover first; allocating a point does not refill it. Current G16 checks midway holding/exhaustion, rather than the old103-stamina refusal.

The10m native extension found a real perched-capsule edge bug. The server now retains its bounded approved drive until the complete landing target is reached, using Unreal's swept walking/step-up after the rounded edge becomes walkable. The owner receives that server-approved target; no client target RPC is added. New jumps, death, cancellation, completed landing and expiry clear it. This is greybox ascent/mantle movement, not a wall-hanging animation system.

## Evidence and limits

Builds use max4 actions, no Live Coding/competing Editor. Installed Build.version is5.8.3 in UE_5.8, although the project instructions requested5.8.2; no engine changes made. Packaged Server remains unsupported by this installed distribution. Server tests below use uncooked dedicated-server `UnrealEditor-Cmd -server -game` and NullRHI clients, never two rendered clients.

Final native `Automation_M12StatsFinalNative_20261013_071721184_16f26d6a` PASS8/8,34.10s, raw warning/error0, peak3.039working/2.932privateGiB:

- PF.Progression.StatRecords: independent budgets, invalid/overflowing ranks, tuning refusal, V1/V2 defaults and V3/corrupt payload.
- PF.Progression.StatRuntime: real damage/craft/carry, controller/client/foreign/duplicate refusal,120-stamina files/repeated restore, death/respawn and physical10m safe landing/drive completion.
- PF.Progression.EarnedUpgrade: real trace gathering/timed crafts earn the first stat point; stamina rank and independent technology spending survive death/respawn. No XP/item seed in this test.
- PF.Progression.Codec, PF.Progression.GatherWindows, PF.Traversal.Contact, PF.Traversal.AutomaticClimb and PF.Traversal.Rules: existing narrow regressions, physical80/110/400/800cm and refusal gates.

Network create plus actual new-process restart before the final balance/perch refinement:

- `M12StatsLive1_20261013_065221271_b3be7a8f`: PASS4/4 roles/phases.
- `M12StatsLive2_20261013_065520651_02a837f9`: PASS6/6 with75ms/1% outgoing impairment. Role1 allocates all four upgrades plus another stamina rank; Role2 allocates weight only. Exact independent budgets, private records/feedback,140vs100 stamina capacities,40kg thresholds, reconnect and latest disconnect health88 verified.

These boundary fixtures explicitly seed1000XP on disposable server-owned players; they are not a claim of earned level6 pacing. The actual earned first-point route is tested separately above. Saved non-default stamina135/95 may recover normally up to its upgraded maximum after a fresh connection; tests distinguish this from fresh100 defaults. Logout correctly captures changes after a manual save.

Rendered menu `M12StatsUI720_20261013_071937849_f3ceaaf0` PASS1/1,42.49s,3.413working/5.441privateGiB. Actual U input, disabled/no-points state, keyboard/gamepad/button allocations, held-repeat refusal, separated budgets, no free refill/items, balanced Esc/U/Pause input and Pause entry verified. Fresh/unspent/spent/Pause captures inspected. Capture-only replay fixes the scroll screenshot being requested immediately before closing the modal. Final network/current-source, rendered replay, compatibility/Game build/log review are recorded below when complete. No human acceptance or fullM12/M14 completion implied.

Known startup baseline:24 warnings/14 experimental ToolsetDefinition/PythonTestRunner error lines per uncooked network role; rendered baseline also includes editor/HLOD widget warnings. Native tests are clean. Private-file/default-setting guards remain required. Uncapped rendering is requested; this is not a sustained FPS/minimum16GB/1440p test. Hardware reports31.93GiB RAM and RTX4060~8GB, differing from the supplied4060Ti16GB reference.1440p remains explicitly deferred.

### Failures retained

- PFM12PlayerStatsBuild.log: C4458 widget local Slot, missing PlayerState include and float warnings; narrow repair buildPASS8.51s.
- Automation_M12PlayerStatsFirst_20261013_063158339_556c54bd and Automation_M12StatsLocation_20261013_063403073_1d5eba9e: synthetic foreign controller incorrectly registered as a second standalone identity. Remove it before solo-save registration; StatRuntime replayPASS17.23s, raw0. Real two-owner network identity remains separately tested.
- PFM12StatsLiveBuild.log: UI fixture used nonexistent input-mapper API; supported GetPrimaryInputDeviceForUser repairPASS7.43s.
- M12StatsLive1_20261013_064528371_a97363ee: Create tests passed but wrapper omitted stats from its existing non-storage contract branch; no restart pass and private comparison uncomputed. Narrow runner repair.
- M12StatsLive1_20261013_064852406_cd704711: restart expected manual-save health91, but correct logout capture restored latest88. Updated expected disconnect state; full fresh one/two-client replays passed.
- Automation_M12StatsTenMetre_20261013_065854358_3244750e: replacement not settled/local for physical approach; repaired fixture. Settled/diagnostic runs070239934_b652a431 and070452631_82a01b2b exposed the real edge-perch bug. Approved-target swept completion repaired it; Automation_M14TallPerchReplay_20261013_070940406_cb600453 and final native suite passed, final10m atX107.888/Z1098.050/drive0/cost102.8.

## User check after technical handoff

Use the normal open-world standalone world; see current G10/G14 in TRELLO_TEST_GUIDES.md. First check grounded F on the4m camp wall with full stamina. A separate low-burden upgrade check is U: observe available stat points, allocate one earned point, confirm that stat's value increases while technology points do not decrease, close and move normally. If at level1, gather distinct resources and finish new recipes until100XP; do not expect a free point. Save/load only if you want to retain allocations. No private saves, reconnect credentials or map/assets need uploading or editor saving.

## Current-source final proof

Current stat create/new-server-restart: M12StatsLive1_20261013_074828350_dd830c31 PASS4/4 and M12StatsLive2_20261013_075341768_201061b2 PASS6/6 (75ms/1%). All tests report zero warnings/errors; raw startup remains24/14 per role and private/default guards pass. These include the final shared Game-config tuning and validated landing-target source.

Final rendered U/Pause/allocations plus actual saved10m F: M12StatsUI720_20261013_073212143_3c179b6c PASS1/1,46.54s,3.412working/5.457privateGiB. Six actual PNGs, including readable stats budgets, scroll, Pause entry and first-person10m top/cost102.8/max140, inspected. Rank2 capacity was earned-boundary seeded in the disposable UI fixture, then spent through actual controls; recovery to140 was natural before the single F request. No free stamina from allocating points.

Current climb network: M14HeightAuthoredClimb1_20261013_072158028_c8824f54 PASS2/2,57.22s, and M14HeightAuthoredClimb2_20261013_073755369_90197f8c PASS3/3,69.77s/75ms1%. Actual80cm/4m W+Space landings, exact costs, owner completion and private/default guards pass with the tall-perch completion repair. Final native8/8 above includes10m full landing, drive cleared. Compatibility6, final Game build and all-role raw severity review follow; no human acceptance inferred.

Final handoff checks: compatibility Automation_M12StatsCompatibility_20261013_080101321_6028789a PASS6/6 (Component,WorldCompatibility,WorldRecords,WorldRuntime,KnowledgeRequests,GatherLifecycle),23.85s/raw0/2.926working2.826privateGiB. Final Game build PFM12M14FinalGameBuild.log PASS71.42s/no compiler warnings; final Editor PFM12StatsAuthoredTenBuild.log PASS12.26s. Current native14, stat-network10, climb-network5 and integrated rendered-menu/saved10m1 =30 successful final execution records. All unique selectors verified; no missing/unexpected test, timeout, fatal or ensure. Normalized final all-role severity comparison adds zero lines to the existing baseline. Ordinary PrimalFrontier.log inspected without new fatal/ensure. Protected personal files/settings unchanged in successful isolated runs; no unrelated assets/config or generated directories changed/committed. Previous failed fixture/build/perch runs retained above. Technical delivery complete; human G10/G14 and fullM12/M14 acceptance remain open,1440p/physical controller/audio unchanged.
