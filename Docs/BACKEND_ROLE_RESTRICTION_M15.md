# M15 development checkpoint role correction preparation

October21,2026. Trello8CGVNsjq. **Local12/12 and combined62/62 PASS.**
Owner has now executed the preparation and returned PF_RESTRICTED_ROLE_PREPARED
with expected pf_checkpoint_app permissions and login disabled; [exact source](BACKEND_ROLE_OWNER_RESULT_M15.json).
No agent-side live mutation. Existing managed pf_app/Worker connection stays
unchanged. Private login/cutover and fullM15/audit remain incomplete.

## Finding / provenance

[Exact owner SQL result](BACKEND_ROLE_OWNER_RESULT_M15.json) reports
Development/neondb/pf_app with CREATEDB/CREATEROLE/neon_superuser membership and
SELECT/INSERT/UPDATE/DELETE. Table owner is neondb_owner; schema CREATE and
TRUNCATE false, RLS/forcedRLS false. This is owner-pasted output, not a direct
agent query or verified Worker endpoint. Backend3bc1ed2 src/index.ts uses status
and latest SELECT plus INSERT ON CONFLICT DO NOTHING RETURNING; runtime does not
need administrator membership or UPDATE/DELETE. No secrets/payloads requested.

Owner's second query reports neon_superuser granted to BOTH neondb_owner and
pf_app by cloud_admin, admin_option=false, inherit_option/set_option=true. No
owner ADMIN grant on pf_app is shown. Treat in-place managed-role demotion as
unsupported here; do not escalate to cloud_admin or silently ignore REVOKE.

