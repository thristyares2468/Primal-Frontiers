# Architecture and Design Decisions

## October21 — quarantine writer-fencing SQL until live audit and adapter gates

Develop the M15 lease/epoch/CAS/idempotency contract in an in-memory PostgreSQL18.6 test fixture under Tools/BackendFencingPrototype, outside the Worker's Git deployment workflow. Pinned Apache2 PGlite0.5.10 is a test-only dependency, not Unreal/Worker runtime. Retain per-world row/fence across release, use database actual clock after lock, preserve original envelope generations/metadata, and require atomic head/checkpoint/receipt. PUBLIC access revoked; SECURITY INVOKER remains a draft, not eventual runtime authorization. Local25tests pass; no approved live migration, grant design, true multi-session contention, authenticated identity or cloud restore/admission claim. Finish live privilege/binding audit and isolated migration/legacy-write cutover before adoption. Evidence CHECKPOINT_FENCING_PROTOTYPE_M15.md.

## October20 — explicit server-only GSDK lifecycle, closed hosted admission

Vendor Microsoft Apache-2.0 Unreal GSDK2.0.1 at673f068f0 with license/patch provenance; allow only Server target and load only after explicit PFPlayFabHosted plus bounded agent JSON validation. Remove machine-specific legacy DLL staging, preserve upstream style and make nine explicit JSON key copies for source5.8.3. Default offline servers and Editor/Game clients do not start agent traffic. Real allocated port/readiness, stateless health callback and weak GameThread shutdown tested with official local mock. Both PreLogin/InitNewPlayer keep hosted gameplay closed until authenticated identity, durable remote restore and writer fencing are implemented. Standby/Active is not durability acceptance.

Do not hide raw messages or patch engine navigation to certify this local mock. Exact optional metrics404, normal SDK termination Warning and post-BeginTearingDown Crowd warning are recorded, source-investigated and narrowly classified in an opt-in local fixture; unexpected severity still fails. The SDK stops heartbeats upon Terminate, so test command receipt/callback/natural exit rather than invent a final heartbeat. Developer-only client readiness starts the existing persistence test after actual connected pawn readiness, retaining startup logs and strict assertions. [Evidence, failed attempts and reproduction](PLAYFAB_GSDK_M15.md). No cloud allocation/service credentials/asset changes.

## October20 — explicit cooked catalog discovery and source build provenance

PFAssetPaths loads four gameplay catalogs by runtime string; cook discovery cannot infer them from a hard map reference. Add only the Items/Crafting/Building/Creatures folders to AlwaysCook, not CookAll or raw asset changes. Non-Shipping PrimalAgentToolsRuntime Server receipts also stage the existing engine HTML report shell as NonUFS. The optional packaged-server test mode isolates both roles and requires actual authority/privacy/restart assertions, clean Server raw logs and exported HTML/JSON before pass.

Official source5.8.3 declares compatibleCL55116800, but UBT ignores it while Changelist0. Stamp only the isolated source Build.version with our clearly labelled local licensee build2026102001, retaining the tag's compatible baseline and source396c9f0. This is not Epic CL58210709 provenance and does not alter runtime network-version checking. Actual Launcher-client connectivity/checksum3283589285 and10 final native role reports prove this bounded interoperability; no general cross-version/cooked-client claim. [Evidence and exact metadata](PLAYFAB_SERVER_BUILD_M15.md). No installed-engine/global association change or paid allocation.

## October19 — isolate source-engine server preparation

Storage/access now permit the previously blocked source-server work. Match the installed actual5.8.3 patch with Epic's pinned5.8.3-release source in C:\UnrealSource; keep Launcher installation and main project engine association unchanged. Build against a managed isolated project worktree so generated source-engine binaries cannot replace ordinary Launcher-editor DLLs or touch personal saves. First gate Windows source Server compilation, then Editor/cook/local package and reviewed GSDK lifecycle. Limit compile concurrency for the32GB host; no paid hosted capacity before local verification. GSDK is downloaded outside the repo for review, not yet a game dependency. [Build gate](PLAYFAB_SERVER_BUILD_M15.md).

First actual source Server build exposed checkpoint-private helper names colliding with legacy persistence files in unity. Rename those two helpers rather than disable unity or weaken warnings. Explicit `-DisableAdaptiveUnity` replay keeps the original four files together and passes; Editor and eight checkpoint/player-save regressions pass. No protocol/save behavior or schema change. Source compilation is now verified; cooking/packaging and GSDK remain separate gates.

## October18 — preserve local save bytes at the development HTTP boundary

Carry the exact existing .pfs envelope, including generation and original saveUTC, inside the development checkpoint payload rather than converting item/owner/expiry records to a new database schema. Use SHA256 plus existing CRC and bound incoming bytes. Built-in Unreal HTTP and engine-bundled OpenSSL are the only added build dependencies; no plugin/download. The generic platform SHA256 function asserts on this installed Windows engine; a retained failed run demonstrated it, and OpenSSL replay passed. Restrict entrypoints to dedicated-server gameplay worlds before reading process-only secrets. No automatic remote restore or file overwrite in this first bridge; local save acknowledgment must not imply remote durability. Production identity, active-writer fencing, timeout reconciliation and admission-blocked restoration require their own gate. Details: CHECKPOINT_TRANSPORT_M15.md.

## October15 — replace unsuccessful Oracle signup with PlayFab evaluation plan

User requests replacement setup after Oracle signup failure. Plan PlayFab x64 evaluation, retaining Neon and EOS candidate; proposed Cloudflare Workers Free provides a separate authenticated API host rather than assuming a persistent service inside a recycled MPS container. Explain this additional service before any implementation/dependency. Review required MPS payment method/account-specific quota first; no purchase or zero-cost guarantee. Add durable, bounded, versioned checkpoint/reallocation design before cloud use; preserve .pfs envelope/expiry semantics and single-world authority. No schema/storage migration/GSDK/API/provider implementation is made here. [Current guide](PLAYFAB_DATABASE_VOICE_SETUP.md).

## October15 — try Oracle Always Free before paid hosting

Follow the user's requested free-first direction with Oracle A1 ARM feasibility, Neon Free metadata/account storage and EOS-managed voice candidate. This supersedes the paidLinuxx64 candidate as the first route; no platform compatibility, availability or media admission guarantee is implied. Prepare a dedicated SSH key outsideGit with owner-only private-file access, but perform no cloud operations without an actual configured account/endpoint. Keep .pfs world authority and existing engine/project association; source-build disk/ARM/plugin and real backend/voice gates remain. [Exact checklist](ORACLE_DATABASE_VOICE_SETUP.md).

## October15 — staged low-cost infrastructure preparation

User reopens infrastructure with a free-or-low-cost budget. Keep existing server-owned .pfs saves as world authority; SQL is optional for initial hosting and future account/metadata services require a separate authenticated backend. Research candidates are Linux x64 VPS, Neon Free PostgreSQL and EOS-managed media; none is provisioned/selected by account agreement. EOS positional/range admission needs proof, not a client-volume-only privacy claim. GitHub source access is verified; disk space blocks the separate source-engine/package gate. No engine association, provider dependency, runtime behavior, credentials or firewall changed. [Current guide](MULTIPLAYER_SETUP_2026_10_15.md) records exact prerequisites,12 setup contract checks and deployment/listening limits.

## October15 — offline home-world progression and additive layout

Keep trusted server tool family/grade and existing inventory/crafting transactions; append original home-world content through Titanium in a focused catalog module. New minerals share the existing Stone gather-XP window, not an independent reward budget per ore; legacy Copper reward behavior is unchanged. Knowledge costs fit the existing level10/27-point maximum and remain separate from stat points. Physical stations and later powered/dimensional materials stay explicit future scope; present refinements say portable greybox. Existing original sprites serve as tinted category placeholders, with custom catalog art taking precedence.

The exact prior15- or16-node open-world layout may append the reviewed nine fixed Eryndor identities (and Copper when absent), preserving original envelope/generation/UTC before applying. Current layout25 must match exactly; renamed/partial/unknown nodes refuse. Read-only inspection reports the two migration requirements separately and never creates a backup. No save schema/capacity change, deleted legacy record or guessed remapping. [OFFLINE_IMPLEMENTATION.md](OFFLINE_IMPLEMENTATION.md).

## October15 — preserve immutable historical-writer fixture bytes

Build production persistence sources from known historical commit c3d221f in an isolated source-only project with a new synthetic harness. Freeze the emitted world/file bytes and source/SHA256/engine provenance once; current tests read them directly rather than recreating old records with today's encoder. Regeneration writes a separate candidate, never replaces the reviewed baseline. All identities are fake; generated binaries/archive/logs remain untracked. This proves old-source reader compatibility on installed5.8.3, not an archived historical executable or production/private-world acceptance. [FROZEN_WRITER_M18.md](FROZEN_WRITER_M18.md).

## October15 — inspect compatibility without restoring a save

