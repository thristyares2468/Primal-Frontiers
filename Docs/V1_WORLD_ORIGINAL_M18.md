# M18 — Preserve compatible V1 world originals

Task https://trello.com/c/EcbkSSLf. October23 bounded compatibility change; Editor/Game, focused native8/8 and currentV2 one-/two-client10/10 role regression PASS. Full M18 remains incomplete. No mandatory owner engine action is needed for this backend handoff.

## What changes

An unchanged-layout worldV1 save already loads with neutral progression. Later normal Capture/Save produces worldV2, which can eventually rotate both A/B files and erase the V1 bytes. Actual server manual Load and startup restoration now preserve the first verified V1 envelope under:

`Saved/Persistence/Originals/<slot>/WorldProgressionV1.pfs`

This happens after record/catalog/map/layout validation and before world application. Existing Copper/Eryndor reviewed layout archives remain in place; a V1 save needing both changes keeps the appropriate layout archive as well as the progression original. CurrentV2 restoration creates no new V1 original. Read-only `PF.CheckSave`/InspectSave creates none. Normal current Save is unchanged.

Preservation uses the existing bounded/no-replacement file API, retaining the exact file bytes, first generation and original SavedUtc. Repeated V1 restoration and subsequent V2 A/B rotation never replace that first valid original. Neither Load nor preservation rewrites the source. Only an ordinary deliberate Save or existing disconnect save publishes current runtime state. Existing playerCapturedUtc and fileSavedUtc remain the food-aging anchors; preservation does not refresh food, award XP, learn technology or replay recipes. No new schema, arbitrary item renaming, content guessing or source-map change is added.

Corrupt originals, a directory occupying the original path, corrupt source siblings or failure to create/verify a bounded original refuse before runtime application. Manual refusal keeps the previous active slot, live actors, bag, attributes and pending craft. Startup refusal blocks login/capture/save, preventing an empty default world from overwriting either source or the prior live slot. Evidence stays in place; retry does not delete or repair it. Once a valid first original already exists, the API retains it according to the existing first-original policy.

## Reproduce safely — no owner action required now

**Level required:** NONE for automated checks. Native temporary Game worlds reproduce a real player, crafting job, historical resource/spawner layout and disk files. The immutable historical fixture comes from actual `c3d221f` production writer source rebuilt on installed5.8.3, using fake identities; tests do not regenerate it. No personal save or proprietary model is used.

**Prerequisites:** close Unreal instances; current `PrimalFrontierEditor` build. Open PowerShell in `C:/UnrealProjects/PrimalFrontier`.

**Start here:** run the first native command below; return its report/summary rather than loading or corrupting a personal world.

```powershell
./Scripts/RunNativeAutomation.ps1 -TestFilter 'PF.Persistence.V1OriginalRefusal+PF.Persistence.FrozenM8Restore+PF.Persistence.FrozenM8Records' -Label M18V1Original -TimeoutSeconds 180
./Scripts/RunNativeAutomation.ps1 -TestFilter 'PF.Persistence.OriginalBackup+PF.Persistence.CopperLayout+PF.Persistence.CopperBackupFailure+PF.Persistence.StartupFailurePreservesSave+PF.Persistence.SaveInspection' -Label M18V1OriginalRegression -TimeoutSeconds 180
./Scripts/RunPersistenceAutomation.ps1 -Players 1 -Port 18081
./Scripts/RunPersistenceAutomation.ps1 -Players 2 -SimulateLagLoss -Port 18081
```

Run one group at a time. Network regression uses exactly `/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld`, one uncooked dedicated server and one client before two NullRHI clients, isolated Automation slots/profiles. It checks currentV2 save/load/ownership/private progression/client-refusal/reconnect across fresh processes; it is **not** a historicalV1 network-migration test. The native frozen-writer test supplies the actual V1 startup/Load/rotation proof. No rendering is needed for this backend-only preservation hook, and no new invented level fixture or human playtest is requested.

Expected automated checks:

1. FrozenM8Records reads the unchanged historical payload/envelope/hash and exact fake owner/vitals/batches/ownership/time; unsupported/corrupt candidates refuse atomically.
2. FrozenM8Restore performs read-only inspection without an archive, then actual startup, login, repeated manual Load and separate currentV2 save/load. Archive is the exact original envelope, same generation1/UTC. Player33health/61stamina/47food/27water, wood3, storedwood7, structure/support/owner/creature/pickup IDs and zero legacy progression remain correct. Historical expired food stays expired.
3. Three deliberate currentV2 saves rotate both active generations in the original test slot. The first V1 original stays byte-for-byte unchanged and the current slot still restores without duplicate actors/items. Restoring a currentV2-only slot produces no V1 original.
4. V1OriginalRefusal runs corrupt-original, occupied-original-directory and corrupt-source-sibling cases, each for repeated manual Load and startup. Expect explicit refusal, unchanged source/live bytes, actor identities, unsaved99health/clock9/wood4/fibre6/stone1 and active Cord job. Failed startup refuses login/capture/save; refused output stays Sentinel. Tests clean only their uniquely owned explicit files/directories.
5. Existing original-backup/Copper migration/refusal/startup and read-only-inspection cases continue to pass. The generic startup preparation error label now accurately covers both layout and V1-original failure; expected-message assertions retain exact occurrence counts.
6. Network baseline reports must pass every role, original identity/items/storage/progression/RPC refusal contracts and runner file-hash guards. One client completes first. Two clients confirm75ms outgoing lag/1% loss; no second rendered client is launched.

