# M15 live backend access audit

## October20 — owner supplied explicit branch targets

The project-targeting prerequisite is resolved by the owner's browser URLs:

| Target | Owner-supplied identifier |
| --- | --- |
| Project | `aged-resonance-28932410` |
| Development branch — audit target | `br-red-fog-a7mp3ux7` |
| Production branch — leave untouched | `br-twilight-bread-a7rjhss5` |
| Database | `neondb` |

These are owner-supplied targets, not live-verified metadata or proof of the Worker's branch binding. Do not default to production when the development target is unavailable.

Current capability refresh: no Neon tools are exposed in this chat; fresh plugin discovery reports installed:false (status:ENABLED), superseding the earlier installed:true snapshot below. No project/schema/role query was attempted after this tool removal. Owner's only next action is enable/connect Neon for this chat and reply ready; do not resend IDs, recreate the database, change roles or paste credentials. Resume the read-only development audit with the explicit targets above when tools return. Prepared SQL remains unexecuted; no database/Worker/security change or new engine test.

## October20 connection refresh — current status

Owner replied **Neon connected**. Fresh plugin discovery confirms installed:true, status:ENABLED. The installed:false/installation prerequisite below is historical and superseded; do not reinstall.

Read-only describe_project without an ID returned `InvalidArgumentError: project_id is required because this connection is not scoped to a project. Pass the target project ID, for example one returned by list_projects.` This is a targeting failure before project inspection, not evidence of broken pf_app permissions or a missing database.

Current tool inventory includes describe_project, describe_branch and describe_table_schema, but no list_projects/list_branches/inspect_database/run_sql. Existing project-doc/backend searches found no verified project/branch ID. Computer Use inventory has only an empty in-app browser and MCP Apps, no Neon tab. No credential access, migration/destructive tool, cloud write or account navigation occurred.

Start here (owner): open your existing Primal Frontier project in the Neon console and paste its **browser URL** into chat. This supplies the non-secret project ID. No connection string/password/API key, new project/branch, role change or Unreal level/playtest is needed.

AI next: use the explicit ID for project metadata, identify the existing development branch, inspect the checkpoint table using supported read-only tools, and verify SQL capability before attempting the prepared metadata query. Worker branch identity and actual grants remain UNVERIFIED. Do not use migration tools to bypass unavailable read access or claim the unexecuted SQL passed. No runtime/C++ change or new build/test is needed for this access/documentation refresh; prior package/GSDK results remain separate.

October20,2026. **Cloudflare read-only checks PASS; direct Neon schema/role audit UNVERIFIED pending owner connection.** Local server/GSDK implementation is published3cdac3a; this audit changes no runtime, Worker, database, account settings or assets.

## Verified live evidence

Authenticated Cloudflare connection succeeds. Intended account3905225272dea5a3fe22f812bc925363, Worker primalfrontier-backend:

- Settings/secrets/deployments GETs each return200/success. Compatibility date2026-10-16. Names/types only inspected: ENVIRONMENT plain_text, NEON_DATABASE_URL secret_text, PF_API_SHARED_SECRET secret_text. No secret values read or printed, no binding/deployment mutation.
- Active deployment ae4a8e15-a12d-4d71-8f1b-7f313a077289, created2026-10-18T06:34:40.233227Z, version a8d144f2-975c-4c26-883f-70617ac65223 at100%. This is provider deployment metadata, not proof of byte equality with local TypeScript.
- GET https://primalfrontier-backend.jackherbig09.workers.dev/health → HTTP200, ok:true, service primal-frontier-backend, environment development.
- GET /v1/checkpoints/status → HTTP200, ok:true,status:READY,database:neondb. Existing code checks current_database and public.pf_checkpoints presence. This proves connectivity/table presence, not exact columns/constraints/branch identity/grants/leases or production readiness.

## Local contract and preserved files

C:\UnrealProjects\PrimalFrontier-Backend and its nested PrimalFrontier-Backend checkout both report HEAD3bc1ed251c0c4de1f54db04e4ee2b8b3f4028e3d (M15 public development status). Outer Git status already has untracked nested directory PrimalFrontier-Backend/. Inner status empty. Both were read only; no nested checkout cleanup/staging/commit, dependency install or deployment.

Checked-in001_initial.sql describes public.pf_checkpoints: world_id text, generation bigint>=0, envelope_version integer>0, checksum text, payload jsonb, created_at default now(), nullable expires_at, primary key(world_id,generation) and descending world/generation index. No writer-lease/fencing table or role grants appear in this file. **Actual live schema/grants are not assumed to match it.** Development Worker uses shared-secret authorization for checkpoint data, INSERT ON CONFLICT DO NOTHING and latest unexpired SELECT. This is a development transport, not authenticated per-player/realm admission, active-writer fencing or complete production input validation.

Before a future production scope, review leases/monotonic writer tokens, atomic generation updates/idempotent timeout reconciliation, original-envelope/expiry verification and admission-blocked restore, plus HTTP size/URI handling and real identity. No migration or design acceptance inferred here.

## One owner prerequisite

Plugin discovery on this task reports Neon available but **installed:false**; no Neon query capability is currently callable here. Cloudflare's live database connectivity is already working. Direct schema/role inspection needs an authorized Neon connection, not a replacement database or secret pasted into chat.

Start here (owner): install the **Neon** plugin in Codex, then finish sign-in/connection to the existing Neon account and Primal Frontier development project. Reply **Neon connected**. Do not recreate the project, rotate working secrets, change roles or rerun migrations for this step. No Unreal level or playtest required. If it is installed on another Codex device/session, enable it for this task's host/account rather than assuming its tools are available here.

After connection, AI will refresh tools/access (never reuse this blocker blindly), identify the intended development project/branch, and run [AuditCheckpointSchema.sql](../Scripts/AuditCheckpointSchema.sql) as a read-only transaction with bounded query/lock times. Inspect only database/role/schema/column/constraint/index/grant/policy metadata, never saved payloads/passwords. Confirm the Worker branch binding before any write; metadata/table absence on a different branch is not permission to create tables. Preserve existing checkpoint rows and least-privilege pf_app. Live results determine the next reviewed migration; no schema changes are prepared/applied on assumptions.

Prepared SQL has been source-reviewed against [PostgreSQL18 information functions](https://www.postgresql.org/docs/18/functions-info.html) and [read-only transactions](https://www.postgresql.org/docs/18/sql-set-transaction.html), **not executed or claimed passed**. No C++ change in this audit; no rebuild/test/Unreal launch needed. Prior local package/GSDK results remain [separate](PLAYFAB_GSDK_M15.md).
