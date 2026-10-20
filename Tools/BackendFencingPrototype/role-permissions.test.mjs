import { after, before, beforeEach, test } from 'node:test';
import assert from 'node:assert/strict';
import { readFile } from 'node:fs/promises';
import { PGlite } from '@electric-sql/pglite';

// No DSN/network/cloud API. The rejected in-place reference and exact current
// staging script run on synthetic PG18 roles/data only; no Neon-admin emulation.
const patch = await readFile(new URL('./rejected-inplace-role.sql', import.meta.url), 'utf8');
const prepare = await readFile(new URL('../../Scripts/PrepareDevelopmentCheckpointRole.sql', import.meta.url), 'utf8');
const baseline = await readFile(new URL('./baseline.sql', import.meta.url), 'utf8');
let db;
let peakRss = 0;
const sampler = setInterval(() => { peakRss = Math.max(peakRss, process.memoryUsage().rss); }, 10);
sampler.unref();
const appMetadata = async () => (await db.query(`SELECT rolcreatedb,rolcreaterole,
  ARRAY(SELECT pg_get_userbyid(roleid) FROM pg_auth_members WHERE member=r.oid ORDER BY roleid) AS memberships
  FROM pg_roles r WHERE rolname='pf_app'`)).rows[0];
const data = async () => (await db.query('SELECT * FROM public.pf_checkpoints ORDER BY world_id,generation')).rows;
const apply = async () => {
  try { return await db.exec(patch); }
  catch (error) { if (error.detail) console.log('ROLE_PATCH_REFUSAL_DETAIL', error.detail); throw error; }
};
async function refused(pattern) {
  const before = await appMetadata();
  await assert.rejects(apply(), pattern);
  await db.exec('ROLLBACK'); // release the explicitly failed transaction, never force COMMIT.
  assert.deepEqual(await appMetadata(), before, 'failed patch preserved role flags/membership');
}

before(async () => {
  // PGlite's database option selects an EXISTING database. Create it first in a
  // throwaway in-memory cluster, then reopen that memory image; no filesystem DB.
  const seed = new PGlite();
  await seed.exec('CREATE DATABASE neondb');
  const image = await seed.dumpDataDir();
  await seed.close();
  db = new PGlite({ database: 'neondb', loadDataDir: image });
  console.log('ROLE_FIXTURE_VERSION', (await db.query('SELECT version() AS version')).rows[0].version);
  await db.exec(`CREATE ROLE neon_superuser NOLOGIN CREATEDB CREATEROLE;
    GRANT pg_read_all_data,pg_write_all_data TO neon_superuser;
    CREATE ROLE neondb_owner LOGIN CREATEDB CREATEROLE;
    CREATE ROLE pf_app LOGIN CREATEDB CREATEROLE;
    CREATE ROLE unexpected_group NOLOGIN;
    GRANT neon_superuser TO neondb_owner WITH ADMIN TRUE;
    GRANT pf_app TO neondb_owner WITH ADMIN TRUE, INHERIT FALSE, SET TRUE;
    ALTER DATABASE neondb OWNER TO neondb_owner;
    SET ROLE neondb_owner;`);
  await db.exec(baseline);
  await db.query(`INSERT INTO public.pf_checkpoints(world_id,generation,envelope_version,checksum,payload)
    VALUES('untouched-checkpoint',1,1,'synthetic-sum','{"fixture":true}')`);
});
beforeEach(async () => {
  await db.exec(`RESET ROLE;
    DROP TABLE IF EXISTS public.owned_by_app;
    REVOKE unexpected_group FROM pf_app;
    GRANT pf_app TO neondb_owner WITH ADMIN TRUE, INHERIT FALSE, SET TRUE;
    ALTER ROLE pf_app LOGIN NOCREATEROLE NOCREATEDB NOBYPASSRLS NOREPLICATION;
    ALTER ROLE pf_app CREATEDB CREATEROLE;
    REVOKE ALL ON public.pf_checkpoints FROM PUBLIC;
    REVOKE ALL ON public.pf_checkpoints FROM pf_app;
    ALTER TABLE public.pf_checkpoints DISABLE ROW LEVEL SECURITY;
    SET ROLE neondb_owner;
    GRANT neon_superuser TO pf_app;`);
  await db.exec(`RESET ROLE; DO $cleanup$ BEGIN
    IF EXISTS(SELECT 1 FROM pg_roles WHERE rolname='pf_checkpoint_app') THEN
      DROP OWNED BY pf_checkpoint_app;
      DROP ROLE pf_checkpoint_app;
    END IF;
    END $cleanup$; SET ROLE neondb_owner;`);
});
after(async () => {
  if (db) await db.close();
  clearInterval(sampler);
  console.log('ROLE_FIXTURE_PEAK_RSS_BYTES', Math.max(peakRss, process.memoryUsage().rss));
});

