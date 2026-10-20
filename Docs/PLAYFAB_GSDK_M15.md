# M15 opt-in PlayFab lifecycle

October20,2026. **Local lifecycle PASS_WITH_KNOWN_LOCAL_MESSAGES**. This verifies the offline Microsoft mock-agent lifecycle and closed admission, not a playable hosted session. GSDK has no remote world restoration, authenticated player admission, active-writer fencing or voice media yet. No PlayFab capacity/build allocation, cost agreement, firewall change, elevation or asset authoring occurred.

## Dependency and behavior

Microsoft PlayFab/gsdk UnrealPlugin2.0.1, Apache-2.0, pinned673f068f079591a820d0720048c6b4b77824825f. Source/license only; no vendor art/screenshots/binaries imported into the project. Descriptor ServerOnly/LoadingPhase None, project TargetAllowList Server, game module dependency only on Server. Default offline Server never loads the SDK or requires agent env. Editor/Game clients omit the SDK. License/patch notes are staged with the Server; original notices retained. Exact local changes in Plugins/PlayFabGSDK/PF_VENDOR_NOTES.md: explicit opt-in loading, portable runtime/license staging instead of legacy machine-specific D3DCompiler_43, and nine explicit FString key conversions for source5.8.3 JSON storage. No vendor protocol/log-severity change.

UPFHostingSubsystem requires compiled dedicated Server plus PFPlayFabHosted and bounded GSDK_CONFIG_FILE JSON. Validates types/fields/size/port before loading the vendor parser, rejects conflicting fixed-port/local-save overrides, sets agent UDP gameport before netdriver creation, checks actual bound port/open-world/catalog readiness before ReadyForPlayers. Real health callback is stateless/thread-safe to call; worker shutdown dispatches through a weak reference to GameThread. Deinitialization leaves worker delegates immutable and stops weak callbacks from acting after teardown.

**Both PreLogin and InitNewPlayer refuse hosted gameplay, including after Active**, with explicit NOT IMPLEMENTED: durable restore/authenticated identity required. Standby is map/process readiness only; termination makes no durability acknowledgment. No API/database credential is required by these local fixtures.

## Exact verification

Game target dependency-boundary build PASS91.85s wall/91.37s UBT, exit0, no compiler warnings/errors (`PrimalFrontierGame_GSDKBoundary.{log,result.json}`). Receipt inspection: PlayFabGSDK absent from Game and Editor, present in source Server. This proves the build boundary, not a Shipping or rendered-game certificate.

- Strict config native test `PF.Hosting.AgentConfiguration`: PASS1/1 in `Saved/AutomationReports/Automation_M15HostingConfigFix_20261020_001924492_505d7843`;20.65s, exit0, raw/test severity0,2.991GiB working/2.835GiB private. Real vendor schema plus13 malformed/bounds/type variants; failed parse preserves output.
- Editor final health build PASS14.40s wall (14.07s UBT); source Server explicit unity final health build PASS54.20s wall (51.39s UBT). Logs/receipts `PrimalFrontierEditor_GSDKHealth` / `PrimalFrontierServer_GSDKHealth` under C:\UnrealSource\BuildLogs; no compiler warnings/errors.
- Ordinary packaged offline persistence: PASS4/4 PF.Persistence.Live role executions (Create/Restart x Server/Client1) in `Saved/AutomationReports/M8PackagedLive1_20261020_003646213_8e9312a4`. Create112.79s/Restart37.06s, exits0, native severity0, Server raw severity0.632pre-existing primary private save/settings hashes unchanged. Clients retain26/14 then24/14 raw startup warning/Python error lines. Tested package M15GSDKReady; later only hosted health logging changed in code. Baseline package60c0999 separately passed one-client4/two-client6 before SDK integration.
- New opt-in packaged client readiness launcher requires actual connected open-world local pawn stable2s, bounded90s deadline, then queues the original test once. Existing startup warnings remain in raw logs; test assertions/zero-warning verdict are unchanged. Prevents slow initial map loading from racing the test start. Only packaged baseline mode opts into PFRunPersistenceWhenReady.
- Final no-debug stage PASS40.77s wall/exit0 (`PFStage_WindowsServer_GSDKHealth`); package `C:\UnrealSource\Packages\M15GSDKHealth\WindowsServer`. No PDB uploaded; source/earlier artifacts retain symbols separately. Inner EXE SHA256 `8E13AE5BD63ED374A32F0AD6520AE8085C7091973E6911053078D4E9F9352EFD`.
- Official Microsoft-signed LocalMultiplayerAgentv0.9.6 process-mode run `C:\UnrealSource\TestRuns\M15GSDK_20261020_005012_beb3d8de` passes, wrapper exit0. Actual mock Standby/Active, real health callbacks healthy1, SDK Terminate-command receipt, game-thread shutdown, agent/Server/probe natural exits0, cleanup empty. `PF.Hosting.ClosedAdmissionLive` PASS1/1/zero test severity, actual loopback server refusal; report `Saved/AutomationReports/M15GSDK_20261020_005012_beb3d8deProbe/index.json`. Archive SHA256 `0B5B13E76C99D065AC266B2492D5EABDC5449A1D41737CA2854CE1C743BF3AC2`.

