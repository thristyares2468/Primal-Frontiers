# M13 original greybox creature definitions and loot - October 13, 2026

Two original data-driven variants use the existing primitive creature and server state machine. No external artwork, new asset package, new spawn point or save schema. Existing Forager/Prowler stats and single-food-drop behavior remain. This bounded definition/loot scope does not by itself activate the new variants in ordinary open-world spawn tables or finish M13.

| Definition | Behavior | Health | Speed cm/s | Sight cm | Damage | Death loot |
| --- | --- | --- | --- | --- | --- | --- |
| Creature_Scrabbler | Passive flee/patrol | 40 | 260 | 900 | 8 | 1 found food + 1 plant fibre |
| Creature_DuskPouncer | Hostile chase/attack | 140 | 240 | 1100 | 12 | 4 found food + 2 plant fibre |

Fibre is placeholder material loot using the existing item definition; no final animal/pelt art or biological realism is claimed. Actual loaded DA_CreatureCatalog contains both new defaults, so no migration or binary asset change was needed. They are available to the existing authoritative `PF.SpawnCreature <ID>` tooling. Client spawning/damage still refuses; global creature cap8 and nav/capsule checks remain.

Supplemental loot is an editable bounded array, at most three unique IDs with quantities1-10. It cannot duplicate legacy food. Before a creature becomes live, its primary/supplemental IDs must resolve in the real item catalog and quantities fit item stack limits. Unknown/duplicate/malformed or nonstackable oversize rewards refuse. Server death emits standard APFItemPickup batches with unique identities; food uses existing expiry, fibre is nonperishable. Repeated damage to a corpse emits nothing. Existing restore avoids death payouts; new creature definition/GUID/dead records validate and round-trip through actual save-file I/O.

## Verification

Final loot-guard Editor build: `Saved/Logs/PFM13VarietyLootGuardEditorRetry.log`, PASS5.12s/no compiler warnings. Production Game build result is recorded after completion below. Earlier successful builds28.70s/11.38s/5.67s/5.51s remain in logs.

| Test | Result | Directory under Saved/AutomationReports |
| --- | --- | --- |
| Final native selection | PASS5/5,17.01s, raw/test severity0 | Automation_M13VarietyLootGuard_20261012_211115740_911313fe |
| Real dedicated server + one NullRHI client | PASS PF.Creatures.VarietyLive2/2,43.51s | M13EcologyVariety1_20261012_205542264_d6e0ad24 |
| Dedicated server + two NullRHI clients,75ms outgoing lag/1%loss confirmed each role | PASS PF.Creatures.VarietyLive3/3,52.27s | M13EcologyVariety2_20261012_210202656_c5f6868b |
| Corrected rendered720p/HUD1.5 | PASS PF.Creatures.VarietyPreviewLive1/1,31.08s; three PNG inspected | M11CreatureVariety720_20261012_210650917_b6140208 |

Exact five native cases: **PF.Creatures.Variety, PF.Creatures.Lifecycle, PF.Creatures.SpawnPolicy, PF.Persistence.CorpseLootRestore, PF.Persistence.WorldRuntime**. Variety loads the real catalog, checks stats, malformed/duplicate/unknown/nonstackable loot, actual spawn/death and exact conserved food/fibre batches/expiry; writes/reads/validates/decodes both new dead-creature identities through a unique real file and cleans only its own files with UE APIs. Baseline corpse/runtime tests remain intact. This is not a new-server variant-persistence certificate; authored baseline restart evidence is separate in AUTHORED_ECOLOGY_M13.md.

Live variant fixture extends the existing M6 scenario without weakening its original selector: nav path around obstacle, physical flee/chase, real AI damage, owned melee RPC death, E loot recovery of4food+2fibre, peer replication and client spawn/damage/tooling refusals. Existing M6 fixed-ID assets remain untouched. One client preceded two.

