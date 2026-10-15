# Primal Frontier — current state

Updated October 15, 2026. This is the current handoff. The previous chronological journal is preserved byte-for-byte in [History](History/CURRENT_STATE_JOURNAL_2026_10_15.md); its old pending statements are historical, not current instructions. Detailed test history belongs in the evidence documents linked below.

**Current free-first route:** [Oracle / Neon / EOS exact checklist](ORACLE_DATABASE_VOICE_SETUP.md) supersedes the paid-host recommendation below. Dedicated Oracle SSH public key prepared at C:\Users\jackh\.ssh\pf_oracle_ed25519.pub; private key outsideGit with verified owner-only ACL.12/12 setup contracts pass again; readiness/source access refreshed, onlyC:48.09GiB. Local UBT recognizes LinuxArm64/aarch64, not proof of project/SDK/plugin package compatibility. No Oracle account/endpoint, DB or voice credentials/integration are confirmed. Owner account steps and source build storage remain separate prerequisites; no service purchase, VM/firewall/game/settings/private-save change. Reports Saved/AutomationReports/M15OraclePreparation_20261015.

**M15 setup reopened, free or low cost:** [current server/database/voice guide](MULTIPLAYER_SETUP_2026_10_15.md). GitHub API confirms Epic source access and the5.8.3-release tag; no account linking is needed. Only C: detected,48.26GiB free: source-engine/package work needs a separate build location (planning400–500GiB free).12/12 PowerShell setup contracts pass; sanitized readiness and localhost NullRHI launch plan generated under Saved/AutomationReports/M15Infrastructure_20261015. Launcher defaults to plan-only; live launch was not tested. No runtime/assets/settings/private-save change, C++ build, cloud/DB/voice provisioning or new gameplay pass. Next owner action: identify suitable build storage, not rent a server yet.

**Publication restored:** user reports GitHub account LFS budgetUSD10; other product budgets remain0. Ordinary retry pushed M12 Eryndor `a6a8b0d` and blocker receipt `3b04006` to origin/master,11/11LFSobjects56KB uploaded. Local/remote SHA equality and clean tree confirmed. No force push, asset omission, history rewrite or other service enabled. LFS allowance task can close; no other GitHub budget increase is required for this local development/push workflow. This is successful publication, not a final billing-usage statement.

## Current game

First-person, server-authoritative C++ survival prototype with Blueprint presentation. Playable development level: **`/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld`**, a small400×400m World Partition greybox, not a completed production-sized world. `/Engine/Maps/Entry` is the main-menu entry. Earlier maps remain regression fixtures.

Implemented supported systems:

- Health/stamina/hunger/thirst, exposure, authoritative damage, death/respawn, first-person HUD and replicated remote-player body.
- Inventory/pickup/drop/stacking/splitting, finite expiring food and cooking. Stackable items cap999,000; gear1. Overweight progressively slows movement and disables sprint, then jump. Chests retain hard capacity.
- Finite gathering/regrowth, timed crafting/cancellation, home-world tools through Titanium, primitive weapons/protection, six-node technology view, server-owned XP and separate stat/technology budgets. Physical stations, ancient rewards and dimensional/powered technology remain planned.
- Primitive foundations/walls/doors/windows/stairs/floors/ceilings/storage, validated placement/ownership/support/collision, R rotation, chest UI/Quick Store/Sort/item filters.
- Sprint/glide and continuous wall climbing. Movement spends stamina; holding still is neutral; zero stamina drops the player. Upgrades are in inventory. [Current traversal/upgrades](INVENTORY_UPGRADES_WALL_REST.md).
- Four greybox creature definitions, bounded ecology/spawning/combat/death/loot, day/night clock, landmarks/hazards.
- Versioned server-owned player/world save/load/reconnect, ownership/storage/resource/creature/pickup records and original food aging. Pause Save, main-menu New/Load/Refresh/Rename and settings. Main-menu hosted-session browser/join UI is not implemented.
- Original illustrated inventory/crafting/building/storage/HUD/menus,61sprites and eight-slot exact-batch session hotbar. It is not a persistent equipment loadout. [UI](UI_OVERHAUL_M11.md), [hotbar](HOTBAR_M11.md), [playtest guide](PLAYTEST.md).

