import { after, before, beforeEach, test } from 'node:test';
import assert from 'node:assert/strict';
import { createHash, randomUUID } from 'node:crypto';
import { crc32 } from 'node:zlib';
import { readFile } from 'node:fs/promises';
import { PGlite } from '@electric-sql/pglite';
import { readCheckpointRequest, MAX_PAYLOAD_BYTES } from './ingress.mjs';

// No DSN, network client, file-backed database or Worker import is used here.
const db = new PGlite();
let peakRss = 0;
const sampler = setInterval(() => { peakRss = Math.max(peakRss, process.memoryUsage().rss); }, 10);
sampler.unref();
const checksum = 'sha256:' + 'ab'.repeat(32);
const payload = { original_file_base64: 'fixture-only', source_utc: '2026-10-18T00:00:00Z' };
const acquire = (world = 'world-a', server = 'server-a', request = randomUUID(), ttl = 30) =>
  invoke(db, 'acquire', [world, server, request, ttl]);

const functions = {
  acquire: 'SELECT pf_fencing_draft.acquire($1::text,$2::text,$3::uuid,$4::integer) AS result',
  renew: 'SELECT pf_fencing_draft.renew($1::text,$2::text,$3::uuid,$4::bigint,$5::uuid,$6::integer) AS result',
  release: 'SELECT pf_fencing_draft.release($1::text,$2::text,$3::uuid,$4::bigint) AS result',
  commit: 'SELECT pf_fencing_draft.commit_checkpoint($1::text,$2::text,$3::uuid,$4::bigint,$5::uuid,$6::bigint,$7::bigint,$8::integer,$9::text,$10::jsonb,$11::timestamptz) AS result',
};
async function invoke(client, operation, values) {
  assert.ok(Object.hasOwn(functions, operation));
  return (await client.query(functions[operation], values)).rows[0].result;
}
function commitArgs(lease, options = {}) {
  return [options.world ?? lease.world_id, options.server ?? 'server-a', options.leaseId ?? lease.lease_id,
    options.fence ?? lease.fence, options.request ?? randomUUID(), options.head ?? null,
    options.generation ?? 1, options.version ?? 1, options.checksum ?? checksum,
    JSON.stringify(Object.hasOwn(options, 'payload') ? options.payload : payload), options.expires ?? null];
}
const commit = (lease, options) => invoke(db, 'commit', commitArgs(lease, options));
const renew = (lease, request = randomUUID(), ttl = 60, server = 'server-a') =>
  invoke(db, 'renew', [lease.world_id, server, lease.lease_id, lease.fence, request, ttl]);
const release = (lease, server = 'server-a') =>
  invoke(db, 'release', [lease.world_id, server, lease.lease_id, lease.fence]);
const expire = world => db.query(
  "UPDATE pf_fencing_draft.worlds SET lease_expires_at=clock_timestamp()-interval '1 second' WHERE world_id=$1", [world]);
const refused = (result, error) => assert.deepEqual(result, { ok: false, error });
async function counts() {
  return (await db.query(`SELECT
    (SELECT count(*)::int FROM pf_fencing_draft.worlds) AS worlds,
    (SELECT count(*)::int FROM pf_fencing_draft.checkpoints) AS checkpoints,
    (SELECT count(*)::int FROM pf_fencing_draft.receipts) AS receipts`)).rows[0];
}

before(async () => {
  console.log('DATABASE_VERSION', (await db.query('SELECT version() AS version')).rows[0].version);
  await db.exec("SET TIME ZONE 'UTC'");
  // Apply the actual checked-in baseline in this fresh, in-memory database.
  await db.exec(await readFile(new URL('./baseline.sql', import.meta.url), 'utf8'));
  await db.query(`INSERT INTO public.pf_checkpoints(world_id,generation,envelope_version,checksum,payload)
    VALUES ('legacy-owner-fixture',7,1,$1,$2::jsonb)`, [checksum, JSON.stringify(payload)]);
  await db.exec(await readFile(new URL('./contract.sql', import.meta.url), 'utf8'));
  await db.exec('CREATE ROLE pf_fencing_untrusted NOLOGIN');
});
beforeEach(async () => {
  await db.exec('TRUNCATE pf_fencing_draft.receipts,pf_fencing_draft.checkpoints,pf_fencing_draft.worlds');
});
after(async () => {
  const baseline = (await db.query('SELECT world_id,generation::text,payload FROM public.pf_checkpoints')).rows;
  assert.deepEqual(baseline, [{ world_id: 'legacy-owner-fixture', generation: '7', payload }]);
  console.log('LEGACY_PUBLIC_FIXTURE_UNCHANGED', true);
  await db.close();
  clearInterval(sampler);
  console.log('PEAK_PROCESS_RSS_BYTES', Math.max(peakRss, process.memoryUsage().rss));
});

