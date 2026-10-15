# PlayFab, Neon, backend and proximity voice setup

October15,2026. User reports Oracle signup failed and requests this replacement guide. Oracle deployment is no longer the active route; its old guide/key are retained but not used for PlayFab. No PlayFab title, payment method, quota, database, backend endpoint or EOS product has been confirmed ready.

## Important cost correction

PlayFab account/title creation and limited MPS evaluation are distinct steps. Microsoft's current [enablement guide](https://learn.microsoft.com/en-us/gaming/playfab/multiplayer/servers/enable-playfab-multiplayer-servers) says a payment method is required to complete Multiplayer Servers enablement. Our prior account-only instructions were free, but must not imply no-card/no-billing-risk server hosting. Owner decides whether to add a payment method/accept metered terms. Do not subscribe to Standard/Premium merely to evaluate; do not enable billable capacity without reviewed terms/limits.

[MPS billing docs](https://learn.microsoft.com/en-us/gaming/playfab/multiplayer/servers/billing-for-thunderhead) list750Dasv4 core-hours in EastUS and NorthEurope, while the [pricing page](https://developer.microsoft.com/en-us/games/products/playfab/pricing/) labels included hours as VM hours. Verify actual account/title meters before deployment; don't assume750hours means a24/7 multi-core world. Standby VMs and egress matter. Free server regions are distant from Australian testers. Budgets/alerts are not a guaranteed spending stop. If a payment method or metered terms are unacceptable, STOP after account/title creation and report that; we have no verified no-card, always-on free replacement ready for this build.

## Intended services

| Service | Role | Initial development choice |
| --- | --- | --- |
| PlayFab Multiplayer Servers | Runs packaged authoritative Unreal game sessions | Limited evaluation, only after exact quota/billing review |
| Neon PostgreSQL | Future accounts/metadata and bounded durable checkpoint records | Free, currently1GB/project and100CU-hours/project/month; sleeps when inactive |
| Cloudflare Workers | Hosts our small authenticated HTTPS backend without another rented VM | Free, currently100000requests/day and10msCPU/invocation; actual payload/CPU limits must be tested |
| Epic Online Services Voice | Voice media service after authenticated room/range integration | Free media candidate, not automatic proximity audio |

Cloudflare is an additional backend-hosting recommendation, not an installed project plugin/dependency or service provisioned here. It runs API requests, not Unreal, continuous voice media or a persistent game process. No paid Workers, domain, R2, load balancer, database add-on or separate VPS is required for account preparation. [Neon limits](https://neon.com/pricing), [Workers limits](https://developers.cloudflare.com/workers/platform/pricing/), [Workers/Neon support](https://developers.cloudflare.com/workers/databases/third-party-integrations/neon/).

## 1. PlayFab — owner account/title first

1. Open https://developer.playfab.com/ and sign up/sign in.
2. In My Studios and Titles, create/reuse studio Primal Frontier and a DEVELOPMENT title Primal Frontier. Keep account MFA enabled where available.
3. Open the title dashboard; record Title ID. It is not a developer secret key. Keep developer keys/account credentials private.
4. Open Multiplayer > Servers and inspect the enablement screen. Do not upload a build, request a server, select a paid subscription or add payment information automatically. Report PLAYFAB_TITLE READY plus Title ID and whether enablement requests a payment method. No screenshot containing keys/card details needed.
5. If you personally accept the required payment method/metered terms, complete Enable through Game Manager. Inspect Quota Summary and usage/billing for exact free-eligible region/SKU, limits and overage behavior; report the nonsecret quota details before any deployment. Do not choose AustraliaEast on the assumption the evaluation is free there.
6. Leave no allocated/running/standby build capacity while package/backend work is pending. A low target standby count is not a billing cap or a replacement for checking actual allocated VMs. Only create a bounded test allocation after all gates below pass; explicitly remove allocation/build capacity after the test and verify usage.

Expected: title ready; optionally reviewed MPS enablement, not a deployed game. Game Manager labels may change; follow the official enablement guide, not guessed paid-plan prompts. No free eligibility inferred from a title simply existing.

## 2. Neon — owner development database

1. Open https://console.neon.tech/ and choose Free (reuse an existing prepared project instead of making duplicates).
2. Create project primalfrontier-dev, closest available region to the backend/game location, stable supported PostgreSQL version; leave scale-to-zero enabled.
3. In database/role controls create development database primalfrontier_dev and a separate app role such as pf_app. Keep owner/admin credentials separate. Agent will define final migration/runtime privileges; do not manually grant broad client permissions.
4. Use Connect to obtain connection information. Keep password/URI in your password manager until configured as a backend secret through a secure local/provider mechanism; never paste it into chat, Trello, Git, Unreal settings or client binaries.
5. Report NEON_PROJECT READY, region and PostgreSQL version only. Creating the project doesn't connect the game. Free quota is for intermittent development, not a guaranteed busy24/7 database; review current plan usage and do not defeat sleeping with constant keepalives.

## 3. Cloudflare Workers — owner backend account, agent code

1. Open https://dash.cloudflare.com/sign-up and create/reuse a Cloudflare account. Verify email and enable MFA.
2. Open Workers & Pages / Workers and confirm the Free plan. A purchased domain is not needed; use the provider's workers.dev endpoint when a reviewed Worker is deployed.
3. Report CLOUDFLARE_ACCOUNT READY. Do not deploy a placeholder API with guessed authentication, connect the game repo to auto-deploy, enable paid Workers or add R2/paid add-ons. Agent supplies the reviewed project and deployment commands first.
4. Agent implements authenticated API, verified game/player identity and owner authorization, rate/payload limits, migrations, private Neon/EOS configuration, logging without secrets and safe failure responses. Use Workers secrets for sensitive values, never a plaintext committed .env. Developer/server credentials must not be distributed as client configuration.
5. Agent tests locally first, then deploys using authorized account access, measures actual Free CPU/request/database consumption and runs a protected round-trip test. If limits fail, report before any paid upgrade. No API endpoint is created or ready in the current project.

## 4. Epic Online Services — owner development product, agent voice

1. Open https://dev.epicgames.com/portal/ and sign in with your intended product-owner account.
2. Create/select your organization and Primal Frontier product; reuse any product you already made. Accept relevant terms yourself.
3. Create/select private development sandbox/deployment; keep Live/public settings separate. Record product/sandbox/deployment IDs privately; report EOS_PRODUCT READY with confirmation a development deployment exists, not secrets.
4. Agent then supplies exact least-privilege client/trusted-server policies for the implemented authentication flow, maps PlayFab's verified player identity to authorized EOS voice identity, and configures private token-minting credentials. No broad admin policies, insecure sample room tokens or guessed manual plugin toggles.
5. Agent implements microphone opt-in/PTT/devices/mute/block, expiring room credentials/revocation, reconnect and authoritative near/far admission. Installed EOSVoiceChat Set3DPosition is unimplemented; global room plus local volume is not secure proximity. Provider suitability still needs actual proof before declaring this transport final. [EOS integration](https://dev.epicgames.com/documentation/en-us/unreal-engine/voice-chat-with-epic-online-services).

No separate voice VM or stored speech recordings are part of this plan. This account setup doesn't implement microphone capture or audible voice. Exact playable controls/fixture stay TBD until code exists; eventual listening requires two devices/headsets.

## 5. Package, persistence and allocation — agent, gated implementation

Source access already PASS; installed baseline5.8.3, original requested5.8.2. Launcher distribution refuses packaged Server target. Separate source build still needs adequate storage (prior onlyC:48.09GiB; conservative planning allowance400–500GiB free), compatible toolchain/plugins and reviewed matching client/server. PlayFab x64 avoids Oracle ARM work but does NOT remove source packaging. Do not download source into the existing installation or delete user assets/saves.

Implement/review PlayFab GSDK health/ready/allocation/shutdown lifecycle and test with its local agent before cloud upload. [Official Unreal GSDK guide](https://learn.microsoft.com/en-us/gaming/playfab/multiplayer/servers/server-sdks/unreal-gsdk/). Map remains /Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld. Target OS and exact deployment launch command are TBD until an actual staged package and GSDK version pass; no fabricated working ExecStart or cloudBuildID.

**MPS sessions are recycled. Local Saved files are not our only durable copy.** Before accepting real progress, add an authenticated bounded checkpoint service using Neon through the backend (or separately review different storage if measured payload/cost limits fail). Preserve current .pfs envelope/version/checksum/generation/originalUTC, ownership and expiration semantics. Restore before admitting players; failed/missing required checkpoint must not silently start fresh. Enforce one active authoritative session per world, generation/concurrency guards, idempotency and durable acknowledgement. No schema or migration is implemented by this guide; existing local .pfs tests don't prove cross-container durability.

No SQL or trusted EOS/PlayFab developer key in clients. Clients request authorized allocation through the backend; PlayFab returns actual session endpoint/port. Do not assume a permanent VPS IP or public7777: configure game UDP container port then use returned mapped port. No SSH Oracle key upload/manualVPS firewall steps apply here. A normal persistent background API must not be assumed to live on a recycled game container.

## 6. End-to-end gates

| Test | Required result |
| --- | --- |
| Account/enablement | Actual title and reviewed quota/payment requirements; no unintended paid plan/allocation |
| Packaged server | Source-built matching package, GSDK health/ready/allocation/shutdown, logs inspected |
| Backend identity | Spoofed allocation, checkpoint, database and voice credentials refuse |
| Persistence | Gather/craft/build/store, durable save, terminate old instance, allocate NEW instance, restore/reconnect; items/owners/food ages conserved |
| Failure paths | Quota exhaustion, API/DB outage and stale writes refuse safely without false save acknowledgement/world reset |
| Multiplayer | One client then two; authority and reconnect; high regional latency recorded separately from gameplay FPS |
| Voice | Actual two-device near/far/PTT/mute/block/consent/reconnect/disable tests, not metadata-only proof |
| Cost/teardown | Measured CPU/RAM/egress/request/storage, explicit bounded test teardown and allocation/usage readback |

## Current evidence and limits

Documentation update only: official references refreshed, Oracle signup failure recorded, owner card updated rather than duplicated, links/current scope checked. No new build/Unreal/test/service run, SDK install, paid account/plan, API/schema or voice implementation. Prior12/12 local preparation contracts remain prior evidence, not PlayFab deployment acceptance. Existing Oracle private key stays private/outsideGit and is unused, not deleted. Source storage and account/payment decisions remain owner prerequisites. A platform account-ready report isn't full M15 completion.