Neon documents that Console/API/CLI-created roles receive neon_superuser, and
recommends SQL-created restricted application roles: [roles](https://neon.com/docs/manage/roles),
[access](https://neon.com/docs/manage/database-access). PostgreSQL18
[ALTER ROLE](https://www.postgresql.org/docs/18/sql-alterrole.html) and
[REVOKE](https://www.postgresql.org/docs/18/sql-revoke.html) define administration
and original-grantor limits. Removing only explicit table rights cannot remove
inherited broad access. A role name and READY/neondb do not prove least privilege.

## Current restricted-role preparation

Scripts/PrepareDevelopmentCheckpointRole.sql is the exact locally tested current
operator script. It requires neondb/neondb_owner and the audited table owner/type/
RLS, then creates SQL-owned pf_checkpoint_app NOLOGIN, without password, admin
attributes or memberships. It grants DB CONNECT/schema USAGE/checkpoint SELECT+
INSERT only and validates effective privileges. Broad PUBLIC rights roll back new
role creation; an active/privileged/pre-existing unmanageable staging role refuses
instead of resetting it. Ordinary repeated preparation is idempotent. Existing
managed pf_app, its credentials/privileges and old Worker connection/data are
unchanged; this is preparation, not final privilege remediation or login cutover.
No schema migration/data/owner/PUBLIC/Production changes. SQL cannot prove the
Neon branch; owner must verify Development in the Console.

## Rejected in-place experiment — never run on Neon

The earlier script is retained ONLY as Tools/BackendFencingPrototype/rejected-inplace-role.sql,
with prominent rejection headers; no live operator script named RestrictDevelopmentCheckpointRole.sql exists.
This historical experiment used a transaction with5s statement/
1s lock timeout. It requires neondb/neondb_owner, existing ordinary login pf_app,
audited table ownership/type/RLS, no app-owned objects or unexpected memberships,
and ability to manage each original neon_superuser membership grantor. It removes
only pf_app CREATEDB/CREATEROLE and manageable neon_superuser grants with explicit
GRANTED BY/RESTRICT, resets that role's checkpoint-table grants to SELECT+INSERT,
and ensures DB CONNECT/schema USAGE. Password/login, owner/PUBLIC grants, RLS,
schema/table/data and other roles are not modified. No CASCADE, role deletion,
ownership reassignment, secret, default-privilege or production change.

Postconditions require no app membership/admin/replication/bypass flags, DB/schema
CREATE, mutation/column mutation or grant-option capability; needed CONNECT/
USAGE/SELECT/INSERT remain. An unexpected source of authority refuses and rolls
back rather than revoking unrelated PUBLIC/role rights. Error details contain
permission booleans/counts only. SQL cannot independently prove Neon branch: owner
must select exact Development branch before ANY future use. Concurrent role
administration/Neon extension capabilities and fresh provider login are not
certified by the one-session local fixture. Direct replication/bypass flags not
in owner query are checked by the candidate; unexpected flags refuse.

## Exact local verification and retained failures

Existing pinned test-only PGlite0.5.10/PG18.6; no new dependency/DSN/network or
Worker import. A throwaway in-memory cluster creates neondb, exports its memory
image and reopens it. Setting the database option before creating neondb failed
initialization in an exploratory call; no project/live data changed. Test roles
include non-superuser neondb_owner with explicit app/administrator-role ADMIN
grants and broad synthetic neon_superuser membership. SET ROLE exercises real
PostgreSQL authorization; it is not a Neon Console/API administrator simulation.
Current Worker SQL shape, including INSERT RETURNING and duplicate no-op, runs
as the restricted app. Real forbidden UPDATE/DELETE/TRUNCATE/schema CREATE/
CREATE ROLE/CREATE DATABASE statements are rejected.

Initial M15RolePatch_20261021_First.log:0/7, setup hook permission denied revoking
unexpected_group because SESSION AUTHORIZATION switching was unsuitable for this
fixture. A wrong relative-path edit attempt left source unchanged; narrow
HarnessReplay also0/1. Corrected harness to SET ROLE/RESET ROLE; HarnessReplay2
then reached the real candidate and0/1 postcondition failure. Diagnostic run
confirmed membership1 and inherited mutation still true: original grant was made
by another role, so unqualified REVOKE did not remove it. This is a useful fail-
closed result, not a pass. Added original-grantor manageability preflight and
explicit GRANTED BY revocation; adjusted success fixture to a genuinely manageable
grant and added unmanageable-grantor refusal. No postcondition was weakened.
Narrow GrantorReplay1/1 passes2.522s; full8/8 passes2.623s. Logs retained under
C:/UnrealSource/BuildLogs/M15RolePatch_20261021_{First,HarnessReplay,HarnessReplay2,PostconditionDiagnostic,GrantorReplay,Full}.log.

Final combined M15RolePatch_20261021_Combined.log:58/58,5.083s native/6.016s
wrapper,exit0,no cancelled/skipped/todo; original public checkpoint fixture
unchanged.30fencing/composed+20ingress+8role definitions. Local role titles:

1. exact role patch restricts app, preserves data and supports idempotent replay
2. restricted actual SET ROLE permits checkpoint read/insert and refuses mutation/DDL
3. owned app objects refuse before privilege changes
4. unexpected membership refuses instead of cascading to other roles
5. PUBLIC mutation privilege fails postcondition and rolls all role changes back
6. insufficient role administration aborts atomically without privilege escalation
7. unmanageable original grantor refuses before role changes
8. changed RLS or inspection identity refuses without touching live checkpoint data

After real grantor review, new-role preparation adds four tests, final role-only
12/12 PASS2.797s and final combined62/62 PASS5.522s native/6.050s wrapper,exit0,
no skipped/cancel/todo; legacy public fixture unchanged. Final log
C:/UnrealSource/BuildLogs/M15RolePatch_20261021_StagingFinal.log. Added definitions:

Final separate-process sampled RSS: SQL641855488bytes, ingress123965440bytes,
role499580928bytes. Sequential test-file processes, not a concurrent sum, Worker
isolate measurement, rendered performance or minimum-RAM certification.

9. SQL-created staging role is restricted NOLOGIN and leaves managed app and data unchanged
10. staging role can read and insert checkpoint SQL but cannot mutate existing saves
11. active pre-existing staging role is refused without resetting credentials or login
12. PUBLIC broad rights roll back new role creation without changing other roles

The staging role's actual SET ROLE statement tests are authorization checks, not
proof it can log in (it deliberately cannot), or of actual Worker binding.

No C++ change, so no new Editor build/run required; prior bb3c360 native/build
evidence remains separate. No rendered/FPS/gameplay/manual/provider acceptance.

## Reproduce / owner handoff

AI local test: no level, net mode, Unreal instance or credentials. Start here in
Tools/BackendFencingPrototype with pinned install from README, run pnpm test.
Expect62/62/exit0 and unchanged legacy marker. Narrow replay:
`node --test --test-concurrency=1 --test-reporter=tap --test-name-pattern="exact role patch restricts app" role-permissions.test.mjs`.
Any failed/missing test stops expansion; preserve log and fix only this scope.

Owner preparation guide below is now COMPLETE per actual returned result; retain
for reproduction, do not request it again unnecessarily:

1. Open project aged-resonance-28932410 > Development br-red-fog-a7mp3ux7 > SQL Editor.
2. Confirm database neondb and role neondb_owner. Do not select Production.
3. Start here: open Scripts/PrepareDevelopmentCheckpointRole.sql in Codex, Ctrl+A,
   Ctrl+C, paste the ENTIRE file into Neon SQL Editor, click Run once.
4. Send the final result row. Expected status PF_RESTRICTED_ROLE_PREPARED,
   app_role pf_checkpoint_app; login/createDatabase/createRoles/update/delete false;
   read/insert true. No password is created/returned. If ANY error, run ROLLBACK,
   return exact error and stop; do not retry rejected demotion/CASCADE commands.

Why owner: no callable Neon SQL tool; Computer Use stopped on browser URL
confidence, so agent cannot safely run the live statement through current tools.
This is not another connection/URL setup request. No level/Unreal/gameplay needed.
After actual output, AI reviews permissions, prepares private login credentials
and exact Development endpoint/Worker cutover. Keep old app working until new
connection and disposable write/read/duplicate verification pass; then retire old
runtime credential separately. No current live restriction/authentication/branch
binding/hosted-fencing pass implied. SQL-created role branching/password behavior
needs its own deployment policy; no automatic password-rotation claim.

## Current next action — Development endpoint hostname only

No level/net mode/Unreal required. Start here: Neon Development branch > Connect.
Confirm Development br-red-fog-a7mp3ux7 and database neondb. The role in this
dialog may remain an existing role; only the shared branch endpoint host is
needed, not an owner credential. If SQL-created pf_checkpoint_app is absent from
the dropdown, do not recreate it through Console/API (that would grant managed
privileges again). Read the hostname between @ and /neondb in the shown URL;
send ONLY that hostname, e.g. ep-example-pooler.ap-southeast-2.aws.neon.tech.
Do not send the full URL/password. Pooled host is fine; note which compute/branch
is selected. If no connection string is shown, return the visible issue instead
of guessing a host or selecting Production.

Reference: [Neon connection details](https://neon.com/docs/connect/connect-from-any-app).
The SQL row/database name cannot identify a Neon branch endpoint. This missing
nonsecret target prevents preparing the exact private credential/cutover step;
agent tools currently expose no Neon endpoint lookup and native browser use
stopped on URL-confidence enforcement. Host provision will not by itself prove
the existing encrypted Worker binding. Keep role NOLOGIN until separately
reviewed credential provisioning; existing connection stays available.
