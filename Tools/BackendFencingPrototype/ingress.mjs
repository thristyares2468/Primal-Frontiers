// OFFLINE PROTOTYPE. No Worker handler, authentication, database or file I/O.
export const MAX_PAYLOAD_BYTES = 4 * 1024 * 1024;
export const MAX_REQUEST_BYTES = 6 * 1024 * 1024;
const HEADER_BYTES = 32;
const MAX_GENERATION = 9007199254740991n;
const MAX_BASE64_CHARS = Math.ceil((MAX_PAYLOAD_BYTES + HEADER_BYTES) / 3) * 4;
const fail = error => ({ ok: false, error });
const object = value => value !== null && typeof value === 'object' && !Array.isArray(value);
const validId = value => typeof value === 'string' && /^[A-Za-z0-9._:-]{1,128}$/.test(value);

function generation(value) {
  if (typeof value === 'number') return Number.isSafeInteger(value) && value >= 1 ? BigInt(value) : null;
  if (typeof value !== 'string' || !/^[0-9]{1,16}$/.test(value)) return null;
  const parsed = BigInt(value);
  return parsed >= 1n && parsed <= MAX_GENERATION ? parsed : null;
}

function expiration(value) {
  if (value === null) return null;
  if (typeof value !== 'string') return NaN;
  const m = /^(\d{4})-(\d{2})-(\d{2})T(\d{2}):(\d{2}):(\d{2})(?:\.\d{1,7})?(Z|[+-]\d{2}:\d{2})$/.exec(value);
  if (!m) return NaN;
  const [year, month, day, hour, minute, second] = m.slice(1,7).map(Number);
  const days = [31, (year%4 === 0 && (year%100 !== 0 || year%400 === 0)) ? 29 : 28,
    31,30,31,30,31,31,30,31,30,31];
  if (year < 1 || month < 1 || month > 12 || day < 1 || day > days[month-1]
      || hour > 23 || minute > 59 || second > 59) return NaN;
  return Date.parse(value);
}

const crcTable = Uint32Array.from({ length: 256 }, (_, index) => {
  let c = index;
  for (let i=0; i<8; i++) c = (c & 1) ? 0xedb88320 ^ (c >>> 1) : c >>> 1;
  return c >>> 0;
});
function crc32(bytes) {
  let crc = 0xffffffff;
  for (const byte of bytes) crc = crcTable[(crc ^ byte) & 255] ^ (crc >>> 8);
  return (crc ^ 0xffffffff) >>> 0;
}

/** expectedWorld/minimumGeneration are trusted adapter context, NOT caller identity.
 * Successful output snapshots exact immutable strings; no food/world time is redated. */
export async function validateCheckpoint(record, expectedWorld, minimumGeneration = 1) {
  const floor = generation(minimumGeneration);
  if (!validId(expectedWorld) || floor === null) return fail('invalid_context');
  if (!object(record)) return fail('invalid_checkpoint');
  // Snapshot every caller value before the asynchronous digest, avoiding TOCTOU.
  const world = record.world_id, gen = generation(record.generation), version = record.envelope_version;
  const checksum = record.checksum, expires = record.expires_at;
  const payload = record.payload;
  const format = object(payload) ? payload.format : undefined;
  const encoded = object(payload) ? payload.envelope : undefined;
  if (!validId(world)) return fail('invalid_world_id');
  if (world !== expectedWorld) return fail('world_mismatch');
  if (gen === null) return fail('invalid_generation');
  if (gen < floor) return fail('stale_generation');
  if (version !== 1) return fail('invalid_envelope_version');
  if (typeof checksum !== 'string' || !/^sha256:[a-f0-9]{64}$/.test(checksum)) return fail('invalid_checksum');
  const expiry = expiration(expires);
  if (Number.isNaN(expiry)) return fail('invalid_expiration');
  if (expiry !== null && expiry <= Date.now()) return fail('checkpoint_expired');
  if (format !== 'pf-pfs-base64-v1' || typeof encoded !== 'string'
      || Object.keys(payload).length !== 2) return fail('invalid_payload_format');
  if (encoded.length > MAX_BASE64_CHARS) return fail('envelope_too_large');
  if (encoded.length === 0 || encoded.length%4 !== 0 || !/^[A-Za-z0-9+/]*={0,2}$/.test(encoded)) return fail('invalid_base64');
  let bytes;
  try {
    const binary = atob(encoded);
    if (btoa(binary) !== encoded) return fail('invalid_base64');
    bytes = new Uint8Array(binary.length);
    for (let i=0; i<binary.length; i++) bytes[i] = binary.charCodeAt(i);
  } catch { return fail('invalid_base64'); }
  if (bytes.length < HEADER_BYTES || bytes.length > HEADER_BYTES + MAX_PAYLOAD_BYTES) return fail('invalid_envelope_size');
  const view = new DataView(bytes.buffer, bytes.byteOffset, bytes.byteLength);
  const savedUtc = view.getBigInt64(16, true), length = view.getUint32(24, true);
  if (view.getUint32(0, true) !== 0x46534650 || view.getUint32(4, true) !== 1
      || view.getBigUint64(8, true) !== gen || savedUtc <= 0n
      || savedUtc > BigInt(Math.floor(Date.now()/1000) + 60)
      || length === 0 || length !== bytes.length - HEADER_BYTES) return fail('invalid_envelope_header');
  let actual;
  try {
    const digest = new Uint8Array(await globalThis.crypto.subtle.digest('SHA-256', bytes));
    actual = 'sha256:' + Array.from(digest, byte => byte.toString(16).padStart(2,'0')).join('');
  } catch { return fail('checksum_unavailable'); }
  if (actual !== checksum) return fail('sha256_mismatch');
  if (crc32(bytes.subarray(HEADER_BYTES)) !== view.getUint32(28, true)) return fail('payload_crc_mismatch');
  // Expiry may have passed while digesting; fail closed before publishing output.
  if (expiry !== null && expiry <= Date.now()) return fail('checkpoint_expired');
  return { ok: true, checkpoint: Object.freeze({ world_id: world, generation: Number(gen),
    envelope_version: version, checksum, payload: Object.freeze({ format, envelope: encoded }), expires_at: expires }),
    file: Object.freeze({ generation: Number(gen), savedUtc: Number(savedUtc), payloadBytes: length }) };
}

