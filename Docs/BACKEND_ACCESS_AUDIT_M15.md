# M15 live backend access audit

October21,2026. **Owner role result received: least privilege FAILS.** Existing connection/schema metadata verified; exact Worker branch binding remains UNVERIFIED. Installation/URL prerequisites stay resolved. Full M15 is incomplete; hosted gameplay stays closed.

## October21 owner browser inspection

Computer Use stopped because it could not confidently identify the Windows browser URL; no bypass attempted. Owner offered assistance, confirmed NEON SQL READY on Development/neondb and pasted the read-only SELECT result. [Exact sanitized result/provenance](BACKEND_ROLE_OWNER_RESULT_M15.json): pf_app exists, table owned by neondb_owner, schema USAGE true/CREATE false, SELECT/INSERT/UPDATE/DELETE true/TRUNCATE false, RLS/forcedRLS false, superuser false, CREATEDB/CREATEROLE true and direct membership neon_superuser. This is owner-provided SQL evidence, not a new direct-agent query or cryptographic branch proof.

Current backend3bc1ed2 src/index.ts only reads status/latest and INSERT ON CONFLICT DO NOTHING RETURNING. Runtime needs SELECT+INSERT/schema USAGE/DB CONNECT; broad administrative membership and UPDATE/DELETE are unnecessary. Neon documents Console/API/CLI-created roles receive neon_superuser and recommends SQL-created restricted roles for application access: [roles](https://neon.com/docs/manage/roles), [database access](https://neon.com/docs/manage/database-access). Do not infer least privilege from role name or successful connection. Do not enable RLS blindly or revoke owner/PUBLIC privileges. Prepare a tested narrow transactional correction first; no live changes made and exact Worker branch/role binding still unverified. Legacy full AuditCheckpointSchema.sql remains unexecuted; only the supplied subset has actual owner output.

Owner's second query now reports BOTH neon_superuser memberships (neondb_owner and pf_app) granted by cloud_admin, admin_option=false, inherit/set=true. Grantor-query prerequisite is resolved; no additional read-only grantor action needed. Do not try forcing an in-place managed-role demotion. Local8failure-safety tests proved that an unmanageable grantor refuses or rolls back rather than silently passing. Current [tested preparation](BACKEND_ROLE_RESTRICTION_M15.md) instead creates separate SQL-owned pf_checkpoint_app NOLOGIN with SELECT/INSERT only, retaining old managed app/connection/data. Operator execution and later login/Worker credential cutover are still pending; no password or Worker binding changed.

## Verified development target

Authenticated project description succeeds for project `aged-resonance-28932410`, named `primalfrontier-dev`, region `aws-ap-southeast-2` (Sydney), PostgreSQL18. Explicit development description confirms name `Development`, branch `br-red-fog-a7mp3ux7`, parent `br-twilight-bread-a7rjhss5`, non-default and not protected. Database inspected: `neondb`. Production branch was not queried directly or modified. Project metadata necessarily identifies the parent; it does not verify production schema/data.

Three supported metadata reads succeeded: describe_project, describe_branch, describe_table_schema. Returned schema is preserved without owner contact information, credentials or saved payloads in [BACKEND_SCHEMA_SNAPSHOT_M15.json](BACKEND_SCHEMA_SNAPSHOT_M15.json). Snapshot recorded2026-10-20T03:19:41Z. Project metadata reports subscription launch_v3, defaults0.25–8CU/suspend_timeout_seconds0; these are project defaults, not proof of active endpoint settings, billing or a free-cost guarantee. No plan/compute settings changed or hosted capacity allocated.

## Actual checkpoint contract

Development public.pf_checkpoints matches checked-in backend migrations/001_initial.sql:

| Column | Type | Null/default |
| --- | --- | --- |
| world_id | text | NOT NULL |
| generation | bigint | NOT NULL |
| envelope_version | integer | NOT NULL |
| checksum | text | NOT NULL |
| payload | jsonb | NOT NULL |
| created_at | timestamp with time zone | NOT NULL, now() |
| expires_at | timestamp with time zone | nullable |

Primary key is(world_id,generation). Checks require generation>=0 and envelope_version>0. Unique btree primary index and pf_checkpoints_world_latest_idx(world_id,generation DESC) are present. Five comparisons against the actual returned metadata pass: exact seven-column contract, primary key, generation bound, version bound and latest index definition. These are metadata checks, not Unreal gameplay tests or write-path tests. Plugin output additionally labels PostgreSQL18 NOT NULL constraints as CHECK; nullability was independently compared through returned column flags.

The development object tree returns pf_checkpoints, both indexes and show_db_tree function; no pf_world_leases table is shown. This is an object-tree observation, not a cluster-wide proof against differently named mechanisms. Reported table size0/index24kB is not a certified row count; no payload/row SELECT was performed by the agent.

## Remaining audit limits and next AI work

Current exposed Neon tools have no run_sql/inspect_database; local Get-Command found neither neon nor neonctl. Full [AuditCheckpointSchema.sql](../Scripts/AuditCheckpointSchema.sql) remains UNEXECUTED. The owner subset above now reports table owner, role flags/direct membership, RLS flags and effective schema/table privileges; these are no longer wholly unknown, but their least-privilege assessment FAILS. Other ownership, recursive/SET memberships, policy metadata, direct replication/bypass flags and exact Worker binding remain to verify. Project ADMIN capability is not runtime pf_app authority. Do not infer least privilege from connection success or schema descriptions.

The Cloudflare encrypted runtime binding cannot establish branch/role identity from its name alone. The exact Worker endpoint/branch/role binding remains UNVERIFIED; public READY/neondb is insufficient because both branches can have neondb and pf_checkpoints. Do not change credentials, grant permissions or apply a migration on that assumption. No production query, migration/grant/deployment/security change was requested. Supported description tools may have their own internal implementation; this audit certifies only the returned metadata, not their internal transaction mode.

Next independent AI work can prepare a reviewed offline writer-lease/fencing and idempotent checkpoint contract using this observed schema. Before any live schema/deployment change, finish metadata permission/binding inspection using an available authorized read-only SQL path; test migrations on a separately approved disposable development branch. Never invoke destructive/migration tools as a workaround for missing read access. Original-envelope/expiry verification, admission-blocked remote restoration, authenticated player/world identity, request-size/URI handling and timeout reconciliation remain required before hosted gameplay opens. No migration is included in this audit.

## Reproduction — AI, no engine level

1. Use explicit project aged-resonance-28932410 with describe_project. Expect primalfrontier-dev/Sydney/PG18.
2. Use describe_branch with project above, branch br-red-fog-a7mp3ux7, database neondb. Expect Development and checkpoint/index tree. Never omit branch or fall back to default production.
3. Use describe_table_schema with the same targets and table public.pf_checkpoints. Compare seven columns/nullability/defaults, PK, two bounds and descending index against the snapshot/local001_initial.sql. Any mismatch fails this metadata comparison.
4. If authorized read-only SQL becomes available, run AuditCheckpointSchema.sql with5s statement/1s lock timeouts against the same development target. Do not mark role/RLS/ownership verified before inspecting actual output. Missing data/tool capability is unverified, not PASS or permission to create objects.

No C++/runtime/assets changed, so no new build/Unreal launch, FPS, memory or manual acceptance claim. Prior package60c0999 and local GSDK3cdac3a results remain separate.

## Earlier Cloudflare and local evidence — retained, not refreshed here

Cloudflare account3905225272dea5a3fe22f812bc925363, Worker primalfrontier-backend: settings/secrets/deployments GETs200; compatibility2026-10-16; expected ENVIRONMENT/plain_text, NEON_DATABASE_URL/secret_text and PF_API_SHARED_SECRET/secret_text names/types, no values read. Deployment ae4a8e15-a12d-4d71-8f1b-7f313a077289/version a8d144f2-975c-4c26-883f-70617ac65223 at100%, created2026-10-18T06:34:40.233227Z. Public health200/development and checkpoint status200 READY/neondb. These prove connectivity/table presence only, not current source equality or branch identity.

Backend C:/UnrealProjects/PrimalFrontier-Backend remains HEAD3bc1ed251c0c4de1f54db04e4ee2b8b3f4028e3d; pre-existing untracked nested PrimalFrontier-Backend/ is preserved without edits/staging/cleanup. Development shared-secret INSERT ON CONFLICT DO NOTHING/latest unexpired SELECT is transport, not player identity or active-writer fencing.

Earlier connection interruptions and missing-ID refusal are historical, superseded by the three successful explicit-target reads. Installation/URL actions are complete; do not ask the owner to repeat them. Git/card history retains prior snapshots. SQL sources were previously reviewed against PostgreSQL18 information functions/read-only transactions; no SQL execution is claimed.