test('acquisition returns a database-owned epoch, bounded deadline and empty head', async () => {
  const lease = await acquire();
  assert.equal(lease.ok, true);
  assert.equal(lease.fence, '1');
  assert.equal(lease.head_generation, null);
  assert.match(lease.lease_id, /^[a-f0-9-]{36}$/);
  const row = (await db.query(`SELECT active,extract(epoch FROM lease_expires_at-clock_timestamp()) AS remaining
    FROM pf_fencing_draft.worlds`)).rows[0];
  assert.equal(row.active, true);
  assert.ok(Number(row.remaining) > 25 && Number(row.remaining) <= 30);
});

test('frozen native lease-reply fixture matches actual SQL result types and bounds', async () => {
  const fixture = JSON.parse(await readFile(new URL('../../Tests/Fixtures/M15LeaseReplies/fixture.json', import.meta.url), 'utf8'));
  assert.equal(fixture.contractSha256, createHash('sha256').update(await readFile(new URL('./contract.sql', import.meta.url))).digest('hex'));
  await db.query('INSERT INTO pf_fencing_draft.worlds(world_id,fence) VALUES($1,$2::bigint)',
    [fixture.world, '9223372036854775806']);
  const request = randomUUID();
  const lease = await acquire(fixture.world, 'server-a', request);
  const retry = await acquire(fixture.world, 'server-a', request);
  const renewalRequest = randomUUID();
  const renewal = await renew(lease, renewalRequest);
  const renewalRetry = await renew(lease, renewalRequest);
  const options = { generation: '9007199254740991', request: randomUUID() };
  const committed = await commit(lease, options);
  const committedRetry = await commit(lease, options);
  for (const [name, actual] of Object.entries({ acquire: lease, acquireReplay: retry,
    renew: renewal, renewReplay: renewalRetry, commit: committed, commitReplay: committedRetry })) {
    const frozen = fixture[name];
    assert.deepEqual(Object.keys(actual).sort(), Object.keys(frozen).sort(), name);
    for (const key of Object.keys(actual)) {
      assert.equal(typeof actual[key], typeof frozen[key], `${name}.${key}`);
      if (!['lease_id', 'expires_at'].includes(key)) assert.deepEqual(actual[key], frozen[key], `${name}.${key}`);
    }
    if (actual.lease_id) assert.match(frozen.lease_id, /^[a-f0-9]{8}(-[a-f0-9]{4}){3}-[a-f0-9]{12}$/);
    if (actual.expires_at) assert.ok(Number.isFinite(Date.parse(frozen.expires_at)));
  }
  assert.equal(lease.fence, '9223372036854775807');
  assert.equal(lease.head_generation, null);
  assert.equal(committed.generation, 9007199254740991);
});

test('queued competing acquisitions yield exactly one owner', async () => {
  const results = await Promise.all([acquire('world-a', 'server-a'), acquire('world-a', 'server-b')]);
  assert.equal(results.filter(r => r.ok).length, 1);
  assert.equal(results.filter(r => r.error === 'lease_held').length, 1);
  assert.deepEqual(await counts(), { worlds: 1, checkpoints: 0, receipts: 1 });
});

test('same request retry preserves original lease/deadline and rejects changed arguments', async () => {
  const request = randomUUID();
  const first = await acquire('world-a', 'server-a', request);
  assert.deepEqual(await acquire('world-a', 'server-a', request), { ...first, replayed: true });
  refused(await acquire('world-a', 'server-b', request), 'request_conflict');
  refused(await acquire('world-a', 'server-a', request, 60), 'request_conflict');
});

test('an expired acquisition request cannot silently reacquire', async () => {
  const request = randomUUID();
  await acquire('world-a', 'server-a', request);
  await expire('world-a');
  refused(await acquire('world-a', 'server-a', request), 'lease_expired');
  const next = await acquire('world-a', 'server-a');
  assert.equal(next.fence, '2');
  refused(await acquire('world-a', 'server-a', request), 'lease_lost');
});

