# Gather reward-window records — M12

October10,2026. This is a persistence/transaction prerequisite, not a playable gathering reward. The actual resource-node hook, active-world clock, reward feedback and multiplayer window delivery/restart still require a separate gate. Full M12 and Personal checks remain open.

## Contract

FPFProgressionRecord now supports at most five windows, keyed by exact native Gameplay Tags Progression.Gather.Wood/Stone/Fibre/Food/Water. The trusted pure transaction credits5XP for one successful event, up to five credits/category in1800active-server seconds. A gathering action with a better tool still represents one event, not one event per item/hit; this event hook is not connected yet. Existing item IDs map to known categories, crafted food/tools/unknown IDs do not.

Only complete validated candidates commit. Unknown/duplicate categories, invalid counts, NaN/Infinity/negative or oversized duration, inconsistent minimumXP and cap refusals preserve state. The ten-level2700XP cap and first-craft/knowledge accounting remain unchanged. At cap, no extra window is created. Trusted elapsed0 preserves exact state; expiry removes only the expired category, never XP. Another category never renews an existing window. Large finite elapsed values safely expire bounded entries without clock overflow. Pure elapsed is not a client timestamp or proof of active play; future integration must supply authoritative active-world time, never UTC, PF.SetTimeOfDay or offline elapsed.

Persist remaining duration/count/category only. Separate PFXP inner envelope supportsV1 andV2 within16KiB/16byteheader/CRC. If windows are empty, encode the originalV1 format byte-for-byte. Nonempty windows append a bounded array and useV2. OldV1 reads empty windows with no retroactive rewards; outer worldV2 owner binding and original playerV1 defaults remain unchanged. Mixed old/new inner records can coexist in one validated world. Unknown tags are resolved only from known native tags, never interned/requested from untrusted strings; ASCIIletters/dot and bounded lengths are enforced. CRC detects accidental corruption, not malicious authentication. Unknown future versions fail closed; failed writes/reads preserve all outputs.

This does not rewrite, load or migrate personal saves. Normal gameplay currently produces no windows and retains original inner bytes. No map/assets, gathering yields, input bindings, catalogs, client RPC or runtime clock changed. At this checkpoint an explicit trusted record Restore can carry windows but does not run their elapsed clock. Do not treat that fixture as earned progression.

## Evidence

Final technical record gate passed; full M12/Personal and live earning gates remain open:

| Check | Report/log | Result |
|---|---|---|
| Initial Editor | PFM12GatherWindowsEditorBuild.log |23.45s,Succeeded,no compiler warnings |
| Initial parser Editor | PFM12GatherWindowsFinalEditorBuild.log |5.62s,clean |
| Failed-only alias retry | Automation_M12GatherWindowsRetry_20261009_211830570_ffd6c325 | GatherWindows1/1,16.05s,working/private2.917/2.780GiB |
| Complete native record/runtime/compatibility | Automation_M12GatherWindowsFinal_20261009_211952205_e63e1b0b |5/5,16.52s,3.025/2.861GiB |
| Final focused parser replay | Automation_M12GatherWindowsParser_20261009_212045800_3aaacf59 | Codec/GatherWindows2/2,16.13s,2.938/2.801GiB |
| Final Editor | PFM12GatherWindowsParserEditorBuild.log |5.15s,clean |
| Final Game | PFM12GatherWindowsGameBuild.log |25.20s,Succeeded,no compiler warnings |

Five distinct cases: PF.Progression.Codec, Component, GatherWindows, Records, WorldCompatibility. Exact selected filters/no missing or extra records, engine/report0, all test errors/warnings0, no timeout. Both final unique raw logs actually reviewed:0warning/error/assert/fatal/ensure. Ordinary Saved/Logs/PrimalFrontier.log remains stale; unique logs are current evidence. Existing specific native no-skeletal-mesh fixture messages remain expected; no broad failure suppression.

GatherWindows tests cover independent budgets, sixth-event refusal, exact/just-before expiry, fractional duration, cap/accounting, duplicate/oversized/unknown buckets, malformed elapsed and unchanged outputs. Independent literal28byte legacyV1 zero record checks exact write compatibility; repeatedV2 reads/writes preserve all fields and do not advance/renew time. Correct-CRC corruption probes cover count/length/tag/NUL/nonASCII/duplicate/reward/duration/NaN/Infinity/truncation/missing second category/trailing/version fields. WorldCompatibility passes mixed innerV1/V2, existing owner binding/foreign-owner refusal and independent zero second-player data. Component uses explicitly trusted1234.5s window seed in an owned temporary Game world and real save files: repeated world load retains windows/XP/knowledge, V1 load defaults all new data without item retrocredit. It does not prove a running server clock or a real gather reward.

Retained failure: Automation_M12GatherWindows_20261009_211707369_b7dadaf4,22.85s,engine3/strict1,2.956/2.788GiB,missing report. Component logged Success before the new GatherWindows fixture crashed; the incomplete batch was not counted passed. Raw TArray alias assertion at PFProgressionGatherWindowTests.cpp:46: Add referenced its own container element. Fixed only the fixture with a copied value, rebuilt5.26s and immediately replayed the failed test clean. No assertion was waived. Later strengthened boundary tests/real-file seed required Editor14.58s and complete5/5; final read review moved the explicit pre-length guard into the correct gather parser loop, then focused2/2 passed.

## Replay and next gate

Start here: close competing Unreal processes and build PrimalFrontierEditor Development Win64. No opened level required. Run Scripts/RunNativeAutomation.ps1 -TestFilter 'PF.Progression.Records+PF.Progression.Codec+PF.Progression.GatherWindows+PF.Progression.WorldCompatibility+PF.Progression.Component' -Label M12GatherWindows. Require exactly five clean records, current raw-log review and engine/report0. Build Development Game after tests. Reports are Saved/AutomationReports/<run>/{run-summary.json,index.json}; raw Saved/Logs/PFAutomation_<label>_<timestamp>_<guid>.log. No screenshot/rendering claim for this data-only increment.

Next separate task: allow only the real successful authoritative gather transaction to prepare/commit a reward; active-world elapsed captured/restored safely, cooldown/cap/full bag/dead/client/foreign requests cannot award, time-of-day/offline/reconnect cannot reset windows. Update earned route assertions to actual added reward policy, run rendered earned route, one client before two NullRHI clients with independent owner windows/lag-loss/separate restart, and inspect logs. Do not grant mid-game acceptance from the pure numerical budget285XP/level3 (125gather+160firstcraft); availability and pacing need real earning tests and human observations.

Actual host31.93GiB/installed engine5.8.3 vs requested5.8.2. Native memory does not certify rendered FPS, hitching,16GB minimum or physical controller/navigation. No assets or user action needed for this record checkpoint. Trello https://trello.com/c/x19QhKcg; parent M12 remains Doing.
