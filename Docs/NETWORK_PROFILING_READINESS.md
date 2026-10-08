# Networking and profiling readiness — M15–M16 plan

October 8, 2026 source audit, not implementation or milestone acceptance. Selected from Trello's future hardening/optimization task. Current M7/M8 manual gates remain open; no new gameplay, authentication service, asset, rendering setting or engine installation is authorized by this plan.

## Current request contract

The owning survivor/controller requests an action; C++ recomputes targets, inventory and costs on authority. The local HUD/preview is advisory. Request codes are declared in PFRequestCodes.h; unknown codes must never map to a valid action by default.

| Route | Current server checks | Current application cooldown |
| --- | --- | --- |
| ServerInteract | Possessed living pawn; server eye trace; pickup/gather validates target, reach, capacity, expiry and node availability | 0.25 s per controller; gathering also 0.5 s per node |
| ServerInventoryAction | Existing owning stack GUID; split/drop/consume code, positive bounded quantity, living pawn; consume exactly one | 0.15 s per controller |
| ServerCraftAction | Server catalog recipe/ingredients, busy/life state; cancel delegates to crafting component | 0.25 s per controller |
| ServerAttackCreature | Living pawn; server eye trace within 250 cm; live creature; five stamina; inventory-derived damage | 0.5 s per controller |
| Building ServerPlace/TargetAction/Transfer | Shared gate; server eye trace/candidate, quarter-turn 0–3, ownership/support/collision/resources; transfer 1–100 and open owned storage | 0.25 s shared per building component |

These are reliable RPCs. Cooldowns bound accepted application work; they do **not** certify transport queue/backlog limits or flood resistance. Early rate refusals usually return silently. Future profiling must measure reliable queue pressure and feedback cost before adding more client requests. Do not add a client grant, arbitrary damage/target/location or save/load RPC. Developer commands are local authoritative tools and excluded from Shipping by the plugin descriptor; that is not a remote admin/authentication system.

Inventory insertion/crafting stage proposed values before committing; transfer accepts the destination before subtracting the source on the same game thread. Save restoration replaces inventories. Existing tests cover conservation, bad quantities/IDs, expired batches, ownership and repeat restore; actual simultaneous network request races and delayed stale selections still need a dedicated fixture. Preserving quantity also requires counting world drops and storage, not just the owner's bag.

## Replication and identity

PFInventoryComponent.Stacks uses COND_OwnerOnly, including storage whose actor owner is rebound to the correct reconnecting controller. Public PersistentPlayerId/structure ownership IDs replicate; private reconnect credentials are separate save/profile data and sent to the owning client. Pawn vitals, structure shape/door/health/support and creature health/state/target replicate. No FastArray, replication graph, large-world relevancy or production bandwidth claim follows from these small arrays.

Development identity is a GUID capability stored under profile plus endpoint hash. PreLogin/InitNewPlayer check malformed/unknown/duplicate-connected credentials; deferred PostLogin restores after connection attachment, and controller destruction captures departure before losing the pawn. Loaded-world unknown credentials refuse; a fresh world may issue new identity. Standalone may adopt a sole saved record. This is trusted local/LAN development behavior, not online account authentication, credential encryption or proof against theft/replay. Production identity/session authorization must be designed separately before Internet hosting, with migration and reconnect tests.

Keep raw login URLs, local profile files and world payloads out of shared evidence. Application logs already record accepted/refused operations and IDs/counts without printing the private capability; engine logs can still contain login options. Future export tooling must redact secrets before upload rather than assuming raw logs are safe.

## Current limits and likely profiling sites

- Building placement globally caps 128 pieces and scans actors for caps/support. Preview repeats candidate validation at 0.1 s intervals. Every piece has a storage component; non-storage inventory ticks are disabled. This is bounded greybox code, not scalable settlement indexing.
- Creature spawns cap eight globally, require navmesh projection and capsule clearance; creature tick interval is 0.1 s, spawner 1 s. Do not raise the cap before measuring navigation/perception/replication cost.
- Survival authority ticks at 0.1 s, inventory expiry at 1 s, world clock at 0.25 s. ForceNetUpdate follows several mutations; measure actual bytes/updates before changing replication cadence. Preserve predictable needs/expiry timing.
- The save codec caps 32 identities, 128 structures/resources/pickups, eight creatures, 32 spawners and a 4 MiB world payload. Save/validation/loading is synchronous game-thread work. No timed autosave exists. File byte limits are not a frame-time guarantee.
- Catalogs load from fixed project paths. The imported asset inventory is disk/registry metadata, not evidence of runtime residency or the source of low FPS. Source/license approval still precedes integration; no asset is replaced by this audit.