Keep `UPFWorldPersistence::InspectSave` separate from destructive restoration. Reuse the actual layout predicate and validated codecs, prepare only a local copy of the reviewed Copper addition, and return sanitized schema/generation/UTC/counts/recovery/migration metadata atomically. Do not invoke Apply, Save or original-backup publication; a successful inspection cannot guarantee actor allocation or placement. `PF.CheckSave` uses the existing non-Shipping server-only command registry/history, with no client RPC or new dependency. [SAVE_INSPECTION_M18.md](SAVE_INSPECTION_M18.md) records bounded verification. Hosting/accounts/database/voice provisioning is deferred by the user; no provider or production authentication change is made.

## October15 — infrastructure setup guidance before voice implementation

User requests server/database/voice-hosting steps first. Recommend prove source-built/cooked WindowsServer locally before renting a private host; keep current .pfs authoritative until an explicit PostgreSQL+authenticated HTTPS backend/migration exists. Proposed future clients never receive DB credentials. Recommend EOS-managed media rather than renting a separate voice VM, with trusted room-token service and verified authoritative proximity integration; no provider/host/service selected or installed here. Source access requires account owner; next91ZwTwxp Personal1. SERVER_DATABASE_VOICE_SETUP_M15.md records exact current-build steps, proposed sizing/dependency boundaries and remaining gates. No actual architecture migration completed.

## October15 — keep provider selection explicit after transport audit

Current multiplayer uses direct-IP/IpNetDriver/engineNullfallback, no productiononlinevoiceidentity/token setup. InstalledEOS positionalupdate isunimplemented; genericinterface/legacy mute forwarding does not enforce distancepolicy by itself. Recommend boundedLANvoicefirst, but user product-scope decision91ZwTwxp remainspending before newtransport dependencies or service setup. No provider chosen/installed, no credentials requested, no absentmicrophonetest. VOICE_TRANSPORT_AUDIT_M15.md records exact available APIs/limits and secure nextgate.

## October15 — derive voice metadata from authoritative world actors

Nonticking Game/PIE subsystem accepts only active survival server worlds. Derive public ID, connection, real pawn position and life from owned actors; weak controller keys make reconnect consent defaultoff independently of persistent ownership GUID. Reject invalid ownership/identity/config/preferences atomically, no new voice RPC/provider/save fields. Real one2/2 and impairedtwo3/3 roles verify client refusal and server range/mute/block; existing consumption acknowledgements require300ms fixture spacing, keep real150ms cooldown. Finish server after allowner barriers without requiring exited pawns. PROXIMITY_VOICE_AUTHORITY_M15.md separates native/network metadata proof from future capture/listening/transport identity.

## October15 — separate proximity metadata policy from audio transport

Use a pure bounded32-participant native helper with explicit public identity/space, default-off independent transmit/receive consent, PTT, receiver-only mute and bidirectional block. Reject malformed input atomically and attenuate trusted positions3–15m. No waveform/log/save/provider dependency or invented microphone pass. The helper requires trusted callers; derive real server snapshots in a separately tested adapter before exposing preferences or routing to any transport. PROXIMITY_VOICE_POLICY_M15.md records two clean native tests and both targets; fullM15 stays open.

## October15 — Original expedition sprites and local hotbar

Use five original shared sprite atlases plus a static title illustration, catalog soft-icon overrides, square image bounds and a three-source-texel UV inset. Registered source dimensions/seams remain stable while Unreal compiles proxy textures; cancel an old UImage soft request before switching selected art. No real-time 3D inventory previews, blur, new plugin or arbitrary Fab mesh assignment. Existing Fab packages are staged/unassigned, so illustrations are provisional until actual licensed meshes/materials are selected and visually compared. The three-screen concept sheet is reference-only, not a new recipe catalog. Runtime labels retain all quantities, expiry and refusal details independently of color/icon recognition.

Hotbar uses eight session-local exact owned batch GUIDs, not copied inventory or replicated client-authoritative equipment. Inventory1–8 assigns/0 clears; gameplay1–8 selects and Q consumes through existing ServerInventoryAction with server ownership, life, rate and freshness validation. D-left/right cycles and D-down uses outside menus, preserving modal/build/craft contexts. No auto replacement after expiry/removal, no free items. Current best-carried tool/protection benefits remain automatic; selecting a tool is not an equipment claim. Shortcuts survive pawn respawn while this controller exists but reset on travel/new controller; persistent preferences/equipment are separate future work. Existing camp resources provide a reachable real-item fixture without modifying the map or persistent identities.

## October14 — Inventory owns upgrades; wall rest is neutral

Reuse the existing stat presentation as an actual inventory child tab instead of creating a separate viewport modal or duplicate authority. Inventory owns input locks/cursor, page switch retains selection, nested Back→Items and Tab/View close; Pause has no competing entry. Keep all point requests/data budgets unchanged. Idle wall input costs0 and blocks recovery; moving still costs12/s and exact zero drops. Existing camp sign only, no geometry/identity/save migration. User WALL_CLIMB PASS records the prior behavior, not pre-acceptance of this follow-up. INVENTORY_UPGRADES_WALL_REST.md records fresh narrow proof.

## October14 — stamina limits duration instead of prepaying wall height

Replace full-height ballistic launch with server-approved custom CharacterMovement, swept up/down/sideways motion and4/s holding versus12/s moving drain. Do not scan a whole tall wall or require enough stamina for its top; exhaustion releases into normal gravity. Allow only a local short clear walkable mantle, no collision bypass. Retain native saved-move input and sequenced owner acknowledgment instead of accepting client normals/targets/costs. Preserve serialized legacy fields as inert compatibility shims; no save/content-layout migration. Keep existing4m/10m wall identities and migrate only reviewed sign text. Old4m user PASS stays accepted in its original scope; the changed feel gets separate WALL_CLIMB/G16 acceptance. Exact tests/limits in CONTINUOUS_CLIMB_M14.md.

## October14 — one earned Copper slice and reviewed runtime layout edge

Ship the first mineral loop before later tiers: optional Wood tool cannot mine Copper; existing Stone/Bound stone Pick+Axe capabilities unlock it, Copper Pick improves compatible speed to4hits without larger total yield. Append portable ore+Wood refinement and reuse FieldTools knowledge gate; no station/charge/durability claim. Preserve original old15resource identities and add only a tagged near-camp node/sign through supported Editor Python. Server Load/startup approves only that exact manifest and preserves a verified original outside A/B before apply using originalUTC; do not immediately rewrite remaining food-life values with a new timestamp. Ordinary Save captures aged runtime16. Unsupported layouts block safely, no general mapping guessed. Real earned native/UI and actual server restart use isolated profiles/sources; human pacing is a separate gate before more tiers.

## October14 — reviewed additive resource manifests preserve old state

Use a pure exact-map/name/definition transform before any live migration integration. Validate complete source/candidate, preserve all existing resources/world bytes and append only bounded unique catalog-derived full defaults. Do not guess identities by order/proximity, discard unknown nodes, replace definitions or repeatedly append. Separate source/output; failures preserve both. This offline primitive has no live registered edge or file/actor effects; authoring, original backup, time-anchor-safe publication, restart and manual gates remain required.

## October 14 — preserve migration originals outside rotating generations

Add a trusted, game-thread no-overwrite original envelope copy under bounded project-local Originals/slot/edge paths. Verify exact selected generation/UTC/payload; keep the first healthy original across ordinary A/B rotation and refuse corrupt or occupied originals. Bounded archive copy preserves raw envelope rather than creating a newer generation. No destructive cleanup of partial destinations and no crash-atomic/cross-process/authentication guarantee. Future callers must validate semantic world/source/edge separately. API is not wired into normal Save/Load/startup and does not enable private-world migration.

## October 14 — harvest access is separate from speed

Use exact native Pick/Axe/Sickle Gameplay Tags and a bounded material-grade enum in trusted item/node data. Resolve the best compatible fresh owned tool per action; never accept client tier/target/yield claims. Empty/None defaults preserve legacy unrestricted gathering without assigning old generic speed tools new gated access. Tech fails explicitly until charge is implemented. Access refusal precedes inventory expiry pruning, progression preparation and node mutation. No permanent resource/layout/schema expansion in this backend slice; use disposable automation fixtures. Legacy actor identities, catalog IDs and saved inventory remain unchanged.

## 2026-10-13 - Separate player stat budget and persist validated ranks

Use existing earned XP/ten-level slice; one independent stat point per level, versus3 technology points. Four bounded ranks derive damage/max stamina/carry/new-job speed; owner ID-only request checks living own pawn/controller/server/cooldown and complete candidate. Shared native progression Game-config tuning, no INI/dependency change; allocation doesnot refill stamina or accelerate a running job. PlayerState lifetime survivesdeath; freshspawn applies/fills capacity. InnerPFXPV3 forallocations preserves zero-rankV1/V2 bytes and legacyXP; outerworldV2 validates owner-bound ranks before accepting/restoring upgraded vitals. Unknown/corrupt/overspent records fail closed, no private-file auto rewrite. Ancient technology planned separately. PLAYER_STATS_M12.md exactproof. Tall perches require reaching the full server-approved landing target through swept walking/step-up, not merely firstwalkable capsule contact; owner-only goal RPC and expiry remainbounded.


## 2026-10-13 - Replace arbitrary mantle ceiling with full stamina budget

