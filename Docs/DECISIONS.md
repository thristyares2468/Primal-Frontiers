# Architecture and Design Decisions

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
