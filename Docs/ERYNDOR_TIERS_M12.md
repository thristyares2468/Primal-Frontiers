# Eryndor minerals and technology — M12

October15 offline expansion. Bounded task: https://trello.com/c/zg9mp2Tl. This implements home-world progression through Titanium, not the entire offline roadmap, dimensional travel, powered Tech or a release-ready technology tree.

Publication: local scoped commit `a6a8b0d`,40files, M12 title/Sol coauthor, clean tree after commit. Normal push rejected by GitHub LFS budget;0/11objects uploaded and remote master remains `9c29ea8`. Implementation/Trello technical Done does not mean published. No force push/history rewrite or missing-asset bypass. Owner capacity/budget action required for publishing, not for future local work.

## Content and controls

Current catalogs have36 items,21 recipes,15 resource definitions and6 knowledge nodes. Existing baseline definitions are unchanged. Added19 items/10 recipes/9 minerals:

| Mineral | Minimum compatible pick | Exact fixture position (cm) | Current use |
| --- | --- | --- | --- |
| Coal | Stone | -2600,-1200,60 | Iron/gold/silver/titanium refining fuel |
| Flint | Stone | -2600,-800,60 | Iron refinement and quartz lens |
| Iron ore | Stone | -2600,-400,60 | Iron ingots → Iron pick |
| Diamond | Iron | -2600,0,60 | Diamond pick |
| Gold ore | Iron | -2600,400,60 | Gold ingots → conductive parts |
| Silver ore | Iron | -2600,800,60 | Silver ingots → conductive parts |
| Quartz | Copper | -2600,1200,60 | Quartz lens |
| Obsidian | Iron | -2600,1600,60 |70-damage blade and titanium refinement |
| Titanium ore | Diamond | -2600,2000,60 | Titanium ingots → Titanium pick |

Each mineral has8 hits,2 units/hit and45-second regrowth after depletion. Iron/Diamond/Titanium picks give5/6/7 compatible hits/action. Higher grade does not override tool family; removal is checked on every action. Refusal gives no yield, depletion, XP or cooldown mutation. Existing Wood/Stone/Copper behavior remains supported. New minerals share the existing Stone reward window (5 successful credited actions/30 active minutes); they do not each add an XP budget. Old Copper reward behavior is unchanged.

`C` opens crafting. Choose **Technology**, select a branch, then click Learn or press `K` when eligible. The list shows level/cost/learned status/prerequisite; details list every recipe unlocked. Switch to a crafting category to manufacture. Enter in Technology cannot accidentally craft its representative recipe. Learning spends technology points and gives no items/XP; inventory stat upgrades retain their independent budget. Keyboard/controller category and selection navigation remain available.

| Knowledge | Level | Cost | Parent | Recipes |
| --- | --- | --- | --- | --- |
| Field tool knowledge |2|2|None|Bound stone tool, Copper pick|
| Field weapon knowledge |3|3|Field tools|Stone-bound club|
| Iron metalworking |3|2|Field tools|Iron refinement/pick, gold/silver refinement|
| Gem cutting |3|2|Metalworking|Diamond pick, quartz lens, obsidian blade|
| Titanium metallurgy |4|3|Gem cutting|Titanium refinement/pick|
| Conductive fabrication |5|2|Metalworking|Conductive parts|

Main-path prerequisite total9 points is affordable at level4; all utility branches plus FieldTools cost11 at level5, excluding optional FieldWeapons. A pure transaction test proves465XP from25 bounded successful gather credits and17 unique baseline/metal/gem craft credits reaches Titanium knowledge without a reward-window reset. This is an economy-contract proof, not an ordinary-input route/pacing playtest.

Recipes use acquired materials; no normal game grants. Iron ingot:2ore+1coal+1flint/10s. Iron pick:3ingots+1Copper pick+1cord/12s. Diamond pick:3diamonds+1Iron pick+1cord/14s. Titanium ingot:2ore+2coal+1obsidian/12s. Titanium pick:3ingots+1Diamond pick+1cord/16s. Upgrade consumes the old pick exactly once; cancellation preserves inputs. Refining is explicitly **portable greybox**, not a physical station claim. Conductive parts and quartz lenses are craftable intermediates; their advanced equipment consumers are not implemented yet.

