# M15 source-server and PlayFab package gate

## Final local package acceptance — October 20

**PASS for the local package gate only.**48 files /3111219005bytes (2.898GiB, including debug PDB), archive `C:\UnrealSource\Packages\M15ReportTemplate\WindowsServer`. Inner Server executable SHA256 `C37FAE32CBD465C2B76C03279EB0F6F8B800BD63C2072D103EF1A55DB8DD9B9E`. No generated package or symbols committed to Git. No GSDK/lifecycle/hosted capacity, remote restore, cooked-client or voice-media acceptance.

Final build/stage receipts under C:\UnrealSource\BuildLogs:

- `PrimalFrontierServer_LocalVersion`: PASS46.73s wall/43.14s UBT, exit0, no compiler warnings/errors.
- `PrimalFrontierServer_ReportTemplate`: PASS8.00s wall/5.85s UBT, exit0, receipt-only dependency staging change.
- `PrimalFrontierEditor_ReportTemplate`: PASS88.74s wall/88.34s UBT, exit0, no compiler warnings/errors.
- `PFStage_WindowsServer_ReportTemplate`: PASS41.29s wall, exit0. Installed matching UAT resolves the23,353-byte engine report shell; source engine's corresponding file is22,727bytes. Neither file was modified; actual HTML/JSON export is verified. Catalog cook remains577packages/9platform-skipped/0cookseverity.
- `PackagedRunnerRefusals.json`:2/2 expected exit2 preflights (missing HTML shell; unsupported packaged fixture), no launch. PowerShell parser0errors.

Final test reports under `C:\Users\jackh\.codex\worktrees\m15-source-server\PrimalFrontier\Saved\AutomationReports`:

| Run | PF.Persistence.Live reports | Create / Restart duration | Result |
| --- | --- | --- | --- |
| M8PackagedLive1_20261020_000342359_09651695 |4: Server+Client1 in both phases|63.85s /34.26s|PASS|
| M8PackagedLive2_20261020_000556890_e003ca4d |6: Server+Client1+Client2 in both phases|62.11s /37.65s|PASS|

All10 final role reports: native warnings/errors0, process/verdict exits0, authority/refusal/privacy/network contracts true, no timeout/cleanup failure/fatal/ensure. Exact `index.json` plus server `index.html` in each role directory; per-run `run-summary.json`; unique `PF<run><phase><role>.log` under isolated Saved/Logs. Separate owner/foreign storage privacy and real file-backed process-restart identities/inventories/progression/ownership verified; private-file hash guards unchanged. This is10 role executions of one existing native test selector, not10 distinct test definitions. Earlier8/8 checkpoint/player-save native regressions remain separately listed below.

Isolation limit: PrivateFileCount0 in each final run means the snapshot guard has no pre-existing nonfixture private save/settings to compare. This is not a checksum certificate for personal saves. Fresh explicit per-role UserDir and synthetic slots avoided loading primary/private Saved data.

Final Server raw logs have **zero warnings/errors** in all four phases. Existing Editor-client Python/toolset/HLOD startup baseline retained: one-client26/14 then24/14 warnings/error lines; two-client Create26/14 and27/14, Restart24/14 and25/14. All14 errors per client are the known StateTreeToolset/ToolsetRegistry `unreal.ToolsetDefinition` / `unreal.PythonTestRunner` startup tracebacks. Concurrent Client2 adds a Trace Server startup warning; Create adds two settings-priority warnings preserving motion blur/depth-of-field0. These are not suppressed or presented as clean Editor logs. The early immediate-quit Recast warning did not recur in sustained final runs; existing creature fixtures spawn/restore. No new gameplay failure observed.

Sampled peaks: Server up to0.319GiB working/0.247GiB private; clients up to1.992working/1.983privateGiB each. No rendered frames, GPU performance/stutter, all-day memory or16GiB minimum certification. Free disk602.0GiB at final package measurement.

### Reproduce the verified bounded test

