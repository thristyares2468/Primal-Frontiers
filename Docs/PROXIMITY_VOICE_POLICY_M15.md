# M15 proximity voice routing policy foundation

Trello: https://trello.com/c/f5MTwa3W. This is metadata policy, not playable voice chat. No microphone capture, audio packets, decoder/playback, service, plugin, dependency, RPC, UI, map, save or device setting was added.

## Contract

`FPFProximityVoicePolicy::Evaluate` accepts a trusted caller-provided server snapshot, never a client position or identity claim. It returns identity-sorted recipient decisions, excludes self, and leaves output unchanged on invalid input. At most32 participants; finite coordinates within1e12cm, valid unique public GUIDs and space GUIDs, finite0–1 receiver volume, bounded distinct nonself mute/block lists and a finite range no larger than100m. Unknown valid preference IDs remain inert.

Default disabled; transmit consent, receive consent and push-to-talk defaultfalse. Missing/disconnected/dead participants or different spaces cannot hear. Receiver mute only suppresses that receiver; either participant's block prevents the route. Receiving does not require microphone consent. Full volume through3m, linear fade to15m, zero at/beyond15m; receiver volume multiplies gain. Defaults are provisional native tuning, not a final world acoustic/occlusion design. Configuration enablement alone does not grant consent.

This pure helper cannot prove caller authority. The next separately gated adapter must derive the roster, positions, life and identities from real authoritative controllers/player states/pawns and reject client worlds. No private reconnect credential, waveform or preference history is logged/saved.

## Reproduce

No level required: isolated native Unreal Editor commandlet fixtures, NullRHI, no devices. Start here from the repository PowerShell:

```
.\Scripts\RunNativeAutomation.ps1 -TestFilter 'PF.Voice.Policy+PF.Voice.PolicyValidation' -Label M15VoicePolicy -TimeoutSeconds 180
```

Close competing Unreal instances first. Expected exactly two clean Success records, no missing selectors/test warnings and exit0. Inspect index.json/run-summary.json and the corresponding PFAutomation log; any missing, failed or incomplete test is a failure. No manual listening test is available for this helper.

## Evidence — October15

- Baseline Editor PFM15VoiceBaselineBuild.log PASS23.36s before changes.
- Editor PFM15VoicePolicyEditorBuild.log PASS12.94s, forced source discovery, no compiler warnings.
- Saved/AutomationReports/Automation_M15VoicePolicy_20261014_232351964_21d5f49c/index.json and run-summary.json: PASS2/2,19.12s,2.996working/2.861privateGiB. PF.Voice.Policy covers consent/PTT/near/far/volume/mute/block/offline/life/space/order; PF.Voice.PolicyValidation covers malformed bounds/identities/preferences/32cap and atomic refusal.
- Saved/Logs/PFAutomation_M15VoicePolicy_20261014_232351964_21d5f49c.log: readable [PrimalVoice] evidence, raw warnings/errors/fatal/ensure0, exit0. Headless run offers no FPS/stutter/listening measurement.
- Game PFM15VoicePolicyGameBuild.log PASS30.86s, no compiler warnings. No dependency or plugin added.

Installed engine reports5.8.3 in UE_5.8 despite requested5.8.2; distribution unchanged. Actual machine31.93GiB RAM/RTX4060 8GB. Ordinary Saved/Logs/PrimalFrontier.log is a separate pre-change rendered Editor session ending23:00UTC October14 with D3D12 DEVICE_HUNG/device-removed and GPU page fault; do not publish its private crash/account context. This change neither caused nor fixes that crash.

FullM15, transport security/bandwidth, Internet identity, packaged server, physical devices and intelligible proximity listening remain separate gates. No user acceptance was inferred.
