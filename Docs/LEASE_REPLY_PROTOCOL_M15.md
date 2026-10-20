# M15 native draft lease-reply decoder

October21,2026. Trello jfiav3jF. **Native10/10 and offline50/50 PASS.**
This is an unwired parser, not an authenticated service, cloud restore, hosted
admission or a full M15 pass. No level/manual playtest is required for this pure
backend fixture; no player-facing feature or authored level was changed.

## Changes and contract

Hosting/PFLeaseReplyProtocol.h/.cpp decodes acquire, renewal and durable-commit
results from Tools/BackendFencingPrototype/contract.sql. No new dependency.
It requires completed contracted HTTP200 and a flat JSON object, at most4096
UTF8 bytes. Unknown/case-distinct/duplicate/escaped-duplicate fields, nesting,
trailing input, error-success mixtures, missing fields and malformed types fail.
Optional replayed must be boolean true; ordinary fresh results omit it.

Acquire requires the exact case-sensitive expected world, canonical lowercase
nonzero UUID, positive decimal-string fence through MAX_int64, valid UTC calendar
expiration and an explicit nullable head. Null remains TOptional-unset. Persisted
zero, unsafe/negative/fractional/exponent/leading-zero heads are refused, never
mapped to an empty world. Canonical safe positive JSON integers and Neon int8
strings are accepted through9007199254740991 without rounding numeric lexemes.
Renewal requires the exact expected lease/fence. Commit requires committed=true,
exact world/fence/generation and canonical sha256:lowercase checksum. Failed
decoding preserves caller output; fixed tooling errors never echo remote content.

Expiration is UTC metadata only (Z/+00:00, optional1–6fractional digits).
It never extends authority or compares service UTC to local wall clock. The
existing authority guard checks conservative monotonic send-time deadlines.
Neither decoder nor guard authenticates caller/server/player, binds HTTP to wire
request UUID, validates actual restoration or changes GameMode. A future trusted
adapter must authenticate, retain the original request nonce/send time across
retries, bind world/server/operation and current local callback, and fail closed
on uncertainty. Replayed receipts must never get a fresh TTL from retry/reply time.
The legacy deployed /v1/checkpoints201 reply is not this draft lease protocol.

Six immutable actual PostgreSQL replies are captured under
Tests/Fixtures/M15LeaseReplies. Their UUIDs/world/server are synthetic, not
credentials; commit used a synthetic payload, not a valid Unreal save. Fixture
provenance records PG18.6/PGlite0.5.10 and exact LF contract SHA256
37b282bd1620e2fffdf9c880b9fb2b3e55ce531ed05f0f100b0a1d6e9696617a.
The explicit capture-lease-replies.mjs generator refuses overwrite; normal tests
only read. New SQL regression compares actual six result keys/types and stable
values with the frozen fixture, including MAX_int64 fence and max-safe generation.
Existing PGlite dependency remains test-only; no Worker import/deployment.

## Retained failure and verification

First Editor build29.288s/exit0. Initial native run
Automation_M15LeaseReply_20261020_225008332_a9a13c7e:9/10,47.36s,
PF.Hosting.LeaseReply.Refusals fails one canonical UUID assertion, rawErrors2.
FString default comparison ignored case. Expansion stopped; decoder proof
comparisons changed to explicit case-sensitive equality. World/checksum case
regressions added. Rebuild6.050s and narrow failed-selector replay
Automation_M15LeaseRefusalReplay_20261020_225223594_ae38a10c:1/1,
17.08s/exit0/raw+test severity0. Combined10/10 replay then passes56.68s.
Final review also makes JSON member-name whitelist explicitly case-sensitive;
OK/REPLAYED mutations are refused. Final Editor6.389s and Game17.255s pass,
exit0/no compiler severity (earlier Game39.843s also passed).

Final native run Automation_M15LeaseReplyKeyFinal_20261020_225546345_b24ec8cc:
18.65s,10/10 Success, exit0, no missing/unexpected selectors, raw/test
warnings/errors/fatal/ensure0. Peak2.994GiB working/2.862private for one NullRHI
Editor; not rendered FPS/stutter or16GiB minimum certification. Exact definitions:

- PF.Hosting.LeaseReply.Binding
- PF.Hosting.LeaseReply.GuardBridge
- PF.Hosting.LeaseReply.NumericBoundaries
- PF.Hosting.LeaseReply.Refusals
- PF.Hosting.LeaseReply.SqlFixtures
- PF.Hosting.Authority.Durability
- PF.Hosting.Authority.ExpiryAndRenewal
- PF.Hosting.Authority.Initialization
- PF.Hosting.Authority.Recovery
- PF.Hosting.Authority.Refusals

GuardBridge uses explicitly synthetic trusted proofs in a value object; it creates
no actual authenticated identity/world and never opens real player admission.

Offline pnpm test:50/50 (30SQL/composed+20ingress),2.569s native/3.624s wrapper,
exit0/no skips/cancellations/todos, legacy public fixture unchanged. SQL process
peak644104192bytes RSS, separate ingress process see raw log; this is not Worker
isolate memory or genuine concurrent Neon-session certification.

## Reproduce / inspect

No level, server, client, cloud account or credentials required. Start here in the
project PowerShell after closing competing Unreal processes:

```powershell
.\Scripts\RunNativeAutomation.ps1 -TestFilter 'PF.Hosting.LeaseReply+PF.Hosting.Authority' -Label M15LeaseReply -TimeoutSeconds 180
```

Expect exactly ten named clean Success records above and exit0. Inspect generated
index.json, run-summary.json, HTML and raw log; any missing/failed/incomplete or
warning record fails. Native tests use actual fixture file, not a regenerated
expected result. To replay only initial failure select PF.Hosting.LeaseReply.Refusals.
For local SQL run pnpm test from Tools/BackendFencingPrototype (pinned install
instructions in its README); expect50 and LEGACY_PUBLIC_FIXTURE_UNCHANGED true.

Evidence: Saved/AutomationReports/Automation_M15LeaseReplyKeyFinal_20261020_225546345_b24ec8cc/
and Saved/Logs/PFAutomation_M15LeaseReplyKeyFinal_20261020_225546345_b24ec8cc.log.
Earlier named reports/logs remain. Build logs:
C:/UnrealSource/BuildLogs/PrimalFrontierEditor_LeaseReply{,Replay,KeyCase}_20261021.log,
PrimalFrontierGame_LeaseReply{,Final}_20261021.log and M15LeaseReplySQL_20261021.log.
Source Server explicit-unity build PASS59.955s wall/57.11s UBT/exit0/no compiler
severity, PrimalFrontierServer_LeaseReply_20261021.log and .result.json in the same
folder. Only our three new source/test files copied to existing isolated worktree;
own final incremental differences inspected and source hashes matched. Unity
sources explicitly include protocol/tests. No restage/upload or hosted run.
Installed/source actual engine5.8.3; no engine association change, assets,
private saves/settings, paid capacity, secret, database grant or deployed code
changed. Full live privilege/binding audit and authenticated adapter/restore/
true-contention/migration gates remain required before hosted gameplay.
