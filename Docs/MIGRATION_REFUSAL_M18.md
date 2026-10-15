# M18 — migration backup refusal verification

October15,2026. Test-only follow-up to the implemented old15→current16 Copper layout edge. No production source, assets, map, schema, rendering, service or personal save changed. The test uses current-generated synthetic prior-layout records, not archived previous-writer proof.

`PF.Persistence.CopperBackupFailure` creates six disposable native worlds: manual Load or real ConfigureStartup/ApplyStartup, each with corrupt original backup, a directory occupying the original destination, or a corrupt A/B sibling. Read-only inspection still reports its explicitly limited file/layout result; it does not certify that an original can be published.

Repeated actual Load refuses the specific backup problem before Apply and retains the active live slot. Assertions preserve exact source/other-live envelope bytes (including generation/UTC), corrupt/occupied evidence, actual actor pointers/count, unsaved inventory, health99/hour9/Copperhits1 and active recipe/finish time. Failed startup blocks login, capture and writes to either the target or previous live slot. Capture output remains unchanged. Expected startup refusals are matched individually, not blanket-suppressed. Cleanup removes only explicitly named unique fixture files/empty directories.

## Reproduce and evidence

No authored level required. Start with Unreal windows closed and run at project root:

```powershell
.\Scripts\RunNativeAutomation.ps1 -TestFilter 'PF.Persistence.CopperBackupFailure+PF.Persistence.CopperLayout+PF.Persistence.OriginalBackup+PF.Persistence.StartupFailurePreservesSave' -Label M18BackupRefusal -TimeoutSeconds 180
```

Expected all four exact selectors Success, zero test warnings/errors, normal exit and unchanged refusal state. A missing selector, unexpected severity, mutation or missing refusal is a failure.

- Editor `Saved/Logs/PFM18BackupRefusalEditorBuild.log` PASS6.21s; Game `PFM18BackupRefusalGameBuild.log` PASS14.00s. No compiler warnings.
- `Saved/AutomationReports/Automation_M18BackupRefusal_20261015_023759073_e0d4c7bf/index.json` and `run-summary.json`: PASS4/4,17.57s, working/private sampled peaks3.033/2.877GiB, exit0, zero raw/test warnings/errors/fatal/ensure, exact selection confirmed.
- Log `Saved/Logs/PFAutomation_M18BackupRefusal_20261015_023759073_e0d4c7bf.log` contains three individually expected startup reasons: corrupt original preserved, destination occupied by directory, corrupt source preserved. Automation records these expected errors at Verbose; production errors remain errors.
- Ordinary `PrimalFrontier.log` still ends in the separate October14 D3D12 GPU crash; this NullRHI run adds no current crash evidence.

This verifies existing guards; no production repair was needed. No network behavior changed, so the prior one4/two6 CheckSave/persistence role evidence remains historical and was not relabeled as a new run. No rendered/manual/FPS, OS crash/power-loss, real disk-full/race or full M18 acceptance is claimed. Frozen genuine older-writer fixtures remain the next independent gap; hosting and unrelated Personal gates stay deferred/unverified.