Climb server traces within affordable full-height budget: base15 through1.2m plus10/metre above; jump-contact pays normal20 separately. Legacy MaxClimbHeight serialized field retains compatibility but now denotes base-cost height. Require walkable top, full swept capsule/overhead clearance, living pawn/weight/menus/cooldown/finite coordinates. Bounded falling ascent uses drive timed to above-top interval, clears on landing/death/cancel/jump; no wallhang animation/collision bypass. Stateless4m/10m fixtures near camp preserve all old persistent identities. Exact tests/failures STAMINA_CLIMB_M14.md; no manual acceptance assumed.


## 2026-10-13 - Share the bounded mantle drive across F and jump-contact climbs

The contact regression reproduced F requests accepted at valid80/110cm walls without reaching their tops. Manual mantles now retain the same server-derived horizontal drive used by automatic mantles. Preserve it through grounded pre-movement only while an approved climb has a queued Unreal launch and an unexpired1.5-second window; landing, cancellation, ordinary jumps and death still clear it. Continue swept falling collision. Original ground-based height, reach, clearance, weight, stamina and authority validation stay intact. The controller reuses its existing owner-only feedback RPC for explicit acceptance/refusal text; no new movement target RPC or inventory action is introduced.

User confirmed attempts at the4m danger landmark. The implemented ability remains a45–120cm ledge mantle, not vertical wall scaling. Actual key routing now verifies W/F and W/Space from ordinary walking contact, including two clients. Built-in private ApplicationCore is needed for the owning input-device mapper in automation; no plugin/external dependency. Initial fixture readiness waits for confirmed player setup/open input, with only its Shift readiness cue re-armed if cleared. A previous pre-request timeout is retained as an unproven timing cause, not disguised as a fixed gameplay defect. Exact evidence CLIMB_CONTACT_M14.md.

## 2026-10-13 - Discoverable feature fixtures beside normal spawn

Every new playable feature must ship with a clearly marked reachable fixture near the current open-world starting area and exact test instructions before handoff. Backend-only work uses automated fixtures. Preserve normal spawn/resource access and persistence identities; explicit migration is required if persistent records change. For climbing, relocate only the existing lane A80/110cm blocks and sign east of both starts, retaining their GUIDs and the southern lane. This is stateless layout repair, not a new gameplay system or larger-world gate. Existing clear low-ledge45-120cm mantle limits remain.

## 2026-10-13 - Authored stateless traversal area before broader world expansion

Keep fourclear80/110cm ledges nearcamp usingexistingprimitive/material and nonspatial loaded collision. No persistentresource/spawner ID or navigationchange; compare every oldactor before/after and repeat onlyverification. Python wrappers are not stable identity: compare UObjectasset paths, GameplayTag names and ActorGuid values rather than strrepr/memoryaddresses. Testactualsavedgeometry bytags beforemovement, no substitute spawnedcourse; useowning jump/savedmoves and servercost/landing. Render-onlywaiting/overview provesreadable sign without alteringheadlesspath. This boundedtechnicalpass doesnotreplace humanCLIMB or fullworldexpansion acceptance. LEDGE_TRAVERSAL_M14.md exactevidence.

## 2026-10-13 - Planned resource grades separate access from tool speed

Latest user steering requires dimension-aware origin and names. Use explicit trusted region/Frontier spawn permissions alongside tool grades. Titanium precedes Deepforge as Eryndor preparation; proposed Deepforge volcanic ingredients and Resonance crystal/ancient-tech inputs motivate expeditions. No final region name, map or raw exotic material name is invented as implemented. Avoid access cycles (first crossing must not need beyond-crossing material), arbitrary saved map paths and simultaneous loaded worlds. See RESOURCE_TIERS_M12.md for future save/travel tests.

Use the user's eight material tiers ending in original Tech, not the earlier bronze/steel proposal. Future tool-family Gameplay Tags and trusted material grades gate node access; best valid compatible server-carried tool preserves current automatic-selection behavior initially. Iron pick is the diamond minimum. Copper may improve speed without a separate mining access level. Existing stone/bound tool IDs and baseline recipes remain compatible; no retrospective item deletion/knowledge rewards. Powered Tech requires real charge, advanced infrastructure and bounded special research. Node eligibility refusal must preserve hits/items/stamina/XP/power; permanent new node layout waits for tested save compatibility. These are decisions for future implementation, not current gameplay changes. See RESOURCE_TIERS_M12.md.

## 2026-10-13 - Automatic short-ledge climb uses real authoritative contact

Arm one bounded0.75s window only from the first actual paid ground jump. Server movement impact selects the existing ledge path, measured from original ground feet so airborne extra jumps cannot reanchor taller walls. Approval costs normal jump20 plus climb15 and preserves capsule collision sweeps. Automatic-only approved horizontal drive persists after the initial wall slide until landing/cancel/death/new jump; client acknowledgment carries only server-selected impulse/mode. ManualF keeps its path. No per-frame idle probing, client target/origin authority or unlimited wall climbing. Shared live-course floor receipt barrier is test-only, using existing saved-move intent before raised teleport; no production net setting/RPC added. Exact failure and replay evidence in AUTOMATIC_CLIMB_M1.md.

## 2026-10-13 - Creature variety and stable-layout weighted integration

Use two additional native data-driven archetypes on the existing primitive class, with capped unique existing-item rewards and real item/stack validation before spawn. Keep legacy food payouts and references intact. Append alternate IDs to the existing authored policy, retain original tickets and dormant Ridge day, preserve point names/positions/count and resident saves. Do not add points or relax layout validation just to increase variety. Predicate tests permit the valid passive/hostile pools rather than assuming random draws always choose baseline IDs. Real-file/newserver/client/rendered proof precedes technical completion; human encounter acceptance precedes M14. Optional cross-species targeting would change combat/economy and needs a separate scope; taming remains unapproved, adaptations future. No art or new dependency. CREATURE_VARIETY_M13.md and WEIGHTED_ECOLOGY_M13.md record all limits.

## 2026-10-13 - Map-owned open-world ecology configuration

Assign the tested biome/day-night data asset to the two existing always-loaded open-world points through supported Editor APIs. Keep point names/positions and world-save schema stable: adding points would invalidate existing layout checks. Shore is active by day/night; Ridge is dormant by day but retains an already living/corpse resident at dawn. Server nav/clearance/global and per-biome budgets remain authoritative. Saves restore GUIDs/enabled state/delay, never arbitrary policy paths. Regression maps retain fixed-ID defaults. Real separate-process restart and authored rendered proof precede this technical completion; no full-human/FPS acceptance is inferred. See AUTHORED_ECOLOGY_M13.md.

## 2026-10-12 — Soft player carrying and bounded first-person traversal

Keep per-unit item weights. Stackable resources and food use a999,000 ceiling; independent tools/weapons/protection stay single items. Player30kg is a movement threshold, slots remain hard, and chests retain hard60kg capacity. Trusted runtime codec policy permits legal overweight player/pickup records, never a capacity supplied by a save/client. Preserve V1 inventory and V1–V2 world formats, GUID/freshness/strict finite validation and legacy reads.

Use a focused CharacterMovement subclass installed through the standard FObjectInitializer default-subobject override; the template constructor accepts that initializer without changing its meshes/input. Sprint/glide intentions use custom saved-move bits, combination boundaries and correction replay. Server speed is derived from the owned bag/life/stamina; client intentions never include speed, weight or target transforms. Walk multiplier is max(0.1,1/max(1,weight/limit)^2); any excess prevents sprint/glide, >=110% prevents jumping/ledge climbing. Grounded sprint multiplies configured walk speed by1.5 and spends10stamina/sec; glide reduces gravity to15%, caps downward speed220cm/sec and spends8/sec.

Climb is a short45–120cm ledge assist: server-derived forward/down traces, walkable top, full capsule/overhead sweeps,15stamina and1.5s cooldown. Server computes the bounded launch; reliable owner acknowledgment uses the same impulse and standard movement correction. Scoped falling braking/air-control guards restore normal values automatically. Clear intentions on modal menus/death and glide on landing; clear queued launch on death and refuse late corpse acknowledgment. Replicate only server-confirmed glide to simulated proxies for matching gravity; owner prediction stays independent and bag contents stay private. These are greybox abilities without external assets, equipment visuals, free wall climbing or third-person mode.

Rendered tests remain uncapped. NullRHI movement transport diagnostics use a process-only120FPS configuration override to avoid startup saved-move queue exhaustion; personal settings are not persisted. This is not a FPS benchmark. Dedicated uncooked runs remain supported; installed packaged Server restriction and human/controller/16GB acceptance stay separate.

## 2026-10-12 — Separate build selection from Back/Pause

B in placement returns to the picker instead of disabling building; B on picker closes it. Esc uses an explicit local BackOrPause binding for gameplay overlays and Pause only when none is open; focused modal widgets handle their own one-layer Back, with repeat guards on picker/chest/settings. Closing craft does not cancel the server job. P remains global secondary Pause. Keep existing authoritative close-storage RPC/placement and standalone-versus-multiplayer pause policy. User explicitly clarified standalone Esc is fine: no PIE input preprocessor, Editor shortcut interception or machine preferences change. MENU_NAVIGATION_M11.md distinguishes registered/Slate tests from human/device acceptance.

