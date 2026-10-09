# Native automation verdict gate

`Scripts/RunNativeAutomation.ps1` runs bounded, non-rendered Editor automation. Live server/client tests and human playtest gates are separate. Unreal can exit 0 after assertion failures or an unmatched selection; always check the report.

From the project root, run the native persistence tests:

```powershell
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunNativeAutomation.ps1 -TestFilter PF.Persistence -Label M8Persistence
```

Use `PF.Persistence.StartupFailurePreservesSave` for the focused startup regression. A filter can join project test prefixes with `+`; it cannot contain arbitrary console commands. Default timeout is 300 seconds, bounded to 30–1800 seconds. This invocation changes execution policy only for its child process, not saved Windows policy; it does not override Group Policy.

Every Run selector must match an exact test or a namespace child with a dot boundary; unmatched selectors and returned tests outside the selection refuse the gate. This prevents a registered subset hiding a misspelled/unregistered term. Read-only Inspect can receive `-ExpectedFilter` for the same check. Without it, older Inspect callers retain count/severity checks with `SelectionChecked=false` and need their own exact test contract. This verifies selector coverage, not all possible registrations beneath a broad prefix. Exact regression evidence and replay: [NATIVE_SELECTION_CONTRACT.md](NATIVE_SELECTION_CONTRACT.md).

The runner refuses a second Unreal Editor process. Finish an existing session before launching it. It starts its own hidden `UnrealEditor-Cmd.exe` with NullRHI, unattended, no sound, no Live Coding and `-NoSaveConfig`. A timeout terminates only that owned process. It does not build, save assets, mutate rendering settings or close the user's editor. Compile first after C++ changes.

Each launch generates a unique `Saved/AutomationReports/Automation_<label>_<UTC>_<id>` folder with the engine report and `run-summary.json`, plus a matching `Saved/Logs/PFAutomation_<...>.log`. The summary records individual test names/states/counts, exit, timeout, log severity counts and sampled working/private memory. It does not print log events, login credentials or save data. Generated evidence stays outside Git.

Exit meanings:

- **0:** nonempty report, every test a clean Success, counts agree, no failed/unrun/in-process/warning tests, engine exit 0 and no logged fatal/ensure; supplied selection fully covered with no unexpected tests.
- **1:** failed, missing, malformed, empty, incomplete or warned report; missing requested selector or test outside supplied selection; nonzero engine exit; timeout or fatal/ensure.
- **2:** setup/safety refusal, such as a running Editor or invalid report path. PowerShell parameter-binding/loader errors can themselves return 1 before the runner starts.

Counters must be nonnegative JSON integers; null/missing counters and duplicate test identifiers are refused. `LogReviewRequired` is separate from the verdict: inspect raw warning/error causes even when tests pass. A missing report is a failure. Report success does not certify rendering, manual traversal, overnight survival, controller use or multiplayer restart.

Read a retained report without launching Unreal or modifying it. Supply its **recorded** process exit; do not assume 0:

```powershell
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunNativeAutomation.ps1 -ExistingReport C:/UnrealProjects/PrimalFrontier/Saved/AutomationReports/M8StartupFailureVerified_1791432964361/index.json -RecordedProcessExitCode 0
```

Inspection is restricted to project-local `Saved/AutomationReports/.../index.json`. It checks report/process outcomes only; it does not retroactively review raw logs or certify which binary produced an old report.

## October 8 verification

Historical checkpoint notes below are superseded for current native coverage by the October9 [35-test checkpoint](NATIVE_CHECKPOINT_M11.md) on source `df648eb`:32+3 sequential batches, all clean Success, exact identities reconciled, raw logs clean. They remain valid historical evidence; no new live/rendered/manual acceptance is implied.

Active-craft addition increases the current native selection to 33. `PF.Persistence.ActiveCraftCancellation` passed separately in Automation_M8CraftFinal_20261008_082409203_d54be484; no combined 33-test or new live checkpoint is implied. Focused usage: `-TestFilter PF.Persistence.ActiveCraftCancellation -Label M8ActiveCraft -TimeoutSeconds 120`. Real files, old completion ticks and actual controller teardown exercise cancellation/conservation; native fixture evidence is separate from multiplayer/manual play. Exact build/report/log/memory: [PERSISTENCE_M8.md](PERSISTENCE_M8.md).

Current native selection contains 32 distinct tests after the focused DeadPlayerRespawn and OfflineFoodAging additions. Each new test passed separately; the earlier combined 30-test checkpoint remains historical, not a rerun of all 32. Focused food verification:

```powershell
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunNativeAutomation.ps1 -TestFilter PF.Persistence.OfflineFoodAging -Label M8Food -TimeoutSeconds 120
```