test('takeover fences old writes, renewals and releases even for the same server identity', async () => {
  const first = await acquire();
  await expire('world-a');
  const next = await acquire();
  assert.equal(next.fence, '2');
  assert.notEqual(first.lease_id, next.lease_id);
  refused(await commit(first), 'lease_lost');
  refused(await renew(first), 'lease_lost');
  refused(await release(first), 'lease_lost');
  assert.equal((await commit(next)).ok, true);
});

test('expiry rejects commit and renewal before any takeover', async () => {
  const lease = await acquire();
  await expire('world-a');
  refused(await commit(lease), 'lease_expired');
  refused(await renew(lease), 'lease_expired');
  assert.equal((await counts()).checkpoints, 0);
});

test('release is idempotent, preserves the head and never recycles a fence', async () => {
  const lease = await acquire();
  await commit(lease, { generation: 42 });
  assert.deepEqual(await release(lease), { ok: true, released: true, fence: '1' });
  assert.deepEqual(await release(lease), { ok: true, released: true, fence: '1' });
  refused(await commit(lease, { head: 42, generation: 43 }), 'lease_expired');
  const next = await acquire('world-a', 'server-b');
  assert.equal(next.fence, '2');
  assert.equal(next.head_generation, 42);
  refused(await release(lease), 'lease_lost');
});

test('renewal is idempotent and does not shorten or repeatedly extend the deadline', async () => {
  const lease = await acquire();
  const request = randomUUID();
  const renewed = await renew(lease, request);
  assert.equal(renewed.ok, true);
  assert.ok(Date.parse(renewed.expires_at) > Date.parse(lease.expires_at));
  assert.deepEqual(await renew(lease, request), { ...renewed, replayed: true });
  refused(await renew(lease, request, 90), 'request_conflict');
  const shorter = await renew(lease, randomUUID(), 5);
  assert.equal(shorter.expires_at, renewed.expires_at);
});

test('world and owner are both validated; no cross-world capability reuse', async () => {
  const a = await acquire();
  const b = await acquire('world-b', 'server-b');
  refused(await commit(a, { server: 'server-b' }), 'lease_lost');
  refused(await renew(a, randomUUID(), 60, 'server-b'), 'lease_lost');
  refused(await release(a, 'server-b'), 'lease_lost');
  refused(await commit(a, { world: 'world-b', server: 'server-b' }), 'lease_lost');
  assert.equal((await commit(a)).ok, true);
  assert.equal((await commit(b, { server: 'server-b' })).ok, true);
  assert.equal((await counts()).checkpoints, 2);
});

test('lost commit acknowledgment reconciles exactly once without changing original envelope fields', async () => {
  const lease = await acquire();
  const request = randomUUID();
  const expires = '2099-01-01T00:00:00Z';
  const args = { request, expires };
  const first = await commit(lease, args);
  assert.equal(first.ok, true);
  const original = (await db.query('SELECT created_at,expires_at,checksum,payload FROM pf_fencing_draft.checkpoints')).rows;
  assert.deepEqual(await commit(lease, args), { ...first, replayed: true });
  assert.deepEqual((await db.query('SELECT created_at,expires_at,checksum,payload FROM pf_fencing_draft.checkpoints')).rows, original);
  assert.deepEqual(original[0].payload, payload);
  assert.equal(original[0].checksum, checksum);
  assert.equal((await counts()).checkpoints, 1);
});

test('same commit request cannot substitute payload, checksum, expiry, version or generation', async () => {
  const lease = await acquire();
  const request = randomUUID();
  await commit(lease, { request });
  for (const changed of [{ payload: { forged: true } }, { checksum: 'sha256:' + 'cd'.repeat(32) },
    { expires: '2099-01-01T00:00:00Z' }, { version: 2 }, { generation: 2 }, { head: 1 }]) {
    refused(await commit(lease, { request, ...changed }), 'request_conflict');
  }
  assert.equal((await counts()).checkpoints, 1);
});

test('fresh requests cannot overwrite a generation or regress the durable head', async () => {
  const lease = await acquire();
  await commit(lease, { generation: 7 });
  const stale = await commit(lease, { generation: 8 });
  assert.deepEqual(stale, { ok: false, error: 'head_conflict', head_generation: 7 });
  refused(await commit(lease, { head: 7, generation: 7 }), 'generation_not_newer');
  refused(await commit(lease, { head: 7, generation: 6 }), 'generation_not_newer');
  assert.equal((await commit(lease, { head: 7, generation: 11 })).ok, true);
  assert.equal((await counts()).checkpoints, 2);
});

