# M18 — Safe original-save copy command

Bounded task https://trello.com/c/LMVAG5Xz. October22 scoped build/native/client-authority gates PASS. This is an operator/developer recovery step, not automatic migration, an ordinary player menu or a whole-M18 pass.

`PF.CopyOriginalSave SourceSlot Edge NewSlot` is registered with PrimalAgentTools, discoverable in PF.Help and excluded from Shipping with the plugin. It requires three identifiers, each containing 1–64 ASCII letters, digits or underscores; a distinct destination; and an authoritative gameplay world with GameMode. Network clients, no-world/editor-only contexts and off-thread invocation refuse; there is no RPC/remote forwarding. No dependencies added.

The command wraps the separately tested `FPFSaveFileStore::CopyOriginalToNewSlot` API: validate a bounded preserved checksummed envelope and copy its exact bytes into an unused destination A generation. Any existing A/B/pending file/directory/original evidence refuses. Source active A/B and original remain unchanged; generation and SavedUtc are preserved, so food age is never refreshed. Active world/time/slot do not change. Missing/corrupt/future/oversized originals refuse. Partial write failures retain evidence. The command does not validate world payload semantics, inspect/list private originals, load a world or advertise that a recovered save can be restored. No cross-process whole-slot lock/OS-crash/disk-full guarantee.

## Operator guide — only when a recovery is actually needed

Automated backend fixture requires **no level**. No personal recovery is required to verify this feature. Optional use of the command requires an existing preserved original, not an invented example file. Do not repeat earlier accepted normal Save/Load tests or delete saves to manufacture a failure.

Prerequisites: preserve your world progress, keep an independent backup of `C:\UnrealProjects\PrimalFrontier\Saved\Persistence`, close other Unreal/game processes using that same project save directory. This command operates on the running server machine's local files only; it does not fetch Cloudflare data. Use a Development standalone server/host on the original save's actual map. Existing normal test map: `/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld`; `/Engine/Maps/Entry` is only the menu. Do not load a copied world with missing/changed catalog/layout/owner identities.

**Start here:** in Windows File Explorer open `C:\UnrealProjects\PrimalFrontier\Saved\Persistence\Originals`.

1. Find the source world's folder. Its folder name is **SourceSlot**. The `.pfs` filename inside, without extension, is **Edge**. Both must satisfy the identifier grammar. If the folder/file is absent, stop: the command cannot reconstruct an original that was never preserved. Do not rename, edit or move those files.
2. Choose a new unused label such as `Recovery_20261022_A`. Confirm neither `Recovery_20261022_A.a.pfs`, `.b.pfs`, their `.pending` entries nor an Originals folder of that name exists. Keep all existing entries. If any exist, choose a different label; do not delete evidence for a retry.
3. Open the appropriate map as one **Standalone Game** host, or use the dedicated server's local console. Use ~ for its console. A remote multiplayer client's console will be refused. Do not use an ordinary editor-only world console or issue a save/load while this recovery is running. No walking, fixture placement or rendering is necessary.
4. Enter `PF.CopyOriginalSave SourceSlot Edge NewSlot`, replacing the three words with the exact names from steps1–2. Example **syntax only**: `PF.CopyOriginalSave ExampleWorld ExampleEdge Recovery_20261022_A`. The example does not assert those originals exist. Read `[PrimalAgentTools]`: require command PASS with **OriginalCopied** and the warning in its information text that semantic validity is not certified. Refusal means STOP and keep all evidence; it is not successful recovery.
5. Your live world should remain exactly as it was; no map reload, teleport, inventory reset or active-slot switch. The destination A file now matches the original bytes; no B generation is created by this command. Repeating the command with that same destination must report **RecoveryCopyRejected** and preserve it.
6. Before any separate load, run `PF.CheckSave NewSlot` using the actual new label on the correct map. This read-only inspection checks catalog/layout and connected membership. If it refuses, STOP: the copy still exists, but loading has not been certified. Do not clear/reset the live world or overwrite your old source. Player roster mismatch may require a correctly prepared server restart; do not force a load into the wrong session.
7. Export command evidence if needed with `PF.ExportTestReport OriginalCopyReview`. Keep the reported JSON path and unique process log. Actual restoration/manual recovery requires its own reviewed plan and save/owner/map compatibility gate; this guide deliberately stops at safe copy and read-only inspection.

Optional pass name **ORIGINAL_COPY_COMMAND** means observed PASS/copy/world-unchanged/second-copy-refused only. It must not be reported as successful full-world restoration, automatic rollback or food refresh.

