# Offline checkpoint writer-fencing prototype

**Not a migration or deployed feature. Do not run contract.sql on Neon.** This
folder is not imported by the Worker or Unreal. Its PostgreSQL database exists
only in memory during the Node test run. No credentials, database URLs, HTTP
clients or cloud SDKs are used. Keep hosted gameplay admission closed.

The one test-only dependency is Apache-2.0 `@electric-sql/pglite` **0.5.10**,
locked with integrity in pnpm-lock.yaml. The observed engine is PostgreSQL18.6
on WASM, Node24.19.0, pnpm11.25.0. No install scripts are needed. This folder
lives in the Unreal repository so publishing it does not enter the backend
repository's Worker deployment workflow. node_modules is ignored.

## Run the fixture

No Unreal level, server, client, editor or account setup is required.

```powershell
cd C:\UnrealProjects\PrimalFrontier\Tools\BackendFencingPrototype
pnpm install --frozen-lockfile --ignore-scripts
pnpm test
```

On this host, pnpm is available at
`C:\Users\jackh\.cache\codex-runtimes\codex-primary-runtime\dependencies\bin\fallback\pnpm.cmd`.
Node is on PATH; use the full pnpm path if required. First installation needs
the public package registry. Test execution itself needs no network.

Expect **50 tests PASS** (30SQL/composed +20ingress), process exit0, `LEGACY_PUBLIC_FIXTURE_UNCHANGED true` and
the actual database version/RSS printed. On any assertion/SQL failure, stop;
preserve output, fix this bounded prototype and replay the failed selector.
Use `--test-name-pattern="exact test title"` before the filename for a narrow
replay. `--test-concurrency=1` intentionally bounds memory.

## Contract

The additional SQL test compares six frozen replies used by native lease decoding
against current actual SQL keys/types/values. `capture-lease-replies.mjs` is an
explicit one-time generator; normal tests never overwrite the synthetic fixture
at Tests/Fixtures/M15LeaseReplies. It is a reply-shape test, not an authenticated
service or valid native save payload. Contract bytes use LF for a stable recorded
SHA256. See Docs/LEASE_REPLY_PROTOCOL_M15.md for native results and limits.

- One retained `worlds` row per world serializes acquire, renew, release and
  checkpoint commit using `SELECT ... FOR UPDATE`. Never delete that row to
  release a lease. A new acquisition advances a non-reused signed64-bit fence
  and gets a database-generated UUID. Wire fence values are decimal strings,
  avoiding JavaScript precision loss.
- A database-clock deadline controls liveness; expiry is checked **after** the
  world-row lock is taken. Do not accept a clock/deadline from a caller or use
  transaction-start `now()` for this check. The operation linearizes at its
  successful locked guard; a later takeover waits for the transaction.
- The server, world, lease UUID and fence must all match before any mutation.
  A dead/retired epoch cannot commit, renew or release a newer writer's state.
  Release is same-epoch idempotent and preserves both fence and durable head.
- Persistent request receipts make acquire/renew/commit retries idempotent;
  altered arguments under the same operation/request UUID refuse. Acquisition
  retries never allocate a second epoch or silently renew an expired one.
  Renewal retries return the original deadline and do not prolong it again.
  A client must check the returned deadline even on an OK replay: an older
  receipt can be conservative after later renewals. Never infer renewed
  authority from an old response or from a network timeout.
- Commit checks the exact expected head, requires a newer original envelope
  generation (gaps allowed), and inserts immutable checkpoint plus new head and
  receipt in one transaction. Rollback discards all three. Duplicate request
  replay does not change payload, checksum, creation time or expiry.
- A new writer sees the durable head on acquire and can inspect/restore that
  checkpoint through a future authenticated adapter. The prototype does not
  restore Unreal worlds or open admission. After takeover, old commit retries
  refuse even if the old write was durable; reconciliation must occur through
  the current writer's verified read path.

The copied baseline.sql is the backend's actual migrations/001_initial.sql at
3bc1ed251c0c4de1f54db04e4ee2b8b3f4028e3d, SHA256
`C47998E61F8563BE44AD0738BEE820D38A6280DB0C947B34D7B51A2371583386`.
Tests apply that baseline only to their new in-memory database, preserve a
public legacy fixture, and use separate `pf_fencing_draft` tables/functions.
Existing envelope columns retain their meaning. The prototype's first head is
seeded from pre-existing generations in the throwaway checkpoint table.

## Explicit limits before any rollout

PGlite has one PostgreSQL session. Promise.all cases exercise queued SQL
interleavings, **not simultaneous independent sessions or Neon contention**.
Live permissions, RLS, schema ownership, actual Worker branch/role binding,
multi-session blocking/serialization, networking/timeouts, crash/restart disk
durability and isolated Neon migration still need separate verification.

Caller identity here is a supplied fixture server ID, **not authenticated
PlayFab/EOS identity**. No HTTP route or token verifier exists. SQL validates
checksum shape/immutable metadata. The separate `ingress.mjs` offline validator
now checks actual SHA256, CRC32, native fileV1 header/size/generation/original UTC,
canonical base64, expiry and trusted-context world/floor. Its streaming reader
enforces6MiB UTF8 bytes, max65536chunks and a default5second whole-body deadline,
cancels refused bodies without awaiting a stalled cancellation, and snapshots
data before async crypto. It rejects compressed bodies and requires JSON POST.
These helpers are not imported by the deployed Worker and do not authenticate
caller context or validate the inner game-world serialization schema.
The SQL checksum spelling is the exact native `sha256:` plus lowercase64hex;
bare/uppercase/incorrect-length forms refuse. A genuine frozen Unreal fixture
is also committed/retried in memory with its independent known SHA256, original
base64 bytes, generation and saved UTC unchanged. This does not validate every
incoming file: SQL still treats the payload as immutable JSON metadata.

The trusted future adapter must verify capability/world/server identity and
invoke envelope validation before SQL. Never expose these
functions as a public API or accept caller-selected server IDs as authority.

The draft uses SECURITY INVOKER and explicitly revokes PUBLIC schema/table/
function access. An untrusted fixture role is tested for refusal. This is not
the eventual runtime role/grant/SECURITY DEFINER design, and does not prove
that live pf_app is restricted. Direct table grants could bypass fencing.
Production role separation/search_path/functions require a reviewed migration.

Other unresolved rollout work: disable legacy unfenced writes atomically at
cutover; validate/import historical generations; define receipt retention and
quotas (no cleanup here); verify the aligned6MiB request/SQL JSON budget and
4MiB native payload limit in an actual Worker isolate; rate-limit
request IDs; reconcile expired/deleted checkpoint receipts conservatively;
restore original timestamps/ownership and reject corrupt/expired saves; freeze
server gameplay on unknown/lost lease; require authenticated player admission.
No automatic migration, cloud settings/deployment or paid capacity is included.

Evidence/remaining live audit: [ingress results/failures](../../Docs/CHECKPOINT_INGRESS_PROTOTYPE_M15.md),
[M15 SQL prototype results](../../Docs/CHECKPOINT_FENCING_PROTOTYPE_M15.md),
[live metadata audit](../../Docs/BACKEND_ACCESS_AUDIT_M15.md).

## Sources

- [PostgreSQL18 row locks](https://www.postgresql.org/docs/18/explicit-locking.html).
- [PostgreSQL18 actual versus transaction clock](https://www.postgresql.org/docs/18/functions-datetime.html).
- [PGlite API and transactions](https://pglite.dev/docs/api).

These informed the local design; they do not substitute for the missing live
concurrency/security tests.