test('queued same-base commits allow only one compare-and-swap winner', async () => {
  const lease = await acquire();
  const results = await Promise.all([commit(lease, { generation: 3 }), commit(lease, { generation: 5 })]);
  assert.equal(results.filter(r => r.ok).length, 1);
  assert.equal(results.filter(r => r.error === 'head_conflict').length, 1);
  assert.equal((await counts()).checkpoints, 1);
});

test('a request identifier may be reused in another world without sharing a receipt', async () => {
  const request = randomUUID();
  const a = await acquire('world-a', 'server-a', request);
  const b = await acquire('world-b', 'server-a', request);
  assert.equal((await commit(a, { request })).ok, true);
  assert.equal((await commit(b, { request })).ok, true);
  assert.equal((await counts()).receipts, 4);
});

test('takeover preserves already committed checkpoints but rejects old acknowledgment retries', async () => {
  const lease = await acquire();
  const request = randomUUID();
  await commit(lease, { request });
  await expire('world-a');
  const next = await acquire('world-a', 'server-b');
  refused(await commit(lease, { request }), 'lease_lost');
  assert.equal(next.head_generation, 1);
  assert.equal((await commit(next, { server: 'server-b', head: 1, generation: 2 })).ok, true);
  assert.equal((await counts()).checkpoints, 2);
});

test('pre-existing historical generations seed the first head without being rewritten', async () => {
  await db.query(`INSERT INTO pf_fencing_draft.checkpoints(world_id,generation,envelope_version,checksum,payload)
    VALUES ('world-a',88,1,$1,$2::jsonb)`, [checksum, JSON.stringify(payload)]);
  const lease = await acquire();
  assert.equal(lease.head_generation, 88);
  assert.equal((await commit(lease, { head: 88, generation: 91 })).ok, true);
  assert.deepEqual((await db.query('SELECT generation::text FROM pf_fencing_draft.checkpoints ORDER BY generation')).rows,
    [{ generation: '88' }, { generation: '91' }]);
});

test('rollback discards checkpoint, head update and receipt as a single unit', async () => {
  const lease = await acquire();
  const request = randomUUID();
  const before = await counts();
  await assert.rejects(db.transaction(async tx => {
    assert.equal((await invoke(tx, 'commit', commitArgs(lease, { request }))).ok, true);
    throw new Error('fixture failure after commit function, before transaction commit');
  }), /fixture failure/);
  assert.deepEqual(await counts(), before);
  assert.equal((await db.query('SELECT head_generation FROM pf_fencing_draft.worlds')).rows[0].head_generation, null);
  assert.equal((await commit(lease, { request })).ok, true);
});

test('invalid acquisition identity, TTL or request never creates state', async () => {
  for (const values of [
    ['', 'server-a', randomUUID(), 30], ['../world', 'server-a', randomUUID(), 30],
    ['x'.repeat(129), 'server-a', randomUUID(), 30], ['world-a', '', randomUUID(), 30],
    ['world-a', 'server-a', null, 30], ['world-a', 'server-a', randomUUID(), null],
    ['world-a', 'server-a', randomUUID(), 4], ['world-a', 'server-a', randomUUID(), 121],
  ]) refused(await invoke(db, 'acquire', values), 'invalid_acquire');
  assert.deepEqual(await counts(), { worlds: 0, checkpoints: 0, receipts: 0 });
});

test('invalid checkpoint shapes, numbers and digest lengths leave all state unchanged', async () => {
  const lease = await acquire();
  const before = await counts();
  for (const changed of [{ generation: -1 }, { generation: '9007199254740992' }, { version: 0 },
    { checksum: 'not-sha256' }, { checksum: 'Z'.repeat(64) }, { payload: null },
    { payload: [] }, { payload: 'text' }, { payload: { huge: 'x'.repeat(6291457) } }]) {
    refused(await commit(lease, changed), 'invalid_checkpoint');
  }
  assert.deepEqual(await counts(), before);
});

test('expired or infinite checkpoint retention is refused; maximum safe generation is preserved', async () => {
  const lease = await acquire();
  refused(await commit(lease, { expires: '2000-01-01T00:00:00Z' }), 'checkpoint_expired');
  refused(await commit(lease, { expires: 'infinity' }), 'checkpoint_expired');
  assert.equal((await commit(lease, { generation: '9007199254740991' })).ok, true);
  assert.equal((await db.query('SELECT generation::text FROM pf_fencing_draft.checkpoints')).rows[0].generation, '9007199254740991');
});

