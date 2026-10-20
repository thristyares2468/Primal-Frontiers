# M15 one-time restricted checkpoint login preparation

October21,2026. Trello vPA1nOPg. **Focused5/5 and combined67/67 local PASS.**
No live login activation, credential, Worker change or new provider/gameplay pass.

## Target / behavior

Owner's earlier result confirms prepared pf_checkpoint_app NOLOGIN with read/
insert true and admin/mutation flags false. Owner now provides Development pooled
host ep-autumn-cell-a7d9uojb-pooler.ap-southeast-2.aws.neon.tech. Exact source in
BACKEND_ROLE_OWNER_RESULT_M15.json; reported branch br-red-fog-a7mp3ux7/neondb,
not an independent endpoint lookup or proof of current Worker binding.

Scripts/ActivateDevelopmentCheckpointLogin.sql is the exact tested operator
script. It revalidates neondb/neondb_owner, existing restricted inactive role,
creator ADMIN option, no app membership/owned objects, audited checkpoint table/
RLS and effective privileges. It generates two random v4 UUIDs inside PostgreSQL,
removes hyphens (64hex characters,244random bits), and uses quoted ALTER ROLE to
enable LOGIN with SCRAM-SHA-256 password storage. No extension/dependency or
literal password is added. It returns a private exact-host URL for the new role,
database neondb and sslmode=require. Password/URL are never source constants.

The URL passes through a transaction-local custom setting only, read by the sole
SELECT before COMMIT; it disappears on commit/rollback. The SELECT row is usable
only if the complete script/COMMIT has no errors. The caller sees this secret:
keep it out of chat, Git, screenshots and reports. SQL editor/provider result
retention is not controlled by this script; keep access private. No secret is
written into project files or test logs. Existing pf_app, Worker credential,
data, ownership/RLS and grants remain unchanged. A second activation refuses
rather than rotating an already-active login. Losing the private result needs a
separately reviewed rotation; never bypass the refusal blindly.

References: [PostgreSQL UUIDs](https://www.postgresql.org/docs/18/functions-uuid.html),
[transaction-local settings](https://www.postgresql.org/docs/18/functions-admin.html),
[Neon TLS](https://neon.com/docs/connect/connect-securely).

## Actual local tests / evidence

Existing pinned PGlite0.5.10/PG18.6, no new dependencies/DSN/network/cloud API.
Five added definitions in role-permissions.test.mjs:

1. one-time activation generates private exact-host URL and SCRAM hash without changing old app/data
2. activation retry refuses without replacing the first credential
3. activation refuses extra effective mutation privilege and leaves staging disabled
4. activation refuses missing staging role without creating credentials or touching old app
5. activation refuses wrong inspection identity and clears private URL on rollback

Actual checks validate fixed host/user/database/TLS/password format, LOGIN flag,
SCRAM boolean, unchanged old role/data and transaction-local secret cleanup. Hash
comparisons are boolean-only; generated values/verifiers are not printed even as
assertion operands. This is local SQL execution/SET ROLE, NOT an actual network
password/SCRAM authentication exchange against Neon/PgBouncer.

First focused5/5 PASS2.943s, exit0, role-process RSS495493120bytes.
Final combined67/67 PASS5.219s native/5.812s wrapper, exit0, no skip/cancel/todo,
legacy public fixture unchanged. Separate process RSS: SQL641507328bytes,
ingress124239872bytes, role504512512bytes; not a concurrent sum or Worker/Unreal/
rendered/minimum-RAM certification. Logs:
C:/UnrealSource/BuildLogs/M15CheckpointLogin_20261021_First.log and _Final.log.
Output review found no generated postgresql://pf_checkpoint_app: URL or SCRAM
verifier. No failed build/test in this slice. SQL-only change: no new C++ build
or Unreal launch warranted; prior native/build results remain separate evidence.

## Owner next action — one script, private result

No Unreal level/net mode/launch required. Start here:
C:/UnrealProjects/PrimalFrontier/Scripts/ActivateDevelopmentCheckpointLogin.sql.

1. In Neon SQL Editor select Development br-red-fog-a7mp3ux7, neondb,
   neondb_owner. Preserve Production and the existing Worker connection.
2. Copy the ENTIRE script into the editor and Run once.
3. Expect PF_CHECKPOINT_LOGIN_ENABLED and neon_database_url in the SELECT result,
   with successful COMMIT/no errors. Copy/keep the complete URL privately and
   keep the result open; do not paste it into chat or commit it.
4. Reply only CHECKPOINT_LOGIN_READY. If an error occurs, run ROLLBACK and send
   the error with any password/URL redacted. An already-enabled refusal is not
   permission to rerun/rotate; report it. Do not rerun old preparation after login.

Why owner: current chat has no callable Neon SQL tool; native browser automation
stopped on URL-confidence enforcement. No more hostname/account/role-preparation
action needed. Follow-up AI work prepares safe fresh-connection verification and
Worker candidate-secret/cutover. Do not replace the working Worker URL before a
replacement connection is verified. Restricted LOGIN is not yet verified,
hosted authority/lease/restore/identity and full M15/voice remain incomplete.