These are three distinct selectors with1config/4persistence-role/1final handshake executions, not six different test definitions. Native loopback refusal errors are expected only by exact NOT IMPLEMENTED phrase; timeout/unrelated failure still fails. Probe starts on /Engine/Maps/Entry; actual Server world is /Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld. No manual rendered acceptance inferred.

## Known messages and performance limits

All raw logs retained. Final local Server has Error1/Warning2, **not clean raw logs**:

1. SDK startup POST /v1/metrics/<id>/gsdkinfo receives404 from older official mock0.9.6; heartbeat endpoint remains200. AllowLegacyMetrics404 is an explicit fixture-only exception, requires exactly one matching SDK error AND actual mock route/404 trace. Production telemetry remains unverified. Vendor severity/code is unchanged.
2. SDK logs normal received Terminate operation as Warning; one exact message plus callback/natural exit is required terminal evidence. The SDK stops heartbeats immediately, so the mock does not print a final Terminating heartbeat. Do not invent one.
3. Engine UCrowdManager::CreateCrowdManager emits the known missing-Recast warning after this world's BeginTearingDown. Source CrowdManager.cpp:905 checks MyNavData null. Likely teardown ordering; no such warning occurred during gameplay. Explicit AllowKnownCrowdTeardownWarning accepts at most that exact one after teardown, never earlier. Engine navigation code and assets unchanged; broader shutdown/nav cleanup remains a separate investigation.

Final receipt has UnexpectedServerErrors0/UnexpectedServerWarnings0, no fatal/ensure and valid nonzero memory samples. Editor probe startup Python/toolset messages and expected connection refusal remain visible; native test itself is clean. These limited known-message exceptions do not suppress logs or permit arbitrary severity.

Sampled GiB working/private: Server0.306/0.240; agent0.064/0.132; Editor probe1.888/1.862. No GPU frames/FPS/sustained stutter/16GiB minimum/container/hosted performance certification. Machine remains32GiB; use NullRHI/one client.

## Reproduce the bounded gate

Prerequisites: compiled Editor with probe, source Server and supported cook/stage described in PLAYFAB_SERVER_BUILD_M15.md; close other Unreal instances. Existing package has no PDB and includes vendor license. Official local agent folder is C:\UnrealSource\Dependencies\LocalMultiplayerAgent_v0.9.6, signature validated by script. No account secrets, owner world or manual engine interaction.

Start here: PowerShell in C:\UnrealProjects\PrimalFrontier.