test('large fencing epochs are exact strings and exhaustion fails without wraparound', async () => {
  let lease = await acquire();
  await release(lease);
  await db.exec("UPDATE pf_fencing_draft.worlds SET fence=9007199254740992 WHERE world_id='world-a'");
  lease = await acquire();
  assert.equal(lease.fence, '9007199254740993');
  assert.equal((await commit(lease)).ok, true);
  await release(lease);
  await db.exec("UPDATE pf_fencing_draft.worlds SET fence=9223372036854775807 WHERE world_id='world-a'");
  const before = await counts();
  refused(await acquire(), 'fence_exhausted');
  assert.deepEqual(await counts(), before);
});

test('missing worlds and malformed typed arguments cannot create a checkpoint', async () => {
  const unknown = { world_id: 'absent', lease_id: randomUUID(), fence: '1' };
  refused(await commit(unknown), 'world_not_found');
  refused(await renew(unknown), 'world_not_found');
  refused(await release(unknown), 'world_not_found');
  await assert.rejects(invoke(db, 'acquire', ['world-a', 'server-a', 'bad-uuid', 30]), /uuid/i);
  assert.deepEqual(await counts(), { worlds: 0, checkpoints: 0, receipts: 0 });
});

test('expiry uses the actual database clock even inside an older transaction', async () => {
  const lease = await acquire();
  await db.transaction(async tx => {
    await tx.query(`UPDATE pf_fencing_draft.worlds
      SET lease_expires_at=clock_timestamp()+interval '20 milliseconds' WHERE world_id=$1`, [lease.world_id]);
    await tx.query('SELECT pg_sleep(0.05)');
    const clocks = (await tx.query(`SELECT now()<lease_expires_at AS transaction_clock_still_valid,
      clock_timestamp()>lease_expires_at AS actual_clock_expired FROM pf_fencing_draft.worlds`)).rows[0];
    assert.equal(clocks.transaction_clock_still_valid, true);
    assert.equal(clocks.actual_clock_expired, true);
    refused(await invoke(tx, 'commit', commitArgs(lease)), 'lease_expired');
    refused(await invoke(tx, 'renew', [lease.world_id,'server-a',lease.lease_id,lease.fence,randomUUID(),30]), 'lease_expired');
  });
});

test('prototype has no PUBLIC entry point and untrusted role cannot acquire or edit state', async () => {
  await db.exec('SET ROLE pf_fencing_untrusted');
  try {
    await assert.rejects(acquire(), /permission denied/i);
    await assert.rejects(db.exec("UPDATE pf_fencing_draft.worlds SET active=false"), /permission denied/i);
  } finally {
    await db.exec('RESET ROLE');
  }
  assert.deepEqual(await counts(), { worlds: 0, checkpoints: 0, receipts: 0 });
});

test('long takeover chain preserves immutable history and never admits a retired epoch', async () => {
  const retired = [];
  let head = null;
  for (let epoch = 1; epoch <= 24; epoch++) {
    const lease = await acquire();
    assert.equal(lease.fence, String(epoch));
    assert.equal(lease.head_generation, head);
    const generation = epoch * 3;
    assert.equal((await commit(lease, { head, generation })).ok, true);
    head = generation;
    for (const stale of retired.slice(-3)) {
      refused(await commit(stale, { head, generation: generation+1 }), 'lease_lost');
      refused(await release(stale), 'lease_lost');
    }
    await release(lease);
    retired.push(lease);
  }
  assert.equal((await counts()).checkpoints, 24);
  assert.deepEqual((await db.query('SELECT generation::int FROM pf_fencing_draft.checkpoints ORDER BY generation')).rows,
    Array.from({ length: 24 }, (_, i) => ({ generation: (i+1)*3 })));
});

test('native Unreal sha256-prefixed checkpoint checksum is accepted and bare hex refused', async () => {
  const lease = await acquire();
  const native = 'sha256:47d0a2035da74393511ea2f39463de0a4eea822fc0d822584e3799ab899a17ed';
  assert.equal((await commit(lease, { checksum: native })).ok, true);
  for (const bad of [native.slice(7), native.toUpperCase(), native+'0', 'sha256:'+native.slice(7).toUpperCase()]) {
    refused(await commit(lease, { head: 1, generation: 2, checksum: bad }), 'invalid_checkpoint');
  }
});

