# M12 — craftable greybox creature-hit protection

October 9, 2026. Bounded independent M12 implementation verified; full M11/M12 and Personal M8/UI/controller acceptance remain open. Trello: https://trello.com/c/EN7CXRq3. Editor/Game, native, rendered and one-/two-client restart gates passed within the limits below.

## Play the feature

Level: `/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld`. One-player Standalone first; use P for Pause in PIE because Esc can stop PIE. Gather ordinary physical fibre and wood nodes with E. No starting gear is granted.

Gather 12 fibre and 2 wood. C → Material → Twist fibre cord uses 4 fibre in 4 seconds. C → Protection → deliberately select Weave protective guard → Enter/Craft selected uses the remaining 8 fibre, 1 cord and 2 wood in 6 seconds. One `Item_WovenGuard` weighs 1 kg, occupies one stack, does not expire and is not consumable. R/Cancel aborts without consuming inputs. Completion revalidates exact fresh inputs and capacity; repeated requests cannot duplicate output.

The best valid fresh guard in your carried bag automatically reduces creature-caused hits by 25%. A 20-damage creature hit removes 15 health. Carrying a second guard does not increase protection. Dropping/removing it removes the benefit immediately; PlayerState inventory already survives pawn death/respawn. This is carried greybox equipment, without equip slots, worn armor meshes, durability or repair. Starvation, dehydration, exposure and generic damage remain unchanged. Protection grants no gathering or melee bonus. Numbers are provisional, not human combat-balance acceptance.

## Authority, data and compatibility

Native Item.Category.Protection and Recipe.Category.Protection tags extend validated loaded catalogs without changing binary assets. Designer-facing CreatureHitReduction is finite, bounded 0–0.5, restricted to stack-1 Protection definitions. Inventory derives the strongest fresh valid value rather than persisting a cached bonus. Server survivor TakeDamage applies it only when the actual damage causer is APFCreature; existing authority/life/finite damage guards remain. No client damage amount, item grant or save authority is added. Health replicates publicly; whole inventories stay owner-only. Original menu pictures draw woven geometry without texture imports or extra world rendering.

Existing V1 save data stores stable item IDs; no schema extension or migration is needed for this item. Restoring an owned guard derives mitigation once, without a saved modifier field. All existing tools, weapons, recipes, finite yields and private saves remain intact. This is not the proposed XP/knowledge ledger.

## Verified evidence

- First Editor build FAILED18.96s/exit6 C3535: native fixture inferred auto* from TObjectPtr. Test-only `.Get()` correction; retry PASSED5.59s without compiler warnings. Logs: `Saved/Logs/PFM12ProtectionEditorBuild.log`, `PFM12ProtectionEditorRetry.log`. Failure retained.
- Native7/7: `Automation_M12Protection_20261009_102744385_9c0acc59`,16.35s,working/private2.910/2.772GiB,raw/test severity0 and exits0. PF.Crafting.Protection,ToolProgression,Transactions,WeaponProgression,PF.Persistence.WorldRuntime,PF.Survival.Lifecycle,PF.UI.InventoryDetails. Real loaded recipe cost/cancel/capacity, creature20→15 vs generic20, combined needs/exposure10 unchanged, no stacking, removal, death/respawn, client direct refusal, invalid data and V1 encode/decode/restore checked. Report:`Saved/AutomationReports/<run>/index.json`; log:`Saved/Logs/PF<run>.log`.
- Rendered ProtectionLive1/1 each720p/1440p,maxHUD1.5: `M12Protection720_20261009_102851546_fe05d71f` and `M12Protection1440_20261009_103335442_3f0bea5f`.45.78/45.72s;working/private3.092/4.887 and3.207/5.123GiB. All test errors/warnings and engine/report/runner exits0. Six PNGs inspected: selected recipe/cost/25% limit, actual completion and bag detail/Health85 fit. Reports:`Saved/AutomationReports/<run>/index.json` and `run-summary.json`; logs:`Saved/Logs/<run>.log`; PNGs:`Saved/AutomationReports/ControlsUI/<run>/{guard_selected,guard_complete,guard_inventory}.png`. Default settings/200–201 unrelated saves unchanged.
- One-client Create/Restart4/4: `M12ProtectionLive1_20261009_103443674_dc2be119`,working/private1.717–1.814/1.570–1.732GiB per process. All test severity/exits0. Real server-owned timed craft, actual server creature-caused damage, save/startup restoration/original player record, owned bag/Health85/client identity/save and direct-creation refusals passed. Reports:`Saved/AutomationReports/<run>{Create,Restart}{Server,Client1}/index.json`; summary:`Saved/AutomationReports/<run>/run-summary.json`; logs:`Saved/Logs/PF<run><phase><role>.log`.