Rendered fixture uses `/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld`, temporarily clears runtime creatures without loot and pauses points in its isolated unsaved world. It spawns both variants through actual navigation/clearance checks, frames readable portraits, directly applies authoritative test damage to the Pouncer, collects drops through normal E interaction and opens bag through the real Tab delegate. This is a presentation/pickup exercise; normal combat is exercised by dedicated live tests. Three inspected PNGs: `ControlsUI/M11CreatureVariety720_20261012_210650917_b6140208/{scrabbler,dusk_pouncer,variety_loot}.png`. Labels are readable,40/140HP distinct; inventory shows4food/2fibre,1.0kg/30kg and999000 stack limit. Body still uses the same primitives.

The final pure item/stack preflight was added after these rendered/network passes; the final native run exercises actual BeginPlay and those additional refusals. Live behavior and assets were unchanged for valid definitions. No misleading final-source rendered/network claim is inferred. All passing reports require exact selectors, exits0, test warnings/errors0, no timeout/fatal/ensure/assertion and unchanged private-save/default-preference guards.

Native peak3.024GiBworking/2.983private; one-client1.813/1.779, two-client1.822/1.785 per-role maximum, corrected720p3.208/5.068. Private files/preferences unchanged:401one-client,403two-client,405corrected render. Host31.93GiB/installedUE5.8.3; no16GBminimum, sustainedFPS, human stutter/physical-device or packaged Server claim. Rendering uncapped; NullRHI for nonvisual roles. Known startup widget/Python severity remains; raw baseline review is recorded before publication.

## Retained failures

Final publication checkpoint: Game31.03s PASSED/no compiler warnings (PFM13VarietyGameBuild.log). Five network logs match21warning14knownPythonerror baseline with zero new categories; corrected render matches26warning14knownPythonerror baseline with zero new categories. Native raw0. No Unreal process remains. Earlier Game/raw-review pending statements above are superseded. Bounded definition/loot technical task has11 successful final records; ordinary authored spawn distribution remains the next AI scope.

- Rendered fixture build23.89s/exit6 C2248 called private ToggleInventory; replaced only test call with actual Tab binding, rebuilt5.67s clean.
- Initial rendered run M11CreatureVariety720_20261012_210002889_ada3c222 passed backend assertions but screenshot review found mirrored Scrabbler caption and stale corpse visuals because frozen actor tick had not refreshed after camera movement. That visual evidence was rejected. Corrected fixture allows ordinary engine ticks during camera settling, freezes at capture; corrected three PNGs pass inspection.
- Attempted direct refresh hit protected Tick C2248, build4.35s/exit6 (PFM13VarietyFinalEditorBuild.log). Normal engine ticking replaced it;5.51s retry clean. No production API exposed.
- Loot-guard build23.03s/exit6 C2039 used MaxStack; actual field is StackLimit. Read header, corrected field and rebuilt5.12s clean. Final native5 passed. Retain PFM13VarietyLootGuardEditorBuild.log.

## Replay / next

No saved level needed for native temporary worlds. Close Unreal before isolated tests. From repository PowerShell, one step at a time:

```powershell
./Scripts/RunNativeAutomation.ps1 -TestFilter 'PF.Creatures.Variety+PF.Creatures.Lifecycle+PF.Creatures.SpawnPolicy+PF.Persistence.CorpseLootRestore+PF.Persistence.WorldRuntime' -Label M13VarietyLootGuard
./Scripts/RunEcologyAutomation.ps1 -TestCase Variety -Players 1 -Port 17998
./Scripts/RunEcologyAutomation.ps1 -TestCase Variety -Players 2 -SimulateLagLoss -Port 17998
./Scripts/RunControlsAutomation.ps1 -TestCase CreatureVariety -Resolution 720 -HUDScale 1.5
```

Network map `/Game/PrimalFrontier/Maps/L_M6Creatures`; rendered map OpenWorld above. Runners own unique profiles/reports and protect personal state. Inspect summaries/index/unique logs/captures, reject warnings/errors/missing selectors/timeouts/private hash changes. Evidence native `index.json` and `run-summary.json`; network parent summary/per-role indices; render summary/ControlsUI PNGs. Stop/fix failed scope before broadening. Next AI scope is adding these existing IDs to authored biome weights while retaining the same two point identities and old resident saves. Full M13/human ecology and later M14 gates remain open;1440 explicitly deferred, no new art needed.
