# Primal Frontier — current state

Updated October 8, 2026. This is the current handoff, not a chronological test log. Update it after meaningful changes and build/test results. Detailed history: [MILESTONES.md](MILESTONES.md) and [preserved October 8 snapshot](CURRENT_STATE_HISTORY_2026-10-08.md).

## Current game

First-person, server-authoritative greybox survival with Health/Stamina/Hunger/Thirst, exposure, damage/death/respawn, finite food with expiry, inventory/pickup/drop, gathering/crafting, primitive building/storage/ownership, passive/hostile creatures and time of day. Pause/settings and controller bindings exist. Remote players retain full-body presentation; no third-person gameplay is implemented.

The current open-world development map is **L_PrimalFrontier_OpenWorld**: a 400 x 400 m World Partition candidate. **L_M7SurvivalArena** and earlier maps remain small regression fixtures. This is not the final production-scale island. Existing user-imported assets are preserved; verified greybox fixtures use primitives/template assets. No new asset is currently required. [PLAYTEST.md](PLAYTEST.md) explains launching, controls, pickup, settings, food and save/restart.

## Milestone status and next action

| Milestone | Current status | Remaining gate |
| --- | --- | --- |
| M0–M6 | Historical supported checkpoints passed | Evidence in MILESTONES.md; do not infer later acceptance |
| M7 — greybox world | Implemented; automation/streaming evidence passed; **manual acceptance unverified** | Sustained walking between zones and overnight survival on the open-world map |
| M8 — persistence/multiplayer | Implemented; bounded native/live checks passed; **manual acceptance unverified** | Rendered gather/craft/build/store/save/close/restart/reconnect; packaged Server setup remains blocked |
| M9 — asset pipeline | Independent inventory/intake audit completed; approval open | Exact local source/version/license mapping before candidate integration |
| M11 — UI/UX | Controls/help slice PASSED Editor/Game + native + rendered720p/1440p | Next independent slice: stable inventory selection after expiry/removal; full UI acceptance open |
| M10, M12–M25 | Future plans; no gameplay/art milestone accepted | Independent slices now authorized; retain asset provenance and technical dependencies |

**Completed independent slice: M11 Controls & help through Pause.** Editor final PASSED6.31 s; Development Game PASSED41.75 s, no compiler warnings. PF.Input.Gamepad and PF.Settings.Preferences PASSED2/2 (Automation_M11Controls_20261008_090734672_36578953;16.07 s; working/private2.920/2.782 GiB). Final rendered PF.UI.ControlsLive PASSED1/1 each at720p/1440p: M11Controls720_20261008_091400974_9c4f8bcf (32.61 s;3.079/4.090 GiB) and M11Controls1440_20261008_091446844_c3fb51c8 (37.27 s;3.166/4.367 GiB). Test warnings/errors0, engine/strict verdict0. Six final screenshots inspected. [CONTROLS_HELP_M11.md](CONTROLS_HELP_M11.md) gives exact evidence/commands. No assets/remapping added. Next independent step: stable inventory selection after expiry/removal. Latest authorization makes Personal playtests nonblocking but UNVERIFIED; no manual pass assumed.

Retained failures: initial C2248 protected accessor build and M11Controls720_20261008_090915891_b2dca088 (three focus assertions, engine0/strict verdict1). Public mapping view/include corrected build; frame boundary after queued SetInputMode corrected the fixture, followed by successful replay. Screenshot review then corrected the fixed pause background. Known startup widget/HLOD warnings and engine Python toolset errors persist in raw rendered logs, separately from clean test reports; no crash/ensure/fatal. Uncapped/VSync0 confirmed; stationary menu tests do not prove sustained gameplay FPS/stutter.

