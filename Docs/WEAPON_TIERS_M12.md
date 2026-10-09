# M12 — craftable greybox melee weapons

October9,2026. Bounded independent M12 implementation verified; full M11/M12 and Personal M8/UI/controller acceptance remain open. Trello: https://trello.com/c/MnrDEHgZ. Editor/Game, native, rendered and one-/two-client restart checks below passed; this is not full milestone or human acceptance.

## Play the feature

Level: `/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld`. One-player Standalone first; ordinary PIE uses P for Pause because Esc can stop PIE. Start with an empty bag, gather with E from physical wood/stone/fibre nodes; normal players receive no free weapons. Use C, Material, Twist fibre cord:4fibre→1cord/4seconds.

| Weapon | Recipe / duration | Effect while carried |
| --- | --- | --- |
| Wooden club (`Item_Club`) | Shape wooden club:3wood+1cord /6s |40creature melee damage,1.5kg,stack1 |
| Stone-bound club (`Item_BoundClub`) | Bind stone club:1wooden club+1cord+2stone+2wood /8s |60creature melee damage,2.5kg,stack1 |

Total from nothing for the bound club:5wood+2stone+8fibre, two separate cord crafts, then both club crafts. No station/unlock is required yet. Choose Weapon in C, explicitly select the row, Enter/Craft selected. R/Cancel aborts without input consumption/output. Exact fresh input batches and capacity are validated again at completion; no duplicated output on extra ticks or double-start.

Close C/Tab/B before aiming at a living creature and left-clicking. Server uses the existing2.5m visibility trace, living possessed pawn,0.5s cooldown and5stamina. No client-supplied target/damage, extended range or PvP is added. Best validated fresh carried melee item is automatic: a45damage bound gathering tool supersedes a40damage club;60damage bound club supersedes that tool. Gathering independently uses the best tool, not the weapon. Dropping/removing/expiring an item removes its benefit; no equip slots, stacking bonus, weapon durability or repair exists. Values are provisional greybox tuning, not a measured balance pass.

## Authority, presentation and save contract

Native Item.Category.Weapon and Recipe.Category.Weapon defaults extend actual loaded catalogs without editing binary assets. Weapons require stack1,finite damage>20 and<=100, and zero GatheringHits; unrelated categories cannot declare melee power. Existing tool2/3gather hits35/45damage and finite yields remain.

Inventory derives the best melee item freshly on each query; server character publishes only held stable item ID, keeping whole bags owner-only. Existing engine cube components change to a thick club shape locally/remotely. Both club tiers intentionally share that greybox held shape; original texture-free menu pictures distinguish their binding/stone cap. No external assets, imports, dependencies or new render-heavy components. Designer-facing getters are read-only. V1 save records already store stable item IDs; no schema/field migration or retrospective award is needed.

## Verification checkpoint

- First Editor build FAILED19.01s/exit6 C2027 because the native test omitted PFItemCatalog.h. Required include fixed; retry5.58s and final17.81s passed without compiler warnings. Logs:`Saved/Logs/PFM12WeaponEditorBuild.log`, `PFM12WeaponEditorRetry.log`, `PFM12WeaponFinalEditorBuild.log`. Failure retained.
- Native6/6: `Saved/AutomationReports/Automation_M12Weapon_20261009_100220696_ba69f620/index.json`; matching `Saved/Logs/PFAutomation_M12Weapon_20261009_100220696_ba69f620.log`. PF.Crafting.WeaponProgression,ToolProgression,Transactions,PF.Creatures.Lifecycle,PF.Persistence.WorldRuntime,PF.UI.RecipeDetails.16.63s,working/private2.988/2.865GiB,all test/raw severity0/exits0. Real traced club40 then bound60 death,5stamina/cooldown, loaded catalogs, cancel/capacity/exact costs, independent gathering, carried fallback/removal, invalid data/client direct refusal and V1 encode/decode/restore checked.
- Rendered PF.Crafting.WeaponProgressionLive1/1 each720p/1440p,maxHUD1.5: `M12Weapon720_20261009_100342657_e0766342` and `M12Weapon1440_20261009_100508041_3c1da9c0`.54.27/54.14s,working/private3.066/4.858 and3.239/5.413GiB. Reports:`Saved/AutomationReports/<run>/index.json` and `run-summary.json`; logs:`Saved/Logs/<run>.log`; six inspected PNGs:`Saved/AutomationReports/ControlsUI/<run>/{club_selected,bound_club_selected,bound_club_complete}.png`. Real Slate category/recipe input, original pictures/40/60stats, menu bounds, actual6/8second completion/duplicate/cancel, Close/Pause, owned held identity and exact costs passed. Settings/188–189 unrelated saves unchanged. All test severity/engine/report/runner0. Screenshot review verifies menu presentation; no actual mouse/physical-controller/combat-feel claim.