**PASS:** exact expected selectors, zero test warnings/errors, engine/runner0, no timeout/fatal/ensure, required file/conservation assertions all pass. **FAIL:** any missing report/assertion/refusal or changed protected file; stop and retain failed evidence. No genuine OS power-loss, disk-full, cross-process writer, archived original executable/multi-engine, generic semantic mapping, live rollback or ordinary recovery UI acceptance is implied.

## Optional operator inspection

No need to open Unreal or touch an old personal save for this handoff. For a separately backed-up disposable V1 world on its **actual correct map**, use an authoritative Development game/server. Existing normal map is `/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld`; the frozen native layout is deliberately different and must not be loaded into that map.

1. Read-only `PF.CheckSave ActualSlot` first. `ActualSlot` means the real bounded slot ID from its filename/menu metadata, not a string to paste literally. InspectSave reports version and refusal/recovery details; it guarantees no complete restoration.
2. Only if that disposable world is the intended one, use the normal world-select Load/startup flow. A valid V1 restoration now logs `Verified V1 world original retained before restoration; source UTC unchanged`. Verify the explicit archive path exists. Loading changes runtime state, so do not do this merely to test a command on a world you wish to keep.
3. If restoration refuses, preserve all evidence and report the exact refusal; do not delete the archive, toggle to a default world and Save, or rename guessed item/actor records.
4. If a separate recovery is deliberately required later, the already tested operator command is `PF.CopyOriginalSave ActualSlot WorldProgressionV1 UnusedRecoverySlot`. It copies only into a distinct unused slot and never loads/switches live worlds. Follow [ORIGINAL_COPY_COMMAND_M18.md](ORIGINAL_COPY_COMMAND_M18.md), then a separate read-only `PF.CheckSave` on the correct map before considering restore. This is not an automatic migration/rollback or a new mandatory owner task.

## Evidence — October23

- Initial Editor `C:/UnrealSource/BuildLogs/M18V1Original_Editor_20261023.log` failed26.79s/C2059 at PFFrozenM8Tests.cpp:136: extra closing parenthesis in new assertion. Retained original log; test-only syntax repaired. Editor retry `M18V1Original_EditorRetry_20261023.log` PASS6.40s, no compiler warnings/errors.
- Focused `Automation_M18V1Original_20261022_221342169_6f7d38c8`:3/3PASS17.57s, raw/test severity0/exits0/no timeout,3.012working/2.853privateGiB.
- Related `Automation_M18V1OriginalRegression_20261022_221440374_501ff6a2`:5/5PASS17.06s, raw/test severity0/exits0/no timeout,3.029working/2.869privateGiB.674 preexisting persistence files hash unchanged after native tests, including originals; synthetic fixtures clean their own new V1 archive.
- CurrentV2 one-client `M8Live1_20261022_221600878_20e729ea`:4/4Create+Restart role reportsPASS60.87/39.14s, engine/strict0/test severity0/no timeout/fatal/ensure,670 unrelated profiles/saves/default settings unchanged. Per-role1.729–1.820working/1.634–1.785privateGiB. Raw baseline review and two-client evidence follow.
- Two-client `M8Live2_20261022_221805692_82a37f5b`:6/6Create+Restart role reportsPASS62.79/40.68s, confirmed75ms outgoing lag/1%loss. Per-role1.727–1.823working/1.623–1.802privateGiB,674 protected profiles/saves/default settings unchanged. All ten final network role reports/exits/test warnings/errors0/no timeout/fatal/ensure, Client2 no crash. Each raw log33unique normalized severity messages exactly equal successful prior owned-station baseline,24known warnings/14experimental Python startup-error lines; no new gameplay severity.
- Development Game `C:/UnrealSource/BuildLogs/M18V1Original_Game_20261023.log` PASS29.77s, no compiler warnings/errors. All674 preexisting persistence files, including nested originals, still hash unchanged after native and both network groups. Native/network reports `Saved/AutomationReports/<run>/index.json` and `run-summary.json`; unique logs `Saved/Logs/PF<run>.log`. No new screenshot is relevant to this backend hook. UTC report dates20261022 correspond to October23 in Brisbane. Scoped Milestone18/Sol commit and remote-equality receipt follow on Trello.

Installed engine5.8.3CL58210709 versus requested5.8.2; OS31.93GiB RAM. Tests do not certify16GiB/1440p/frame-time behavior. Default PrimalFrontier.log is historical, so unique current logs above are the evidence. No private owner world, credential/provider/database/cloud setting, binary asset or authored map is changed. FullM18 and broader offline roadmap remain incomplete.
