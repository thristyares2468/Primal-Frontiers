# Native automation verdict gate

`Scripts/RunNativeAutomation.ps1` runs bounded, non-rendered Editor automation. Live server/client tests and human playtest gates are separate. Unreal can exit 0 after assertion failures or an unmatched selection; always check the report.

From the project root, run the native persistence tests:

```powershell
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunNativeAutomation.ps1 -TestFilter PF.Persistence -Label M8Persistence
```

Use `PF.Persistence.StartupFailurePreservesSave` for the focused startup regression. A filter can join project test prefixes with `+`; it cannot contain arbitrary console commands. Default timeout is 300 seconds, bounded to 30–1800 seconds. This invocation changes execution policy only for its child process, not saved Windows policy; it does not override Group Policy.

The runner refuses a second Unreal Editor process. Finish an existing session before launching it. It starts its own hidden `UnrealEditor-Cmd.exe` with NullRHI, unattended, no sound, no Live Coding and `-NoSaveConfig`. A timeout terminates only that owned process. It does not build, save assets, mutate rendering settings or close the user's editor. Compile first after C++ changes.

Each launch generates a unique `Saved/AutomationReports/Automation_<label>_<UTC>_<id>` folder with the engine report and `run-summary.json`, plus a matching `Saved/Logs/PFAutomation_<...>.log`. The summary records individual test names/states/counts, exit, timeout, log severity counts and sampled working/private memory. It does not print log events, login credentials or save data. Generated evidence stays outside Git.

Exit meanings:

- **0:** nonempty report, every test a clean Success, counts agree, no failed/unrun/in-process/warning tests, engine exit 0 and no logged fatal/ensure.
- **1:** failed, missing, malformed, empty, incomplete or warned report; nonzero engine exit; timeout or fatal/ensure.
- **2:** setup/safety refusal, such as a running Editor or invalid report path. PowerShell parameter-binding/loader errors can themselves return 1 before the runner starts.

Counters must be nonnegative JSON integers; null/missing counters and duplicate test identifiers are refused. `LogReviewRequired` is separate from the verdict: inspect raw warning/error causes even when tests pass. A missing report is a failure. Report success does not certify rendering, manual traversal, overnight survival, controller use or multiplayer restart.

Read a retained report without launching Unreal or modifying it. Supply its **recorded** process exit; do not assume 0:

```powershell
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunNativeAutomation.ps1 -ExistingReport C:/UnrealProjects/PrimalFrontier/Saved/AutomationReports/M8StartupFailureVerified_1791432964361/index.json -RecordedProcessExitCode 0
```

Inspection is restricted to project-local `Saved/AutomationReports/.../index.json`. It checks report/process outcomes only; it does not retroactively review raw logs or certify which binary produced an old report.

## October 8 verification

- Retained `M8StartupFailureReproduced_1791432857773`: engine exit 0, one failed test/12 assertions; runner exit **1**. Retained `M8StartupFailureVerified_1791432964361`: one clean pass; runner exit **0**.
- Fresh runner launch `Automation_M8ReportGate_20261008_042918633_adb3d3f7` passed `PF.Persistence.StartupFailurePreservesSave`: **1 passed, 0 failed, 0 test warnings**, engine/runner exits 0. Raw log severity counts zero. Sampled working/private **2.999/2.856 GiB**, 18.16 seconds.
- Actual unmatched selection `PF.Verification.NoSuchTest`, run `Automation_M8EmptyReportGate_20261008_043016202_b207d390`: engine exit 0, **no index.json**, runner exit **1**. Log reports “No automation tests matched”; 1 error/0 warnings/0 fatal/ensure. Sampled working/private **2.964/2.886 GiB**, 16.27 seconds. This intentional diagnostic failure is retained; it is not a gameplay regression.
- Missing report and unsafe filter rejected; outside-report inspection refused. PowerShell parsing and nonzero-exit, null-count and duplicate-ID rejection checks passed. Temporary malformed-reader fixtures were isolated outside the project and removed; engine evidence was not edited.

Installed engine: **5.8.3**, originally requested 5.8.2. No C++ changed in this tooling increment; earlier successful M8 Editor/Game builds remain applicable. M7/M8 manual acceptance remains open. See [PERSISTENCE_M8.md](PERSISTENCE_M8.md) and [PLAYTEST.md](PLAYTEST.md).
