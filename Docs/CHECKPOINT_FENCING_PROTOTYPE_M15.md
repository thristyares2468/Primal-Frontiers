# M15 offline checkpoint writer fencing

**Current checkout expected count:62/62 via pnpm test (30SQL/composed +20ingress +12role-preparation/safety).**
The twelve role fixtures are a separate synthetic database, not live migration/permission acceptance; see [role preparation](BACKEND_ROLE_RESTRICTION_M15.md).
The extra SQL/native reply-shape bridge is documented in [lease replies](LEASE_REPLY_PROTOCOL_M15.md); historical49 results below remain unchanged.
[Ingress follow-up](CHECKPOINT_INGRESS_PROTOTYPE_M15.md) adds actual SHA/CRC/header
validation and max-envelope SQL integration; draft JSON cap is now aligned6MiB.
Earlier25/27counts and1MiB mismatch discussion below are historical run evidence.
Runtime/deployment/auth/live SQL limits remain; do not deploy the draft.

## October21 native wire-format correction — PASS27/27

Follow-up task AusYLCpp found draft900cd16 checksum shape accepted bare64hex,
whereas actual PFCheckpointProtocol emits `sha256:` plus lowercase64hex.
Native-format selector reproduced0/1 FAIL/exit1 (false != true) in
C:/UnrealSource/BuildLogs/M15FencingWire_20261021_Repro.log. Expansion stopped;
only draft regex/fixture checksum corrected. Narrow replay1/1 PASS/exit0,
M15FencingWire_20261021_Replay.log (1.447s), then26/26 regression PASS/exit0,
M15FencingWire_20261021_SQLRegression.log (1.776s).

Added genuine frozen Unreal fixture commit/retry proof using unmodified
Tests/Fixtures/M8_c3d221f/fixture.json, SHA256
47d0a2035da74393511ea2f39463de0a4eea822fc0d822584e3799ab899a17ed.
Original2054bytes/base64, generation1 and SavedUtc1792032698 remain exact.
Final27/27 PASS/exit0, no skipped/cancelled/todo,1.808s native/2.035wrapper,
peak649109504bytes RSS, legacy public fixture unchanged. Exact final log:
C:/UnrealSource/BuildLogs/M15FencingWire_20261021_Final.log.

Additional passed titles: native Unreal sha256-prefixed checkpoint checksum is
accepted and bare hex refused; genuine frozen Unreal upload preserves historical
bytes, generation and saved UTC across commit retry. Bare/uppercase/wrong-length
checksum forms refuse. Prior25tests below remain historical local SQL evidence;
use current27 expected count for reproduction. These tests verify a known
fixture's fidelity, not arbitrary input checksum/file validity or live cloud
authority. All original single-session/security/deployment limits remain.