User-added Fab/model assets are preserved; staged models are not integrated into gameplay. No third-person gameplay or production world expansion. Asset acquisition/integration still needs the relevant provenance/scope checks.

## Material progression

Planned ladder: **Wood → Stone → Copper → Iron → Diamond → Titanium → Deepforge Alloy → Riftglass (Tech)**. Later dimensional sources are required by design; Frontier maps/travel and powered Tech are not implemented.

October15 user authorizes all remaining offline features, starting with resources/technology. The former wait-for-COPPER_LOOP implementation restriction is superseded; that human result remains unverified. [Eryndor extension](https://trello.com/c/zg9mp2Tl) technically passes: nine minerals, Iron/Diamond/Titanium upgrades and four new knowledge branches. Thirteen distinct native selectors pass, including new content/chest/filter save restoration, exact manual/startup migrations and old-writer compatibility. Rendered720p/HUD1 and1.5 tree/craft/harvest pass with inspected PNGs; one-client4/two-client6 impaired NullRHI create/restart reports pass. Editor/Game build; private/settings guards unchanged and0new raw severity. Catalogs36items/21recipes/15definitions/six knowledge nodes; saved map25nodes/97actors, all87 original actor packages unchanged. Dimensional/poweredTech and broader offline roadmap remain incomplete. [Evidence](ERYNDOR_TIERS_M12.md), [offline queue](OFFLINE_IMPLEMENTATION.md).

## Latest completed independent work

Two bounded M18 tasks are pushed with Codex GPT-6.1 Sol coauthor:

| Task | Final verification | Evidence |
| --- | --- | --- |
| M12 Eryndor minerals/technology | Editor12.86s/Game43.27s;13distinct native selectors,2rendered runs,10network-role reports pass. Boundary-seeded rendering is not ordinary earned pacing. | [Eryndor tiers](ERYNDOR_TIERS_M12.md) |
| Migration-backup refusal,4be10db | Editor6.21s/Game14.00s; native4/4. Six manual/startup refusal scenarios preserve live state/active craft/files/evidence; failed startup blocks login/capture/save. No production repair needed. | [Migration refusal](MIGRATION_REFUSAL_M18.md) |
| Frozen historical M8 writer,b5ee144 | Historical source generation1/1; current Editor23.70s/Game29.09s; current native4/4 and expired-food replay1/1. Startup/repeated Load/currentV2 Save-Load conserve owners/items/identities/neutral defaults without renewing food. | [Frozen writer](FROZEN_WRITER_M18.md) |

All final runs above have zero raw/test warnings/errors, no timeout/fatal/ensure and normal exits. Initial historical generation had a missing-directory warning; creating an EMPTY Content directory fixed it, with retained evidence/replay. No historical assets/private saves copied or downloaded. The owned temporary build root was verified before removal.

The immutable [fixture](../Tests/Fixtures/M8_c3d221f/README.md) contains actual historical c3d221f production-writer bytes rebuilt on the installed engine with fake identities. This is not an archived original executable or a multi-engine matrix. Normal tests never regenerate the baseline.

Earlier completed `PF.CheckSave [slot]` is discoverable, server-only and non-Shipping: read-only file/catalog/map/layout/connected-membership inspection with original generation/UTC/counts/recovery/reviewed Copper-migration indication. It explicitly does not guarantee full restoration. Prior native7 distinct checks and actual one-client4/two-client6 NullRHI save/restart role reports passed; these network runs were not repeated for later test-only work. [Inspection evidence](SAVE_INSPECTION_M18.md).

## Milestone and acceptance status

| Scope | Current status |
| --- | --- |
| M0–M6 | Historical supported greybox checkpoints passed; no later production scope implied. |
| M7/M8 | User accepted supported world check and solo Save/Load/chest quantities/filter/vitals. Current automated one/two-client persistence/restart passes; human rendered multiplayer/packaged-server acceptance remains open. Do not repeat accepted solo tests unnecessarily. |
| M9/M10 | Staged-asset source/license mapping and real model integration remain future. Original UI sprites do not complete production art. |
| M11 | Named building/menu/crafting/chest/Pause/Settings and UI_HOTBAR accepted. Unreported UI/device/performance observations remain separate. |
| M12 | Home-world resources/tool ladder through Titanium, six knowledge nodes and earlier combat/stats technically pass. COPPER_LOOP and earned STATS_UPGRADE remain unreported; stations/Deepforge/Riftglass/ancient path incomplete. |
| M13/M14 | Ecology encounters/dawn/loot, world spacing and WALL_CLIMB accepted. Neutral wall rest/inventory upgrades have technical evidence. Expanded worlds/dimensions remain future. Do not reopen accepted climbing. |
| M15 | Voice metadata/policy technical checks pass. Free/low-cost setup reopened; source access verified, source packaging blocked by disk space. Audible voice/provider/cloud/database remain unimplemented. |
| M16/M17 | Wider measured optimization/listening acceptance open. Physical controller/audio untested;1440p traversal explicitly deferred. |
| M18 | **Doing, incomplete.** Inspection, exact Copper edge, refusal tests and one frozen V1 writer fixture verified. Generic migrations/rollback, wider version/catalog/engine coverage and crash/power-loss durability not certified. |
| M19–M25 | Future content/alpha/beta/release plans; no release acceptance implied. |

Trello: bounded [inspection](https://trello.com/c/DcXavHs8), [refusal](https://trello.com/c/cMbhLES7) and [historical writer](https://trello.com/c/M7OVzrdS) are Done; [parent M18](https://trello.com/c/JnS1fkkf) is Doing/incomplete. Personal cards require named user results; unrelated deadlines/membership were preserved.

## Unverified playable observation

**One next action: [COPPER_LOOP — Personal 2](https://trello.com/c/jzDHaiGJ).** Use a normal rendered standalone Single player world on `L_PrimalFrontier_OpenWorld`. Start at **MINING TEST: Wood > Stone > Copper**, a few metres north of camp. Follow the card's G15 earned route: Stone tool → ore → refinement/cord → FieldTools knowledge → Copper pick. No item/XP grants. Report `COPPER_LOOP PASS` or `COPPER_LOOP FAIL` with the first problem.

Vein `(0,-850,60)`, sign `(-230,-910,150)`, camp starts `(-250,-1100,100)` / `(250,-1100,100)`. P pauses while reading in solo. Full controls/costs/timings/quantities are on the card. This ordinary-input pacing/usability observation remains unverified; latest user authorization permits independently tested offline implementation while it waits.

Current infrastructure request is active: source access already works; build storage is the next owner prerequisite for packaging. No credentials or hosting payment needed yet. Copper and other Personal gameplay checks stay separate; already accepted results need no repeat.

## Environment, warnings and limits

- Windows project `C:\UnrealProjects\PrimalFrontier`; targets PrimalFrontierEditor/PrimalFrontier.
- Requested5.8.2; installed/tested **5.8.3 CL58210709**, `C:\Program Files\Epic Games\UE_5.8`. No engine/settings change in these tasks.
- Recorded test host31.93GiB RAM/RTX4060 8GiB VRAM.16GiB minimum certification remains unverified. Use incremental builds/max4 compile actions, bounded maps, one client first and NullRHI when rendering is unnecessary.
- Latest native sampled roughly2.81–3.03GiB working/private per process, not total-system/sustained rendered FPS/stutter evidence. Prior rendered two-client D3D12 crash is not fixed or certified by NullRHI passes.
- Ordinary PrimalFrontier.log still ends in the separate October14,23:00UTC D3D12 device crash; current unique native logs are clean. Prior OpenWorld network startup baseline24warnings/14Python error lines per role had zero new normalized severity in last inspection runs.
- Launcher distribution blocks packaged Server targets; uncooked dedicated-server tests are distinct from packaged/cloud support. No SQL backend/hosting account/voice media service provisioned.
- Latest M18 tasks changed no assets/maps/production codecs/private worlds/profiles/rendering preferences/network behavior. Reports/build products remain generated/untracked; no owned Unreal process left running.

## Continue workflow

Read this page, [MILESTONES](MILESTONES.md), relevant evidence and [TRELLO_SYNC](TRELLO_SYNC.md); inspect live Trello before selecting an eligible bounded task. Keep it Doing, verify before completion, update evidence/Done immediately, make scoped commits with Sol coauthor. Preserve unrelated work and unverified Personal observations. User authorizes offline features and current free/low-cost infrastructure preparation; no paid resource or new service agreement has been accepted. Source fetch/build waits for adequate storage. Update this page after meaningful changes/results instead of appending another chronological journal.
