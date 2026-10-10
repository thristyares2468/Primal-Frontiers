# Active-time gather reward clock — M12

October10,2026. Technical lifecycle gate passed, not a live gathering XP source or full milestone acceptance. Trello https://trello.com/c/6JeKDMcU. GATHER_WINDOWS_M12.md records the preceding pure record/codec prerequisite.

## Behavior

The authoritative PlayerState component stores a server-only game-time epoch beside the replicated progression record. Capture computes elapsed Unreal UWorld::GetTimeSeconds and advances a candidate's remaining durations; it never mutates the stored record/epoch or awards XP. Multiple captures at the same time produce the same result, and later captures subtract total elapsed only once. Timers freeze with paused game time and follow time dilation; real/UTC time and time-of-day are not inputs. No per-frame tick, timer or countdown replication was added.

Valid restore rebases saved remaining durations onto the current server epoch. Time spent offline does not consume or renew the reward budget. Invalid/backward/nonfinite clock, malformed record, client or invalid authority/catalog refuses with unchanged record/output. Knowledge purchase and actual successful first/repeat craft conversion prepare from Capture and commit the same aged candidate/epoch; refusals and failed conversions cannot reset a timer. A clock-only repeat commit does not emit a misleading first-craft reward log. XP, counts, knowledge, save wire versions and owner-only replication policy remain unchanged. The raw replicated record holds durations relative to its last commit, not a live client countdown; authoritative persistence uses Capture.

Manual authoritative whole-world reload intentionally restores the saved world checkpoint, including its remaining budget. It is not an untrusted client reset operation. Reconnecting must preserve counts and remaining time through trusted persistence; actual new-process window delivery/restart remains a separate network gate before certifying the real gathering source.

## Verification

| Gate | Evidence | Result |
|---|---|---|
| Editor Development Win64 | Saved/Logs/PFM12GatherClockEditorBuild.log | Succeeded25.14s, no compiler warnings |
| Native clock | Automation_M12GatherClock_20261010_032832686_3419961d | PF.Progression.GatherClock1/1,35.58s,working/private3.080/2.966GiB |
| Affected regressions | Automation_M12GatherClockRegression_20261010_032937032_ec7be8cd |6/6,16.64s,2.933/2.784GiB |
| Game Development Win64 | Saved/Logs/PFM12GatherClockGameBuild.log | Succeeded27.02s, no compiler warnings |

Six regressions: PF.Progression.Codec, Component, GatherWindows, KnowledgeRequests, RecipeAccess, WorldCompatibility. Reports under Saved/AutomationReports/<run>/{run-summary.json,index.json}; raw logs Saved/Logs/PFAutomation_<label>_<timestamp>_<guid>.log. Exact selector reconciliation, engine/strict exits0, all test warnings/errors0 and no timeout. Both unique raw logs reviewed for warning/error/fatal/assert/ensure; none found. Ordinary Saved/Logs/PrimalFrontier.log is stale at October9 04:53UTC; unique logs are current evidence. No failed build/test in this slice.

GatherClock uses a disposable native Game world, trusted100XP/category records and public Unreal clock fields only inside its fixture. It checks exact fractional category expiry, repeated snapshots, no stored mutation, an actual paused-world tick, malformed/backward/negative/NaN/Infinity/client refusal with preserved output, actual temporary file write/read, a simulated new epoch, failed-restore countdown conservation and real owned purchase/timed first/repeat craft hooks. No free rewards and legacy empty restore. Simulating a new epoch does not prove a process restart, earned pacing or network delivery. No screenshot/rendered feature claim applies to this unhooked data lifecycle.

## Replay / next task

Level: NONE; native temporary Game worlds only. Start here: close competing Unreal processes and build PrimalFrontierEditor. Run Scripts/RunNativeAutomation.ps1 -TestFilter 'PF.Progression.GatherClock' -Label M12GatherClock, then the six selectors above in one plus-separated filter, -Label M12GatherClockRegression. Require exactly1+6 clean tests/current raw-log review; build PrimalFrontier Development Win64. Generated fixture files are created/removed through normal test APIs, not personal-save edits.

Next bounded task: actual successful server resource-node transaction prepares a reward before inventory conversion and commits only after success. Verify one5XP event regardless of tool yield, five-credit category budget, full/dead/client/foreign/cooldown/cap refusal, independent categories and no award for grants/pickup/drop. Update genuine earned-route expectations, inspect rendered HUD and test one client before two NullRHI clients with owner-private windows/countdown/new-process restart. No human pacing/combat feel/controller/FPS/minimum16GB pass or M13 advancement inferred.

Actual installed engine5.8.3 vs requested5.8.2; host31.93GiB. Native process memory is not a rendered performance or16GB certificate. No art/assets/maps/private saves/settings/plugins changed. Full M12 and existing Personal acceptance stay open.