Only disposable fixtures grant ingredients, disable needs drain, shorten native craft durations or reposition players. Live durations remain real catalog values. Rendering uses one process, uncapped/VSync0; headless network runs are separate. Those stationary script runs do not measure sustained FPS or human stuttering. Installed engine reports5.8.3 rather than requested5.8.2; host31.93GiB RAM rather than minimum16GB. Packaged Server target remains unsupported by installed distribution.

### Final network/build/log gate

- One-client Create/Restart4/4:`M12WeaponLive1_20261009_100635127_a96d6222`; working/private1.718–1.810/1.574–1.739GiB per process.
- Two-client Create/Restart6/6:`M12WeaponLive2_20261009_100858033_9be4f161`,session75ms outgoing lag/1%loss confirmed on all processes. Working/private1.719–1.813/1.568–1.757GiB. Client2 no crash. Reports:`Saved/AutomationReports/<run>{Create,Restart}{Server,Client1,Client2}/index.json` (Client2 only for two-client run); summaries:`Saved/AutomationReports/<run>/run-summary.json`; matching logs:`Saved/Logs/PF<run><phase><role>.log`. Every engine/report/runner exit0 and test warning/error0. Real owned RPC timed crafting/cancel/duplicate/refusal, independent private bags/public held IDs, server save and startup restoration, client credential/Restored acknowledgment, original player-record count and client save refusal passed. Native actual attack verified separately; headless crafting/restart fixture does not execute combat swings.
- Game Development build24.11s clean:`Saved/Logs/PFM12WeaponGameBuild.log`.
- All12 rendered/network raw logs reviewed:26rendered/24network warnings per process (21EditorDataStorageUI widget factories,3missing HLOD utility class/import warnings; rendered additionally2blur/DOF setting-priority warnings). Each has14Error lines from existing experimental StateTreeToolset/ToolsetRegistry Python startup tracebacks (`ToolsetDefinition`/`PythonTestRunner` missing). These are retained baseline engine/plugin/map startup findings, not a clean raw log. No other error/warning, ensure/fatal/crash/new gameplay error found. Native log clean. Ordinary `Saved/Logs/PrimalFrontier.log` remains an older launch; the isolated tests use the matching unique logs above.

Trello bounded task updated/read back Done immediately after these gates. Existing Personal G2 descriptions extended with tested weapon controls/costs; no added human assignment or invented acceptance. Next independent development must re-inspect live boards; full M12 remains Doing.

## Reproduce safely

Close Editor before guarded launch. Scripts refuse existing Unreal processes, reuse or unsafe slots/profiles, occupied ports and incomplete/failed reports; only owned process handles are cleaned up. Reports/saves are generated locally, never committed. Do not use private manual saves for automation.

```powershell
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunNativeAutomation.ps1 -TestFilter 'PF.Crafting.WeaponProgression+PF.Crafting.ToolProgression+PF.Crafting.Transactions+PF.Creatures.Lifecycle+PF.Persistence.WorldRuntime+PF.UI.RecipeDetails' -Label M12Weapon
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunControlsAutomation.ps1 -TestCase WeaponProgression -Resolution 720 -HUDScale 1.5
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunControlsAutomation.ps1 -TestCase WeaponProgression -Resolution 1440 -HUDScale 1.5
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunPersistenceAutomation.ps1 -Players 1 -WeaponProgression
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunPersistenceAutomation.ps1 -Players 2 -WeaponProgression -SimulateLagLoss
```

The live test is explicitly opt-in, non-Shipping and requires disposable controls evidence or AutomationM12 slot. One client must pass before two. Actual server startup restoration is observed through inventory/public identity and Capture/Validate, not the client-only UI acknowledgment. Client restart additionally requires Restored identity acknowledgment; foreign bags stay private. Optional75mslag/1%loss is session-only. No baseline persistence fixture weakened; runner still requires exact single test, real engine exit, role-specific contract, network settings and no fatal/ensure.

## Changed files and next

Item/recipe catalogs, inventory melee query, survivor public/primitive presentation, crafting stats and procedural pictures; new PFWeaponProgressionTests/PFWeaponProgressionLiveTests; existing tool live recipe-count expectation5→7; both guarded runners and current-state/milestone/architecture/decision/play guides. No .uasset/.umap/config change.

Remaining M12: protection/armor, original XP/knowledge with deliberate save compatibility, resource/creature balance and optional repair. Do not mark full milestone or Personal gates Done from these automated results.
