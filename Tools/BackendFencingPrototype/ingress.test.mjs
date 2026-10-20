import { test, after } from 'node:test';
import assert from 'node:assert/strict';
import { readFile } from 'node:fs/promises';
import { createHash } from 'node:crypto';
import { crc32 } from 'node:zlib';
import { MAX_PAYLOAD_BYTES, MAX_REQUEST_BYTES, validateCheckpoint, readCheckpointRequest } from './ingress.mjs';

const fixture = JSON.parse(await readFile(new URL('../../Tests/Fixtures/M8_c3d221f/fixture.json', import.meta.url), 'utf8'));
const historical = Buffer.from(fixture.envelopeBase64, 'base64');
const sha = bytes => 'sha256:' + createHash('sha256').update(bytes).digest('hex');
function record(bytes = historical, extra = {}) {
  return { world_id: 'world-a', generation: Number(bytes.readBigUInt64LE(8)), envelope_version: 1,
    checksum: sha(bytes), payload: { format: 'pf-pfs-base64-v1', envelope: bytes.toString('base64') },
    expires_at: null, ...extra };
}
function syntheticEnvelope(payloadBytes = Buffer.from('native-format-fixture'), gen = 1n) {
  const bytes = Buffer.alloc(32 + payloadBytes.length);
  bytes.writeUInt32LE(0x46534650, 0); bytes.writeUInt32LE(1, 4);
  bytes.writeBigUInt64LE(gen, 8); bytes.writeBigInt64LE(BigInt(Math.floor(Date.now()/1000)-10), 16);
  bytes.writeUInt32LE(payloadBytes.length, 24); bytes.writeUInt32LE(crc32(payloadBytes), 28);
  payloadBytes.copy(bytes, 32);
  return bytes;
}
const refused = (result, error) => assert.deepEqual(result, { ok: false, error });
const request = (body, headers = {}) => new Request('https://offline.invalid/checkpoint', {
  method: 'POST', headers: { 'content-type': 'application/json', ...headers }, body,
  ...(body instanceof ReadableStream ? { duplex: 'half' } : {}),
});
let peakRss = 0;
const sampler = setInterval(() => { peakRss = Math.max(peakRss, process.memoryUsage().rss); }, 10);
sampler.unref();
after(() => { clearInterval(sampler); console.log('INGRESS_PEAK_RSS_BYTES', Math.max(peakRss, process.memoryUsage().rss)); });

test('genuine historical envelope validates independent SHA and CRC with exact saved UTC', async () => {
  assert.equal(sha(historical), 'sha256:47d0a2035da74393511ea2f39463de0a4eea822fc0d822584e3799ab899a17ed');
  assert.equal(crc32(historical.subarray(32)), historical.readUInt32LE(28));
  const input = record(), result = await validateCheckpoint(input, 'world-a');
  assert.equal(result.ok, true);
  assert.deepEqual(result.checkpoint, input);
  assert.deepEqual(result.file, { generation: 1, savedUtc: fixture.savedUtc, payloadBytes: 2022 });
  assert.equal(result.checkpoint.payload.envelope, fixture.envelopeBase64);
});

test('trusted world and rollback floor are required and independent of caller claims', async () => {
  refused(await validateCheckpoint(record(), '../world'), 'invalid_context');
  refused(await validateCheckpoint(record(), 'world-a', 0), 'invalid_context');
  refused(await validateCheckpoint(record(), 'world-b'), 'world_mismatch');
  refused(await validateCheckpoint(record(historical, { world_id: '../world-a' }), 'world-a'), 'invalid_world_id');
  refused(await validateCheckpoint(record(), 'world-a', 2), 'stale_generation');
});

test('safe number and Neon string generations are supported without precision loss', async () => {
  const safe = record(historical, { generation: '0000000000000001' });
  assert.equal((await validateCheckpoint(safe, 'world-a')).checkpoint.generation, 1);
  for (const value of [0, -1, 1.5, NaN, Infinity, '9007199254740992', '-1', '1e0', null, true]) {
    refused(await validateCheckpoint(record(historical, { generation: value }), 'world-a'), 'invalid_generation');
  }
  const largest = record(syntheticEnvelope(Buffer.from('max-safe-generation'), 9007199254740991n));
  assert.equal((await validateCheckpoint(largest, 'world-a')).checkpoint.generation, 9007199254740991);
});

test('unsupported root version and payload formats are explicit refusals', async () => {
  for (const version of [0, 2, '1', null]) {
    refused(await validateCheckpoint(record(historical, { envelope_version: version }), 'world-a'), 'invalid_envelope_version');
  }
  for (const payload of [null, [], {}, { format: 'other', envelope: fixture.envelopeBase64 },
    { format: 'pf-pfs-base64-v1', envelope: fixture.envelopeBase64, hidden: true }]) {
    refused(await validateCheckpoint(record(historical, { payload }), 'world-a'), 'invalid_payload_format');
  }
});