## 2026-10-12 — View-relative cardinal wall edges and contextual R

Wall/Door/Window default edge is the nearest cardinal direction to the possessed pawn's current eye yaw, plus the validated quarter-turn offset. The shared Candidate routine derives preview and authoritative transform; clients still send only piece ID/offset, and reach/support/ownership/collision/wood checks remain unchanged. No client-supplied transform or stored-schema migration. Other piece grid rules and existing saved transforms remain. One R binding rotates during first-person building, cancels only with crafting open, and stays inert in other contexts; modal crafting's existing R handler remains. Help/HUD use the registered action and call the value an offset, because looking across a cardinal boundary changes its origin. Standard InputCore private dependency is required for the non-Shipping plugin's real-key live fixture; no new plugin/service. WALL_ROTATION_M11.md records native/rendered/network evidence and manual limits.

## 2026-10-12 — Visible building selection and owner-private chest transactions

Replace the implicit cycling/storage shortcuts as the primary UX with a local eight-piece picker and owned E-driven chest UMG window. Stable stack GUIDs and bounded quantities go through existing server RPC validation; the server recomputes view/reach/ownership/living state. Sort preserves batch identities/deadlines; Quick Store attempts whole eligible batches atomically and leaves filtered/capacity-refused batches in the bag. Chest filters restrict future deposits only, retain existing contents/withdrawal and replicate only to the owner. Apply sends validated known unique item IDs (max64); Cancel never sends a request. Optional validated structure metadata defaults legacy saves to unrestricted, without modifying private saves. Append Window/Stairs enum IDs; cube windows support platforms, eight40cm exterior risers reach a solid upper platform without requiring slab cuts. Actual character traversal and owner/private replication are tested separately. BUILDING_MENUS_M11.md records bounded evidence; full human M11/M8 acceptance remains separate.

## 2026-10-12 — Opt into authoritative ecology without changing existing maps

Keep fixed-ID spawner defaults and add explicit map-owned policy/biome configuration. Require a server game world, one replicated clock, whole-catalog validation and one shared policy/catalog domain; disabled-point residents and corpses still consume biome budgets. Use existing nav/clearance/global-eight checks. Dormant phases refuse new spawning, while existing residents survive phase/configuration changes. Failed attempts retry every three seconds with change-only refusal logs. Existing save records retain resident identity/cooldown; dynamic policy configuration is not a new save field.

PF.PreviewEcology is a non-Shipping, fresh standalone L_M7SurvivalArena runtime preview, with complete layout/catalog preflight, personal-save refusal, no damage/loot grants and no asset save. Stop PIE discards it. Native/rendered and one-/two-client technical tests passed; ECOLOGY_RUNTIME_M13.md distinguishes same-process restoration from untested authored startup/reconnect and human acceptance. No new art, dependency, population increase, taming or full M13 completion.

## 2026-10-10 — Separate ecology policy validation from live spawning

Prepare a focused Unreal data-asset type with native Ecology.Biome tags, existing creature IDs, bounded day/night weights, finite respawn tuning and combined resident budgets no greater than eight. Validate the entire catalog before selection, including unrelated malformed rows. A pure zero-based ticket choice returns one eligible ID or an explicit refusal without modifying the output; no client seed, RNG state, actor creation, class loading or world mutation. A zero-total phase is valid dormant data and must never silently fall back to another phase/creature.

Proposed Shore/Woodland/Ridge rows remain inactive until a separate server-only spawner integration derives biome/time and validates nav/clearance/global count. No asset/map/schema/population change is part of this prerequisite. Future integration must reconcile clock/biome changes, resident identity, bounded retries and persistence before activation; new art, creatures or taming are separate scopes. Full M11/M12 human acceptance remains open under the user's independent-continuation permission. ECOLOGY_SPAWN_POLICY_M13.md records verification when complete.

## 2026-10-10 — Gate only optional stone-bound weapons with original knowledge

Use the existing validated catalog/owned purchase/recipe reservation and completion path for Tech_FieldWeapons, level3/threepoints/prerequisite FieldTools. Gate only existing BoundClub; baseline survival recipes remain available. Keep loaded legacy carried weapons valid at zero knowledge, without retrospective reward or schema migration; recreating them requires learning. Generic graph validation derives exact point spend and fails closed for unmet prerequisites. FIELD_WEAPONS_M12.md distinguishes grant-free earning from explicitly seeded network boundaries and retains live verification failures. No gameplay mutation RPC, item identity, asset or stat tuning change.

## 2026-10-10 — Reconcile crafting selection after layout settles

Request supported UMG ScrollWidgetIntoView when selected recipe, cached row size or browser dimensions change, rather than on every refresh. This handles accessibility wrapping and server-feedback footer growth while preserving stable manual scrolling. Rendered tests measure full selected-row button/title/body/picture bounds in its actual browser; fixed controls fit the screen, painted text fits allocation. Culled nonselected children retain stale cached geometry and are verified after selection/painting, not forced simultaneously onscreen. No gameplay/input/authority change. CRAFTING_SCROLL_M11.md records reproduction and narrow/regression replays.

## 2026-10-10 — Commit gathering rewards with successful inventory conversion

Derive reward category from the loaded server node yield, verify the living owned pawn/controller/PlayerState and prepare an aged progression candidate before granting resources. Commit only after Grant succeeds on the same authoritative game thread. Reward one action regardless of tool hit count; exhausted/capped reward budgets permit normal finite gathering. Keep existing lazy active-game-time windows and private owner-only replication/save binding; no new client reward RPC or UTC clock. GATHER_EVENTS_M12.md records actual native/rendered and one-/two-client new-server duration conservation. No mutation of personal saves or world assets, and no human pacing certificate.

## 2026-10-10 — Lazy active-time reward budget

Use server UWorld game time and a nonreplicated component epoch for gather reward windows. Capture is a validated read-only elapsed snapshot; successful craft/purchase commit and trusted restore rebase the epoch. Paused/dilated game time controls this budget, with no UTC/offline aging, per-frame ticking or client countdown promises. Save remaining duration/count/category through existing bounded innerV1/V2, not absolute timestamps. Invalid/backward clocks refuse atomically. GATHER_CLOCK_M12.md records native boundaries; real gather event/private new-process network gates remain separate. Manual whole-world restore is an authoritative checkpoint rollback, not a client budget reset.

## 2026-10-10 — Persist bounded gather windows before connecting rewards

Use one exact native GameplayTag category window for each of five ordinary gathered resources,5XP per confirmed success event/five events per1800active-server seconds. Save remaining duration and bounded count, not UTC/machine clocks or item/node event histories. Pure validated transactions commit candidates atomically; a separate future server hook must supply actual success and active elapsed. Preserve byte-compatible innerV1 writes when no windows exist, support innerV2 only for nonempty windows, and retain existing outer world owner binding. Decoder never interns unknown tag strings and rejects malformed counters/time/version/CRC without changing outputs. GATHER_WINDOWS_M12.md records adversarial/native/actual-file compatibility gates; pure budget arithmetic and trusted window seeds are not live earning or multiplayer acceptance.

## 2026-10-10 — Knowledge availability is a read-only view of server transactions

Describe selected recipe learning by validating a copy of the owner record against the complete catalog. Show explicit level/point/prerequisite refusals and already-learned state; never mutate XP/items in the menu. Real Learn button/K/D-right sends only the owned knowledge ID. Server feedback and later replicated record control the displayed outcome and Craft availability. Keep growing details inside the existing scroll panel. Screenshot fixtures wait for deferred captures/layout and measure the examined text inside that panel; do not weaken clipping assertions or change production layout to satisfy a stale capture. KNOWLEDGE_MENU_M12.md records native and real Slate-rendered gates. Synthetic input and trusted boundary XP are not human controller or earned pacing acceptance.

## 2026-10-10 — Gate optional recipes without removing legacy gear

Use the validated knowledge catalog mapping and private owner record to authorize optional crafting both at start and before conversion. Treat client read-only availability as presentation, never authority. Keep ordinary survival recipes and existing carried tools usable, including V1 zero-default saves; knowledge controls future optional crafts, not ownership of previously created items. Do not retroactively award or rewrite private saves. Tier-mechanics fixtures explicitly seed XP/learn via the real server request; separate access tests prove locked refusal/conservation and restart. Purchase UI remains its own rendered gate. RECIPE_ACCESS_M12.md.

## 2026-10-10 — Verify owned knowledge request before access/UI

Accept only a lookup ID through the owned controller; validate authoritative live pawn/PlayerState/catalogs,level/prerequisites/price/duplicate and cooldown using server state. Commit candidate only after complete validation; never grant XP/items or accept target/price/amount fields. Private feedback explicitly keeps recipe integration pending. Preserve existing recipes/legacy gear while verifying request/persistence independently; adding purchase controls or a useful restriction needs a separate real gameplay/UI gate. Native/network tests explicitly seed100XP and disclose that this is boundary verification rather than earned-level pacing. KNOWLEDGE_REQUESTS_M12.md has current evidence/limits.

## 2026-10-09 — Publish private player progression with real craft transactions

