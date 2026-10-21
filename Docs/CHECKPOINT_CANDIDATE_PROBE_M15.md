# M15 restricted database candidate probe

October21 2026. Bounded implementation/publishing task QvxVFMZZ PASS. Full M15
and runtime permission audit hKddPkEg remain incomplete. Owner reported
CHECKPOINT_LOGIN_READY: activation/private URL availability is complete as an
owner report, not an independently verified network login. Do not repeat it.

## Implemented and published

Separate repo C:/UnrealProjects/PrimalFrontier-Backend, main commit
c8a35756723bcb91d92e3ad49e1cbad5f789a0c7, remote SHA equality verified. Title
Milestone15, Codex GPT-6.1 Sol coauthor. Existing untracked nested checkout
PrimalFrontier-Backend/ preserved. No Unreal source, assets, engine settings,
private worlds, Neon grants/schema, active connection or paid resources changed.

New GET /v1/checkpoints/candidate/status uses optional encrypted runtime secret
PF_CHECKPOINT_DATABASE_URL_CANDIDATE. It never reads/falls back to
NEON_DATABASE_URL or selects which database existing checkpoint routes use.
Exact recorded Development host, pf_checkpoint_app, neondb, generated64hex
password, sslmode=require and no extra URL options are required. Existing Neon
HTTPS driver runs one static SELECT over catalog metadata, not checkpoint rows.
Nine exact boolean proofs cover database/role, flags, memberships, ownership,
database/public-schema rights, table owner/RLS and checkpoint grants. Require
SELECT/INSERT; refuse checked mutation/DDL/grant-option privileges. Response has
only ok/status, no-store and nosniff. Non-development404, non-GET405. Invalid
URL/metadata503 CANDIDATE_REFUSED, missing binding503 CANDIDATE_NOT_CONFIGURED,
driver/error/10-second deadline503 CANDIDATE_UNAVAILABLE. Driver exceptions are
never logged/returned. Timeout abort is not proof of server-side cancellation.

No new runtime dependency: pinned test-only Apache2 PGlite0.5.10 is reused from
the main offline prototype. Bundled Worker excludes PGlite. Generated bindings
are made by Wrangler using synthetic .dev.vars copied temporarily from example;
no private env file was present, overwritten or committed. Temporary copy was
removed. Missing-secret runtime behavior intentionally remains optional.

## Actual results and exact evidence

All logs below are in C:/UnrealSource/BuildLogs/:

- M15BackendCandidate_Tests_20261021_First.log:11/11,2.962s,exit0.
- M15BackendCandidate_Tests_20261021_Final.log:11/11,2.847s,exit0; no skips,
  cancellation or todo. SQL process peak516427776bytes RSS (about0.481GiB).
- M15BackendCandidate_Typecheck_20261021.log: retained initial TS6200 DOM/Worker
  type collision. ES2022-only lib correction exposed optional timer TS2345 in
  Typecheck_20261021_Final.log. Guarded timer cleanup; final
  M15BackendCandidate_Typecheck_20261021_Replay.log PASS/exit0. No skipLibCheck
  suppression. First types generation passed, Types_20261021.log.
- M15BackendCandidate_Bundle_20261021.log: Wrangler4.151.2 dry-run PASS/exit0,
 209.67KiB/gzip56.62KiB. Bundle in M15BackendCandidate_Bundle_20261021/ contains
  no PGlite/electric-sql/DB-WASM references. No upload during this dry-run.
- M15BackendCandidate_LocalResponses_20261021.json and
  M15BackendCandidate_LocalRuntime_20261021.log: actual workerd GET health200,
  GET candidate503/not-configured, POST candidate405, unauthenticated checkpoint
  GET401, active status503/not-configured with no local secrets. All five pass.
  Local runtime deliberately stopped after checks; no competing instance left.
- M15BackendCandidate_LiveResponses_20261021.json: deployed public health200/
  development, candidate503/CANDIDATE_NOT_CONFIGURED, active status200/READY/
  neondb at 2026-10-20T23:51:40Z (October21 Brisbane). Before push, candidate
  path was old401 auth fallback; new route is now serving. Exact provider
  deployment ID/Git build association unavailable through failed connector.
- M15BackendCandidate_LiveRoutes_20261021.json: deployed POST candidate405,
  unauthenticated checkpoint GET401. No candidate secret set or query made.