test('checksum syntax is canonical and actual SHA mismatch is rejected', async () => {
  for (const bad of [sha(historical).slice(7), sha(historical).toUpperCase(), 'sha256:bad', null]) {
    refused(await validateCheckpoint(record(historical, { checksum: bad }), 'world-a'), 'invalid_checksum');
  }
  refused(await validateCheckpoint(record(historical, { checksum: 'sha256:'+'0'.repeat(64) }), 'world-a'), 'sha256_mismatch');
});

test('base64 must be canonical padded text with no whitespace or alternate alphabet', async () => {
  for (const encoded of ['', 'AB==', 'not-base64', fixture.envelopeBase64+'\n',
    fixture.envelopeBase64.slice(0,-1), '____', 'AA===']) {
    const input = record(); input.payload.envelope = encoded;
    refused(await validateCheckpoint(input, 'world-a'), 'invalid_base64');
  }
  const input = record(); input.payload.envelope = 'A'.repeat(Math.ceil((MAX_PAYLOAD_BYTES+33)/3)*4+4);
  refused(await validateCheckpoint(input, 'world-a'), 'envelope_too_large');
});

test('truncated envelope, trailing bytes and forged declared payload length refuse', async () => {
  const small = record(); small.payload.envelope = historical.subarray(0,31).toString('base64');
  refused(await validateCheckpoint(small, 'world-a'), 'invalid_envelope_size');
  refused(await validateCheckpoint(record(Buffer.concat([historical,Buffer.from([0])])), 'world-a'), 'invalid_envelope_header');
  const changed = Buffer.from(historical); changed.writeUInt32LE(2023,24);
  refused(await validateCheckpoint(record(changed), 'world-a'), 'invalid_envelope_header');
  refused(await validateCheckpoint(record(syntheticEnvelope(Buffer.alloc(0))), 'world-a'), 'invalid_envelope_header');
});

test('resigned bad magic, file version, saved UTC and mismatched generation still refuse', async () => {
  for (const mutate of [b=>b.writeUInt32LE(0,0), b=>b.writeUInt32LE(2,4),
    b=>b.writeBigInt64LE(0n,16), b=>b.writeBigInt64LE(-1n,16),
    b=>b.writeBigInt64LE(BigInt(Math.floor(Date.now()/1000)+3600),16)]) {
    const changed = Buffer.from(historical); mutate(changed);
    refused(await validateCheckpoint(record(changed), 'world-a'), 'invalid_envelope_header');
  }
  refused(await validateCheckpoint(record(historical,{generation:2}), 'world-a'), 'invalid_envelope_header');
});

test('inner CRC is checked independently even when altered bytes have a matching SHA', async () => {
  const changed = Buffer.from(historical); changed[32] ^= 1;
  refused(await validateCheckpoint(record(changed), 'world-a'), 'payload_crc_mismatch');
});

test('expiration must be present, real ISO timestamp, future or null', async () => {
  const missing = record(); delete missing.expires_at;
  refused(await validateCheckpoint(missing, 'world-a'), 'invalid_expiration');
  for (const bad of ['bad', 0, '2099-02-31T00:00:00Z', '2099-13-01T00:00:00Z',
    '2099-01-01T24:00:00Z', '2099-01-01', '0000-01-01T00:00:00Z']) {
    refused(await validateCheckpoint(record(historical,{expires_at:bad}), 'world-a'), 'invalid_expiration');
  }
  refused(await validateCheckpoint(record(historical,{expires_at:'2000-01-01T00:00:00Z'}), 'world-a'), 'checkpoint_expired');
  assert.equal((await validateCheckpoint(record(historical,{expires_at:'2099-01-01T10:00:00+10:00'}), 'world-a')).ok,true);
});

test('async digest snapshots caller data and publishes immutable original metadata', async () => {
  const input = record();
  const pending = validateCheckpoint(input, 'world-a');
  input.world_id = 'foreign-world'; input.generation = 2; input.expires_at = '2000-01-01T00:00:00Z';
  input.payload.envelope = 'AAAA'; input.checksum = 'sha256:'+'0'.repeat(64);
  const result = await pending;
  assert.equal(result.ok,true);
  assert.equal(result.checkpoint.world_id,'world-a');
  assert.equal(result.checkpoint.payload.envelope,fixture.envelopeBase64);
  assert.equal(result.file.savedUtc,fixture.savedUtc);
  assert.throws(()=>{result.checkpoint.generation=77;},TypeError);
  assert.throws(()=>{result.checkpoint.payload.envelope='bad';},TypeError);
});

