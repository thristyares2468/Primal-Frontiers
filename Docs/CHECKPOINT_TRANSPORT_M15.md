# M15 development checkpoint transport

October 18, 2026. Bounded integration of the existing local save format with the development Cloudflare/Neon API. No map/asset or save-schema migration.

Bounded task: https://trello.com/c/q0JFyXMw. Full M15 parent: https://trello.com/c/DDrtiIWC remains Doing.

## Verified provider boundary

Owner supplied READY/neondb, authenticated POST/GET, duplicate generation_exists, and CHECKPOINT_VERIFY_PASS for dev-neon-check-2. Temporary secret was rotated according to the owner. Do not reuse the exposed test value. Build-trigger secrets and runtime Worker secrets are separate; runtime bindings resolved the previous unauthorized/NOT_CONFIGURED responses. These are provider API observations, not Unreal transport passes.

## Contract

Built-in Unreal HTTP and engine-bundled OpenSSL are the added build dependencies. No external SDK/plugin/download. Dedicated-server gameplay world plus authoritative GameMode is required before process environment credentials can be read. Clients, listen servers, standalone and editor worlds cannot initiate this transport. No RPC or replicated credentials.

The allowlisted HTTPS endpoint is https://primalfrontier-backend.jackherbig09.workers.dev/v1/checkpoints. Only the dedicated-server process environment supplies PF_API_SHARED_SECRET; it is never read from committed config/assets or accepted as a console argument. No request bodies, response bodies or secrets are logged. Thirty-second request deadline and 6MiB streaming response cap. No automatic retries after ambiguous timeouts or duplicate409; local save is preserved.

Upload inspects an existing local world-save slot and sends its fileV1 envelope as payload.format=pf-pfs-base64-v1 plus payload.envelope Base64. Outer envelope_version is1 (file format, not world schema). SHA256 covers exact envelope bytes; the existing inner CRC is checked independently. Original generation and SavedUtc are preserved. Remote generation must be1..9007199254740991 for JSON integer precision; Neon bigint decimal strings are supported on reads. Checkpoints currently have null retention expiry; food expiration remains inside the saved payload/original UTC and is never reset by transport.

Fetch validates world ID, supported file version, caller minimum generation, expiration, digest, Base64, inner header/generation/UTC/CRC before returning a memory candidate. Failed reads preserve caller output. No local file is overwritten and no live world is restored. Full world/catalog/layout and ownership/session admission remain separate restoration requirements. A matching201 acknowledgment is required for upload success;200,409,401,503, malformed acknowledgments and transport failures are not successful saves.

## Developer commands (not a manual playtest requirement)

No level is required for the automated fixture. The normal open-world level remains `/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld`; these commands must run on its dedicated server, never in a player's standalone/client console. They do not install, start or allocate a hosted server.

1. Before launching an already configured dedicated-server process, set `$env:PF_API_SHARED_SECRET = Read-Host 'Paste current runtime secret'` in its parent PowerShell. Enter the long secret only when prompted, with no added quotation marks. Nothing is stored permanently; a running process cannot inherit variables set later. Never put the secret in Unreal command arguments, assets, config or Git.
2. Start the project's dedicated-server workflow from that same parent session. A packaged server is not currently available from the installed Launcher engine; do not assume the game executable is a packaged Server. Source package/GSDK setup remains separate. An uncooked Editor `-server -nullrhi` development launch can exercise the adapter when explicitly configured for a disposable test world.
3. In that SERVER console, create an ordinary local save with `PF.SaveWorld M15Disposable`, or use an existing inspected disposable slot. Saving is explicit and does not upload automatically.
4. Run `PF.UploadCheckpoint M15Disposable dev-disposable-unique-world`. Wait for a terminal `CheckpointComplete` or `CheckpointRejected` matching the queued request ID. `CheckpointPending` is NOT success. Uploading the same saved generation again is expected HTTP409/refusal; it never overwrites the remote row.
5. Run `PF.VerifyCheckpoint dev-disposable-unique-world`. Expected terminal success means file-envelope identity/UTC/generation/CRC/SHA256 passed in memory. No world is loaded, no file is published, and no catalog/layout restore is promised. Existing manual JSON probes (`dev-neon-check-2`) are not file-envelope fixtures and are deliberately rejected.
6. Run `PF.ExportTestReport M15Checkpoint` AFTER terminal results appear. Preserve its unique Saved/AutomationReports JSON and the server log. On any rejection, report only the request ID/status/sanitized details; never credentials or full save payloads. Do not treat a queued request or local save as remote durability.

## Verification

Installed engine is5.8.3 CL58210709 (project instructions request5.8.2; engine was not changed). Initial Editor34.52s succeeded. First PF.Checkpoints run `Automation_M15Checkpoints_20261018_064817014_e6c18db9` exited3 after BoundsAndIntegrity passed: generic platform SHA256 asserted `No SHA256 Platform implementation`; no complete report. Failure/log retained. Engine-bundled OpenSSL replacement built23.59s, and `Automation_M15CheckpointRetry_20261018_065033561_d82f4b24` passed4/4, zero severity.

Command/HTTP-fixture integration built Editor19.04s; final fixture guard Editor5.76s/Game35.39s passed with no compiler warnings. Final NullRHI run18.16s, process exit0, no timeout, fatal/ensure, raw/test warnings or errors;3.075GiB working/2.953GiB private sampled peak. This is low-cost headless evidence, not rendered FPS/stutter certification.

Final report: `Saved/AutomationReports/Automation_M15CheckpointFinal_20261018_065359814_ccc19e3b/index.json` and `run-summary.json`. Raw log: `Saved/Logs/PFAutomation_M15CheckpointFinal_20261018_065359814_ccc19e3b.log`. Ordinary `Saved/Logs/PrimalFrontier.log` is an older October14 user D3D12 crash session, not this run; no new rendering launch was performed.

Exact10/10 passed:

- PF.Checkpoints.BoundsAndIntegrity
- PF.Checkpoints.FrozenEnvelopeRoundTrip
- PF.Checkpoints.HttpRoundTrip
- PF.Checkpoints.ResponseRefusals
- PF.Checkpoints.ServerAuthority
- PF.Persistence.FileGenerations
- PF.Persistence.FrozenM8Records
- PF.Persistence.FrozenM8Restore
- PF.Persistence.OriginalBackup
- PF.PrimalAgentTools.CommandArguments (includes actual PF.Help, discoverability and new command grammar/null-world refusals)

HTTP fixture uses engine-bundled Python standard library, loopback ephemeral port, fake fixture-only credentials and a90s self-exit. Owned process is terminated/closed and port file removed after the test. It independently checks POST SHA256 with Python hashlib and returns Neon-style bigint strings. Eight real HTTP cases:201ack,200fetch,409duplicate,401wrong key,200corrupt digest refusal,404missing,503outage,200discarded when server-world validity ends. Public production APIs still enforce authority; only the friend test can inject loopback/fake credentials. No Cloudflare/Neon data or real secret is used by this test. Frozen manifest SHA256 independently matches; owner/item/food-UTC bytes remain exact. No automatic retry/restore or two-client network pass inferred.

## Remaining gates

Real authenticated Unreal HTTPS create/fetch/duplicate test with a fresh disposable world; automatic durable startup restore before admission; rollback/concurrency/session leasing and ambiguous write reconciliation; production PlayFab/EOS authentication; source-built packaged server/GSDK and real container recycle/reallocation. Shared-secret development auth is not production identity. Current Worker permits unvalidated arbitrary development payloads and does not yet enforce a single active writer. Existing local save/load remains the supported player flow. No invented map fixture or manual gameplay pass for this backend-only task.