## Bounded future network test order

Use disposable unique Automation slots/profiles and explicit timeouts. One client first, then two NullRHI clients. Keep real same-endpoint reconnect identity across a separately restarted server; never reuse a personal save as a corruption fixture.

1. Preserve the current clean native checkpoint and no-emulation create/restart baseline. Current evidence: M8_POST_FIX_VERIFICATION.md (30 native tests/ten live reports), followed by the focused DeadPlayerRespawn test.
2. Repeat the existing PF.Persistence.Live scenario under one modest session-only lag/loss profile, then verify the profile actually applied in engine logs. Installed Engine/Private/Net/NetEmulationHelper.cpp registers NetEmulation.PktLag/PktLoss under DO_ENABLE_NET_TEST. They are development diagnostic commands, not Shipping features; do not write default project network settings.
3. Add a separate narrow race/stale-request fixture when authorized: competing pickup, repeated split/drop/consume, storage transfer then range/ownership change, cancel/finish race, and death/disconnect during accepted work. Compare total item quantity/batch lifetime and stable player/world identities before/after. Do not infer this coverage from the current live fixture, which mostly drives accepted actions on the server.
4. Add bounded reconnect/JIP/save-load tests during moving, depleted resources, creature corpse/loot and open-door transitions. Test failed restoration preservation and no repeated cosmetic rewards. Known restore staging collision fixes remain regression prerequisites.
5. Only then schedule a longer, capped soak and profiles with jitter/reordering/greater loss. Record exact direction/settings, duration, deadline, retries and disconnect cause. Reliable delivery and a passing short LAN test do not establish arbitrary-latency or Internet stability.

Stop on failed report, ensure/fatal or unexplained gameplay errors; retain initial artifacts and fix only that scope. Client 2's old rendered D3D12 crash and the successful NullRHI run are separate diagnoses, not proof of rendered recovery.

## Dedicated server and rendering gates

PrimalFrontierServer.Target.cs exists, but the installed distribution refused the Server build: "Server targets are not currently supported from this engine distribution." Uncooked UnrealEditor-Cmd -server tests are valid development evidence; no packaged server artifact exists. A suitable engine distribution/build host and matched client/server cook must be arranged later. Do not bypass UnrealBuildTool, remove the Server type, or call an Editor executable a packaged server. No source-engine download/build is started here.

For rendering, use one rendered client and the same map/settings/resolution/session. Record whether Selected Viewport + F11, New Editor Window PIE, standalone Editor-game or packaged Game; they are different presentation paths. The earlier stationary Selected Viewport sample cannot certify the reported sustained 14–17 FPS in New Editor Window PIE or a traversal route.

Warm up, then capture a fixed route and a stationary sample separately. Uncap using session t.MaxFPS=0 and r.VSync=0, recording smoothing/external limits. Capture Game/Render/RHI/GPU frame-time distributions, present waits, median/p95/p99, process working/private memory, total committed memory and actual VRAM residency. Record startup/shader/streaming stalls separately, and compare CPU/GPU/presentation evidence before an optimization. Use measured timings rather than average FPS alone; make one bounded change and replay the same capture. Never increase quality or asset load to diagnose a low-FPS complaint.

Actual host reports 31.93 GiB RAM; the runtime adapter previously reported RTX 4060 with approximately 7,956 MB dedicated memory, distinct from the supplied 4060 Ti 16 GB reference. See TEST_MACHINE.md. NullRHI process memory is not rendered VRAM, frame rate or a 16 GB minimum-spec pass. A provisional 60 FPS budget is 16.67 ms per frame, not accepted performance; native 1440p traversal and a real constrained target still need measurement.

## Evidence and next action

Read source: PFSurvivalPlayerController, PFBuildingComponent/PFBuildPiece, PFInventoryComponent/PlayerState, PFWorldPersistence/PFLocalPlayer, PFCreatureSpawner, Server.Target.cs and the plugin descriptor. Checked against FULL_PROJECT_ROADMAP.md, PERSISTENCE_M8.md, SAVE_COMPATIBILITY_PLAN.md, ASSET_PIPELINE_M9.md and TEST_MACHINE.md. No runtime build/test/performance improvement is claimed from this document-only audit. M15/M16 remain To Do; the bounded plan can be Done.

Next current gameplay acceptance is the Personal open-world route/overnight, then rendered save/restart/reconnect. Current automated work can test the existing M8 session under a modest network profile independently; new hardening systems, broader world/assets and production hosting remain gated.