test('genuine frozen Unreal upload preserves historical bytes, generation and saved UTC across commit retry', async () => {
  const fixture = JSON.parse(await readFile(new URL('../../Tests/Fixtures/M8_c3d221f/fixture.json', import.meta.url), 'utf8'));
  const original = Buffer.from(fixture.envelopeBase64, 'base64');
  const nativeChecksum = 'sha256:' + createHash('sha256').update(original).digest('hex');
  assert.equal(nativeChecksum, 'sha256:47d0a2035da74393511ea2f39463de0a4eea822fc0d822584e3799ab899a17ed');
  assert.equal(original.readBigUInt64LE(8), BigInt(fixture.generation));
  assert.equal(original.readBigInt64LE(16), BigInt(fixture.savedUtc));
  const lease = await acquire();
  const options = { request: randomUUID(), generation: fixture.generation, checksum: nativeChecksum,
    payload: { format: 'pf-pfs-base64-v1', envelope: fixture.envelopeBase64 } };
  assert.equal((await commit(lease, options)).ok, true);
  assert.equal((await commit(lease, options)).replayed, true);
  const row = (await db.query('SELECT generation::text,checksum,payload FROM pf_fencing_draft.checkpoints')).rows[0];
  assert.equal(row.generation, String(fixture.generation));
  assert.equal(row.checksum, nativeChecksum);
  assert.deepEqual(row.payload, options.payload);
  assert.deepEqual(Buffer.from(row.payload.envelope, 'base64'), original);
  assert.equal(Buffer.from(row.payload.envelope, 'base64').readBigInt64LE(16), BigInt(fixture.savedUtc));
  assert.equal((await counts()).checkpoints, 1);
});

test('maximum native envelope survives bounded validation and SQL commit', async () => {
  const lease = await acquire();
  const raw = Buffer.alloc(32+MAX_PAYLOAD_BYTES,11);
  raw.writeUInt32LE(0x46534650,0); raw.writeUInt32LE(1,4); raw.writeBigUInt64LE(1n,8);
  raw.writeBigInt64LE(1700000000n,16); raw.writeUInt32LE(MAX_PAYLOAD_BYTES,24);
  raw.writeUInt32LE(crc32(raw.subarray(32)),28);
  const input = { world_id:'world-a',generation:1,envelope_version:1,
    checksum:'sha256:'+createHash('sha256').update(raw).digest('hex'),expires_at:null,
    payload:{format:'pf-pfs-base64-v1',envelope:raw.toString('base64')} };
  const request = new Request('https://offline.invalid/checkpoint',{
    method:'POST',headers:{'content-type':'application/json'},body:JSON.stringify(input)});
  const checked = await readCheckpointRequest(request,'world-a');
  assert.equal(checked.ok,true);
  assert.equal((await commit(lease,{checksum:checked.checkpoint.checksum,payload:checked.checkpoint.payload})).ok,true);
  const saved = (await db.query('SELECT payload FROM pf_fencing_draft.checkpoints')).rows[0].payload;
  assert.equal(saved.envelope,input.payload.envelope);
  assert.equal((await counts()).checkpoints,1);
});

test('validated historical ingress commits unchanged and corruption is refused before SQL', async () => {
  const fixture = JSON.parse(await readFile(new URL('../../Tests/Fixtures/M8_c3d221f/fixture.json', import.meta.url), 'utf8'));
  const raw = Buffer.from(fixture.envelopeBase64,'base64');
  const input = {world_id:'world-a',generation:1,envelope_version:1,expires_at:null,
    checksum:'sha256:'+createHash('sha256').update(raw).digest('hex'),
    payload:{format:'pf-pfs-base64-v1',envelope:fixture.envelopeBase64}};
  const lease = await acquire();
  const process = async value => {
    const checked = await readCheckpointRequest(new Request('https://offline.invalid/checkpoint',{
      method:'POST',headers:{'content-type':'application/json'},body:JSON.stringify(value)}),'world-a');
    if (!checked.ok) return checked;
    return commit(lease,{generation:checked.checkpoint.generation,
      checksum:checked.checkpoint.checksum,payload:checked.checkpoint.payload});
  };
  const before = await counts();
  refused(await process({...input,checksum:'sha256:'+'0'.repeat(64)}),'sha256_mismatch');
  assert.deepEqual(await counts(),before);
  assert.equal((await process(input)).ok,true);
  const saved = (await db.query('SELECT generation::text,checksum,payload FROM pf_fencing_draft.checkpoints')).rows[0];
  assert.equal(saved.generation,'1'); assert.equal(saved.checksum,input.checksum);
  assert.equal(saved.payload.envelope,fixture.envelopeBase64);
  assert.equal(Buffer.from(saved.payload.envelope,'base64').readBigInt64LE(16),BigInt(fixture.savedUtc));
});
