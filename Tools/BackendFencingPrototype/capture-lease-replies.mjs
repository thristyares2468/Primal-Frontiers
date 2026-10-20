// Explicit fixture regeneration only. Normal tests NEVER overwrite this file.
// No DSN/network/Worker imports. Run from this directory with the pinned install.
import { PGlite } from '@electric-sql/pglite';
import { readFile, writeFile } from 'node:fs/promises';
import { randomUUID, createHash } from 'node:crypto';
const db = new PGlite();
const contract = await readFile(new URL('./contract.sql', import.meta.url));
try {
  await db.exec("SET TIME ZONE 'UTC'");
  await db.exec(await readFile(new URL('./baseline.sql', import.meta.url), 'utf8'));
  await db.exec(contract.toString('utf8'));
  const call = async (sql, values) => (await db.query(sql, values)).rows[0].result;
  const world = 'native-lease-fixture';
  const checksum = 'sha256:' + 'ab'.repeat(32);
  // A deliberately large exact int8 fence detects accidental JS/double rounding.
  await db.query('INSERT INTO pf_fencing_draft.worlds(world_id,fence) VALUES($1,$2::bigint)',
    [world, '9223372036854775806']);
  const id = randomUUID();
  const acquireSql = 'SELECT pf_fencing_draft.acquire($1,$2,$3::uuid,$4) AS result';
  const acquireArgs = [world, 'synthetic-server', id, 30];
  const acquire = await call(acquireSql, acquireArgs);
  const acquireReplay = await call(acquireSql, acquireArgs);
  const renewSql = 'SELECT pf_fencing_draft.renew($1,$2,$3::uuid,$4::bigint,$5::uuid,$6) AS result';
  const renewArgs = [world, 'synthetic-server', acquire.lease_id, acquire.fence, randomUUID(), 60];
  const renew = await call(renewSql, renewArgs);
  const renewReplay = await call(renewSql, renewArgs);
  const commitSql = 'SELECT pf_fencing_draft.commit_checkpoint($1,$2,$3::uuid,$4::bigint,$5::uuid,$6::bigint,$7::bigint,$8,$9,$10::jsonb,$11::timestamptz) AS result';
  const commitArgs = [world, 'synthetic-server', acquire.lease_id, acquire.fence, randomUUID(), null,
    '9007199254740991', 1, checksum, JSON.stringify({ synthetic: true }), null];
  const commit = await call(commitSql, commitArgs);
  const commitReplay = await call(commitSql, commitArgs);
  if (!acquire.ok || !renew.ok || !commit.ok) throw new Error('Fixture SQL failed');
  const fixture = {
    provenance: 'Synthetic in-memory PostgreSQL results, not a deployed service or native save payload',
    postgres: (await db.query('SELECT version() AS version')).rows[0].version,
    contractSha256: createHash('sha256').update(contract).digest('hex'),
    world, checksum, acquire, acquireReplay, renew, renewReplay, commit, commitReplay,
  };
  const target = new URL('../../Tests/Fixtures/M15LeaseReplies/fixture.json', import.meta.url);
  await writeFile(target, JSON.stringify(fixture, null, 2) + '\n', { flag: 'wx' });
  console.log('Captured six actual SQL results into new fixture; existing files are never overwritten.');
} finally { await db.close(); }
