# M15 offline checkpoint ingress validation

October21,2026 — **PASS49/49 local tests**, Trello RgXzqdou. No deployed
Worker/Unreal feature, authentication, live migration or hosted gameplay pass.

## Changes and preservation

Tools/BackendFencingPrototype/ingress.mjs uses standard Web APIs only, no database,
filesystem, network or Node imports. A future route must authenticate its caller
before invoking it; expected world and minimum generation are trusted adapter
context, not identity verified by this helper. It currently has no runtime import.

The reader caps actual streamed UTF8 bytes at6MiB, checks optional Content-Length,
requires JSON POST/uncompressed body, enforces a default5second whole-body deadline
and max65536chunks. It cancels overflow/error/timeout bodies without blocking on
an underlying cancel promise, and also checks elapsed time during already-resolved
microtasks so empty chunks cannot starve the timer. UTF8 decoding is strict.

Validation checks canonical base64, actual SHA256, independent payload CRC32,
native little-endian32-byte PFS fileV1 header/magic/version, nonempty max4MiB payload,
exact declared length/generation, safe generation/floor, saved UTC/future60s bound,
world/context and expiration. Metadata is snapshotted before async digest and
returned immutable; original envelope/base64, UTC and generation are not rewritten.
The validator does not deserialize or authorize the inner world/player contents.

Draft SQL JSON payload budget is now6MiB, aligned with native transport capacity;
the existing4MiB file payload fits after base64 wrapping. This changes only the
quarantined prototype, not deployed SQL or Worker settings. Root baseline fixture,
original frozen Tests/Fixtures/M8_c3d221f and backend checkout remain untouched.
No new dependency beyond prior pinned test-only PGlite0.5.10 is introduced.

## Exact run results and retained failure

All logs under C:/UnrealSource/BuildLogs/:

| Run basename | Passed/failed | Result |
| --- | --- | --- |
| M15Ingress_20261021_First.log | 20/0 | Pure ingress first run,0.239s native/exit0/RSS121856000bytes |
| M15Ingress_20261021_BudgetRepro.log | 0/1 | Valid maximum envelope passes validator but draft1MiB JSON SQL cap refuses, false!=true/exit1 |
| M15Ingress_20261021_BudgetReplay.log | 1/0 | Only draft cap/bound test aligned to6MiB, exact failed selector replay1.630s/exit0 |
| M15Ingress_20261021_Combined.log | 49/0 | Combined suite2.299s native/exit0; command supplied reporter after filenames, so retained output is spec format |
| M15Ingress_20261021_Final.log | 49/0 | Package script pins TAP before filenames,2.296455s native/2.927608s wrapper/exit0 |

Expansion stopped after the budget failure, exact refusal reported, narrow fix
and failed-test replay passed before the combined gate. No suppressed assertion.
Final49definitions:29SQL/composed and20pure ingress. Zero skipped/cancelled/todo.
Legacy public fixture unchanged. Final sampled SQL process RSS627392512bytes
(0.584GiB), separate ingress process124260352bytes (0.116GiB); not concurrent sum,
not Worker-isolate heap,128MiB-runtime certification, Unreal memory/FPS/stutter or
minimum-machine certification. No Unreal build/launch needed (no C++/assets changed).

Additional SQL/composed tests after previous27:

1. maximum native envelope survives bounded validation and SQL commit.
2. validated historical ingress commits unchanged and corruption is refused before SQL.

Pure ingress20 exact passed titles:

1. genuine historical envelope validates independent SHA and CRC with exact saved UTC.
2. trusted world and rollback floor are required and independent of caller claims.
3. safe number and Neon string generations are supported without precision loss.
4. unsupported root version and payload formats are explicit refusals.
5. checksum syntax is canonical and actual SHA mismatch is rejected.
6. base64 must be canonical padded text with no whitespace or alternate alphabet.
7. truncated envelope, trailing bytes and forged declared payload length refuse.
8. resigned bad magic, file version, saved UTC and mismatched generation still refuse.
9. inner CRC is checked independently even when altered bytes have a matching SHA.
10. expiration must be present, real ISO timestamp, future or null.
11. async digest snapshots caller data and publishes immutable original metadata.
12. maximum native four-MiB payload validates within the six-MiB request budget.
13. split UTF8 stream preserves the exact envelope and counts bytes rather than characters.
14. method, content type and compression are refused before body processing.
15. declared oversized or invalid lengths are refused and body cancelled.
16. unknown-length stream overflow is bounded and cancelled without trusting a header.
17. a stalled body times out even if underlying cancellation never completes.
18. empty-chunk microtasks cannot starve the whole-body deadline.
19. malformed JSON and invalid UTF8 are explicit refusals.
20. stream errors and already-consumed requests fail without exposing source messages.

The genuine frozen2054-byte envelope has independent known SHA256
47d0a2035da74393511ea2f39463de0a4eea822fc0d822584e3799ab899a17ed,
generation1/SavedUtc1792032698 unchanged. Independent Node crypto/zlib construct
altered/synthetic fixtures; recomputing SHA for a bad CRC/header cannot make it pass.
Maximum-size synthetic data proves outer-envelope capacity only, not an actual
serialized playable world or load/food-expiry acceptance.

## Reproduce — AI, no level or manual action

1. PowerShell at C:/UnrealProjects/PrimalFrontier/Tools/BackendFencingPrototype.
2. pnpm install --frozen-lockfile --ignore-scripts (bundled pnpm path in README).
3. pnpm test. Require49passed/0failed/exit0, actual PG18.6/PGlite0.5.10 version,
   legacy public fixture unchanged and both process RSS lines. No credentials,
   server, client, editor or live database. First dependency installation needs
   public registry access; tests use only memory and the checked-in fake fixture.
4. Narrow ingress: node --test --test-reporter=tap ingress.test.mjs (20tests).
   Narrow maximum SQL replay: node --test --test-reporter=tap
   --test-name-pattern='maximum native envelope survives' fencing.test.mjs.
5. On failure stop, retain log, fix only current scope, replay that selector.

## Remaining gates

Actual Cloudflare isolate/request handling and memory, verified server/player
identity, schema roles/grants/RLS/ownership, Worker exact development endpoint
binding, simultaneous independent PostgreSQL sessions, lease-loss gameplay freeze,
remote restoration/world-schema validation and played restart/reconnect remain
unverified. PGlite is single-session; queued calls are not live contention proof.
Draft SQL remains SECURITY INVOKER/PUBLIC-revoked, not reviewed production grants.
SQL shape checks alone are not ingress authentication or file validation; no
existing Worker path has been upgraded merely by testing this standalone helper.

Receipt quotas/retention, legacy unfenced writer cutover, isolated migration and
durability timeouts still need review. The reader's deadline bounds body reading,
not a database operation. Do not deploy this folder or open hosted admission.
Next independent AI scope: a native server-side authority/admission state guard,
keeping actual hosted entry points closed until live restoration/identity gates.
Full hKddPkEg audit and M15 remain incomplete; no repeated owner setup request.

## Primary references and native source

[Cloudflare Web Crypto](https://developers.cloudflare.com/workers/runtime-apis/web-crypto/),
[ReadableStream](https://developers.cloudflare.com/workers/runtime-apis/streams/readablestream/),
[independent Node CRC32](https://nodejs.org/api/zlib.html#zlibcrc32data-value).
Native contract: Source/PrimalFrontier/Persistence/PFCheckpointProtocol.cpp and
PFSaveFileStore.cpp/.h; tests read the frozen genuine writer fixture unchanged.