Latest exact food report/build/memory/log evidence and scope: [PERSISTENCE_M8.md](PERSISTENCE_M8.md). Fixture waits 3.2 wall-clock seconds with no world ticks to exercise actual file UTC age. It does not launch a separate client/server or certify manual/rendered gameplay.

For the combined M8 native checkpoint, use these two selections. They covered the original 26 native tests plus four persistence safety/collision cases at 31dce6b (30 distinct tests). The subsequent DeadPlayerRespawn addition increases current selection to 31; it passed a separate focused run, not a repeated full suite. Splitting stays within the runner's 200-character filter bound:

```powershell
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunNativeAutomation.ps1 -TestFilter PF.Building+PF.Crafting+PF.Creatures+PF.Input+PF.Interaction+PF.Inventory+PF.Persistence+PF.Settings+PF.Survival+PF.World -Label M8Native
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunNativeAutomation.ps1 -TestFilter PF.PrimalAgentTools.CommandArguments+PF.PrimalAgentTools.MissingSystemsAreBlocked+PF.PrimalAgentTools.TeleportAndRuntimeReset -Label M8Commands
```

Do not select the whole PF.PrimalAgentTools prefix for this checkpoint. Its editor ScenarioIdempotence needs an isolated L_Automation and explicit PFRunScenarioTests opt-in; ViewportCapture needs a rendered editor and PFRunViewportTest. Without these prerequisites they produce setup warnings, not executed scenario/screenshot coverage. The strict runner correctly refuses that warned selection even when the engine labels the records Success. Preserve the report; do not suppress warnings or alter the fixture to claim a pass.

- Retained `M8StartupFailureReproduced_1791432857773`: engine exit 0, one failed test/12 assertions; runner exit **1**. Retained `M8StartupFailureVerified_1791432964361`: one clean pass; runner exit **0**.
- Fresh runner launch `Automation_M8ReportGate_20261008_042918633_adb3d3f7` passed `PF.Persistence.StartupFailurePreservesSave`: **1 passed, 0 failed, 0 test warnings**, engine/runner exits 0. Raw log severity counts zero. Sampled working/private **2.999/2.856 GiB**, 18.16 seconds.
- Actual unmatched selection `PF.Verification.NoSuchTest`, run `Automation_M8EmptyReportGate_20261008_043016202_b207d390`: engine exit 0, **no index.json**, runner exit **1**. Log reports “No automation tests matched”; 1 error/0 warnings/0 fatal/ensure. Sampled working/private **2.964/2.886 GiB**, 16.27 seconds. This intentional diagnostic failure is retained; it is not a gameplay regression.
- Missing report and unsafe filter rejected; outside-report inspection refused. PowerShell parsing and nonzero-exit, null-count and duplicate-ID rejection checks passed. Temporary malformed-reader fixtures were isolated outside the project and removed; engine evidence was not edited.

Installed engine: **5.8.3**, originally requested 5.8.2. No C++ changed in this tooling increment; earlier successful M8 Editor/Game builds remain applicable. M7/M8 manual acceptance remains open. See [PERSISTENCE_M8.md](PERSISTENCE_M8.md) and [PLAYTEST.md](PLAYTEST.md).

## Repeatable M8 live create/restart runner

`Scripts/RunPersistenceAutomation.ps1` replaces the untracked temporary helpers for the existing opt-in `PF.Persistence.Live` test. Run sequentially from the project root after closing any active Editor session:

```powershell
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunPersistenceAutomation.ps1 -Players 1
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunPersistenceAutomation.ps1 -Players 2 -SimulateLagLoss
```

The first command runs a server and one client, creates/saves the disposable scenario, waits for both processes, then launches a separate server/client restart using the same test slot, endpoint and private profiles. The second runs the same flow with two independent clients under fixed outgoing 75 ms lag and 1% simulated loss. Default baseline explicitly sets outgoing PktLag=0/PktLoss=0; both profiles require confirmation in every engine log. These settings are session-only, are not measured RTT/loss and do not certify other emulation parameters.

Map is fixed to L_PrimalFrontier_OpenWorld, all processes use NullRHI/NoSaveConfig, and slots/profiles are generated fresh; there is no personal-slot parameter. The runner deliberately changes disposable runtime state and writes test saves. It does not build, save maps/assets, change settings, delete prior evidence or close user processes. C++ changes require a build first. Packaged Server compatibility and rendered/manual acceptance remain separate.