October21,2026 — **local prototype PASS25/25; no deployed gameplay feature**.
Trello [bounded task kGQxQHIS](https://trello.com/c/kGQxQHIS).

## What changed

Added quarantined Tools/BackendFencingPrototype: SQL contract, Node native tests,
exact copied baseline, pinned test-only PGlite0.5.10/lockfile and no-level guide.
Actual PostgreSQL18.6 runs in memory; no cloud credentials, Worker import,
migration registry, runtime C++ or assets. Prototype was initially prepared in
the backend checkout, then its own source files moved into this repo before
publication to stay outside the Worker repository's Git deployment workflow.
Backend remains HEAD3bc1ed2 with only its pre-existing nested untracked checkout.

The contract serializes each world's operations under a retained row lock,
checks expiry with the actual database clock, advances monotonic string-encoded
fences on new acquisition, and couples checkpoint insert/head/receipt in one
transaction. Retries preserve original metadata; altered replay arguments,
stale writers and stale expected heads refuse. Release retains epoch/history;
PUBLIC access is revoked. See [full contract and limits](../Tools/BackendFencingPrototype/README.md).

## Exact verification

Runtime Node24.19.0; pnpm11.25.0; Apache-2.0 PGlite0.5.10, PostgreSQL18.6 WASM.
Dependency installation with --ignore-scripts and final --frozen-lockfile pass.
Copied baseline SHA256 C47998E61F8563BE44AD0738BEE820D38A6280DB0C947B34D7B51A2371583386
matches actual backend migrations/001_initial.sql. No new production dependency.

| Run | Passed/failed | Native duration | Evidence |
| --- | --- | --- | --- |
| Initial scope | 22/0 | 1749.4391ms | C:/UnrealSource/BuildLogs/M15Fencing_20261021_First.log |
| Expanded clock/PUBLIC/history | 25/0 | 1795.4754ms | C:/UnrealSource/BuildLogs/M15Fencing_20261021_Final.log |
| Final source location/baseline replay | 25/0 | 1941.0419ms | C:/UnrealSource/BuildLogs/M15Fencing_20261021_PublishedPath.log |

All runs native exit0, no skipped/cancelled/todo or failed assertions; no failed
suite to hide. Final wrapper2.181s; final sampled Node RSS652099584bytes (0.607GiB).
Initial/expanded peaks642138112/649191424bytes. Legacy public fixture unchanged
including original JSON payload. This is local process memory, not Unreal FPS,
game stutter, hosted-server resource usage or16GiB certification. No Unreal
build/launch required because no C++/runtime/asset files changed.

Final25 exact passed tests:

1. acquisition returns a database-owned epoch, bounded deadline and empty head.
2. queued competing acquisitions yield exactly one owner.
3. same request retry preserves original lease/deadline and rejects changed arguments.
4. an expired acquisition request cannot silently reacquire.
5. takeover fences old writes, renewals and releases even for the same server identity.
6. expiry rejects commit and renewal before any takeover.
7. release is idempotent, preserves the head and never recycles a fence.
8. renewal is idempotent and does not shorten or repeatedly extend the deadline.
9. world and owner are both validated; no cross-world capability reuse.
10. lost commit acknowledgment reconciles exactly once without changing original envelope fields.
11. same commit request cannot substitute payload, checksum, expiry, version or generation.
12. fresh requests cannot overwrite a generation or regress the durable head.
13. queued same-base commits allow only one compare-and-swap winner.
14. a request identifier may be reused in another world without sharing a receipt.
15. takeover preserves already committed checkpoints but rejects old acknowledgment retries.
16. pre-existing historical generations seed the first head without being rewritten.
17. rollback discards checkpoint, head update and receipt as a single unit.
18. invalid acquisition identity, TTL or request never creates state.
19. invalid checkpoint shapes, numbers and digest lengths leave all state unchanged.
20. expired or infinite checkpoint retention is refused; maximum safe generation is preserved.
21. large fencing epochs are exact strings and exhaustion fails without wraparound.
22. missing worlds and malformed typed arguments cannot create a checkpoint.
23. expiry uses the actual database clock even inside an older transaction.
24. prototype has no PUBLIC entry point and untrusted role cannot acquire or edit state.
25. long takeover chain preserves immutable history and never admits a retired epoch.

## Reproduce — AI, no level required

1. Open PowerShell at C:/UnrealProjects/PrimalFrontier/Tools/BackendFencingPrototype.
2. Run pnpm install --frozen-lockfile --ignore-scripts (use bundled full pnpm path
   from README on this host). Public registry needed only for first installation.
3. Run node --test --test-concurrency=1 --test-reporter=tap fencing.test.mjs.
4. Require25passed/0failed, exit0, actual DATABASE_VERSION, legacy fixture unchanged
   and memory line. Every database/role/schema belongs to that disposable memory
   instance. Tests take roughly2seconds; no credentials, engine or saved worlds.
5. Failure: stop expansion, retain log, replay affected exact title using Node's
   --test-name-pattern before the filename after fixing only the failed scope.

## Boundaries and next gates

These25 tests are **single-session embedded PostgreSQL**. Promise.all means
queued requests, not two independently overlapping database sessions. True row
contention, Neon runtime privileges/RLS/ownership/Worker branch binding, original
envelope SHA/file validation, authenticated server/player identity, live network
timeouts and crash/restart disk durability remain unverified. Caller server IDs
are fixture inputs, not a trusted authentication implementation. No HTTP endpoint
or automatic restoration/admission was added. PGlite is not the Unreal server.

Draft functions are SECURITY INVOKER and PUBLIC-revoked; live runtime role/grants
are a separate reviewed security boundary. No security-definer grants, legacy
write-route cutover, receipt cleanup/rate limits, or production body-budget
agreement is included. Draft JSON payload cap1MiB differs from current Unreal
response cap6MiB; do not enable it without envelope/body-budget work. No active
world row may be deleted to recycle an epoch. Frozen receipt replies may retain
older deadlines; the future server must check expiry and freeze on uncertainty.

Full live audit hKddPkEg remains incomplete; current Neon tools unavailable,
last verified development schema snapshot97c1809 retained. Do not repeat owner
account setup or ask for secrets; never use a write/migration tool to bypass
missing reads. Before any rollout: finish permissions/binding audit, test a
reviewed migration on a separately approved isolated development branch, verify
multi-session contention and legacy route closure, then implement admission-
blocked original-envelope restoration and authenticated ownership. FullM15 and
hosted gameplay remain incomplete/closed; no cloud deployment/capacity change.

## Primary design references

[PostgreSQL18 locking](https://www.postgresql.org/docs/18/explicit-locking.html),
[actual versus transaction clock](https://www.postgresql.org/docs/18/functions-datetime.html),
[PGlite API/transactions](https://pglite.dev/docs/api).
References inform the contract; actual evidence is the local SQL test output.
