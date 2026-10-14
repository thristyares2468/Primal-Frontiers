# M15 authoritative voice metadata snapshots

Trello https://trello.com/c/VbaQfuF2, ProgrammingDone/complete confirmed after final gates. Depends on published policy3117b35. No actual voice audio implementation or Personal listening assignment.

## Authority contract

`UPFVoiceAuthority` is a nonticking Game/PIE world subsystem. Native entry points require begun play, survival GameMode and a nonclient world. Roster comes from actual server controller iteration, public PersistentPlayerId, correct PlayerState ownership, real possessed pawn position and survivor life state. Dedicated connections must be open; local/standalone controllers are supported. No client-supplied position, space or speaker GUID entry point. Policy enablement and bounded metadata preferences are trusted native calls only, not new RPCs.

Each world gets an ephemeral space GUID. Preferences use weak controller keys, not reconnect secrets/public GUID aliases; new controllers always start without transmit/receive/PTT consent. Invalid config/preferences/snapshot/routing leaves the prior config/preferences/output unchanged. Invalid duplicate ownership/identity refuses, missing/dead pawn is unavailable; policy validation still caps32 players/100m and finite coordinates. Normal world initialization does not enable policy or perform capture/playback. No tick/capture, online module/plugin, waveforms, save fields, device UI or gameplay setting changes.

Future transport must separately bind authenticated origin, bound packet/capture work, validate and rate-limit owning-controller preferences/PTT, expire stale intent, manage devices and stop recording on disable/disconnect. The current native setters are not a transport security claim. No voice history/credentials/preferences are printed by this code.

## Exact AI verification guide

No level required for native actor fixture. Start here: close competing Unreal processes, repository PowerShell:

```
.\Scripts\RunNativeAutomation.ps1 -TestFilter 'PF.Voice.AuthoritySnapshots+PF.Voice.Policy+PF.Voice.PolicyValidation' -Label M15VoiceAuthority -TimeoutSeconds 180
```

Expected exactly3 clean Success records and exit0. Actual transient survival server actors prove public ownership/position, defaultoff, distance, mute/block, dead/unpossessed exclusion, pawn replacement, duplicate identity/config/preference refusal and reconnect consent reset. Failed output remains unchanged. These actors are not a substitute for network roles.

Actual live level `/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld`, uncooked dedicated server plus1client first,2clients second, all NullRHI/no audio. Disposable slot/profile chosen by runner; private save/settings hashes must remain unchanged. No persistent map fixture/asset edit. Run:

```
.\Scripts\RunEcologyAutomation.ps1 -TestCase VoiceMetadata -Players 1 -TimeoutSeconds 150
.\Scripts\RunEcologyAutomation.ps1 -TestCase VoiceMetadata -Players 2 -SimulateLagLoss -TimeoutSeconds 150
```

1. Existing runner verifies UDP17993 availability, server readiness, exact PF.Voice.AuthorityLive flag/filter and report paths; bounded120fps headless execution is diagnostic pacing, not an optimization FPS benchmark.
2. Server derives real player IDs/positions, enables only disposable metadata, places controlled pawns near then far, tests receiver mute and bilateral block, malformed preference preservation and one-player PTT stop.
3. Every actual client directly attempts config/preferences/snapshot/routes and must refuse atomically. Three replicated clock barriers use existing owned water consumption RPC acknowledgements; no voice RPC or client grant introduced. Each send waits300ms to respect the real150ms inventory cooldown.
4. Server waits for each owner's exact3 acknowledgements before announcing final phase, then stays alive15seconds independent of departing pawn readiness. Client gets exactly1remaining water and exits. No test may pass merely because a client exited.
5. Expect all2/3role reports cleanSuccess/no test warnings, contract and network-applied markers true, exit0, no timeout/cleanup/private/config changes. Compare raw severity to known engine startup baseline; any new gameplay/error/ensure needs investigation. Keep failed reports for diagnosis.

No physical mic/speaker/device/range listening test exists yet; no FPS/stutter/minimum-memory/rendered/packaged-Server/Internet acceptance follows from NullRHI.

## Evidence checkpoint

- First Editor PFM15VoiceAuthorityEditorBuild.log FAIL26s C3535/C2440 auto* from TObjectPtr<APawn>; explicit APawn* correction, replay PFM15VoiceAuthorityEditorReplayBuild.log PASS6.56s.
- Automation_M15VoiceAuthority_20261014_233020845_e40d7b64 PASS3/3,17.05s,2.998working/2.865privateGiB, test/rawseverity0. See Saved/AutomationReports/<run>/index.json/run-summary.json and Saved/Logs/PFAutomation_<run>.log.
- Live fixture PFM15VoiceLiveFixtureBuild.log PASS6.99s.
- M15VoiceMetadata1_20261014_233246663_b611d563 FAIL0/2: both timed out, second consumption arrived inside inventory cooldown. Personal588files/settings unchanged; first failure client3saved-move warnings retained. Added fixture spacing, never weakened cooldown.
- Barrier build PFM15VoiceBarrierReplayBuild.log PASS5.81s.
- M15VoiceMetadata1_20261014_233554939_d1cb238f FAIL1/2: clientSuccess0errors/0warnings/all3acknowledgements; server timeout stage4/ready0 after clientexit. Completion wait moved before readiness. Failed report retained.
- PFM15VoiceCompletionReplayBuild.log PASS5.68s, no compiler warnings.
- M15VoiceMetadata1_20261014_233831191_1f24e9c9 PASS2/2,39.99s,1.731–1.784working/1.632–1.720privateGiB; both role reports PF.Voice.AuthorityLive Success/testerrors/warnings0. Personal591files/settings unchanged.
- M15VoiceMetadata2_20261014_233932734_8d162526 PASS3/3,48.76s,75ms outgoing lag/1%loss,1.728–1.799working/1.625–1.786privateGiB. All actual roles PF.Voice.AuthorityLive Success/testerrors/warnings0, all3owned barriers perclient and near/far/mute/block/atomic malformed preference onserver. Personal593files/settings unchanged.
- Both successful live runs: Saved/AutomationReports/<run>/run-summary.json plus <run>Server/Client1[/Client2]/index.json and corresponding Saved/Logs/PF<run><role>.log. Raw24warnings14errors per role, no ensure/fatal,0new normalized severity versus PFM14HeightAuthoredClimb2_20261014_025936766_fcde88a7Client1.log. Existing editor widget factory registration, missing editor HLOD builder classes and ToolsetDefinition/PythonTestRunner startup errors are not new gameplay faults; retained, not fixed here.
- Final Game PFM15VoiceAuthorityGameBuild.log PASS23.44s, no compiler warnings. No unresolved failure after bounded repairs; five successful final live role records and3native records.

Known installed5.8.3 versus requested5.8.2 and separate prechange D3D12 GPU crash remain; no engine/settings change. Actualhost31.93GiB RAM, no16GiB certification.
