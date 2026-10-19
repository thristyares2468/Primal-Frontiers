# M15 source-server and PlayFab package gate

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
