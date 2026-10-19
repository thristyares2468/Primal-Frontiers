# Multiplayer setup: server, database and proximity voice

**October 19 current source result:** storage/source access and actual source5.8.3 Server compilation now pass. Two private checkpoint helper names were fixed for unity; explicit unity Server replay, Editor and8/8 focused regressions pass. The earlier48GiB blocker below is historical. Follow [current build/package gate](PLAYFAB_SERVER_BUILD_M15.md); no cooked/packaged/hosted/GSDK pass yet. Development Unreal→Cloudflare→Neon transport is separately verified in [checkpoint evidence](CHECKPOINT_TRANSPORT_M15.md).

**Latest:** Oracle signup failed; [PlayFab / Neon / backend / EOS guide](PLAYFAB_DATABASE_VOICE_SETUP.md) is current. Payment method required for MPS enablement and recycled-session checkpoint durability are explicit gates. All provider recommendations below are historical alternatives, not current deployment proof.

**Current free-first choice:** follow [Oracle / database / voice checklist](ORACLE_DATABASE_VOICE_SETUP.md). The OVH recommendation below is now a researched paid fallback, not a selected purchase. Dedicated Oracle public key and current owner steps are in the new guide; ARM packaging and account/capacity gates remain unverified.

Current recommendation and audit: October 15, 2026. User now permits free **or very low-cost** services. This replaces the earlier free-only/deferred direction in [the original guide](SERVER_DATABASE_VOICE_SETUP_M15.md). No subscription, account, database or online voice service was created.

## Recommended cost and services

| Part | Recommendation | Advertised cost / qualification |
| --- | --- | --- |
| Game simulation, initial local development | Existing private Windows NullRHI dedicated server | No hosting subscription; uses your PC/electricity/internet. This does not stay online when your PC is off. |
| Small externally hosted test server | Linux x64 VPS near the players; OVHcloud Australia VPS-2 is a candidate | Australian page advertises **from A$11.90/month excluding GST**,4vCPU/8GB/75GB. About A$13.09 including10%GST at that starting price; checkout region, term, renewal, availability and actual package size must be checked. APAC quota1TB then10Mbps cap. Not an acquired quote or certified player capacity. |
| Database, when backend integration starts | Neon **Free** PostgreSQL | Current pricing: $0, no card required,1GB/project and100CU-hours/project/month; inactive compute sleeps after5minutes. Good for intermittent development metadata, not guaranteed always-on service. Keep the Free plan; do not enable paid upgrades. |
| Voice media | Epic Online Services Voice | Epic offers free hosted voice. Our trusted backend and game still need identity/token/proximity integration. No separate voice-server rental is needed if this provider passes the routing proof. |