test('exact role patch restricts app, preserves data and supports idempotent replay', async () => {
  const rows = await data();
  const results = await apply();
  assert.equal(results.at(-1).rows[0].status, 'PF_ROLE_RESTRICTED');
  assert.deepEqual(await appMetadata(), { rolcreatedb: false, rolcreaterole: false, memberships: [] });
  assert.deepEqual(await data(), rows);
  assert.equal((await apply()).at(-1).rows[0].status, 'PF_ROLE_RESTRICTED');
  assert.deepEqual(await data(), rows);
});
test('restricted actual SET ROLE permits checkpoint read/insert and refuses mutation/DDL', async () => {
  await apply();
  await db.exec('SET ROLE pf_app');
  assert.equal((await data()).length, 1);
  // Same SQL shape used by current Worker, synthetic disposable world only.
  const statement = `INSERT INTO pf_checkpoints(world_id,generation,envelope_version,checksum,payload,expires_at)
    VALUES('role-only-fixture',1,1,'fixture-checksum','{"synthetic":true}',NULL)
    ON CONFLICT(world_id,generation) DO NOTHING RETURNING world_id,generation`;
  assert.equal((await db.query(statement)).rows.length, 1);
  assert.equal((await db.query(statement)).rows.length, 0);
  for (const sql of ["UPDATE pf_checkpoints SET checksum='bad'", 'DELETE FROM pf_checkpoints',
    'TRUNCATE pf_checkpoints', 'CREATE TABLE public.forbidden_table(id integer)',
    'CREATE ROLE forbidden_role', 'CREATE DATABASE forbidden_database']) {
    await assert.rejects(db.exec(sql), /permission denied/);
  }
  await db.exec('RESET ROLE');
  await db.query("DELETE FROM public.pf_checkpoints WHERE world_id='role-only-fixture'");
});
test('owned app objects refuse before privilege changes', async () => {
  await db.exec('RESET ROLE; CREATE TABLE public.owned_by_app(id integer); ALTER TABLE public.owned_by_app OWNER TO pf_app; SET ROLE neondb_owner');
  await refused(/PF_ROLE_REFUSED: pf_app owns objects/);
});
test('unexpected membership refuses instead of cascading to other roles', async () => {
  await db.exec('RESET ROLE; GRANT unexpected_group TO pf_app; SET ROLE neondb_owner');
  await refused(/PF_ROLE_REFUSED: unexpected app membership/);
});
test('PUBLIC mutation privilege fails postcondition and rolls all role changes back', async () => {
  await db.exec('GRANT UPDATE ON public.pf_checkpoints TO PUBLIC');
  await refused(/PF_ROLE_REFUSED: effective privilege postcondition failed/);
  assert.equal((await db.query("SELECT has_table_privilege('pf_app','public.pf_checkpoints','UPDATE') AS allowed")).rows[0].allowed, true);
});
test('insufficient role administration aborts atomically without privilege escalation', async () => {
  await db.exec('RESET ROLE; REVOKE ADMIN OPTION FOR pf_app FROM neondb_owner; SET ROLE neondb_owner');
  await refused(/permission denied|ADMIN option|admin option/);
});
test('unmanageable original grantor refuses before role changes', async () => {
  await db.exec(`RESET ROLE;
    REVOKE neon_superuser FROM pf_app GRANTED BY neondb_owner;
    GRANT neon_superuser TO pf_app;
    SET ROLE neondb_owner`);
  await refused(/PF_ROLE_REFUSED: membership grantor cannot be managed/);
});
test('changed RLS or inspection identity refuses without touching live checkpoint data', async () => {
  const rows = await data();
  await db.exec('ALTER TABLE public.pf_checkpoints ENABLE ROW LEVEL SECURITY');
  await refused(/PF_ROLE_REFUSED: checkpoint table ownership\/type\/RLS/);
  await db.exec('RESET ROLE');
  await refused(/PF_ROLE_REFUSED: select Development\/neondb and neondb_owner/);
  assert.deepEqual(await data(), rows);
});

