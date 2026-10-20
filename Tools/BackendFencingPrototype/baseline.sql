CREATE TABLE IF NOT EXISTS pf_checkpoints (
  world_id text NOT NULL,
  generation bigint NOT NULL CHECK (generation >= 0),
  envelope_version integer NOT NULL CHECK (envelope_version > 0),
  checksum text NOT NULL,
  payload jsonb NOT NULL,
  created_at timestamptz NOT NULL DEFAULT now(),
  expires_at timestamptz,
  PRIMARY KEY (world_id, generation)
);

CREATE INDEX IF NOT EXISTS pf_checkpoints_world_latest_idx
  ON pf_checkpoints (world_id, generation DESC);