Use a no-tick owner-only component on the persistent PlayerState. Prepare a validated first-craft candidate before inventory conversion; commit 20 XP/recipe ID only after actual success, never on grants/cancellation/failed output. Runtime capture now emits owner-bound world V2; strict V1 reads supply zero defaults and preserve existing items/ownership/freshness without retrospective reward. Publish in-memory roster records and version atomically on disconnect, even before the first manual save. PLAYER_PROGRESSION_M12.md retains the actual mixed-version reconnect failure and successful correction. No XP RPC/purchase UI/recipe restriction or other earning source yet.

## 2026-10-09 — Opt-in owner-bound world V2 before player integration

Preserve normal V1 writer field shape and empty legacy progression defaults. Explicit world V2 requires bounded owner-bound progression for every player and full candidate validation; future world versions start at3. Runtime refuses valid V2 until PlayerState capture/restore is integrated, preventing silent progression loss through ignored fields. Separate metadata acceptance from actual runtime migration. No automatic private-file rewrite or retrospective item rewards. WORLD_PROGRESSION_COMPATIBILITY_M12.md records exact native/live gates and limits.

## 2026-10-09 — Separate bounded progression bytes precede live save changes

Use explicit16KiB/version/length/CRC progression bytes and resolve known catalog names without interning disk strings. All outputs change only after full candidate validation. Legacy V1 player records first pass their existing decoder, then get zeroXP/empty ledgers; owned items are not proof of past events. No existing main save writer/format changes in this codec slice; next owner-bound world compatibility must preserve actual world records before component/event hooks. PROGRESSION_CODEC_M12.md records exact tests/builds and integration limits. CRC is corruption detection, not trusted-source authentication.

## 2026-10-09 — Derive progression points and validate complete native candidates first

Before live XP or save extensions, introduce bounded native records/catalog transactions. Derive level/earned points from cumulativeXP and spent points from unique validated knowledge IDs; do not persist redundant point/level fields. First-craft identities dedupe even at cap. Knowledge prerequisite graphs are acyclic and affordable at declared level; baseline recipes cannot be gated. Pure methods are trusted-server building blocks, not proof of a successful event/network authority. No existing recipe access/save changes until explicit compatibility and real successful-event/server tests. PROGRESSION_RECORDS_M12.md records exact native scope, retained test alias crash and future integration gate.

## 2026-10-09 — Carried guard mitigates only creature-caused server hits

One craftable woven guard provides a bounded25% best-carried benefit, freshly derived from valid owned inventory. Do not stack bonuses or cache/save a modifier. Apply through server survivor TakeDamage only for actual APFCreature causers; generic damage, starvation/dehydration/exposure stay unchanged. Native Protection tags/stack1/finite0–0.5 validation and original procedural picture/details keep the contract explicit. Exact timed inputs/capacity use existing atomic crafting; V1 stable item IDs need no new schema. Public Health and private bags retain their replication boundaries. No equipment slots/worn meshes/durability or immunity added. PROTECTION_M12.md records actual damage, native/rendered/network/restart gates and human limits; balance remains provisional.

## 2026-10-09 — Weapon tiers preserve the existing server melee boundary

Two original club recipes use the existing timed exact-batch crafting path and native Weapon Gameplay Tags. Inventory derives best validated fresh carried melee item separately from best gathering benefit, with no equip cache/stack bonus/client damage parameter. Server still requires its own2.5m creature trace, life,0.5s cooldown and5stamina. Publish only held stable ID for local/remote engine primitive shape; full bag stays private. Item IDs fit existing V1 records, avoiding needless save migration. Original texture-free UI pictures/statistics are advisory. Weapons confer no gathering benefit; baseline recipes/yields remain. No PvP,XP,armor,repair or human balance acceptance implied. Exact gates/failures/limits: WEAPON_TIERS_M12.md.

## 2026-10-09 — Recipe categories are local browsing state

Build crafting tabs from validated Recipe.Category Gameplay Tags. A category change clears the selected recipe and requires a deliberate row choice; it must never send a craft RPC, consume inputs, select a substitute, cancel or restart a job. Baseline numeric shortcuts act only if visible. Keep job state/feedback authoritative and category selection absent from save data. Share the real modal footer text with Pause help so contextual shoulder controls remain discoverable without changing world/build bindings. Native regressions and actual Slate/rendered filtering/conservation checks are recorded in CRAFTING_CATEGORIES_M11.md; human acceptance is separate.

## 2026-10-09 — Tool tiers change effort, not resource totals

Make tool hit count and melee damage bounded validated item data, queried from the authoritative owned inventory rather than supplied by a client or cached after dropping. Best valid carried benefit per action preserves the existing automatic-tool behavior; explicit equipment slots/durability remain future work. Fibre cord plus the existing stone tool and wood produce a bound tool through the existing timed exact-batch conversion. The upgraded node action spends up to three remaining hits and inserts the corresponding finite yield, preserving depletion/regrowth/cooldown/capacity and total available resources. Existing attack trace/life/stamina/cooldown checks remain mandatory; replicated public held-tool presence does not expose private inventory. Stable IDs reuse V1 save validation without an asset/schema migration. No retroactive XP/unlocks or external assets. TOOL_TIERS_M12.md records native/rendered/network checks and retained fixture/build failures; full M12/human acceptance remains separate.

## 2026-10-09 — Building presentation keeps advisory and authority separate

Reuse the texture-free PFUITheme for the existing read-only building/storage overlay. Separate control footer from the inset selected-piece/storage data and keep the last authoritative result outside the inset. The preview remains explicitly advisory; storage countdown is its existing server-time deadline, with no preservation implied. No placement/ownership/collision/transaction/input/save changes. Reduced body/result/control base fonts with the same user scale leave room for real storage/refusal text at720p and maximum HUDScale1.5; expanded existing rendered bounds checks and screenshot review prove the bounded layout. BUILDING_THEME_M11.md records evidence/limits; human acceptance remains separate.

## 2026-10-09 — Recipe availability is a fresh-batch advisory

Existing recipe shortcut cards query validated catalog definitions and count only nonexpired replicated batches at synchronized server time. The UI must not prune inventory, queue work, promise output-space approval or turn ingredients-present into an action success. Busy/wait/cancel and missing data are explicit; existing component/RPC feedback remains the authoritative result. Three bounded native UMG cards share the texture-free theme, scale at10Hz and preserve current controls. General catalog navigation/selection and expanded outputs are separate work. RECIPE_DETAILS_M11.md records build/native/rendered evidence and remaining manual gates.

## 2026-10-09 — Inventory details are catalog/batch queries

Keep the original input/RPC authority and stable stack GUIDs. A read-only presentation model derives item category, unit/batch weight, quantity/limit, expiry and per-portion effects from validated definitions plus synchronized server time. Availability is not a server action acknowledgment: missing/expired data is explicit and real recovery remains capped/validated by existing gameplay. Native UMG inset reuses the original texture-free palette, maintains four rows/10Hz and leaves first-person aim/vitals clear; no icon loading, data/schema migration or new dependencies. Narrow build/native/rendered evidence: INVENTORY_DETAILS_M11.md. Full human acceptance remains separate.

## 2026-10-09 — Explicit solo world menu and cosmetic world names

Use the existing lightweight /Engine/Maps/Entry shell with a native session GameInstance and replaceable UMG menu, without new binary assets or dependencies. One-shot menu New/Load intent takes precedence over stale process save flags. GameDefaultMap is Entry; ServerDefaultMap is the open-world development map; EditorStartupMap/template assets are preserved. Pause Save is local-authority only and visibly reports the real server save result; it never resets/loads gameplay. Multiplayer menu currently provides information only, not a host/join/browser implementation.

Rename changes a1–64-character display label in a bounded version1 checksummed Metadata_WorldNames registry, preserving world file generations, IDs and ownership. Identity_/Metadata_ are reserved from gameplay saves. Reject invalid/duplicate labels and corrupt metadata without overwrite; missing labels fall back to stable slot IDs. This avoids migrating personal save files. Solo CheckLogin now matches the pre-existing sole-owner Login adoption when another solo world's endpoint credential replaced the local profile; unknown network credentials and connected-identity reuse remain rejected. Final builds, native4/4, rendered UI and one-/two-client restart regressions passed; exact evidence/retained failures: WORLD_MENU_M11.md. Personal rendered full-loop acceptance remains separate.


## 2026-10-09 — Server setup acknowledgment is separate from profile storage

Use the existing deferred PostLogin restore result to send one reliable owner-client acknowledgment of NewSurvivor/Restored; remain Unconfirmed until it arrives. Track successful restoration transiently per login rather than infer it from a nonempty saved record: saving alone is not restoring. Failed setup returns to menu without acknowledgment. Pause labels and Blueprint getters are read-only presentation, preserve focus and reveal no credentials. This confirms server setup for this connection, not client replication arrival, later manual world loads, world-save success or production authentication. No save schema or gameplay RPC mutation is added. Exact evidence/limits: SERVER_SETUP_FEEDBACK_M11.md.

## 2026-10-09 — Local reconnect credential feedback is a separate UI outcome