test('SQL-created staging role is restricted NOLOGIN and leaves managed app and data unchanged', async () => {
  const oldRole = await appMetadata(); const rows = await data();
  const result = (await db.exec(prepare)).at(-1).rows[0];
  assert.deepEqual(result, { status: 'PF_RESTRICTED_ROLE_PREPARED', app_role: 'pf_checkpoint_app',
    login_enabled: false, can_create_database: false, can_create_roles: false,
    can_read: true, can_insert: true, can_update: false, can_delete: false });
  assert.deepEqual(await appMetadata(), oldRole);
  assert.deepEqual(await data(), rows);
  assert.deepEqual((await db.exec(prepare)).at(-1).rows[0], result);
  assert.equal((await db.query(`SELECT count(*)::int AS count FROM pg_auth_members
    WHERE member=(SELECT oid FROM pg_roles WHERE rolname='pf_checkpoint_app')`)).rows[0].count, 0);
});
test('staging role can read and insert checkpoint SQL but cannot mutate existing saves', async () => {
  await db.exec(prepare);
  await db.exec('SET ROLE pf_checkpoint_app');
  assert.equal((await data()).length, 1);
  const sql = `INSERT INTO public.pf_checkpoints(world_id,generation,envelope_version,checksum,payload)
    VALUES('staging-role-fixture',1,1,'synthetic','{"fixture":true}')
    ON CONFLICT(world_id,generation) DO NOTHING RETURNING world_id,generation`;
  assert.equal((await db.query(sql)).rows.length, 1);
  assert.equal((await db.query(sql)).rows.length, 0);
  for (const statement of ['UPDATE public.pf_checkpoints SET generation=2', 'DELETE FROM public.pf_checkpoints',
    'TRUNCATE public.pf_checkpoints', 'CREATE ROLE forbidden_stage_role', 'CREATE DATABASE forbidden_stage_db']) {
    await assert.rejects(db.exec(statement), /permission denied/);
  }
  await db.exec('RESET ROLE');
  await db.query("DELETE FROM public.pf_checkpoints WHERE world_id='staging-role-fixture'");
});
test('active pre-existing staging role is refused without resetting credentials or login', async () => {
  await db.exec(prepare);
  await db.exec('ALTER ROLE pf_checkpoint_app LOGIN');
  await assert.rejects(db.exec(prepare), /PF_ROLE_REFUSED: existing staging role/);
  await db.exec('ROLLBACK');
  assert.equal((await db.query("SELECT rolcanlogin FROM pg_roles WHERE rolname='pf_checkpoint_app'")).rows[0].rolcanlogin, true);
});
test('PUBLIC broad rights roll back new role creation without changing other roles', async () => {
  const oldRole = await appMetadata();
  await db.exec('GRANT UPDATE ON public.pf_checkpoints TO PUBLIC');
  await assert.rejects(db.exec(prepare), /PF_ROLE_REFUSED: effective staging privileges/);
  await db.exec('ROLLBACK');
  assert.equal((await db.query("SELECT count(*)::int AS count FROM pg_roles WHERE rolname='pf_checkpoint_app'")).rows[0].count, 0);
  assert.deepEqual(await appMetadata(), oldRole);
});