/** Incremental UTF8 byte limit and whole-body deadline. No request.json() before bounds.
 * This validates data only; a future route must authenticate BEFORE invoking it. */
export async function readCheckpointRequest(request, expectedWorld, { minimumGeneration=1, timeoutMs=5000 } = {}) {
  const refuseBody = error => {
    if (request.body && !request.body.locked) void request.body.cancel().catch(() => {});
    return fail(error);
  };
  if (!validId(expectedWorld) || generation(minimumGeneration) === null
      || !Number.isInteger(timeoutMs) || timeoutMs < 1 || timeoutMs > 30000) return refuseBody('invalid_context');
  if (request.method !== 'POST') return refuseBody('invalid_method');
  if (request.headers.get('content-type')?.split(';')[0].trim().toLowerCase() !== 'application/json') return refuseBody('invalid_content_type');
  const encoding = request.headers.get('content-encoding');
  if (encoding && encoding.trim().toLowerCase() !== 'identity') return refuseBody('unsupported_encoding');
  const declared = request.headers.get('content-length');
  if (declared !== null && !/^[0-9]{1,10}$/.test(declared)) return refuseBody('invalid_content_length');
  if (declared !== null && Number(declared) > MAX_REQUEST_BYTES) return refuseBody('request_too_large');
  if (!request.body || request.bodyUsed) return refuseBody('body_unreadable');
  let reader;
  try { reader = request.body.getReader(); } catch { return fail('body_unreadable'); }
  let timer, done = false, total = 0, chunkCount = 0;
  const deadline = Date.now() + timeoutMs;
  const chunks = [];
  const timeout = new Promise((_, reject) => { timer = setTimeout(() => reject(new Error('body_timeout')), timeoutMs); });
  try {
    while (true) {
      // Also bound already-resolved microtasks: an endless empty-chunk producer
      // must not starve the timer or allocate an unbounded chunks array.
      if (Date.now() >= deadline) return fail('body_timeout');
      const part = await Promise.race([reader.read(), timeout]);
      if (part.done) { done = true; break; }
      if (!(part.value instanceof Uint8Array)) return fail('body_unreadable');
      if (++chunkCount > 65536) return fail('body_too_fragmented');
      total += part.value.byteLength;
      if (total > MAX_REQUEST_BYTES) return fail('request_too_large');
      // A stream producer can reuse its buffer; snapshot each chunk immediately.
      chunks.push(part.value.slice());
    }
  } catch (error) {
    return fail(error?.message === 'body_timeout' ? 'body_timeout' : 'body_unreadable');
  } finally {
    clearTimeout(timer);
    if (!done) void reader.cancel().catch(() => {});
    reader.releaseLock();
  }
  if (declared !== null && total !== Number(declared)) return fail('invalid_content_length');
  const bytes = new Uint8Array(total);
  let offset = 0;
  for (const chunk of chunks) { bytes.set(chunk, offset); offset += chunk.length; }
  let record;
  try { record = JSON.parse(new TextDecoder('utf-8', { fatal: true }).decode(bytes)); }
  catch { return fail('invalid_json'); }
  return validateCheckpoint(record, expectedWorld, minimumGeneration);
}