All visuals reuse engine primitives/existing greybox material and tinted original category sprites. Mineral/tool/ingot names and capabilities remain exact. No new external art, Fab model integration, online feature, settings or third-person work.

## Playable fixture / optional human observation

Level: **`/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld`**. One rendered **Standalone, Single player** world, not the Entry menu or L_Automation. Existing saves require the reviewed migration below. Preserve your world before experimenting; a fresh temporary named world avoids altering personal progress.

**Start here:** at camp, find **ERYNDOR MINING TEST** about24m in the negative-X direction. Sign at `(-2400,-1450,180)`; nine nonblocking primitive nodes run4m apart from `(-2600,-1200,60)` to `(-2600,2000,60)`. No old actor/resource/spawner/start was moved. Do not confuse this row with the Copper vein/sign beside camp.

1. Approach a node within2.5m, look directly at it and press `E`. With a Copper pick, Diamond must say **Requires Iron pick**, keep all8hits and give nothing. Bare hands cannot harvest any new mineral. No tool/XP grants are part of the ordinary guide.
2. In `C` → Technology, inspect Iron metalworking. It lists level3,2points and Field tools prerequisite. Learn using `K` only when earned eligibility is satisfied; otherwise explicit locked feedback is expected, not failure. `P` opens Pause while reading in solo.
3. Gather iron/coal/flint using Stone or Copper. Refine3iron ingots, retain a Copper pick and1cord, then craft Iron pick. Expected: old pick removed, exactly1Iron pick. Cancel one refinement first if testing cancellation; ingredients must remain unchanged.
4. Mine Diamond with Iron, learn Gem cutting, upgrade using3diamonds+Iron pick+cord. Diamond pick must gather Titanium; Copper/Iron must refuse it. Learn Titanium metallurgy and refine3ingots using6ore+6coal+3obsidian; combine with Diamond pick+cord to get one Titanium pick. Use a chest/drop spare materials if the real8-slot bag fills. No capacity increase was introduced.
5. Optional utility branch: gold/silver refine2ore+coal each; conductive parts use1gold ingot+1silver ingot+1copper ingot. Lens uses3quartz+flint. These are intermediates with future advanced uses, not powered equipment.
6. Save with Pause Save or `PF.SaveWorld <your-slot>`. Require explicit success. Reload from main menu and compare tool/knowledge/material counts and chest filters. A failed save is a FAIL; do not quit assuming it exists.

Optional report: `ERYNDOR_TIERS PASS`, `ERYNDOR_TIERS FAIL — <first issue>`, or `NOT TESTED`. No new immediate Personal action is required; existing human Copper/pacing/device observations remain unverified. Expected memory stutter alone is not a gameplay failure.1440p and rendered multiplayer remain separately deferred.

## Compatibility

Original15 or Copper16 resource layouts may expand to the exact reviewed25 identities. New names are `PF_Eryndor_<Coal|Flint|Iron|Diamond|Gold|Silver|Quartz|Obsidian|Titanium>` with matching `Node_` definitions and trusted `PF.M12.EryndorTest` actor tag. Partial/renamed/unknown layouts refuse; no record is silently dropped or remapped.

Before applying, preserve the original envelope under `Saved/Persistence/Originals/<slot>/M12EryndorLayout1.pfs`, retaining original source generation/UTC until ordinary Save. Old15 includes a default Copper record as well. Current25 loads exactly, without remigration or resource reset. Read-only `PF.CheckSave` reports `RequiresEryndorMigration` and `RequiresCopperMigration` separately, without writing/applying. Original corruption refuses before live mutation. World/player schema and bag/storage limits are unchanged.

## Verification evidence

