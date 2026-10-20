-- OFFLINE PROTOTYPE ONLY. Not a migration; never run against a cloud database.
-- No Worker route imports this folder. All names live in a throwaway schema.
CREATE SCHEMA pf_fencing_draft;

-- The existing checkpoint envelope columns/meaning are preserved.
CREATE TABLE pf_fencing_draft.checkpoints (
  world_id text NOT NULL,
  generation bigint NOT NULL CHECK (generation BETWEEN 0 AND 9007199254740991),
  envelope_version integer NOT NULL CHECK (envelope_version > 0),
  checksum text NOT NULL,
  payload jsonb NOT NULL,
  created_at timestamptz NOT NULL DEFAULT clock_timestamp(),
  expires_at timestamptz,
  PRIMARY KEY (world_id, generation)
);
CREATE INDEX checkpoints_latest_idx
  ON pf_fencing_draft.checkpoints(world_id, generation DESC);

-- Keep this row across release/expiry: deleting it would recycle fencing tokens.
CREATE TABLE pf_fencing_draft.worlds (
  world_id text PRIMARY KEY,
  fence bigint NOT NULL DEFAULT 0 CHECK (fence >= 0),
  server_id text,
  lease_id uuid,
  lease_expires_at timestamptz,
  active boolean NOT NULL DEFAULT false,
  head_generation bigint CHECK (head_generation BETWEEN 0 AND 9007199254740991)
);

-- Durable request receipts survive takeover. Never discard them without a separate
-- bounded replay/retention design; this prototype intentionally has no cleanup.
CREATE TABLE pf_fencing_draft.receipts (
  world_id text NOT NULL REFERENCES pf_fencing_draft.worlds(world_id),
  operation text NOT NULL CHECK (operation IN ('acquire','renew','commit')),
  request_id uuid NOT NULL,
  arguments jsonb NOT NULL,
  result jsonb NOT NULL,
  PRIMARY KEY (world_id, operation, request_id)
);

CREATE FUNCTION pf_fencing_draft.valid_id(value text) RETURNS boolean
LANGUAGE sql IMMUTABLE SET search_path = pg_catalog
AS $$ SELECT value IS NOT NULL AND value ~ '^[A-Za-z0-9._:-]{1,128}$' $$;

CREATE FUNCTION pf_fencing_draft.lease_error(
  w pf_fencing_draft.worlds, p_server text, p_lease uuid, p_fence bigint
) RETURNS text LANGUAGE plpgsql SET search_path = pg_catalog AS $$
BEGIN
  IF p_server IS NULL OR p_lease IS NULL OR p_fence IS NULL
     OR w.server_id IS DISTINCT FROM p_server OR w.lease_id IS DISTINCT FROM p_lease
     OR w.fence IS DISTINCT FROM p_fence THEN RETURN 'lease_lost'; END IF;
  IF NOT w.active OR w.lease_expires_at IS NULL
     OR w.lease_expires_at <= clock_timestamp() THEN RETURN 'lease_expired'; END IF;
  RETURN NULL;
END $$;

CREATE FUNCTION pf_fencing_draft.acquire(
  p_world text, p_server text, p_request uuid, p_ttl integer
) RETURNS jsonb LANGUAGE plpgsql SET search_path = pg_catalog AS $$
DECLARE
  w pf_fencing_draft.worlds%ROWTYPE;
  r pf_fencing_draft.receipts%ROWTYPE;
  args jsonb;
  result jsonb;
  deadline timestamptz;
  new_lease uuid;