## Technical reproduction and current evidence

No external services, user credentials, new assets, world fixture or private files are needed. Close existing Unreal processes. From `C:\UnrealProjects\PrimalFrontier`, build PrimalFrontierEditor Win64 Development. Run:

```powershell
./Scripts/RunNativeAutomation.ps1 -TestFilter 'PF.PrimalAgentTools.OriginalCopyCommand+PF.PrimalAgentTools.CommandArguments' -Label M18CopyCommand
./Scripts/RunNativeAutomation.ps1 -TestFilter 'PF.Persistence.OriginalCopy.Exact+PF.Persistence.OriginalCopy.Refusal+PF.Persistence.OriginalBackup' -Label M18CopyRegression
./Scripts/RunPersistenceAutomation.ps1 -Players 1 -Port 18081
./Scripts/RunPersistenceAutomation.ps1 -Players 2 -SimulateLagLoss -Port 18081
```

Native command test uses a synthetic Engine GameModeBase world with uniquely owned files, deliberately non-world payload bytes, generation37 and two-day-old UTC. This proves the command never loads or claims semantic restoration. It invokes both ExecuteCommand and the actual registered console command, checks report history, exact original/source/copy preservation and unchanged active slot/time/GameMode. Argument tests cover every identifier position, path/empty/long/hyphen/case-alias/count refusal and PF.Help discoverability. Existing copy/refusal tests cover byte/time/destination evidence and genuine frozen writer boundaries. The separate ordinary PF.Persistence.Live run includes an explicit **NotAuthority** check for this new command on each real connected client during create and restarted sessions; no recovery source exists or is read on those clients. Normal save/restart/owner/inventory assertions remain intact. Game must build separately; no updated cooked Server package is claimed.

- Initial Editor18.37s/probe rebuild6.53s PASS.
- `Automation_M18CopyCommand_20261022_000933281_ec68b435`: OriginalCopyCommand + CommandArguments **2/2 PASS17.01s**, exits0/raw/test severity0,2.979working/2.833privateGiB. Actual console success recorded; no private world/state read or mutation.
- Final native `Automation_M18CopyCommandFinal_20261022_001511162_4bb69044`: **5/5 PASS17.04s** — OriginalCopyCommand, CommandArguments, OriginalCopy.Exact, OriginalCopy.Refusal and OriginalBackup. Exact selection/exit0/raw and test severity0/no timeout/fatal/ensure,3.011working/2.871privateGiB. The genuine frozen-source envelope is included in OriginalCopy.Exact. No failed gate in this command slice.
- One-client `M8Live1_20261022_001050481_a1273367`: **4/4** real server/client Create+Restart reports cleanSuccess,59.44/38.81s. Client original-copy NotAuthority and existing saved state/ownership/reconnect assertions pass.640 unrelated save/profile files hash unchanged. Per-role1.730–1.819working/1.627–1.773privateGiB.
- Two-client `M8Live2_20261022_001304802_39bc63e1`: **6/6** real server/Client1/Client2 Create+Restart reports cleanSuccess,62.95/41.86s,75ms outgoing lag/1%loss confirmed in every role. Original-copy refusal checked on both clients in both phases; separate-owner storage/privacy/inventory/knowledge/health/reconnect assertions remain intact.644 unrelated files hash unchanged. Per-role1.721–1.824working/1.627–1.791privateGiB. All10role process exits0, no timeouts/fatal/ensure/test warnings/errors; no recovery source read or copied on clients.
- All10raw log sets contain the same33unique normalized warning/error messages as the existing successful uncooked OpenWorld baseline (24warning/14experimental Python startup error lines each); no new gameplay severity or saved-move warning. Main default PrimalFrontier.log remains historical; use these unique abslogs. No rendered FPS/stutter inference from NullRHI.
- Final Game24.87s PASS alongside Editor18.37s/probe6.53s; no compiler warning/error, no new C++ edits after verification. No sourceServer/cook/stage/package update for this slice.

Reports `Saved/AutomationReports/<run>/index.json` and `run-summary.json`, unique logs `Saved/Logs/PF<run>.log`, build/runner transcripts `C:/UnrealSource/BuildLogs/M18CopyCommand_*_20261022.log`. Installed engine5.8.3 despite originally requested5.8.2. This backend-only command needs no screenshot; rendered game/controller/hosted/voice/manual-recovery acceptance remain separate.