Prerequisites: supported source Server build and installed-engine Editor/cook/stage described below; close other Unreal instances. Level is `/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld`, map assets unchanged. Start here: PowerShell in C:\UnrealProjects\PrimalFrontier. No account/API credentials, private save or manual rendered input required.

```powershell
.\Scripts\RunPersistenceAutomation.ps1 -Players 1 -ProjectRoot C:/Users/jackh/.codex/worktrees/m15-source-server/PrimalFrontier -PackagedServerRoot C:/UnrealSource/Packages/M15ReportTemplate/WindowsServer -Port 17995 -TimeoutSeconds 240
# Only after its complete Create+Restart pass:
.\Scripts\RunPersistenceAutomation.ps1 -Players 2 -ProjectRoot C:/Users/jackh/.codex/worktrees/m15-source-server/PrimalFrontier -PackagedServerRoot C:/UnrealSource/Packages/M15ReportTemplate/WindowsServer -Port 17995 -TimeoutSeconds 240
```

Runner uses NullRHI, per-role private UserDir and disposable AutomationM8 slot/profile, loopback-only server, PktLag0/PktLoss0 (not an impairment certificate), owned handles/cleanup and original native assertions. Require both phases PASS, expected4/6 exact reports, exit0, original/private files unchanged, clean Server raw log and actual HTML/JSON exports. STOP on refusal/failure; do not delete/reset personal saves or bypass version checks. Missing package HTML or unsupported advanced-fixture switches refuse before launch. Additional named fixtures remain uncooked-runner scope until separately packaged/tested.

### Exact isolated source metadata input