BEGIN
  IF NOT pf_fencing_draft.valid_id(p_world) OR NOT pf_fencing_draft.valid_id(p_server)
     OR p_request IS NULL OR p_ttl IS NULL OR p_ttl NOT BETWEEN 5 AND 120 THEN
    RETURN jsonb_build_object('ok',false,'error','invalid_acquire');
  END IF;
  -- A pre-existing historical head is seeded only on first acquisition; all
  -- subsequent mutations must pass this same world-row lock. Legacy writes must
  -- be disabled during a real cutover (not implemented by this prototype).
  INSERT INTO pf_fencing_draft.worlds(world_id,head_generation)
    SELECT p_world,max(generation) FROM pf_fencing_draft.checkpoints WHERE world_id=p_world
    ON CONFLICT (world_id) DO NOTHING;
  SELECT * INTO STRICT w FROM pf_fencing_draft.worlds WHERE world_id=p_world FOR UPDATE;
  args := jsonb_build_object('server',p_server,'ttl',p_ttl);
  SELECT * INTO r FROM pf_fencing_draft.receipts
    WHERE world_id=p_world AND operation='acquire' AND request_id=p_request;
  IF FOUND THEN
    IF r.arguments IS DISTINCT FROM args THEN
      RETURN jsonb_build_object('ok',false,'error','request_conflict');
    END IF;
    IF w.fence::text IS DISTINCT FROM r.result->>'fence'
       OR w.lease_id::text IS DISTINCT FROM r.result->>'lease_id' THEN
      RETURN jsonb_build_object('ok',false,'error','lease_lost');
    END IF;
    IF NOT w.active OR w.lease_expires_at <= clock_timestamp() THEN
      RETURN jsonb_build_object('ok',false,'error','lease_expired');
    END IF;
    -- Replay never extends the original deadline or allocates another fence.
    RETURN r.result || jsonb_build_object('replayed',true);
  END IF;
  IF w.active AND w.lease_expires_at > clock_timestamp() THEN
    RETURN jsonb_build_object('ok',false,'error','lease_held');
  END IF;
  IF w.fence = 9223372036854775807 THEN
    RETURN jsonb_build_object('ok',false,'error','fence_exhausted');
  END IF;
  deadline := clock_timestamp() + make_interval(secs=>p_ttl);
  new_lease := gen_random_uuid();
  UPDATE pf_fencing_draft.worlds SET fence=w.fence+1,server_id=p_server,
    lease_id=new_lease,lease_expires_at=deadline,active=true WHERE world_id=p_world;
  result := jsonb_build_object('ok',true,'world_id',p_world,'lease_id',new_lease,
    'fence',(w.fence+1)::text,'expires_at',deadline,'head_generation',w.head_generation);
  INSERT INTO pf_fencing_draft.receipts VALUES(p_world,'acquire',p_request,args,result);
  RETURN result;
END $$;

CREATE FUNCTION pf_fencing_draft.renew(
  p_world text,p_server text,p_lease uuid,p_fence bigint,p_request uuid,p_ttl integer
) RETURNS jsonb LANGUAGE plpgsql SET search_path = pg_catalog AS $$
DECLARE
  w pf_fencing_draft.worlds%ROWTYPE;
  r pf_fencing_draft.receipts%ROWTYPE;
  args jsonb;
  result jsonb;
  problem text;
  deadline timestamptz;
BEGIN
  IF p_request IS NULL OR p_ttl IS NULL OR p_ttl NOT BETWEEN 5 AND 120 THEN
    RETURN jsonb_build_object('ok',false,'error','invalid_renew');
  END IF;
  SELECT * INTO w FROM pf_fencing_draft.worlds WHERE world_id=p_world FOR UPDATE;
  IF NOT FOUND THEN RETURN jsonb_build_object('ok',false,'error','world_not_found'); END IF;
  problem := pf_fencing_draft.lease_error(w,p_server,p_lease,p_fence);
  IF problem IS NOT NULL THEN RETURN jsonb_build_object('ok',false,'error',problem); END IF;
  args := jsonb_build_object('server',p_server,'lease_id',p_lease,'fence',p_fence::text,'ttl',p_ttl);
  SELECT * INTO r FROM pf_fencing_draft.receipts
    WHERE world_id=p_world AND operation='renew' AND request_id=p_request;
  IF FOUND THEN
    IF r.arguments IS DISTINCT FROM args THEN RETURN jsonb_build_object('ok',false,'error','request_conflict'); END IF;
    RETURN r.result || jsonb_build_object('replayed',true);
  END IF;
  deadline := clock_timestamp() + make_interval(secs=>p_ttl);
  -- A retry with a shorter TTL may not shorten a still-valid lease.
  deadline := greatest(deadline,w.lease_expires_at);
  UPDATE pf_fencing_draft.worlds SET lease_expires_at=deadline WHERE world_id=p_world;
  result := jsonb_build_object('ok',true,'lease_id',p_lease,'fence',p_fence::text,'expires_at',deadline);
  INSERT INTO pf_fencing_draft.receipts VALUES(p_world,'renew',p_request,args,result);
  RETURN result;
END $$;

CREATE FUNCTION pf_fencing_draft.release(
  p_world text,p_server text,p_lease uuid,p_fence bigint
) RETURNS jsonb LANGUAGE plpgsql SET search_path = pg_catalog AS $$
DECLARE w pf_fencing_draft.worlds%ROWTYPE;
BEGIN
  SELECT * INTO w FROM pf_fencing_draft.worlds WHERE world_id=p_world FOR UPDATE;
  IF NOT FOUND THEN RETURN jsonb_build_object('ok',false,'error','world_not_found'); END IF;
  IF p_server IS NULL OR p_lease IS NULL OR p_fence IS NULL
     OR w.server_id IS DISTINCT FROM p_server OR w.lease_id IS DISTINCT FROM p_lease
     OR w.fence IS DISTINCT FROM p_fence THEN
    RETURN jsonb_build_object('ok',false,'error','lease_lost');
  END IF;
  -- Same-epoch release is naturally idempotent and never resets the fence/head.
  UPDATE pf_fencing_draft.worlds SET active=false WHERE world_id=p_world;
  RETURN jsonb_build_object('ok',true,'released',true,'fence',w.fence::text);
