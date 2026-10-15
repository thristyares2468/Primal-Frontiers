# Oracle, database and proximity voice: setup checklist

**Superseded:** user reports Oracle signup failed. Follow [PlayFab setup](PLAYFAB_DATABASE_VOICE_SETUP.md); no Oracle VM/signup retry is currently requested. This guide retains previous research/preparation. The SSH key remains private and unused; no Oracle account readiness or ARM deployment is inferred.

October15,2026. Current decision: try Oracle Always Free first; do not purchase an OVH VM. Preparation only: no Oracle account/VM, Neon project, EOS product or credentials have been supplied/created/connected. Owner account steps may be prepared in parallel; a LinuxARM64 Server package must pass before gameplay deployment. This supersedes provider recommendations in MULTIPLAYER_SETUP_2026_10_15.md; its prior evidence remains valid.

## 1. Oracle account — owner

1. Open https://signup.cloud.oracle.com/ and register for Free Tier (reuse your existing account if any; one free account per person).
2. Complete identity/card verification and MFA. A temporary card authorization hold can occur; do not upgrade to Pay As You Go. Trial credits expire; they are not the ongoing free target. [Official signup FAQ](https://www.oracle.com/cloud/free/faq/).
3. Choose Australia East/Sydney as home region if offered. If unavailable, report choices before finalizing: home region cannot be changed. Free compute must be in the home region. [Region rules](https://docs.oracle.com/en-us/iaas/Content/Identity/regions/managingregions.htm).
4. Report ORACLE_ACCOUNT READY and the home region. No passwords, card details, tokens or private keys in chat/Trello.

## 2. Oracle VM — owner, after account and configuration review

The exact console layout can change; use [Oracle's instance workflow](https://docs.oracle.com/en-us/iaas/Content/Compute/Tasks/launchinginstance.htm). Opening the wizard does not create a server. Stop before Create if eligibility/cost is unclear.

| Setting | Intended development value |
| --- | --- |
| Name | primalfrontier-dev |
| Region | Your home region |
| Shape | VM.Standard.A1.Flex, ARM/aarch64 |
| Size | 2OCPU,12GB RAM; account-wide total, not per extra VM |
| Image | Always Free eligible Canonical Ubuntu ARM64; prefer22.04 if offered, subject to actual packaged-server OS compatibility |
| Boot disk | Start50GB, only if package+headroom fit; expand later within verified free allowance |
| SSH | Upload the prepared .pub file below, not the private key |
| Network | Public subnet with Internet Gateway/default route and assigned public IPv4 for direct SSH/game testing; no NAT gateway/load balancer needed |
| Inbound initially | TCP22 from your own public IPv4/32 only; do not leave a default0.0.0.0/0 SSH rule |
| Gameplay | Keep UDP7777 closed until deployment is ready; then restrict initially to intended testers' public IPs |

The official current free allowance is equivalent to2OCPU/12GB and200GB combined boot/block storage across the account. Capacity may be unavailable; idle machines can be reclaimed. Check Always Free eligibility for the entire configuration, disks and optional features, not merely a zero trial-credit estimate. Do not add a paid substitute. [Current Oracle limits](https://docs.oracle.com/en-us/iaas/Content/FreeTier/freetier_topic-Always_Free_Resources.htm).

If Always Free eligibility is confirmed and you choose to create the VM, wait for RUNNING and record its public IP, OS and shape. If OUT OF HOST CAPACITY appears, report it; do not change to a paid shape or create multiple accounts. A VM being RUNNING means infrastructure only, not a working game.

## 3. Prepared SSH access — owner uploads public key; agent verifies connection

Already generated on this PC:

- Public key to upload: `C:\Users\jackh\.ssh\pf_oracle_ed25519.pub`
- Private key: same path without `.pub`; never upload/share/commit it. It is a dedicated unencrypted automation key, with Windows file access restricted to the current user. Revoke its public entry on the server if this PC/key is compromised.
- Fingerprint: `SHA256:3QJq0DLsPf0nZekb4YgwhCI4iGAoiHZxlah+i4zMqug` (ED25519).

For Ubuntu platform images, the default account is `ubuntu`. Once a VM exists, the following is the intended command; replace SERVER_PUBLIC_IP, do not type that placeholder literally:

```powershell
ssh -i "$env:USERPROFILE\.ssh\pf_oracle_ed25519" ubuntu@SERVER_PUBLIC_IP
```

On first connection compare the **server host-key** fingerprint with a trusted instance-console source before accepting it; it is different from our client-key fingerprint above. Do not disable host-key verification. [Oracle connection guide](https://docs.oracle.com/en-us/iaas/Content/Compute/Tasks/connect-to-linux-instance.htm).

After authorized SSH works, agent checks `uname -m` (expected aarch64), OS, memory/disk and network rules, applies reviewed updates and creates a non-root runtime service. Avoid blanket UFW/iptables changes that can override Oracle image rules or lock out SSH. SSH permission and application exposure are separate gates; no SSH/network connection was attempted without an endpoint.

## 4. Game package and deployment — agent, blocked by build space

Installed/tested engine is5.8.3 CL58210709, not original requested5.8.2. Source access and5.8.3 tag are already verified. Local UnrealBuildTool contains LinuxArm64/aarch64 handling; **project/plugin compile/cook/runtime compatibility remains unverified**. LINUX_MULTIARCH_ROOT is unset, which alone is not proof that every possible SDK location is absent.

Provide a separate SSD build folder with planning allowance400–500GiB free; current onlyC:48.09GiB is insufficient for the reviewed full source-build plan. This is build workspace, not required Oracle VM disk size. Do not delete assets/saves/uninstall the working engine. An existing separate build machine is an alternative to discuss.

Agent then verifies exact SDK/platform/plugin support, fetches source separately, builds/cooks/stages PrimalFrontierServer for LinuxArm64 and a matching Windows client, tests local authority/save/restart, and uploads only approved runtime content. Source-built Editor and current narrow tests must pass before rollout. Do not copy the Windows editor/executable to ARM or infer success from a Linuxx64 package.

Actual package entry point/save path are TBD until staging; do not invent systemd ExecStart now. Agent configures tested launch arguments, writable private save directory, graceful save/stop, service supervision and off-machine backups only after the executable passes. Map: `/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld`. Keep .pfs world authority. Current default-off voice and development identities are not public-production authentication.

## 5. Neon PostgreSQL — owner account; agent backend integration

1. Register at https://console.neon.tech/ and select Free. No paid plan/add-ons needed.
2. Create project `primalfrontier-dev`, closest available region to the eventual backend, stable supported PostgreSQL version. Keep compute scale-to-zero enabled. [Console guide](https://neon.com/docs/get-started/signing-up).
3. In the project's database/role controls, create DEVELOPMENT database `primalfrontier_dev` and a dedicated development application role such as `pf_app`. Keep administrator/owner credentials separate; final runtime privileges are assigned by reviewed migrations. Do not grant clients owner credentials.
4. The Connect dialog supplies the host/database/role and connection URI. Save it privately in your password manager. Do not paste it into this chat, Unreal config, source or Trello. Report NEON_PROJECT READY, selected region and PostgreSQL version only.
5. Agent prepares backend configuration outside Git with the private database connection and verified TLS, designs role/schema migrations, tests connections and isolated restore, and adds authenticated game/account APIs. No SQL connector/backend exists yet, so there is currently no game field to paste the URI into.

Current Free includes1GB/project and100CU-hours/project/month; compute sleeps after5minutes inactive. It is an intermittent development allowance, not guaranteed24/7 database compute. [Pricing](https://neon.com/pricing). Existing .pfs saves do not need SQL; retain them until an explicit tested migration exists. Database/auth outage must not wipe or replace a world. Backend code may initially share Oracle's VM if measured CPU/RAM permits; do not install a second database on the VM just to duplicate Neon.

## 6. Epic Online Services voice — owner product; agent transport integration

1. Sign into https://dev.epicgames.com/portal/ using the intended product owner account. Create/select your organization and Primal Frontier product; accept applicable terms personally. Use an existing product if present, not a duplicate.
2. In EOS product settings, create/select a private development sandbox/deployment (names `Dev`/`Development` are fine); retain any automatically supplied Live deployment separately. Record product/sandbox/deployment IDs privately. [Portal hierarchy](https://dev.epicgames.com/docs/dev-portal/dev-portal-intro).
3. Report EOS_PRODUCT READY and that a development deployment exists. No secret values needed. Agent supplies exact least-privilege client/trusted-server policy settings as part of the actual implementation, then configures appropriate credentials privately. Do not guess broad administrator permissions or distribute trusted token-minting credentials to game clients.
4. Agent integrates authentication/identity mapping, temporary room credentials/refresh/revocation, opt-in/PTT/devices/mute/block and authoritative proximity. Client voice media uses Epic; authenticated backend issues permitted room credentials. [Trusted-server voice](https://dev.epicgames.com/documentation/en-us/unreal-engine/voice-chat-with-epic-online-services).
5. Installed EOSVoiceChat Set3DPosition is unimplemented and sessions are NonPositional; provider suitability for actual server-controlled proximity is still a proof requirement. Global room plus client volume is not secure range enforcement. If unsupported, agent must report the transport limitation and compare alternatives instead of claiming audible proximity works.

Voice media is advertised free by [Epic](https://www.epicgames.com/site/news/epic-online-services-launches-free-in-game-voice-and-easy-anti-cheat?lang=en-US). No separate rented voice VM or database full of audio recordings is needed. Microphone controls/test fixture remain TBD until implemented; do not enable plugins or paste sample insecure tokens yourself merely to test an absent feature.

## 7. How they fit and what must pass

The Unreal server owns gameplay and .pfs world saves. A future authenticated backend maps one verified player identity to account metadata in Neon and authorized voice access. Clients use the game endpoint for gameplay and Epic for voice; they never connect directly to SQL with application credentials. Initial co-location of game/backend is a budget plan, not measured capacity.

| Gate | Concrete acceptance |
| --- | --- |
| Oracle access | Verified SSH host key; OS/architecture/CPU/RAM/disk match intended free instance |
| ARM game package | Actual compile/cook/launch on target OS, one then two clients, logs inspected |
| Persistence | Gather/craft/build/store, save, stop/restart/load, reconnect; conserved items/owners/stats |
| Backend/database | Verified TLS/scoped identity; unauthorized/spoofed requests refuse; outage safe; isolated backup restore |
| Voice | Real near/far admission/revocation, consent/PTT/mute/block/reconnect/disable; two-device listening |
| Long-running operation | Measured CPU/RAM/tick/disk, tested graceful save/restart and off-machine restore; no data-loss or false-fresh fallback |

Account creation/VM RUNNING/database connection/metadata checks do not pass the whole system. Current source storage, cloud endpoint/credentials, implementation and physical listening are separate dependencies.

## Preparation actually completed here

Source access PASS; ARM build-target support located in local UBT (not a platform package test); dedicated ED25519 deployment key created outside project and ACL checked; existing setup contracts12/12 PASS again at06:49:56UTC. First ACL attempt needed Windows security privilege and existing explicit grants remained after inheritance removal; native icacls removal of this new key's extra grants succeeded and final owner-only ACL verified. No personal existing key was overwritten. Key outsideGit, no private content logged.

Existing Scripts/GetMultiplayerReadiness.ps1 and StartPrivateDevelopmentServer.ps1 remain read-only/plan-only by default. No Unreal launch, C++/asset/settings/private-save change or new build required for this guide. No Oracle/Neon/EOS provisioning, network connection, schema, service launch or audible test performed. Reports: Saved/AutomationReports/M15OraclePreparation_20261015.