test('maximum native four-MiB payload validates within the six-MiB request budget', async () => {
  const bytes = syntheticEnvelope(Buffer.alloc(MAX_PAYLOAD_BYTES,37));
  const input = record(bytes), text = JSON.stringify(input);
  assert.ok(Buffer.byteLength(text)<MAX_REQUEST_BYTES);
  const result = await readCheckpointRequest(request(text), 'world-a');
  assert.equal(result.ok,true);
  assert.equal(result.file.payloadBytes,MAX_PAYLOAD_BYTES);
  assert.equal(result.checkpoint.payload.envelope,input.payload.envelope);
});

test('split UTF8 stream preserves the exact envelope and counts bytes rather than characters', async () => {
  const input = { ...record(), note:'unicode 🦕' }, text = JSON.stringify(input), bytes = Buffer.from(text);
  const stream = new ReadableStream({ start(c) {
    const middle = bytes.indexOf(Buffer.from('🦕'))+1;
    c.enqueue(bytes.subarray(0,middle)); c.enqueue(bytes.subarray(middle)); c.close();
  } });
  const result = await readCheckpointRequest(request(stream,{'content-length':String(bytes.length)}),'world-a');
  assert.equal(result.ok,true);
  assert.equal(result.checkpoint.payload.envelope,fixture.envelopeBase64);
  refused(await readCheckpointRequest(request(text,{'content-length':String(text.length)}),'world-a'), 'invalid_content_length');
});

test('method, content type and compression are refused before body processing', async () => {
  refused(await readCheckpointRequest(new Request('https://offline.invalid/'), 'world-a'), 'invalid_method');
  refused(await readCheckpointRequest(request('{}',{'content-type':'text/plain'}), 'world-a'), 'invalid_content_type');
  refused(await readCheckpointRequest(request('{}',{'content-encoding':'gzip'}), 'world-a'), 'unsupported_encoding');
  refused(await readCheckpointRequest(request('{}'), 'world-a',{timeoutMs:0}), 'invalid_context');
});

test('declared oversized or invalid lengths are refused and body cancelled', async () => {
  let cancelled = false;
  const stream = new ReadableStream({ cancel(){cancelled=true;} });
  refused(await readCheckpointRequest(request(stream,{'content-length':String(MAX_REQUEST_BYTES+1)}),'world-a'), 'request_too_large');
  assert.equal(cancelled,true);
  for (const length of ['-1','bad','1.5','99999999999999999999']) {
    refused(await readCheckpointRequest(request('{}',{'content-length':length}), 'world-a'), 'invalid_content_length');
  }
});

test('unknown-length stream overflow is bounded and cancelled without trusting a header', async () => {
  let cancelled = false;
  const stream = new ReadableStream({ start(c) {
    c.enqueue(new Uint8Array(MAX_REQUEST_BYTES/2)); c.enqueue(new Uint8Array(MAX_REQUEST_BYTES/2+1));
  }, cancel(){cancelled=true;} });
  refused(await readCheckpointRequest(request(stream),'world-a'),'request_too_large');
  assert.equal(cancelled,true);
});

test('a stalled body times out even if underlying cancellation never completes', async () => {
  let cancelled = false;
  const stream = new ReadableStream({ cancel(){cancelled=true;return new Promise(()=>{});} });
  const started = performance.now();
  refused(await readCheckpointRequest(request(stream),'world-a',{timeoutMs:20}), 'body_timeout');
  assert.equal(cancelled,true);
  assert.ok(performance.now()-started<1000);
});

test('empty-chunk microtasks cannot starve the whole-body deadline', async () => {
  let cancelled = false;
  const stream = new ReadableStream({ pull(c){c.enqueue(new Uint8Array(0));},cancel(){cancelled=true;} });
  const result = await readCheckpointRequest(request(stream),'world-a',{timeoutMs:20});
  assert.equal(result.ok,false);
  assert.ok(['body_timeout','body_too_fragmented'].includes(result.error));
  assert.equal(cancelled,true);
});

test('malformed JSON and invalid UTF8 are explicit refusals', async () => {
  refused(await readCheckpointRequest(request('{bad'),'world-a'),'invalid_json');
  refused(await readCheckpointRequest(request(new Uint8Array([0xff,0xfe])),'world-a'),'invalid_json');
  refused(await readCheckpointRequest(request('[]'),'world-a'),'invalid_checkpoint');
});

test('stream errors and already-consumed requests fail without exposing source messages', async () => {
  const stream = new ReadableStream({start(c){c.error(new Error('private-source-detail'));}});
  refused(await readCheckpointRequest(request(stream),'world-a'),'body_unreadable');
  const used = request('{}'); await used.text();
  refused(await readCheckpointRequest(used,'world-a'),'body_unreadable');
});