Only Engine/Build/Build.version in C:\UnrealSource\UE_5.8.3 changed: Major5/Minor8/Patch3, Changelist2026102001, CompatibleChangelist55116800, IsLicenseeVersion1, IsPromotedBuild0, BranchName `RiftBoundGames+UE5.8.3+396c9f0+LocalServer`. Local build number identifies our build, not Epic's depot sync. Leave BuildId auto-generated. Supported file-based metadata stamping follows [Epic's build version documentation](https://dev.epicgames.com/documentation/en-us/unreal-engine/versioning-of-assets-and-packages-in-unreal-engine). Do not copy Launcher Build.version wholesale or set runtime network overrides. Source checkout commit and this local metadata patch must accompany future build provenance.

The remaining sections retain earlier failures and intermediate progress chronologically; the acceptance above is the current result.

## October 20 compatibility replay in progress

Compatibility replay `M8PackagedLive1_20261019_235915377_12000c72` completed4/4 native role reports (Create/Restart x Server/Client1), all exit0 / zero test warnings/errors / exact authority/privacy contracts passed. Server and client both report NetCL55116800/checksum3283589285; no runtime override. Private-file guard unchanged. However each packaged Server emitted one real packaging error: AutomationController could not read Engine/Content/Automation/Report-Template.html. JSON reports still pass, but this run is not a clean-log package certificate. Narrow fix adds that existing23KB engine file as NonUFS RuntimeDependency in the non-Shipping PrimalAgentToolsRuntime Server build rules. Server receipt regeneration passes5.85s UBT/8.00s wall, exit0; installed Editor rebuild and fresh UAT staging/replay pending. No new plugin/content download or gameplay change. Final package must export both HTML and JSON without this error.

Catalog-only cook fix:577 cooked packages/9 platform-skipped, zero cook warnings/errors; restaging passed. Packaged smoke `C:\UnrealSource\TestRuns\M15PackCats_e3ded01cc833` passed PF.SaveWorld, PF.CheckSave, PF.ExportTestReport and confirmed exit0 (2.69s; startup-only sampled working0.292/private0.237GiB). One immediate-quit Recast/Crowd warning remains to check during sustained execution. Receipt `C:\UnrealSource\BuildLogs\PFPackageSmoke_Catalogs.result.json`.

First packaged-server/Launcher-client full-loop run `M8PackagedLive1_20261019_053550731_64300861` failed connection: source NetCL0, client NetCL55116800; both engine network protocol44/game0/project1.0.0.0. No state/restart pass inferred. Owned children stopped; summary retained under isolated Saved/AutomationReports/<run>/run-summary.json.

Root cause: UBT BuildVersion.EffectiveCompatibleChangelist returns0 when Changelist is0, despite the official source tag declaring CompatibleChangelist55116800. Following [Epic's documented Build.version metadata workflow](https://dev.epicgames.com/documentation/en-us/unreal-engine/versioning-of-assets-and-packages-in-unreal-engine), only the isolated source engine's Engine/Build/Build.version is stamped with local licensee build number2026102001, IsLicenseeVersion1 and BranchName `RiftBoundGames+UE5.8.3+396c9f0+LocalServer`. This number identifies our local build, **not** an Epic/Perforce changelist; official source commit396c9f059903aed5fec78ecd3d437a40c6415368 is unchanged. Major/minor/patch, IsPromotedBuild0 and the tag's declared CompatibleChangelist55116800 remain unchanged. Installed engine metadata, project association, code and asset bytes remain unchanged. No networkversionoverride, checksum delegate, runtime bypass or warning suppression.

Metadata rebuild PASS46.73s wall (43.14s UBT), exit0, three actions (BuildSettings/link/receipt), no compiler warnings/errors: `PrimalFrontierServer_LocalVersion.{log,result.json}` under C:\UnrealSource\BuildLogs. New staging/archive PASS55.57s wall, exit0: `PFStage_WindowsServer_LocalVersion.{log,result.json}`, package `C:\UnrealSource\Packages\M15LocalVersion\WindowsServer`. Existing577-package cook reused without asset edits. One-client replay running with Scripts/RunPersistenceAutomation.ps1 -PackagedServerRoot targeting this archive; both roles get isolated UserDir, owned handles/waits and server loopback binding. Only actual successful connection plus existing native authority/save/restart assertions can accept interoperability. If this fails, use matching source-built clients rather than weakening network validation. Remaining sections retain chronological earlier evidence, not current running claims.

Updated October 19, 2026. **Source Server compile PASS; cooked package and hosting remain unverified.** The user requested preparation of the missing PlayFab build. No paid capacity, new service agreement, account secret or personal save is involved in this preparation.

## Verified prerequisites

- Main project began clean on master at `33487b493dce6f77598f91e5ebecb87a5d0976df`.
- Installed engine is **5.8.3 CL58210709** (compatible CL55116800), despite the older requested 5.8.2 label. Existing `.uproject` association remains `5.8`.
- C: had **833.93 GiB free** at preflight; machine reports 34281455616 bytes physical RAM (31.93 GiB). Earlier 48 GiB storage block is resolved. This does not certify final package size or build capacity.
- GitHub access to EpicGames/UnrealEngine works. Separate shallow official `5.8.3-release` checkout completed at `C:\UnrealSource\UE_5.8.3`; annotated tag `a15e5a4312f0c35c1f373818b30d88ce0966e95c` resolves to `396c9f059903aed5fec78ecd3d437a40c6415368`.
- Managed isolated project worktree `C:\Users\jackh\.codex\worktrees\m15-source-server\PrimalFrontier` protects primary-project build outputs and private Saved data. Assets must be hydrated from existing local Git LFS objects before cooking; a pointer-file checkout is not usable content.
- Local-only `git lfs checkout` hydrated all 7301 isolated assets (45 GB). Subsequent Git status is empty and `git diff --quiet` exits 0. Open-world package restored to 14192 bytes from its 130-byte pointer; sampled asset SHA256 matches committed pointer. No LFS fetch/upload was needed for this hydration.
- Official PlayFab GSDK reviewed clone: `C:\UnrealSource\Dependencies\PlayFabGSDK`, commit `673f068f079591a820d0720048c6b4b77824825f`, plugin 2.0.1. Downloaded outside the project; not integrated/enabled yet.
- Official LocalMultiplayerAgent release v0.9.6 downloaded outside the repo for later local testing, not executed. Archive SHA256 `676c41a0f6d4f9c057f1050c29ff3e59055a30fa28d376c5cc407710b7a9b6c3`; extracted to `C:\UnrealSource\Dependencies\LocalMultiplayerAgent_v0.9.6`. Official release metadata has no published asset digest; this hash records the retrieved artifact, not an independent digest comparison. README/tag checkout at `cd234a12439122276b1575444d3a74bd19e1a500`. Included sample confirms `RunContainer: false` Windows process mode; sample ports/commands are placeholders and have not been used.
- Existing `Scripts/TestMultiplayerSetup.ps1` reran **12/12 PASS at 03:41:32 UTC**. Inventory refreshed at 03:41:34 UTC confirms no Editor process, source access PASS and 775.90 GiB remaining after download/worktree activity. These launcher-plan/refusal checks do not execute source builds or host services.

Trello storage prerequisite [svt4pL1Y](https://trello.com/c/svt4pL1Y) and source build [aUP5dgfc](https://trello.com/c/aUP5dgfc) are Done/complete, confirmed by readback. Package gate [EGunz5f8](https://trello.com/c/EGunz5f8) is Programming / Doing. Parent M15 remains incomplete.

## Source-build execution history

Build dependencies completed successfully: exit 0, 128672/128672 files, 22526.4 MiB downloaded. Used the source engine's own GitDependencies executable, 4 threads, excluding Mac/IOS/Android/Linux/LinuxArm64/TVOS/VisionOS/HoloLens for the first Windows-only gate. No Launcher files, global engine association, machine editor settings or redistributable installation was changed. Evidence: `C:\UnrealSource\BuildLogs\GitDependencies_5.8.3.log`. Source Git status remained clean after setup; 710.45 GiB free before compilation.

Initial actual invocation, including UnrealBuildTool bootstrap (later failed in game compilation as recorded below):

```powershell
& C:/UnrealSource/UE_5.8.3/Engine/Build/BatchFiles/Build.bat PrimalFrontierServer Win64 Development '-Project=C:/Users/jackh/.codex/worktrees/m15-source-server/PrimalFrontier/PrimalFrontier.uproject' '-MaxParallelActions=4' -NoUBA -NoXGE -NoFASTBuild '-CompilerVersion=14.50.35738' '-WindowsSdkVersion=10.0.22621.0' -NoHotReloadFromIDE -WaitMutex
```

UBT bootstrap passed (15.82 seconds, 0 warnings/errors). Initial unquoted dotted arguments were split by PowerShell into bogus targets `.50.35738` / `.0.22621.0`; first invocation failed RulesError/exit 8 before game compilation (7.78 seconds UBT time). Retained `PrimalFrontierServer_First.log` / `.result.json`. Quoted replay above is running; output `C:\UnrealSource\BuildLogs\PrimalFrontierServer_Quoted.log`. Its completion receipt at matching `.result.json` is written only after exit. Inspect actual failures/warnings before Editor/cook preparation. Source setup/download completion alone is not compilation success.

Quoted replay confirmed the requested MSVC 14.50.35738 and Windows SDK 10.0.22621.0. UHT completed in 19.782 seconds (13788 generated files); native compilation scheduled 1055 actions. Build log reports a local UBA executor with individual `[NoUba]` actions and the requested maximum of 4 actions; it is not distributed compilation. Samples during compilation show approximately 10.3–10.5 GiB free physical RAM and 699 GiB free disk, not a measured peak or server-runtime footprint. Compilation remains in progress.

**Failure detected in Module.PrimalFrontier.2.cpp:** C4459 at PFLocalPlayer.cpp:17 points to the checkpoint bridge's anonymous-namespace `Endpoint`; C2084 at PFPlayerSaveFormat.cpp:14 points to the bridge's duplicate anonymous-namespace `Refuse`. C2264 messages are cascading errors from that duplicate definition. This is a project unity-build defect, not the earlier installed-engine Server-target restriction or an RHI problem. Do not proceed to cook/GSDK yet.

Main source fix is limited to PFCheckpointClient.cpp / PFCheckpointProtocol.cpp: rename the two bridge-private helpers to `CheckpointApiEndpoint` / `RefuseCheckpointRecord`. No URL, protocol, save/schema, gameplay or asset change. Original isolated build is allowed to finish its already scheduled actions with its original files; no concurrent mutation of that build snapshot. After exit, copy only these two files into the clean isolated snapshot and rebuild with `-DisableAdaptiveUnity`, so Git's modified-file exclusion cannot hide the unity issue. Editor compile and existing focused checkpoint/player-save tests must pass before closure.

Original quoted batch finished **FAIL**, exit 6 / OtherCompilationError, 4798.78 seconds UBT time (4764.22 seconds executor). Exact diagnostics: one C4459, one C2084 and 24 C2264 cascades; no separate engine/compiler failure was found. Receipt `PrimalFrontierServer_Quoted.result.json`. Its successful engine objects remain cached. Only the two renamed files were then copied into the verified-clean isolated worktree. Explicit `-DisableAdaptiveUnity` replay is running at `C:\UnrealSource\BuildLogs\PrimalFrontierServer_UnityFix.log`, with matching completion `.result.json` written only on exit. No test, warning or unity-build suppression was added.

Editor rename build **PASS**, 84.26 seconds / exit 0 / no compiler warnings or errors. Used the installed engine and main project with `-MaxParallelActions=1` while the original source batch finished independent engine modules. Evidence: `C:\UnrealSource\BuildLogs\PrimalFrontierEditor_UnityFix.log` and matching `.result.json`. Existing eight checkpoint/player-save tests are running next; this Editor adaptive build alone does not prove the failing unity mode fixed.

Focused rename regressions **8/8 PASS** in `Automation_M15UnityFix_20261019_050439165_254032fb` (60.8 seconds, engine/runner exit 0):

- PF.Checkpoints.BoundsAndIntegrity
- PF.Checkpoints.FrozenEnvelopeRoundTrip
- PF.Checkpoints.HttpRoundTrip
- PF.Checkpoints.ResponseRefusals
- PF.Checkpoints.ServerAuthority
- PF.Persistence.CorruptPlayerData
- PF.Persistence.PlayerRoundTrip
- PF.Persistence.PlayerValidation

Zero test warnings/errors and raw-log warning/error/fatal/ensure matches. Sampled peak 2.986 GiB working / 2.881 GiB private. Evidence `Saved/AutomationReports/Automation_M15UnityFix_20261019_050439165_254032fb/{index.json,run-summary.json}` and `Saved/Logs/PFAutomation_M15UnityFix_20261019_050439165_254032fb.log`. No render, live Cloudflare or hosted-gameplay test was run for this name-only fix. Explicit unity Server replay remains required. Setup contract receipt also saved at `C:\UnrealSource\BuildLogs\SetupContracts_20261019.json` (12 distinct checks; repeated capture is not an additional 12 tests).

## Final source-build result

**PASS:** explicit `-DisableAdaptiveUnity` replay compiled Module.PrimalFrontier.2.cpp, linked PrimalFrontierServer.exe and wrote its target receipt in **52.15 seconds** (48.19 seconds executor), exit 0. No compiler warning/error matches. Generated unity source was inspected: PFCheckpointClient.cpp, PFCheckpointProtocol.cpp, PFLocalPlayer.cpp and PFPlayerSaveFormat.cpp still compile together. The fix did not rely on adaptive exclusions or warning suppression. Editor and all eight named regressions above also passed.

- Log / receipt: `C:\UnrealSource\BuildLogs\PrimalFrontierServer_UnityFix.{log,result.json}`.
- Artifact: `C:\Users\jackh\.codex\worktrees\m15-source-server\PrimalFrontier\Binaries\Win64\PrimalFrontierServer.exe`.
- Size: 325622272 bytes. SHA256 `a854009e294b75703c5ef9f4d46f146e40fa498e2074be0d3a10087a7bfec3a1`.
- Completed 05:13:27.962 UTC. C: free space 688.56 GiB after source/build/worktree activity.
- Build-time free physical RAM samples approximately 10–15 GiB; these are not peak server-runtime measurements. Native test process peaks are listed separately above.
- No Server launch/cook/package/GSDK result is implied by successful linking. Main project's engine association/assets/private saves remain unchanged. Main Editor binary was rebuilt for the two helper renames.

## Package gate in progress

Source fix/evidence published as `133d865b551ade327ed3f7be9e587467095a32a4`, local/remote master equal and primary tree clean before starting this gate, Milestone 15 title / Codex GPT-6.1 Sol coauthor. Matching installed-engine Editor build in the isolated workspace passed in **74.45 seconds**, exit 0, no compiler warnings/errors; `PrimalFrontierEditor_CookWorkspace.{log,result.json}` under the external BuildLogs directory. Each Editor process uses its own matched project DLLs; source Server executable remains a separate artifact.

Initial supported commandlet cook is running:

```powershell
& 'C:/Program Files/Epic Games/UE_5.8/Engine/Binaries/Win64/UnrealEditor-Cmd.exe' 'C:/Users/jackh/.codex/worktrees/m15-source-server/PrimalFrontier/PrimalFrontier.uproject' -run=Cook '-targetplatform=WindowsServer' '-map=/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld' -SkipCookingEditorContent -unattended -nop4 -nosplash -nullrhi -nosound -NoLiveCoding -NoSaveConfig -stdout -FullStdOutLogOutput '-abslog=C:/UnrealSource/BuildLogs/PFCook_WindowsServer_Initial.log'
```

External log, stdout and completion receipt share prefix `C:\UnrealSource\BuildLogs\PFCook_WindowsServer_Initial`. Cook success, staging and actual package execution are not yet verified. No main map/assets/settings/private saves are changed by this isolated cook request. This gate does not enable GSDK or create paid capacity.

Initial cook completed **PASS**, exit 0, 68.37 seconds wall. Cook summary: 573 packages cooked / 9 skipped by platform / 582 total; `Success - 0 error(s), 0 warning(s)`. Cook uses the normal local Zen store (`ue.projectstore`), so loose-asset copying is not sufficient. UAT BuildCookRun staging is now running with `-target=PrimalFrontierServer -server -noclient -serverplatform=Win64 -serverconfig=Development -skipbuild -skipcook -stage -pak -iostore -archive -nocompile -nocompileeditor`. Separate stage root `C:\UnrealSource\Staged\M15Baseline`, archive root `C:\UnrealSource\Packages\M15Baseline`. Exact stdout/receipt prefix `C:\UnrealSource\BuildLogs\PFStage_WindowsServer_Initial`. No package launch or GSDK test passed yet.

Initial stage/archive **PASS**, 44.79 seconds wall / exit 0, 47 files / 2.897 GiB, including 2558840832-byte PDB. Packaged inner executable SHA256 matches the verified source binary. UAT successfully reused the installed matching cooker/UnrealPak tools with the source Server receipt; no source Editor rebuild was needed for this staging attempt.

Actual packaged smoke **FAIL**, `M15PackSmoke_13c750718abe`, logs/reports under `C:\UnrealSource\TestRuns\M15PackSmoke_13c750718abe`. Map loaded as BP_SurvivalGameMode_C and listened on loopback17994; PF.Help ran. PF.SaveWorld / PF.CheckSave refused `Missing persistence catalogs`: DA_ItemCatalog, DA_CraftingCatalog, DA_BuildingCatalog and DA_CreatureCatalog were not cooked because their fallback paths are code strings, not hard map references. Result export correctly says Failed. Initial shell invocation returned asynchronously for the GUI-subsystem executable, so `.result.json` exit is null, not a process-exit pass. Log shows normal requested quit, plus an early-teardown Recast warning that must be investigated in a sustained replay.

Bounded cook fix: add four existing gameplay catalog directories to `DirectoriesToAlwaysCook` in Config/DefaultGame.ini (Items, Crafting, Building, Creatures), alongside existing UI/Audio. No asset edits, blanket CookAll or validation bypass. Exact PFAssetPaths were checked first. Config copied only into the isolated workspace; catalog recook is running at `PFCook_WindowsServer_Catalogs.{log,stdio.log,result.json}` under BuildLogs. Replay process management must use an explicit owned process handle/WaitForExit and isolated `-UserDir`, not assume a synchronous PowerShell GUI invocation.

## Remaining acceptance gates

1. Matching Editor/commandlet build in the isolated cook workspace. Investigate reuse of the installed 5.8.3 cooker: AutomationTool's `ProjectParams.UnrealExe` accepts `-unrealexe`, and `WindowsHostPlatform.GetUnrealExePath` accepts an absolute executable path. This is a supported tool path, not verified cross-distribution cooking. Keep each process's engine and project DLLs matched; build the source Editor if reuse fails. Cook/stage only supported referenced content for `/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld`. No edits to the original map/assets to suppress cook errors.
3. Review and explain official GSDK dependency before integrating. Keep ordinary offline server startup supported; correctly bind health/activation/shutdown, allocation port and player readiness. `ReadyForPlayers` transitions the GSDK to StandingBy before allocation; session-specific restoration follows allocation. Maintain a separate gameplay-admission gate through allocation validation and restoration. Do not wait for unknown session data before entering standby or claim restoration merely from the SDK call.
4. Package launches locally without Unreal Editor. Use matching clients and test actual version/connection compatibility; equal patch numbers alone are insufficient proof. Use NullRHI; one client first, then two when practical. Verify save/restart/reconnect, authority, logs and measured package size.
5. PlayFab LocalMultiplayerAgent lifecycle passes before uploading. Its Windows process mode can be investigated without Docker. A successful raw dedicated process is not a GSDK/local-agent pass.
6. Review actual PlayFab title `6B7B6` quota/region/SKU and metered cost before allocating capacity. No empty/unverified hosted build or standby allocation is created by this preparation.
7. Durable restoration/admission/single-writer coordination and EOS voice remain separate unfinished M15 work. Existing Cloudflare/Neon development checkpoint upload/read/duplicate proof remains valid, but does not prove hosted progress survives recycling.

No manual gameplay action is requested during source setup. Current authored level only applies to the later package test; source/toolchain checks require no level.

## GSDK review findings — not implementation

- Preserve the official repository's Apache 2.0 license/provenance if vendoring its source. No client PlayFab SDK or voice SDK is required merely to add server lifecycle callbacks.
- Vendor startup fatals when heartbeat endpoint/server ID are absent. Ordinary offline dedicated servers must remain supported; do not enable unconditional GSDK startup without an explicit hosted/test mode or equivalent reviewed guard.
- `UGSDKUtils::SetDefaultServerHostPort` requires the exact port name **`gameport`**. The local agent's generic sample instead says `game_port` / TCP; it cannot be used unchanged. Unreal gameplay uses UDP. Verify the actual assigned listen port and client endpoint; do not override the allocation with a fixed hosted `-Port` argument.
- The vendor Build.cs stages `C:/Windows/System32/D3DCompiler_43.dll` on Win64 Server. Review this legacy dependency before integration; do not assume that machine-specific path exists or silently install an old graphics runtime for a NullRHI server.
- Bind allocation/health/shutdown and connected-player callbacks. Actual `FGSDKInternal::ReadyForPlayers` asynchronously enters **StandingBy**; allocation later invokes OnServerActive. Warm a safe server before standby, then validate allocation and restore its world while gameplay admission stays closed. Only successful restoration opens gameplay admission. A local lifecycle test is not durable hosted-world acceptance.
- Local test agent executable Authenticode status is **Valid**, publisher Microsoft Corporation. Download/extraction and signature verification do not constitute an agent launch or GSDK lifecycle pass.

References: [Epic dedicated servers](https://dev.epicgames.com/documentation/unreal-engine/setting-up-dedicated-servers-in-unreal-engine), [official Unreal GSDK](https://learn.microsoft.com/en-us/xbox/playfab/multiplayer/servers/server-sdks/unreal-gsdk/), [official source](https://github.com/PlayFab/gsdk), [local deployment](https://learn.microsoft.com/en-us/xbox/playfab/multiplayer/servers/server-sdks/unreal-gsdk/third-person-mp-example-project-local-deployment-and-debugging).
