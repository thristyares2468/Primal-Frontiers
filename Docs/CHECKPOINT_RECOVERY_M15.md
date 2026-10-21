# M15 recovery verification and live candidate receipt — October22

Bounded task https://trello.com/c/vgp1xo9E passes. The fresh deployed candidate
check returned HTTP200/CANDIDATE_READY at2026-10-21T23:09:44.0122658Z; exact safe
receipt CHECKPOINT_CANDIDATE_RESULT_M15.json. Owner secret-entry/activation/
recovery action is complete. Do not ask the owner to repeat it. Candidate login
and the nine implemented metadata checks now pass at query time. This does not
switch the active Worker connection or complete the broader audit/fullM15.

The previously published recovery SQL lacked an explicit ADMIN-option check
and had no automated tests. Add the same checked administration prerequisite
used by activation; PostgreSQL would already refuse ALTER ROLE without that
right, but the recovery script now gives a precise refusal before generating
another password. No live rotation, secret, grant, schema or Worker change was
performed in this slice. The working private credential needs no update.

## Tests and failures retained

Tests extend existing Tools/BackendFencingPrototype/role-permissions.test.mjs;
same pinned test-only PGlite0.5.10, synthetic PostgreSQL18 cluster, no external
network, credentials or dependencies. Native Node crypto independently verifies
both stored SCRAM keys match the private URL password, without printing values.

Five new tests:
1. Recovery rotates only the restricted credential, returns exact-host URL,
   matches SCRAM proof, clears transaction-local output, preserves old app/data,
   and leaves the activation retry refusal intact.
2. Missing/disabled login refuses without creating credentials or changing data.
3. Elevated flags, inherited membership, owned objects, PUBLIC mutation rights
   and RLS changes refuse atomically and retain the current credential.
4. Missing ADMIN option gives explicit refusal and preserves the credential.
5. Wrong inspection identity refuses before changing credentials.

Logs under C:/UnrealSource/BuildLogs/:
- M15CheckpointRecovery_20261022_First.log:2/5 pass. Generic ALTER ROLE permission
  error exposed absent explicit guard. Synthetic membership GRANT needed test
  bootstrap authority; failed assertion also needed unconditional ROLLBACK.
- M15CheckpointRecovery_20261022_Replay.log:5/5 pass after narrow fixes,2.412s.
- M15CheckpointRecovery_20261022_Final.log:72/72 pass,4.803s,exit0, no skipped/
  cancelled/todo.30SQL/composed +20ingress +22role/login tests. Role process
  peak494776320bytes RSS, ingress123506688bytes, separately sampled processes.
  Credential-shaped URL/verifier scan finds no generated secret output.

No C++/Unreal build is required for this SQL/test-only slice. It makes no new
rendered performance, manual acceptance, server deployment or voice claim.
PGlite tests prove role/catalog/transaction behavior, not live Neon contention,
all provider permission policies or database password network authentication.
The separate fresh deployed probe supplies the current candidate login proof.

## Reproduce (AI; no level, launch or net mode)

Start here: PowerShell in C:/UnrealProjects/PrimalFrontier/Tools/BackendFencingPrototype,
with its declared dependency installed and bundled Node24 available.

1. Focused: node --test --test-concurrency=1 --test-reporter=tap
   --test-name-pattern recovery role-permissions.test.mjs. Expect5/5/exit0.
2. Combined: node --test --test-concurrency=1 --test-reporter=tap
   fencing.test.mjs ingress.test.mjs role-permissions.test.mjs. Expect72/72/exit0.
3. Inspect logs for the five names and preserved data assertions. Stop on any
   failure; never assert/log private URL/password/hash as an operand.
4. A future explicit live recheck is GET of
   https://primalfrontier-backend.jackherbig09.workers.dev/v1/checkpoints/candidate/status.
   Expect HTTP200/CANDIDATE_READY; stop on other status. No SQL rerun needed.

Production unchanged; hosted identity/lease/restore/admission, active-connection
cutover/retirement and complete access audit remain open. PlayFab upload and
capacity are deferred while local gameplay work continues. Earlier chat claims
that a supported local Server package does not exist were stale: October20
source-package/one-/two-client local receipts are in PLAYFAB_SERVER_BUILD_M15.md.
Do not rerun those expensive checks or upload capacity based on this SQL slice.