Record Unknown/Saved/Failed only after the existing local profile write returns. Expose read-only Blueprint status/text and update Pause's label without refreshing navigation or modal focus. Credential storage success never implies world state is saved or the server has restored a player. Never include capabilities/login URLs in feedback. Rendered fixtures use separate unique profiles; an expected invalid-payload rejection cannot stand in for disk-full or production identity testing.

## 2026-10-09 — Inventory display windows follow stable selection

Keep the authoritative bag and GUID selection unchanged. Present four rows around the resolved selected index, expose the visible range, and use existing navigation to reach omitted stacks. Missing selection stays explicit after expiry; never choose a replacement for visual convenience. Reuse the scaled read-only overlay with separate feedback and10Hz refresh. Current greybox eight-slot/maximum-scale720p/1440p evidence does not certify arbitrary custom text or manual multiplayer usability.

## 2026-10-08 — Low/medium VSM quality matches the existing budget

Project scalability reduces stationary/moving directional shadow resolution at low/medium instead of increasing the inherited512-page pool. Higher tiers remain engine-defined and settings keep the player's chosen tier. Actual group switching/restoration and two normal1440p rendered replays verify the bounded overflow fix; do not infer a general performance gain without matching traversal measurements. Do not force unrelated groups in a focused fixture when explicit game settings intentionally override blur/DOF.

## 2026-10-08 — HUD observation is read-only and possession scoped

Infer a brief damage cue only from decreased health on the same current pawn; no predicted damage/source/direction. Reset on possession gaps/replacement, clear unknown vitals and suppress dead/missing action prompts. Distinct needs/health/exposure warnings survive disabled control hints. Blueprint presentation gets explicit availability. Keep rendering diagnostic profile separate from normal-profile acceptance: session-only reduced shadow resolution proved the1440p pool warning is avoidable without increasing memory, but does not fix default scalability.

## 2026-10-08 — Inventory selection follows stack identity

Keep local selection as an existing stack GUID and resolve its current row/freshness at action time. Expiry/removal/replication/load must not silently select a replacement; require an explicit navigation input and show its actual name/quantity. Preserve first-use selection/deposit behavior. Server APIs still validate GUID ownership, life, quantity, capacity and deadlines; UI selection grants nothing and does not enter save data. Native bound-input tests and isolated rendered expiry/reselection verify this fix.

## 2026-10-08 — Registered bindings drive read-only controls help

M11 help records descriptions beside real legacy key registration and reads movement/view/jump from the current public Enhanced Input mapping view. Keep UI local/read-only; eventual remapping must change both binding routes. Explicit device pages avoid stealing focus. Pause owns/removes child modals; help Back returns to Pause without resuming or forwarding gameplay keys. Content-size the pause background for wrapped text. Automated Slate input/screenshots complement physical-device/human acceptance. Latest user authorization makes Personal tests nonblocking for independent implementation; their results remain unverified.

## 2026-10-08 — Controls and reconnect UI follow real authority/state

UI_CONTROLS_PLAN.md proposes read-only controls/help before remapping, binding descriptions tied to actual handlers, one action per overlay context and server-accepted transaction feedback. Keep local preferences separate from world state; show sanitized reconnect/profile/save failures without exposing capability data or offering client repair. Future research/adaptation views require their C++ backend before presentation can claim unlocks/effects. This is a document contract, not UI/gameplay implementation.

## 2026-10-08 — Preserve stored corpse collision through staging

SetCollisionEnabled compares effective body collision including the actor override; a staged disabled actor can make a dead capsule's NoCollision request short-circuit without changing its stored mode. Applying the built-in NoCollision profile in dead creature restoration updates that mode before activation. Keep living creature behavior and loot restoration unchanged. Four initial failure assertions, the successful corpse retry, living/dead WorldRuntime regression and Editor/Game builds verify this narrow runtime fix; manual/live rendering remains separate.

## 2026-10-08 — Explicit layered save compatibility plan

SAVE_COMPATIBILITY_PLAN.md proposes bounded known-edge value-record migrations, separate verified original backups beyond A/B rotation, explicit item/layout policies and unchanged food-age/identity anchors. Keep file/world/player/profile versions distinct; unknown future schemas and unmapped content refuse rather than guess or drop data. Synthetic frozen fixtures establish compatibility before any live publication. This is document planning only; no schema/runtime migration or full M18 acceptance.

## 2026-10-08 — Gate native automation on structured outcomes

Unreal exit 0 occurred after a real twelve-assertion failure and an unmatched selection. Native verification therefore checks nonempty clean report records/counts, actual engine exit and fatal/ensure logs, instead of exit alone. Keep raw-log review and manual/live/rendered gates explicit. Use bounded hidden NullRHI processes, unique evidence and only owned-process cleanup; never close a user's Editor or upload identity/save data. Scripts/RunNativeAutomation.ps1 adds no plugin/dependency or gameplay/settings change.

## 2026-10-08 — Never publish the default world after a failed startup restore

Latch startup restoration failure through Capture/Save as well as login. Generic file-checksum validity is insufficient: a future semantic version or changed authored layout can fail world restoration while still being a readable file generation. Refuse publication with a specific diagnose/restart reason instead of making default state the newest save. Keep files, output records and schema unchanged; recovery is deliberate, not a silent overwrite. A test-first failure, successful focused/native regressions and normal one-client create/restart establish the narrow fix; rendered/manual gates remain separate.

## 2026-10-08 — Presentation-only bounded audio plan

AUDIO_FEEDBACK_PLAN.md proposes typed feedback at accepted gameplay outcomes, with local UI and range-limited cosmetic world events. Do not parse text strings or replay initial/save-restored snapshots; keep listen-server playback single-path and missing cues harmless. Dedicated servers must not load audio presentation. Use existing built-in class preferences and proposed concurrency/attenuation limits before any broader sound library; numerical budgets still need measurement and human audibility.

This task changes documents only, not dependencies/assets/source/settings. Future cue selection follows M9 provenance and preceding manual gates. Correct future adaptation planning to acknowledge that generic Exposure needs a typed hazard contract and inspected stamina actions do not include sprint.

## 2026-10-08 — Original progression planning without gating baseline survival

PROGRESSION_PLAN.md is a provisional future M12/M13 contract, not implementation. Preserve baseline gathering/tool/cooking/drying/shelter access. Separate ordinary XP/technology knowledge, unique challenge discoveries, lasting adaptations and equipped biological modifiers. Player-specific authority/owner-only progression must not confer environmental access to a less-progressed co-op partner. Respec cannot replay rewards or refresh food; future V1 migration starts at baseline rather than awarding XP retroactively.

Use the existing catalog/PlayerState/transaction boundaries for the later implementation; no new ability framework, plugin or current save-version change. Gate gameplay and measured balance behind preceding acceptance. Independent documentation is allowed while M7/M8 manual gates wait. Correct the food-policy documentation to reflect actual M8 offline aging rather than its superseded pause proposal.

## 2026-10-08 — Metadata-first intake while manual gates wait

Use a read-only Asset Registry/package-size inventory before candidate integration. Do not load heavy models/textures, move legacy packs or fix redirectors during intake. Registry metadata/disk footprint is screening evidence, not runtime memory, compatibility or provenance approval. Document proposed small material/texture/LOD/collision budgets and replace placeholders through reversible presentation references while preserving gameplay IDs/authority. ASSET_PIPELINE_M9.md records the process; no new runtime dependency or asset operation is introduced.

Volume preferences are separate from authored sound coverage: class/mix controls and a passing preferences test do not prove footstep/creature/UI/music playback. AUDIO_COVERAGE.md records that gap. Further art/gameplay acceptance depends on the open manual gates and actual source/license/candidate checks.

## 2026-10-08 — Bounded world saves and development reconnect

Use a game-owned WorldSubsystem with built-in Json/JsonUtilities and catalog-only V1 records for the small greybox world. Validate the whole record before restoration; stage actors and replace inventories, never append saved contents. Preserve stable structure/player identities and reject invalid ownership/support graphs. Keep PrimalAgentTools as a non-Shipping adapter, with no reverse gameplay dependency.

Retain checksummed A/B generations, explicit backup recovery and refusal to overwrite corrupt evidence. Save manually before exit; capture a departing client before pawn teardown and checkpoint only a configured active slot. Defer credential delivery/player restoration until PostLogin's next tick, after Unreal attaches the connection. A failed restoration must not overwrite its saved player with a default spawn.

Reconnect uses a separate private GUID capability in a profile keyed by endpoint; public PlayerState GUIDs only represent ownership. This is trusted LAN development identity, not production authentication. Do not export credentials, profiles or complete login URLs. Production accounts/transport protection and save migration need later design. Food ages offline; other timers pause; crafting is cancelled. No global rollback-atomic/power-loss guarantee is claimed.

Use one client before two; NullRHI create/restart checks now pass for both. Installed Launcher engine blocks packaged Server targets, so record uncooked dedicated-server evidence separately. Manual/rendered M7/M8 gates remain unverified despite 26 passing regressions and ten passing live process reports.

## 2026-10-08 — Maintained state summary and stable structure-owner key