Live tests spawn a real APFCreature as damage causer and apply damage through the real authoritative pawn path; they do not certify AI attack windup, navigation, physical controller feel or manual combat. Actual Slate category/recipe keys are exercised; bag opening executes the registered Tab delegate, rather than a physical keyboard press. Only disposable fixtures grant ingredients, freeze needs drain and reposition players; live craft durations stay real. Native test uses an isolated duplicate catalog for shorter durations.

- Two-client Create/Restart6/6: `M12ProtectionLive2_20261009_103613412_3d04833a`,75ms outgoing lag/1%loss confirmed,working/private1.719–1.822/1.577–1.750GiB per process. Test severity and all engine/report/runner exits0; Client2 no crash. Independent original player records, actual server-damaged Health85 replicated to other clients, owner-only guard inventory, actual restart/credential restoration, conservation and save refusal passed. Same report/summary/log pattern as one-client above, including Client2.
- Development Game build PASSED25.26s/no compiler warnings: `Saved/Logs/PFM12ProtectionGameBuild.log`.
- All12 rendered/network raw logs reviewed:26rendered/24network warnings per process (21EditorDataStorageUI widget factories,3missing HLOD utility class/import warnings; rendering additionally2blur/DOF-priority warnings keeping0). Each has14Error lines from existing experimental StateTreeToolset/ToolsetRegistry Python startup tracebacks, missing ToolsetDefinition/PythonTestRunner. Network severity compared with the preceding weapon baseline after removing timestamps: no new line. No other warning/error, ensure, fatal or crash. Native log clean. Ordinary `Saved/Logs/PrimalFrontier.log` is an older launch; isolated logs above are the current evidence. Raw logs are not claimed clean.

## Reproduce safely

Close other Unreal sessions before guarded runners. Use disposable automation profiles/slots only; no manual private save or config reuse. Each runner checks exact test/report/real process exit and cleans up only its owned handles. One client must pass before two.

```powershell
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunNativeAutomation.ps1 -TestFilter 'PF.Crafting.Protection+PF.Crafting.ToolProgression+PF.Crafting.Transactions+PF.Crafting.WeaponProgression+PF.Persistence.WorldRuntime+PF.Survival.Lifecycle+PF.UI.InventoryDetails' -Label M12Protection
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunControlsAutomation.ps1 -TestCase Protection -Resolution 720 -HUDScale 1.5
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunControlsAutomation.ps1 -TestCase Protection -Resolution 1440 -HUDScale 1.5
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunPersistenceAutomation.ps1 -Players 1 -Protection
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunPersistenceAutomation.ps1 -Players 2 -Protection -SimulateLagLoss
```

Rendered runs are one process, uncapped/VSync0. Network tests use NullRHI. Stationary scripted screenshots do not measure sustained FPS or stuttering. Host31.93GiB RAM, not minimum16GB; installed engine reports5.8.3 rather than requested5.8.2. Packaged Server target remains unsupported by this installed engine distribution.

Changed source: item/recipe catalogs, inventory reduction query, server survivor damage boundary, crafting stats/inventory details/procedural picture, new native/live protection fixtures, existing tool/weapon live recipe-count expectation7→8, guarded runners and evidence/play guides. No .uasset/.umap/settings/import/plugin changes. Bounded Trello evidence/status is updated immediately after these gates; full milestone and Personal gates stay open. Remaining M12 includes original XP/knowledge with deliberate save compatibility, resource/creature balance and optional repair; inspect live boards before the next task.