Deferred: on L_PrimalFrontier_OpenWorld, sustained traversal and overnight survival remain unverified. See the [Personal M7 card](https://trello.com/c/bDHOueTw). NullRHI cannot establish human movement/presentation/usability evidence.

Then complete the [Personal M8 rendered persistence loop](https://trello.com/c/UZVTTPLE) using the launch/save/restart instructions in PLAYTEST.md. Physical-controller feel/hot-plugging and audio audibility remain separate hands-on checks. Do not mark these cards Done without the corresponding observations.

The latest user request expands independent continuation to implementation; it supersedes the earlier planning-only boundary. Keep asset rights/compatibility checks and actual build/test failures as technical dependencies. Trello is checked first on every continuation; titles use (AI)/(Personal), active bounded work stays Doing, and completed evidence is updated immediately. [TRELLO_SYNC.md](TRELLO_SYNC.md) governs synchronization.

## Latest verified development checkpoint

Source/test commits **27b7ef6** (offline food) and **7869ccd** (active crafting) were pushed normally with **Codex GPT-6.1 Sol** coauthor attribution. Local and remote master equal 7869ccde48283395b0ad457ca1f8065a229e6e2c, rechecked for this documentation task. Generated reports/logs/saves are not committed. The other chat's untracked TRELLO_BOARD_PLAN.md and TRELLO_SETUP_PROMPT.md remain untouched.

Latest Editor rebuild: **passed, 5.67 s, no compiler warnings**, Saved/Logs/PFM8ActiveCraftEditorBuildFinal.log. Previous food rebuild passed 6.12 s. The latest Development Game build is the earlier privacy checkpoint (14.85 s), not a build of the subsequent test-only additions. No new build/test launch is needed for this documentation-only handoff.

| Focused test | Exact final report under Saved/AutomationReports | Result | Sampled working/private GiB |
| --- | --- | --- | --- |
| PF.Persistence.OfflineFoodAging | Automation_M8FoodFinal_20261008_081451534_924b2032 | 1 passed; 0 failed/test warnings/errors; engine/runner 0 | 2.995 / 2.955 |
| PF.Persistence.ActiveCraftCancellation | Automation_M8CraftFinal_20261008_082409203_d54be484 | 1 passed; 0 failed/test warnings/errors; engine/runner 0 | 2.968 / 2.842 |

Retained report summaries and build logs were rechecked during the handoff. Both final raw logs have zero warning/error/ensure/fatal lines. Food coverage includes actual-file UTC aging in player/storage/pickups, repeated loads and an older player departure timestamp. Craft coverage includes active-job cancellation, no old-deadline output, conserved ingredient batches, fresh single completion, a newer on-disk departure generation and checkpoint reload before reconnect. These are native server/lifecycle tests, not rendered or new separate-process multiplayer acceptance. Initial passing oracle reports were retained. [PERSISTENCE_M8.md](PERSISTENCE_M8.md) has full evidence.

The current intended native selection contains **33 distinct tests**. The latest combined checkpoint ran **30**; dead-player, food-aging and active-craft tests each passed separately afterward. **All 33 have not been rerun together.** Exact older combined evidence: [M8_POST_FIX_VERIFICATION.md](M8_POST_FIX_VERIFICATION.md).

Separate one-client Create/Restart and two-client Create/Restart NullRHI checks passed, including real invalid inventory RPC rejection, owner/foreign inventory/storage privacy and modest simulated lag/loss. Client 2 did not crash in those headless runs. The checked-in [RunPersistenceAutomation.ps1](../Scripts/RunPersistenceAutomation.ps1) repeats the bounded scenario with fresh isolated slots/profiles/reports. [AUTOMATION_VERIFICATION.md](AUTOMATION_VERIFICATION.md) and [M8_NETWORK_DIAGNOSTICS.md](M8_NETWORK_DIAGNOSTICS.md) identify exact reports, retained timeout failures and scope. No fresh multiplayer run was performed for this handoff.

## Save and reconnect contract

Server-only versioned player/world saves use bounded catalog/value records and A/B file generations. PF.SaveWorld/PF.LoadWorld and read-only PF.TestPersistence call real server APIs. Player IDs, attributes/location/inventory, structures/storage/ownership, supported resource/creature/spawner/pickup state and world time persist in the tested scenario. Failed startup/rejected loads preserve prior data; corpse/doorway collision fixes and dead-player respawn are regression-tested.

Save explicitly before quitting. **No timed autosave or automatic standalone/server-exit save exists.** Active-slot client departure checkpoints are tested. Food ages offline using UTC (file precision: one second); other world timers pause. Restoring cancels crafting. Development reconnect uses a private LAN GUID capability/profile, not production accounts. Keep credentials, identity files and world saves private and out of Git/Trello. Unknown versions/catalog/layouts refuse safely; migration is planned, not implemented. Do not overwrite/delete failed-save evidence to force a load.

## Known limitations

- Installed UE reports **5.8.3** although the requested baseline is **5.8.2**. No engine migration was performed.
- Packaged PrimalFrontierServer is blocked: **“Server targets are not currently supported from this engine distribution.”** Uncooked Editor -server passed; packaged-server certification needs a suitable engine distribution.
- Prior New Editor Window PIE ran at about **14–17 FPS** with a D3D12 presentation delay. A stationary Selected Viewport + F11 sample averaged 107.81 FPS at 2560 x 1392, 75% render scale; it does not prove traversal or native 1440p performance. Motion blur defaults off; settings are available in P → Settings. Sustained rendered performance remains unverified.
- Actual host RAM is approximately **31.93 GiB**. The supplied 32 GB/RTX 4060 Ti reference and original 16 GB minimum are separate targets. Recent headless runs peak around 3 GiB per native process and 1.82 GiB per live process; they do not certify rendered FPS, VRAM, stutter or a 16 GB minimum.
- Live startup logs retain known **24 editor-widget/HLOD warnings and 14 installed-engine Python traceback error lines per process**. They are distinct from clean automation events; raw logs must still be reviewed. The historical rendered Client 2 D3D12 crash is not resolved by a NullRHI pass.
- Saved/Logs/PrimalFrontier.log is stale (2026-10-06 22:44:15 UTC). Recent runs use unique PF-prefixed absolute logs listed with their reports; do not mistake the stale default log for current launch evidence.
- Asset provenance, physical controller feel, authored audio, sustained manual play, production authentication/migration and final art remain unverified/deferred.

## Independent planning already completed

M9 intake/metadata audit: [ASSET_PIPELINE_M9.md](ASSET_PIPELINE_M9.md). Registry-only inventory found 7,268 assets, 41.723 GiB disk and 13 unchanged redirectors; it loaded/saved no asset objects. Disk size is not a runtime memory or suitability verdict.

Future plans exist in [UI_CONTROLS_PLAN.md](UI_CONTROLS_PLAN.md), [PROGRESSION_PLAN.md](PROGRESSION_PLAN.md), [ADAPTATION_DIRECTION.md](ADAPTATION_DIRECTION.md), [AUDIO_FEEDBACK_PLAN.md](AUDIO_FEEDBACK_PLAN.md), [NETWORK_PROFILING_READINESS.md](NETWORK_PROFILING_READINESS.md) and [SAVE_COMPATIBILITY_PLAN.md](SAVE_COMPATIBILITY_PLAN.md). These do not complete future milestones or authorize new assets. Original adaptation/technology direction remains planned; refresh research before implementation.

Bounded handoff task complete: [Trello](https://trello.com/c/EG6SOrl5). Retained reports/builds, historical snapshot preservation, local links and whitespace reviewed. No fresh build/test launch, gameplay change or manual acceptance. Scoped documentation commit/push is recorded on the task; the next required gameplay action remains the Personal M7 playtest.