Options: Players 1 or 2; Port 1024–65535 (default 17989); TimeoutSeconds 30–600 per phase (default 210); EngineRoot and ProjectRoot for another checkout/installation. Server readiness has a maximum 45-second deadline, bounded by the phase timeout. Phase timing includes launch, waiting, cleanup and report inspection; it is not per-process frame timing. Cleanup attempts every owned handle, allows at most five seconds per exit wait and records any cleanup failure instead of passing or killing processes by name.

Exit **0** requires both phases and all process reports to pass the native verdict reader, exactly PF.Persistence.Live, no fatal/ensure, confirmed network settings, the expected server RPC refusals/client acknowledgements, and exactly one owner-client plus Players-1 foreign-client privacy roles. Exit **1** means a launched run failed, timed out or lacks required evidence; Restart is not attempted after failed Create. Exit **2** is preflight refusal, including missing files, a running Editor or occupied UDP port. Parameter-binding errors can return 1 before execution.

Outputs: each process has Saved/AutomationReports/M8Live<players>_<UTC>_<id><Create|Restart><Server|ClientN>/index.json and matching Saved/Logs/PF<run>.log. The overall summary is Saved/AutomationReports/M8Live<players>_<UTC>_<id>/run-summary.json. Summaries retain per-process tests/exits/contract checks, role coverage, sampled working/private memory, raw severity counts and timeout/cleanup state. Missing reports are failures. No log events, private profile names, reconnect credentials or save payloads are printed in summaries. Raw engine logs can contain login options; redact before sharing them.

`LogReviewRequired` remains independent: a clean report can coexist with engine startup warnings/errors. The runner does not silently waive them. Inspect causes before accepting gameplay; summary success alone does not certify clean raw logs, rendering, FPS, manual playtesting or the 16 GB minimum.

### Runner verification — October 8

- PowerShell parsing passed. Missing engine and deliberately occupied UDP port returned 2, with no launched Unreal processes or report directories.
- Initial deliberate 30-second timeout M8Live1_20261008_070426957_133a89de returned 1, terminated its two owned processes (engine exits -1), failed missing/incomplete reports and did not launch Restart. Summary/logs retained. Baseline was subsequently tightened to explicitly confirm outgoing zero lag/loss.
- One-client baseline M8Live1_20261008_070611505_6d2d8154: four PF.Persistence.Live reports passed 1/1, engine/report/runner exits 0, zero test warnings/errors/fatal/ensure; outgoing 0/0 and owner role confirmed.
- Two-client profile M8Live2_20261008_070924038_40cb7ddc: six reports passed 1/1 with the same clean test outcomes, confirmed outgoing 75/1, exact server refusals and both privacy roles. Client 2 did not crash.
- Final cleanup hardening was followed by a focused 30-second timeout replay M8Live1_20261008_071131922_8e2876ab: exit 1, TimedOut=true, no cleanup failures, both owned exits -1, no remaining Unreal process, missing reports refused and Restart not launched. This is an intentional tooling negative test, not a gameplay regression. Successful full runs above preceded only that cleanup hardening; the changed termination path was replayed separately.

| Normal run | Working GiB | Private GiB | Phase elapsed s |
| --- | --- | --- | --- |
| M8Live1_20261008_070611505_6d2d8154CreateServer | 1.711 | 1.606 | 51.77 |
| M8Live1_20261008_070611505_6d2d8154CreateClient1 | 1.807 | 1.770 | 51.77 |
| M8Live1_20261008_070611505_6d2d8154RestartServer | 1.715 | 1.613 | 40.13 |
| M8Live1_20261008_070611505_6d2d8154RestartClient1 | 1.794 | 1.770 | 40.13 |
| M8Live2_20261008_070924038_40cb7ddcCreateServer | 1.717 | 1.609 | 53.64 |
| M8Live2_20261008_070924038_40cb7ddcCreateClient1 | 1.813 | 1.788 | 53.64 |
| M8Live2_20261008_070924038_40cb7ddcCreateClient2 | 1.817 | 1.783 | 53.64 |
| M8Live2_20261008_070924038_40cb7ddcRestartServer | 1.715 | 1.608 | 41.91 |
| M8Live2_20261008_070924038_40cb7ddcRestartClient1 | 1.791 | 1.762 | 41.91 |
| M8Live2_20261008_070924038_40cb7ddcRestartClient2 | 1.793 | 1.761 | 41.91 |

All ten normal raw logs were reviewed: only the known 24 widget/HLOD warning lines and 14 installed Toolsets Python error lines per process, no other warning/error categories. Summaries correctly set LogReviewRequired=true. Default PrimalFrontier.log remains stale at October 6 22:44:15 UTC because these runs use unique abslog files. Individual process peaks must not be added and called a simultaneous system peak. No C++ changed or repeated Editor/Game build required; prior source/test builds remain applicable. M7/M8 manual gates remain open.