END $$;

CREATE FUNCTION pf_fencing_draft.commit_checkpoint(
  p_world text,p_server text,p_lease uuid,p_fence bigint,p_request uuid,
  p_expected_head bigint,p_generation bigint,p_version integer,p_checksum text,
  p_payload jsonb,p_expires timestamptz
) RETURNS jsonb LANGUAGE plpgsql SET search_path = pg_catalog AS $$
DECLARE
  w pf_fencing_draft.worlds%ROWTYPE;
  r pf_fencing_draft.receipts%ROWTYPE;
  args jsonb;
  result jsonb;
  problem text;
BEGIN
  IF p_request IS NULL OR p_generation IS NULL OR p_generation NOT BETWEEN 0 AND 9007199254740991
     OR p_version IS NULL OR p_version < 1 OR p_checksum IS NULL OR p_checksum !~ '^sha256:[a-f0-9]{64}$'
     OR p_payload IS NULL OR jsonb_typeof(p_payload) <> 'object'
     OR octet_length(p_payload::text) > 6291456 THEN
    RETURN jsonb_build_object('ok',false,'error','invalid_checkpoint');
  END IF;
  SELECT * INTO w FROM pf_fencing_draft.worlds WHERE world_id=p_world FOR UPDATE;
  IF NOT FOUND THEN RETURN jsonb_build_object('ok',false,'error','world_not_found'); END IF;
  problem := pf_fencing_draft.lease_error(w,p_server,p_lease,p_fence);
  IF problem IS NOT NULL THEN RETURN jsonb_build_object('ok',false,'error',problem); END IF;
  -- Actual original-envelope decoding/SHA256 validation belongs to a trusted
  -- server adapter, not SQL. This prototype checks shape/immutability only.
  IF p_expires IS NOT NULL AND (NOT isfinite(p_expires) OR p_expires <= clock_timestamp()) THEN
    RETURN jsonb_build_object('ok',false,'error','checkpoint_expired');
  END IF;
  args := jsonb_build_object('server',p_server,'lease_id',p_lease,'fence',p_fence::text,
    'expected_head',p_expected_head,'generation',p_generation,'envelope_version',p_version,
    'checksum',p_checksum,'payload',p_payload,'expires_epoch',extract(epoch FROM p_expires));
  SELECT * INTO r FROM pf_fencing_draft.receipts
    WHERE world_id=p_world AND operation='commit' AND request_id=p_request;
  IF FOUND THEN
    IF r.arguments IS DISTINCT FROM args THEN RETURN jsonb_build_object('ok',false,'error','request_conflict'); END IF;
    RETURN r.result || jsonb_build_object('replayed',true);
  END IF;
  IF w.head_generation IS DISTINCT FROM p_expected_head THEN
    RETURN jsonb_build_object('ok',false,'error','head_conflict','head_generation',w.head_generation);
  END IF;
  IF w.head_generation IS NOT NULL AND p_generation <= w.head_generation THEN
    RETURN jsonb_build_object('ok',false,'error','generation_not_newer');
  END IF;
  INSERT INTO pf_fencing_draft.checkpoints(world_id,generation,envelope_version,checksum,payload,expires_at)
    VALUES(p_world,p_generation,p_version,p_checksum,p_payload,p_expires);
  UPDATE pf_fencing_draft.worlds SET head_generation=p_generation WHERE world_id=p_world;
  result := jsonb_build_object('ok',true,'committed',true,'world_id',p_world,'generation',p_generation,
    'checksum',p_checksum,'fence',p_fence::text);
  INSERT INTO pf_fencing_draft.receipts VALUES(p_world,'commit',p_request,args,result);
  RETURN result;
END $$;

-- This draft is not an authorization boundary. Make accidental PUBLIC access
-- fail closed; runtime role/SECURITY DEFINER design requires a separate review.
REVOKE ALL ON SCHEMA pf_fencing_draft FROM PUBLIC;
REVOKE ALL ON ALL TABLES IN SCHEMA pf_fencing_draft FROM PUBLIC;
REVOKE ALL ON ALL FUNCTIONS IN SCHEMA pf_fencing_draft FROM PUBLIC;