Primary references: [OVH Australia](https://www.ovhcloud.com/en-au/vps/vps-australia/), [Neon pricing](https://neon.com/pricing), [Epic free voice](https://www.epicgames.com/site/news/epic-online-services-launches-free-in-game-voice-and-easy-anti-cheat?lang=en-US), [EOS integration](https://dev.epicgames.com/documentation/en-us/unreal-engine/voice-chat-with-epic-online-services).

**Practical target: about A$13–20/month for an initial small test VM, with free database and voice tiers.** This is a development planning budget, not a complete production bill: extra storage/backups, taxes, traffic, provider terms and increased capacity can change it. Measure package disk size and sustained game CPU/RAM before ordering. A source-compatible Linux package is mandatory for the quoted Linux option; it cannot run the current Windows editor/executable.

For a strict $0 external host, [Oracle Always Free](https://docs.oracle.com/en-us/iaas/Content/FreeTier/freetier_topic-Always_Free_Resources.htm) currently offers an A1 allowance equivalent to2OCPUs/12GB total. It is **ARM**, capacity is not assured and idle instances may be reclaimed. LinuxARM64 compilation/plugin compatibility is unverified here. This is a port candidate, not a drop-in recommendation. Do not create a paid fallback when free capacity is unavailable. Trials/credits are temporary; neither a free database nor EOS runs Unreal world simulation.

## What is ready, and what is not

- Existing versioned `.pfs` files already save the server's world/player/inventory/structures. **SQL is not required to host the first world.** Retain this authority until a tested, explicit database migration exists.
- Current direct-IP uncooked server/one- and two-client persistence tests have prior passing evidence. There is no cooked/cloud deployment result in this task.
- No SQL connector, authenticated service API or online account backend is implemented. Creating an empty hosted database does not connect it to the game.
- Voice routing policy and authoritative metadata exist; microphone capture, media provider, devices, PTT and real listening do not.
- Source access now **PASS**: authenticated `gh api repos/EpicGames/UnrealEngine` returned the private repository; the `5.8.3-release` tag exists. The prior owner account-link task is resolved without asking for credentials.
- Installed engine is5.8.3 CL58210709, despite the originally requested5.8.2. Source tag5.8.3 reports the same patch and compatible CL55116800, but source Build.version uses Changelist0; this is not a promise of binary-identical builds. Build/test matching client and server from the reviewed engine.
- Only C: is currently detected: **48.26GiB free**. Plan roughly400–500GiB free for a separate source engine, dependencies, builds and staged package; this is an allowance, not a measured minimum. Existing Launcher Server-target refusal remains. Do not replace/uninstall that engine or remove assets/private saves to make room automatically.

## Do first: provide build storage

**Only immediate owner action:** provide a local SSD location with approximately400–500GB free, or choose what personal files you want to move yourself. Then send the folder path, for example `D:\UnrealSource`. No account relinking, server payment or microphone test is needed yet. The agent must recheck actual space before downloading.

No level is needed for this step. Do not create a partition, delete generated/project folders, move the repository, or buy hardware just to satisfy a guessed minimum. A different existing build machine can be discussed if no local volume is suitable. Storage readiness is not a game test.

## Agent work after storage is available

1. Recheck source access, disk, Visual Studio/toolchain and project plugin platform support. Fetch reviewed `5.8.3-release` into the selected **separate** engine folder. Preserve the current project and Launcher installation. Run source setup/generation; owner handles any new service/licensing agreements.
2. Build the source Editor and a protected project checkout with constrained parallelism. Build `PrimalFrontierEditor` and `PrimalFrontierServer`; do not equate ordinary Game build success with Server support. [Epic requires a source build for its dedicated-server workflow](https://dev.epicgames.com/documentation/en-us/unreal-engine/setting-up-dedicated-servers-in-unreal-engine).
3. Install only the matching supported Linux cross-compile toolchain after checking the reviewed source's SDK requirements. [Epic documents Windows-to-Linux cross compilation](https://dev.epicgames.com/documentation/unreal-engine/linux-development-requirements-for-unreal-engine?lang=en-US). Prove Linux x64 plugin compilation/cook; do not infer ARM support from Linux x64 success.
4. Cook/stage the Server package and matching client. Explicit gameplay map: `/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld`. Exclude unrelated staged Fab content and editor-only/Shipping-disallowed tooling as appropriate; review actual references. Measure package size.
5. Run a local packaged save/restart/reconnect and authority test: one client first, then two NullRHI clients. Test the Linux package on the intended supported OS. Capture tick/CPU/RAM/disk, not rendered FPS for a headless server. No paid VM until the executable/package gate passes.

## Owner hosting setup, when the package passes

1. Open the official OVH page above and check **Sydney/Australia**, Linux **x64**, an approximately4vCPU/8GB development configuration, sufficient SSD for the measured package and growth, and a monthly term. Avoid control-panel/Windows/GPU/add-on subscriptions. If unavailable or substantially more expensive than the budget, stop and compare another region/provider before buying. The current advertised page is not a stock guarantee.
2. Create your account and review the final recurring GST-inclusive price, renewal, transfer limits and cancellation. Keep credentials and payment details private. Tell the agent the chosen OS/region/resources and provide authorized SSH access through your normal secure mechanism, not a private key in chat.
3. Agent prepares a non-root service user, OS updates, restricted key-based administration and uploads the **complete staged package**, never the whole project/source engine. Use a persistent private save directory with restrictive permissions; verify the package's actual Saved path. Never deploy private development identity profiles as sample data.
4. Initially restrict gameplay UDP7777 to the intended testers/private network; restrict SSH to administration sources. Database TCP5432 stays private; expose HTTPS443 only when a reviewed backend exists. Add only the provider's documented required voice connectivity later. This task changed no firewall.
5. Before calling hosting ready, prove remote join, authoritative interaction, normal logout/save, stop, startup load and reconnect using the same endpoint/identity. Inspect server logs. Current development GUID identities are not production accounts or public-Internet hardening.
6. Add tested graceful save/shutdown, supervision and backups before unattended uptime. There is no current timed autosave or guaranteed crash save. Back up `.pfs` generations and Originals while stopped; store private backups off-machine and prove restore on an isolated instance. Provider daily snapshots and A/B rotation are not a tested application backup.

## Owner database setup, when integration is ready

1. Use [Neon](https://neon.com/pricing) and choose **Free**, not Launch/Scale or a paid trial. Create a DEVELOPMENT project `primalfrontier-dev`. Pick the nearest offered region to the backend; available choices are determined by the console. PostgreSQL version must be stable and supported by the future connector.
2. Create a development database and scoped application role with the console. Retain the administrator/owner separately. Save the password/connection information in your password manager and later the backend host's private secret configuration. Do not add it to Unreal DefaultEngine.ini, client binaries, Git, chat, Trello or ordinary logs.
3. Verify the project is still on Free and its usage limits/suspend behavior. Leave scale-to-zero enabled. Do not use frequent keep-alive queries solely to defeat sleeping; that consumes the allowance.
4. Agent implements backend-owned migrations and an authenticated HTTPS API, then tests database connectivity from the backend with verified TLS. Game clients never receive SQL credentials. A database and the existing reconnect capability alone do not provide secure user accounts.
5. Introduce accounts/world metadata first, while `.pfs` remains world authority. Any later world-data migration needs transaction/idempotency, ownership, version, retry, outage and reconnect/duplication tests plus explicit rollback/backup validation. Do not write to SQL and files as two competing authorities.
6. Prove protected export/restore into an isolated database. Free retention/history is limited; it is not an independent backup service. No SQL/database was installed or integrated by this setup task.

## Owner voice setup, after the routing design is ready

1. Sign into the [Epic Developer Portal](https://dev.epicgames.com/portal/) with the product owner account. Create/select the Primal Frontier product and a **development** sandbox/deployment; accept the service terms personally. This task did not create the product or accept agreements.
2. Configure the reviewed EOS client policy and trusted-server voice permissions. Public product identifiers, scoped client credentials and privileged token-minting secrets have different trust levels. Keep privileged credentials exclusively in the trusted backend's private configuration. Never use insecure development token generators for public clients.
3. Agent enables only the required reviewed plugins and implements authenticated identity mapping, expiring room credentials, reconnect/leave/disable, opt-in microphone permission, PTT, devices, per-user volume, mute/block and the server-owned distance/life policy. A database is not inherently required for voice; it can support future authenticated accounts.
4. **Proximity is additional code.** Installed EOSVoiceChat's `Set3DPosition` is unimplemented (line652); sessions are NonPositional (line1110). One global room with local volume attenuation does not stop an altered client hearing distant audio. Prove actual server-controlled media admission/revocation and the provider's limitations before committing to this transport; if EOS cannot enforce the required behavior, compare a different relay/provider rather than claim it works.
5. Run automated identity/range/consent/expiry/refusal tests and two actual devices/headsets for near/far, PTT, mute/block, disconnect and disabled-state silence. No need to record private speech. Exact in-game microphone controls/fixture are **TBD until implemented**; metadata tests are not listening acceptance.

## Repeatable preparation already added and tested

From PowerShell in `C:\UnrealProjects\PrimalFrontier`:

```powershell
.\Scripts\GetMultiplayerReadiness.ps1 -CheckGitHubAccess
.\Scripts\TestMultiplayerSetup.ps1
.\Scripts\StartPrivateDevelopmentServer.ps1 -Mode New -Slot DevM15Test
```

These commands inventory prerequisites, run disposable contract checks and **print a launch plan**. No Unreal is launched by default. `GetMultiplayerReadiness -BuildVolumePath D:\UnrealSource` can check a future volume without downloading.

The launcher requires explicit New/Load and refuses existing New slots, absent Load slots, reserved/injected names, invalid ports and missing engine/project paths. A Load plan checks file existence only; Unreal still validates content. `-Launch` is opt-in and starts **localhost only**, with NullRHI, after refusing open Editors/occupied UDP ports. Its result `PROCESS_STARTED_NOT_VERIFIED` is deliberately not a ready/pass verdict. Live launch, normal saving/shutdown and listener detection were **not exercised in this task**. Do not use this localhost wrapper as a public production service.

For a later isolated local test, add `-Launch` to a fresh-slot plan, confirm `IpNetDriver listening on port 7777` in `Saved/Logs`, then use the existing game's direct-IP path to `127.0.0.1:7777` with a disposable identity profile. A remote client cannot use its own127.0.0.1 to reach your server. Current main-menu multiplayer browser is not implemented. Loading uses the same Slot and `-Mode Load`; verify successful load before joining. Current save/reconnect tests and exact client flags are retained in the original guide and `Scripts/RunPersistenceAutomation.ps1`.

### Evidence and limits

`Saved/AutomationReports/M15Infrastructure_20261015/`:

- `SetupContracts.json`: **12/12 PASS**, October15,06:22:49UTC. SyntaxAllThree; NewLocalhostNullRHIQuotedPath; NewPlanCreatesNoSave; LoadMissingRefused; NewExistingRefused; LoadFlagNoCorruptSaveCertification; SaveBytesUnchanged; BackupGenerationOnlyLoadPlan; ReservedSlotRefused; InjectedSlotRefused; MissingEngineRefused; InvalidPortRefused.
- `Readiness.json`: actual source access PASS; installed5.8.3; C:48.26GiB; source build storage BLOCKED_SPACE; Git/GitHubCLI/SSH present, Docker/psql/pg_dump absent;0 Editors.
- `PrivateServerPlan.json`: actual installed paths and OpenWorld/NullRHI/loopback argument plan, **PLAN_ONLY**.

Initial contract run caught array-expression argument joining; replay then caught the test harness's automatic `$args` shadowing. Both were repaired before the final12/12. No test bypass, save rewrite or game process occurred. This task changes scripts/docs only: no C++/asset/map/settings change, so no Editor rebuild or gameplay regression rerun was needed. No cloud/SQL/audio/new gameplay/manual/memory/FPS pass is inferred.