Update Docs/CURRENT_STATE.md after every meaningful change and build/test result, separating implemented code from verified behavior and milestone acceptance. Preserve failures and subsequent retries rather than replacing failure history with a pass. Record future adaptations and active/passive biological modifiers as planning, outside M8 gameplay.

Use the server-issued PlayerState GUID as a structure's stable owner key. Builder remains the current presentation/replication pointer and can be rebound only by a matching stable identity. Public ownership IDs are not credentials. Update the existing ownership fixture to hand over the key as well as the pointer, and verify replacement-PlayerState access plus different-key refusal. Actual authenticated reconnect and restart tests remain pending; a native object replacement is not that evidence.

## 2026-10-08 — Server restoration and preserved file generations

Continue M8 under the user's explicit autonomous-continuation instruction, without converting M7's missing manual evidence into a pass. Separate the public replicated PlayerState ownership GUID from a future private reconnect credential. Validate server identity/capacity/vitals and capsule/ground safety before restoring. Replace inventory atomically, preserve batch GUIDs and subtract elapsed offline age; do not grant new shelf life. Cancel in-progress crafting rather than serializing pending conversions.

Use two bounded checksummed file generations. Unreal's generic file move deletes an existing destination before rename, so only replace the inactive file after pending-write verification. Keep the active generation and refuse overwriting corrupt evidence. No new dependency; file I/O uses Core APIs. Whole-world capture/restore and authenticated reconnect integration remain subsequent M8 work.

## 2026-10-08 — Bounded player-save format before live persistence

The user explicitly deferred the unpassed M7 manual route/overnight gate. Start only an independent M8 data-format checkpoint and keep both milestones incomplete. Earlier gate statements below describe their original checkpoints; this authorization permits preparation, not a claimed manual pass.

Use native Unreal memory archives with explicit fixed scalar fields, bounded ASCII catalog IDs and bounded stack counts. A version/length/CRC envelope rejects incompatible, truncated and accidentally corrupt data before decoding; CRC is not authentication. Use trusted catalog and capacity/max-vital limits rather than values supplied by a save. Decode into a candidate and replace outputs only after full validation. No new module/plugin dependency.

Player and inventory stack GUIDs are values; no actor pointers, display-name identity, arbitrary object paths or serialized absolute server-time expiry. Food stores remaining freshness without merging batches or renewing lifetime. A future server adapter must define authenticated player-ID mapping, offline spoilage policy and atomic capture/restore before these records become usable saves. Unknown versions/items fail explicitly; migration is not yet implemented. No live-world mutation, RPC, disk write or PF save/load command is added in this step.

## 2026-10-08 — Local settings and bounded open-world streaming

Use a project UGameUserSettings subclass for local preferences and native scalability/display controls. Draft until Apply; revert unconfirmed display changes after 15 real-time seconds. Explicitly load/save game scalability in Editor sessions because the base Editor path uses EditorSettings.ini. Motion blur defaults off. Project Music/Effects/UI sound classes and a preference SoundMix provide category routing; existing explicitly assigned third-party classes require deliberate routing. No plugin dependency or external media was added.

Retain the M7 arena as a regression fixture. The open-world request is represented by a separate 400 x 400 m World Partition greybox candidate. Keep its small simulation camp and collision boundary loaded; stream distant stateless landmarks until M8 defines persistent simulation lifetimes. Asset/network checks do not certify full traversal. M8 remains gated by manual route/overnight play. Selected Viewport PIE/F11 is a measured workaround for the New Editor Window presentation delay; retain that unresolved diagnostic explicitly rather than changing project RHI settings without evidence.

The user's October 8 authorization to push everything supersedes leaving imports outside automatic commits. Preserve user packs in a separate LFS commit; do not integrate them into this greybox or claim provenance/rights verification. Archive the reconciled stash before dropping it. Use the requested Codex GPT-6.1 Sol co-author footer going forward.

Prefer source, commandlets, Unreal APIs, logs and bounded NullRHI tests; minimize Computer Use to required visual/manual checks. Use existing assets first and handle straightforward setup through supported scripts; flag specific asset gaps or imports needing extensive UI work when a milestone actually requires them. No new external asset is needed for M7 verification. The isolated streaming probe supplements the pending human route/overnight gate and does not authorize M8 before confirmation.

## 2026-10-06 — Continuous world prototype and finite water

The full M0-M25 roadmap is preserved in FULL_PROJECT_ROADMAP.md; ROADMAP_STATUS.md defines the current M7/M8 execution boundary and the final open-world requirement. Keep the M7 prototype continuously traversable across its zones. Its small bounded geometry is an integration fixture, not a final open-world or streaming implementation. Do not confuse automated path connectivity with a completed manual travel gate.

Add fibre and water through existing item/resource data definitions. Water is a finite, nonperishable placeholder inventory portion; collecting it uses server range/aim/cooldown/depletion/capacity checks, and drinking spends exactly one portion through the existing owned inventory RPC. It restores thirst only and is refused at full thirst. Physical vessels, water contamination, swimming and water simulation are future features. Fibre has no new recipe in this increment. No external dependency or imported art is required.

Recover only missing M7 changes after the user/Claude merge at 6d3ee12; preserve centralized asset paths, named request codes, inventory capacity fix, pickup labels and documentation refactors. Rebuild and retest the merged source rather than treating pre-merge evidence as current certification.

## 2026-10-06 — Preserve user asset imports while finishing the greybox gate

The user has added Adventures_Pack, Bike, DynamicFalling, Modular_Rural_Cabin and Polyphoria and authorized sourcing other assets when needed. Keep these user changes outside the current M7 commit and preserve their paths/references. No imported pack is needed for the small arena verification. Before selecting an asset for a concrete feature, inspect suitability, dependency/memory cost and available usage rights; notify the user if a license, purchase or manual import is needed. This does not expand M1-M8 into final-art production or third-person gameplay.

## 2026-10-04 — Interaction alignment, pause and gamepad input

Keep first-person camera origin on the survivor capsule at the same eye height used by server traces; animated head sockets can differ between rendering and dedicated-server simulation. Use a shared bounded visibility query with 12 cm small-target tolerance and an independent obstruction check. E prioritizes nearby resources/items even when building preview is open. Inventory transactions, reach, life state, cooldowns and authority remain validated on the server. Give visible success/refusal and range prompts.

Pause is local UI: freeze simulation only in Standalone. Network menus explicitly say the world continues. Resume restores game input; ending a session requires two activations and warns that persistence is not yet implemented. Use P in PIE because Editor Esc can end Play. Gamepad survival buttons reuse keyboard action methods/RPCs, while the existing Enhanced Input context supplies analog movement/look and jump. Keep overlays mutually exclusive to make contextual buttons unambiguous. No new plugin or template asset edits. Physical device/OS delivery, feel and hot-plugging are separate from automated binding verification.

## 2026-09-30 — Small integrated greybox arena and server time

M7 reuses existing catalogs and mechanics in a separate 60 x 70 m non-streamed arena. Keep safe, resource and danger zones connected by walkable primitive ground with optional stepped height variation. Baked navigation and two capped spawn points bound AI work. A replicated server clock owns day/night time; clients render two simple directional lights and cannot set server time. No World Partition, water simulation, new art or external dependency is justified at this scale. Engine inspection reports installed 5.8.3; record that version instead of claiming these runs used 5.8.2.

## 2026-09-29 — Bounded server creature AI

M6 uses two original data definitions, native Gameplay Tags for replicated states and 10 Hz server decisions. Built-in AIController/NavMesh handles movement, and server distance/line-of-sight queries provide simple perception. Attack windup rechecks target life, sight and range; finite loot uses existing food expiry and pickup transactions. Each spawn point has one resident and a shared eight-creature limit includes corpses. NavigationSystem is a built-in module dependency for gameplay and its live tests. No behavior-tree asset, external AI plugin or art acquisition is needed. The user has existing art assets: notify them before planning their integration, after the greybox milestone boundary.

## 2026-09-29 — Test hardware and performance reporting

Use the user's 32 GB Ryzen 9 5900X machine as the current reference; record the full supplied configuration and observed GPU-name discrepancy in `TEST_MACHINE.md`. Keep historical 16 GB constraints as historical evidence, not a current measurement. M5 uses an uncapped 960x540 sample with all frames retained, including the capture-start hitch. Do not extrapolate this stationary small-arena sample to native 1440p, large worlds or a different GPU SKU. No persistent rendering settings change is needed.

## 2026-09-28 — Milestone 5 building authority and safe removal

Use a fixed 400 cm structural grid and data-defined piece IDs/costs/health, with view-derived server placement rather than trusting client transforms. Replicate structure ownership/support and reuse the existing inventory for private storage. Transfers retain exact freshness deadlines. Refuse removal or lethal owner damage while a piece supports children or contains items; no automatic cascading loss or refunds. M5 is a cooperative placeholder rule; group access, preservation equipment and persistence require later work. Test uncapped rendering with session overrides and report frame times alongside memory instead of treating a capped FPS value as optimization evidence.

## 2026-09-27 — Milestone 4 atomic completion and finite resource nodes

