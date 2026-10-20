# Synthetic SQL lease replies

`fixture.json` contains six actual results from the pinned, in-memory PostgreSQL
draft contract, captured by `Tools/BackendFencingPrototype/capture-lease-replies.mjs`.
World/server/lease IDs are disposable synthetic identities, never credentials.
The payload used for commit was synthetic, not a valid Unreal save; this fixture
certifies reply shape only. PostgreSQL version and exact contract SHA256 are in
the file. It intentionally includes MAX_int64 fencing and MAX_SAFE_INTEGER
generation to detect precision loss, and a null acquire head for an empty world.

Normal Node/native tests read this immutable fixture; they never regenerate it.
The generator refuses to overwrite an existing file. UTC values are historical
metadata; the native decoder must not use them to extend lease readiness.
Actual authenticated HTTP, request binding, live grants, true concurrent sessions,
restore and hosted gameplay are separate unimplemented/unverified gates.