Tests (all PASS):
1. Exact metadata SQL executes under actual local restricted role on neondb,
   nine proofs true, saved seed rows unchanged.
2. Actual elevated role flags detected.
3. Actual membership/PUBLIC mutation grants detected.
4. Actual changed table security/missing table detected.
5. Development-only and absent candidate never falls back to old DB.
6. Expected candidate gives sanitized readiness/no-store and cleans up signal.
7. Wrong host/role/database/TLS/options refused before connecting.
8. Every missing/false/string proof, wrong rows/extra fields fails closed.
9. Driver exception containing synthetic credential is not returned.
10. Never-settling executor times out/aborts.
11. Real Worker route/method, health, missing configuration and old auth retained.

No new Unreal compilation or playtest required for this backend-only slice.
No rendered FPS/stutter/microphone/hosted restore or multi-client acceptance
inferred. Local tests use no real credentials or Neon network. Current public
active READY proves access/table visibility only, not current branch/role.

## Reproduce local checks (AI, no level or net mode)

Prerequisites: Node24, installed backend dependencies, no private .dev.vars/.env
for the empty-binding workerd checks. Start here: open terminal in
C:/UnrealProjects/PrimalFrontier-Backend.

1. npm run typecheck then npm test (bundled pnpm run typecheck/pnpm test were
   used on this machine where npm is absent from the interactive PATH).
2. Expect exit0 and11/11. Logs retain initial failures separately; stop on any
   new failure. PGlite initializes PostgreSQL18 locally, closes fixture clusters
   after validating unchanged saved seed rows; no Neon account is used.
3. wrangler deploy --dry-run --outdir dist; expect success. Inspect bundle for
   test DB dependency absence; do not use a deploy command for this check.
4. Exact workerd command/five expected routes and type-generation procedure are
   in backend README. No actual secrets should be supplied for those fixtures.

## Recovery if the private activation result was lost (Personal1)

The original password cannot be recovered from PostgreSQL. Do not rerun
`ActivateDevelopmentCheckpointLogin.sql`; it correctly refuses once LOGIN is
enabled. If the private URL was lost, open
`C:/UnrealProjects/PrimalFrontier/Scripts/RotateDevelopmentCheckpointLogin.sql`
in the Development `neondb` SQL editor as `neondb_owner` and run it once. It
requires the existing restricted login and rechecks its permissions before
rotating the password. On `PF_CHECKPOINT_LOGIN_ROTATED` plus successful COMMIT,
copy the entire returned URL privately. If it refuses, stop and report only the
redacted error. Do not paste the URL here.

## Start here — next owner action (Personal1)

No Unreal level, launch, PowerShell, SQL rerun or bearer token needed. The new
route is already deployed. Prerequisite: privately retained URL from the
successful one-time activation, for Development br-red-fog-a7mp3ux7/neondb.

1. If you had to rotate, use the new URL; otherwise use the original privately
   retained URL. Cloudflare > Workers & Pages > primalfrontier-backend > Settings >
   **Variables and Secrets**, outside the **Builds** section.
2. Add type **Secret**, name **PF_CHECKPOINT_DATABASE_URL_CANDIDATE**. Value is
   the complete privately retained postgresql:// URL, without quotes or psql.
   Deploy this secret change. Keep NEON_DATABASE_URL and PF_API_SHARED_SECRET
   unchanged. Do not paste the private value into chat/screenshots/Git.
3. Reply **CANDIDATE_SECRET_READY**. AI can then open the public candidate status
   URL once and report its JSON; owner can also open it in a browser.

Expected subsequent result HTTP200 {"ok":true,"status":"CANDIDATE_READY"}.
Any other result stops cutover. A 503 is a refusal, not a reason to reset the
login or replace the working URL. Return only status JSON if checking manually.
Why owner: chat does not hold the privately generated credential; provider
connector read-only calls currently fail internally, so private UI entry is
required. This action uses a separate encrypted runtime secret and leaves the
working connection active. Official current dashboard procedure:
https://developers.cloudflare.com/workers/configuration/secrets/.

Readiness certifies only this connection and listed metadata at query time;
not all grants/other schemas, schema migration, writes, independent branch
mapping, authenticated server identity, leases/fencing/true contention,
Unreal restore/admission or voice. A later reviewed cutover must preserve
rollback and verify the checkpoint data path before retiring the old credential.