Crafting keeps ingredients in inventory and snapshots exact stack IDs, quantities and deadlines at start. Completion revalidates and performs one atomic conversion. Moving/consuming/spoiling an input can invalidate the job, clearly reported; cancellation requires no refund space and cannot reset food freshness. Death or pawn replacement cancels work. One active job bounds replication and UI cost. Resource nodes validate first-person aim, reach, cooldown and capacity before spending finite hit counts. The carried primitive tool reduces actions required without increasing total node yield. Recipes and resources are editable catalog entries; portable cooking/drying consumes wood fuel and time, while placeable stations wait for M5. No new plugin or external dependency is added.

## 2026-09-26 — Keep supplied visual references outside runtime content

Preserve the user's eight Palworld/ARK screenshots unchanged under [References/SurvivalGames](References/SurvivalGames/README.md), with an index of relevant readability, first-person framing and progression principles. These are documentation references, not imported assets or a copied art/UI specification. M3 keeps its lightweight readable inventory overlay. Technology progression, creature/world visuals and final presentation remain behind their respective plans and gates.

## 2026-09-25 — Milestone 3 inventory ownership and food batches

Keep inventory on PlayerState, independent of the pawn, so death/respawn does not destroy item state. Replicate bounded stack contents owner-only; replicate world pickups to relevant clients. Use a small editable data-asset catalog and stable item/stack identifiers. Preflight insertion capacity, preserve the world actor when pickup fails, and spawn a drop before removing its quantity. Food from different deadlines occupies separate slots; split/drop/pickup never refreshes it. Consume the selected usable stack through validated server requests. No inventory grant RPC exists. Save/reconnect restoration remains M8; M3 reconnect deliberately starts empty. See `INVENTORY_M3.md`.

## 2026-09-25 — Retain technology-tree references for later planning

The user's Palworld and ARK progression lists inform accessible ordinary unlock points and a separate challenge-earned path, with early essentials, prerequisite/station requirements and later preservation upgrades. [TECH_TREE_DIRECTION.md](TECH_TREE_DIRECTION.md) records the original-game design direction and scope boundary. Do not implement a tech tree while finishing M2 or assume the pasted item roster is an approved feature list.

## 2026-09-23 — Food research and preservation direction

Use Palworld and ARK: Survival Evolved's cooking and preservation progression as mechanical research, documented with sources in [FOOD_AND_PRESERVATION.md](FOOD_AND_PRESERVATION.md). Primal Frontier uses original tuning: per-food lifetimes, found/gathered inputs, later cooking/drying and maintained storage. Five-minute M2 rations are test tuning. Moving food cannot reset freshness, spoiled ingredients cannot create edible meals, and preservation slows remaining decay rather than rejuvenating food. Future inventory tracks freshness by acquisition batch; its expiry UX must be playtested because it differs from sequential stack spoilage in the researched games. These later systems remain planned, not implemented or verified.

## 2026-09-22 — Milestone 2 survival needs and placeholder interaction

2026-09-23 refinement requested by the user: world rations are finite and perishable, never a permanent player ability. Use a replicated server-time expiration deadline and server-side consumption rejection/cleanup. Default shelf life is 300 simulation seconds. Carry expiration into future inventory and cooking; do not add those systems ahead of their milestones. Developer recovery remains a non-Shipping testing hook.

Extend the existing replicated vitals snapshot with food/water reserves and exposure. Integrate threshold damage using time spent below the threshold; keep server authority in C++ and use existing death/respawn handling. Pause stamina recovery while starving, dehydrated or exposed. Health regeneration is optional and off by default to preserve the M1 damage contract.

Use a no-tick primitive hazard actor and the survival component's existing 10 Hz server tick to resolve the strongest overlapping exposure. Add a single-use ration with a validated server view trace and bounded fixed recovery, rather than introducing inventory early. Replicated actor destruction prevents duplicate consumption. A small separate M2 map preserves the M1/template assets. No new engine plugin, external art, third-person mode or world expansion is required.



\## 2026-09-12 — First-person primary experience



Decision: First person is the primary camera and gameplay experience.



Reason: It best supports the intended survival, exploration, combat, and building feel.



Consequence: The game uses first-person arms and held-item meshes locally, while retaining a replicated full-body mesh for other players and future third-person support.



\## 2026-09-12 — Third-person is optional later



Decision: Third person is not an early-development requirement.



Reason: The game should establish a strong first-person experience before taking on the extra animation, camera, UI, collision, and balancing work of a second perspective.



Consequence: Camera architecture remains extensible, but early gameplay is tested primarily in first person.



\## 2026-09-12 — Server-authoritative architecture



Decision: Important gameplay state is owned and validated by the server.



Reason: Cooperative multiplayer, eventual PvP, persistence, and anti-cheat all need a trustworthy authority.



Consequence: Inventory, damage, building, crafting, creature ownership, and progression are validated on the server.



\## 2026-09-12 — C++ core, Blueprint presentation



Decision: Core gameplay systems use C++; Blueprints focus on presentation, tuning, UI, and asset composition.



Reason: C++ improves long-term maintainability, testability, networking control, and performance.



Consequence: Security-sensitive or complex gameplay rules are not implemented solely in Blueprints.



\## 2026-09-12 — Survival-to-Frontier progression



Decision: The main progression transforms players from vulnerable survivors into dimensional explorers.



Reason: This progression provides clear goals while allowing survival, settlement, creatures, technology, and exploration to reinforce one another.



Consequence: New systems, resources, and regions should map to a meaningful progression tier.



\## 2026-09-12 — Original setting and content



Decision: Primal Frontier is an original world, setting, creature roster, technology system, and visual identity.



Reason: It can be inspired by broad survival-game genres without copying another game’s protected expression.



Consequence: Avoid copying names, creatures, lore, structures, maps, assets, UI, sounds, or story content from other games.

# 2026-09-21 — Isolated first-person survival foundation

Use a reusable replicated C++ vitals component and a separate survivor subclass, controller and GameMode. Preserve the template camera, owner-only first-person mesh and remote full-body mesh through new project-owned Blueprint compositions. Keep template assets and the project's default-map settings unchanged.

Health and Stamina mutate only on authority; damage uses Unreal's damage pipeline. Respawn creates a fresh pawn at a PlayerStart. Placeholder UMG presentation reads the current possession and exposes a Blueprint event for future layouts. Hunger/Thirst remain tag interfaces only.

PrimalAgentToolsRuntime now depends on the project game module to call the same validated survival APIs. The game module does not depend on tooling, and command registration remains excluded from Shipping. No MCP server, gameplay RPC for arbitrary developer mutations, external assets or new plugin dependency is introduced. One-player command targeting deliberately rejects ambiguity.

Use component/lifecycle automation, an opt-in live replication test, and a manual first-person test as distinct gates. Use one rendered client and a NullRHI server on the 16 GB machine. Do not infer multiplayer success from role simulation or a previous tooling test.
# 2026-09-28 — Native first-person aim marker

The M4 rendered playtest exposed an offset external crosshair overlay. Add a small centered native HUD marker so the player can aim the server-derived gathering trace reliably. This is presentation only; trace reach and server validation remain unchanged. The repeated forage/cooking/drying playtest passed with this marker. Portable cooking/drying remains the M4 scope; placeable stations, equipment durability and technology unlocks are future work.
## 2026-10-08 — Stored shape profiles during world restoration

Apply built-in NoCollision profiles to hidden building shapes and restore BlockAll profiles/responses to selected visible shapes. SetCollisionEnabled can query effective owner collision and skip the stored body update while persistence staging disables an actor. Changing profiles avoids activating hidden geometry when staging ends, and restores closed panels after toggling. Preserve constructor collision policy, authoritative ownership and the existing save format. Actual traces/capsule sweeps across repeat loads verify this narrow fix; rendered route acceptance remains separate.

## 2026-10-09 Centered crafting presentation (M11)

Use a local UMG list/detail modal and original procedural Slate item pictures for placeholder catalog icons; optional soft textures load through supported UImage API. Recipes are validated catalog entries, not a fixed three-row UI. Selected crafting uses existing server RPC/batch transactions. Balance only the modal's movement/look ignores and restore game input/cursor on Close/Pause. World continues during crafting. Keep headless controllers free of invisible modal input locks. Actual Slate keyboard button tests substitute for unreliable background-pointer synthesis, explicitly leaving real mouse/device acceptance open. No new dependency/assets/save format. Full evidence CENTERED_CRAFTING_M11.md.
## October16 — Public stateless camp stations (M12)

Use a focused replicated native station actor with GameplayTag type, bounded radius/speed and real server Visibility/range validation. Workbench/refinery offer optional nonstacking speed bonuses; existing portable recipes and exact-batch conversion remain intact. Snapshot earned speed plus station at start, revalidate the same physical station before conversion, cancel safely on loss. Client UI previews proximity but owner-only replicated job fields describe actual authority. No arbitrary target/multiplier RPC, fuel inventory, unattended processing or new save schema. Authored public stations are static world content rather than player-built persisted structures; player ownership/placement is separate future work. Ignore stripped TextRender presentation on dedicated servers. Supported additive map fixture preserves existing97 actors; no existing resource/layout migration needed. Evidence/remaining verification: CAMP_CRAFTING_STATIONS_M12.md and CURRENT_STATE.md.