```powershell
.\Scripts\RunPlayFabLifecycle.ps1 -PackagedServerRoot C:/UnrealSource/Packages/M15GSDKHealth/WindowsServer -AllowLegacyMetrics404 -AllowKnownCrowdTeardownWarning
```

1. Script validates paths/signature/free ports/no competing Unreal, creates unique external run directory, copies the official tool, zips package, writes synthetic settings. It never edits the downloaded original agent settings or allocates cloud capacity.
2. Server uses current open-world map, NullRHI, loopback17996, explicit UserDir, no local save slot; agent HTTP56011. gameport is UDP. Mock title6B7B6/regionEastUs are fixture metadata, not a region/deployment selection.
3. Observe native probe on isolated Entry client. Real server refuses the handshake; no player admitted, no saved data loaded. Script waits up to150s for natural agent/Server/probe exit and validates native report, health, actual port/state receipts and exact known messages. It can take roughly2minutes.
4. PASS requires Result PASS_WITH_KNOWN_LOCAL_MESSAGES (or PASS if no capability gaps), all three exit0, probe Success/errors0/warnings0, healthy callbacks, termination receipt/natural exit, valid memory, no unexpected severity or cleanup failures. STOP on any failure. Do not broadly ignore warnings, force exits to hide teardown, change rendering or operate personal saves.
5. Evidence: printed C:\UnrealSource\TestRuns\M15GSDK_<stamp>_<id>\run-summary.json, Agent.stdout/stderr.log, Server.log, Probe.log and native report path. No screenshots needed for backend-only fixture. Script touches only captured owned handles on cleanup; no global process kill.

To replay the unchanged offline persistence gate using the compiled readiness helper:

```powershell
.\Scripts\RunPersistenceAutomation.ps1 -Players 1 -ProjectRoot C:/UnrealProjects/PrimalFrontier -PackagedServerRoot C:/UnrealSource/Packages/M15GSDKReady/WindowsServer -Port 17995 -TimeoutSeconds 240
```

## Retained failures and limits

- Initial Editor build passed43.68s; first config native run Automation_M15HostingConfig_20261020_001712200_abd10689 failed6assertions because Unreal TryGetStringField coerces numeric fields. Strict EJson type checks fixed this; exact selector replay passed.
- Initial source Server GSDK failed exit6/52.84s UBT with9C2664/C2665/C2679 key conversions; explicit FString copies fixed it. Source replay54.54s wall/52.60s UBT passed, no warnings; protocol unchanged.
- Initial offline SDK package CreateServer passed but client had five known HLOD/settings startup warnings in its test report. Runner stopped before Restart. Connected-world readiness fixes timing without suppressing raw warnings; final4/4 passes.
- First actual agent run M15GSDK_20261020_003951_8103e913 reached states/exits/refusal but overall fixture failed missing health visibility/metrics404. Its integer-rounded memory fields are invalid and are not footprint evidence.
- Second actual agent run M15GSDK_20261020_004637_3ef51738 proved health/refusal/exits but failed the incorrect final-heartbeat/blanket raw-warning expectations. Vendor protocol and Engine teardown origin were inspected; final real replay uses exact observed semantics/known-message scope. Failed receipts/logs preserved.

Next independent gate: production backend schema/active-writer fencing, authenticated identity and admission-blocked durable restoration. Review actual service access/roles/schema before a migration. Cloud build/container validation and actual account quota/cost review precede any hosted allocation. Do not open gameplay admission or claim fullM15/voice complete from this local lifecycle pass.

Official references: [GSDK](https://learn.microsoft.com/en-us/gaming/playfab/multiplayer/servers/server-sdks/unreal-gsdk/), [local process-mode testing](https://learn.microsoft.com/en-us/gaming/playfab/multiplayer/servers/localmultiplayeragent/run-process-based-gameserver), [Unreal local deployment](https://learn.microsoft.com/en-us/gaming/playfab/multiplayer/servers/server-sdks/unreal-gsdk/third-person-mp-example-project-local-deployment-and-debugging).