- Editor initial26.48s/integrated35.11s/UI18.79s; final production/economy5.30s, test updates6.21s and migration-runner12.86s passed without compiler warnings. Final `M12EryndorGameBuild.log` Development Game PASS43.27s/27actions, zero compiler warnings. No packaged Server/cooked certification.
- Native final `Automation_EryndorFullState_20261015_042027405_91df22f7`:4/4 Catalog, Runtime, Layout, HarvestMatrix. Includes actual timed upgrades, new tool/knowledge/vitals/chest contents/filter/ownership repeated Save/Load, exact15/16→25 manual/startup migrations and corrupt-original refusal.17.53s;0raw/test severity/fatal/ensure;2.922GiB working/2.771private.
- Earlier final `Automation_EryndorFinal_20261015_041218370_4285381d`:7/7 including Harvest Definitions/Selection/AtomicGather and Progression Records. `Automation_EryndorVerified_20261015_041807679_d6c04f2d`:6/6 including CopperLayout/CopperBackupFailure. `Automation_EryndorLegacy_20261015_041848294_8c111075`:4/4 frozen writer records/restore, original earned Copper route and progression. All clean; preserve earlier narrower evidence rather than claim unrun selector coverage.
- One intermediate `Automation_EryndorMigrate_20261015_041622657_2276d0ae` was **runner FAIL** because the requested `PF.Persistence.FrozenM8` is not an exact/dotted test selector. Unreal ran8clean successes, but the strict runner correctly rejected that mismatch. Correct exact selectors replayed successfully above; no runner relaxation.
- Author first `M12EryndorSetup_20261015_040846688` saved10 additions then failed reload comparison because raw Python GUID/array reprs contain transient details. Stable GUID/string-list comparison replay `M12EryndorRepeat_20261015_040934749/setup.json` PASS, zero further mutations.97saved actors/25nodes; all87original external actor package hashes match the pre-edit backup byte-for-byte. Only the open-world metadata/map and10new external actor packages change. Catalog assets inherit constructor additions; no existing serialized tuning overwritten.
- Rendered `M11Eryndor720_20261015_041302805_771b62e8`:1/1 PASS122.65s,1280×720/HUD1, uncapped requested.3.260GiB working/5.418private;607unrelated save/profile files and default settings unchanged. Five PNGs under `Saved/AutomationReports/ControlsUI/<run>`. Boundary level700XP/material seeds and assisted travel; real Slate Learn/Craft, ordinary craft timers and actual first-person harvest. This is not earned pacing, hardware-controller or sustained performance acceptance.
- Raw first render40severity lines/35unique match the prior successful Copper baseline exactly after timestamp normalization. Known experimental Python/HLOD/editor-widget/scalability startup messages remain; no new gameplay/RHI/ensure/fatal. Ordinary `Saved/Logs/PrimalFrontier.log` is the separate preexisting D3D12 crash; unique new logs are the evidence.
- Final rendered clarity/level-economy replay `M11Eryndor720_20261015_042102463_844bd389`:1/1 PASS120.90s at HUD1.5,3.146working/5.307privateGiB. Five more PNGs; actual Learned/Locked/prerequisite labels and no-learning-XP message inspected.608unrelated save/profile files/default settings unchanged;0new unique severity/fatal/ensure. No1440p repeat or physical-device claim.
- Existing migration runner adapted to remove only current task fixtures from its disposable synthetic old-world runtime; authored map stays intact. Actual one-client Create/Restart `M12CopperLive1_20261015_042443000_f1a6a2d0`:4/4 role reports PASS,610private/default entries unchanged. Two-client `M12CopperLive2_20261015_042627347_90e5a91d`:6/6 role reports PASS with75ms lag/1%loss,614private/default entries unchanged. Restart verifies current25, all9new nodes initialized once, immutable source15, independent owners/private stats, authoritative finite shared gather and direct-client refusal. All10process exits0/test severity0; each raw24warnings/14Python startup error lines matches baseline,0new severity/fatal/ensure. Per-process peaks roughly1.727–1.801working/1.629–1.787privateGiB. NullRHI is not rendered-multiplayer/cloud/packaged-server certification or a new online feature.

All report folders are under project `Saved/AutomationReports/`; unique build/test logs under `Saved/Logs/`. Installed engine5.8.3 CL58210709 differs from requested5.8.2. Current machine31.93GiB RAM/RTX4060 8GiB; no16GiB/FPS certification. FullM12/offline roadmap, physical stations, Frontier materials/travel, Deepforge, charged Riftglass, ancient points, adaptations and advanced content remain incomplete.
