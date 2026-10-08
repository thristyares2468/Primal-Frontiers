# Primal Frontier milestone evidence

## 2026-10-08 M8 combined post-fix checkpoint

Source 31dce6b: Editor/Game incremental checks up to date, passed 1.13/1.11 s with no compilation actions/warnings. Native gameplay/persistence selection passed 27 and runtime command selection passed 3: all original 26 plus four new persistence cases, 30 distinct passes, no test/raw-log warnings/errors/ensure/fatal. Exact names and reports: [M8_POST_FIX_VERIFICATION.md](M8_POST_FIX_VERIFICATION.md). Initial overbroad selection correctly failed strict verdict on two opt-in editor setup warnings (engine 0/runner 1); retained, no code/warning suppression.

Fresh one-client Create/Restart and two-client Create/Restart passed all ten PF.Persistence.Live process reports, engine/report exits 0, no test warnings/errors or gameplay error/ensure/fatal. Client 2 passed both phases. Two-client working sets create 1.714/1.812/1.814 GiB, restart 1.717/1.796/1.798 GiB. Known live startup logs contain 24 warnings and 14 installed-engine Python traceback lines per process, documented separately. NullRHI has no rendered FPS/stutter proof; packaged Server remains blocked. Trello bounded verification complete; M7/M8 manual gates remain open. No source/assets/settings changed for this checkpoint.

## 2026-10-08 M8 restored doorway and hidden-shape collision

PF.Persistence.StructureCollisionRestore reproduced four blockers after actual file load: the open door panel and hidden foundation cubes regained collision when disabled staging ended. Retained failed report Automation_M8StructureCollision_20261008_050854512_9d16a5ed (engine 0, runner 1). Fixed only PFBuildPiece shape profiles: NoCollision hidden, BlockAll visible/responses. Repeat loads now preserve clear open/reopened actual player-capsule passage, closed-panel blocking, solid door frames, above-platform clearance and ownership.

Editor fixed build 18.50 s and Development Game 25.26 s passed without compiler warnings. Focused replay 1/1; final Automation_M8ShapeRegression_20261008_051403173_bebb7464 passed PF.Building.PlacementAndStorage, PF.Persistence.StructureCollisionRestore and PF.Persistence.WorldRuntime (3/3), zero test/raw-log warnings/errors/ensure/fatal, engine/runner exits 0. Sampled working/private 2.998/2.861 GiB, 15.82 s, NullRHI. Report/index/run-summary and matching PF log under Saved. Initial test-only compile guard failure 2.28 s retained and corrected. No assets/maps/schema/rendering changed. Trello bounded fix Done; M7/M8 manual acceptance remains open.

## 2026-10-08 M11–M13 independent controls/reconnect/research UI planning

UI_CONTROLS_PLAN.md audits fixed survival BindKey versus Enhanced Input movement, native placeholder HUDs/settings and reconnect log-only/profile behavior. Defines read-only controls first, one-context action semantics, accepted/refused server feedback, future remapping/reflow, sanitized save/reconnect failures and original research/adaptation views tied to future C++ backends. Source/link/whitespace consistency only; no widget/input/settings/source/asset/save changes or new build/test/performance claim. Bounded AI plan Done, full M11–M13 remain planned; M7/M8 manual and physical-device/provenance gates remain open.

## 2026-10-08 M8 corpse collision and loot restoration fix

PF.Persistence.CorpseLootRestore found a real staged-activation bug: all four restored corpses had collision enabled, although loot uniqueness/deadlines held. Initial Automation_M8CorpseLoot_20261008_045111161_0e920d38: 0 passed/1 failed, four errors/zero warnings; engine exit 0, verdict runner exit 1. Retained failure; fixed only dead RestorePersistence to apply built-in NoCollision profile instead of an owner-effective SetCollisionEnabled call. No loot-generation/schema/assets change.

Editor fixed build passed 22.13 s, final test-assertion build 5.99 s, Development Game 30.15 s; no compiler warnings. Focused fixed retry 1/1. Final Automation_M8CorpseRegression_20261008_045603207_88c1206e: PF.Persistence.CorpseLootRestore and PF.Persistence.WorldRuntime both Success, 0 errors/warnings, engine/runner exits 0 and clean raw log. Dead collision off, live collision retained, stable actor/drop IDs, no renewed food life and no loot replay after prior drop removal; unique files cleaned. 16.05 s, sampled working/private 2.933/2.776 GiB. Evidence: Saved/AutomationReports/<name>/index.json and run-summary.json; Saved/Logs/PF<name>.log; PFM8CorpseLootEditorBuild{Fixed,Final}.log; PFM8CorpseLootGameBuild.log. Trello bounded fix Done; M7/M8 manual and packaged Server gates still open. No new multiplayer/rendered/FPS pass claimed.

## 2026-10-08 M18 independent compatibility planning

SAVE_COMPATIBILITY_PLAN.md checks current file/world/player/profile contracts and defines deliberate version edges, catalog/layout/ownership/missing-data rules, separate verified backups before A/B rotation, offline-food time anchors and synthetic fixture/native/live/manual gates. Document/source consistency only; no migration/schema/runtime/save/asset/settings change or new build/test/performance claim. Latest gameplay evidence is the focused M8 rejected-load test, not M18 acceptance. Bounded AI plan task Done; full M18 To Do, preceding manual/provenance gates open.

## 2026-10-08 M8 rejected-load preservation verification

Added PF.Persistence.RejectedLoadPreservesWorld: four real checksummed files (unsupported version, wrong map, resource-layout mismatch, unsafe player ground) are refused without changing running player/pawn/location/health, bag quantity/batch, structure/storage/owner/support, resource/clock, active slot or active generation/payload. Valid saving afterward still succeeds; unique disposable files cleaned. Runtime source/assets/schema unchanged.

First Editor build failed C2665 (TObjectPtr/raw-pointer TestEqual, line 321, exit 6, 2.78 s); fixed only that assertion, Editor rebuild passed 6.96 s with no warnings. Logs PFM8RejectedLoadEditorBuild{,Retry}.log preserve both. Focused Automation_M8RejectedLoad_20261008_044007915_1b40fffb: 1 Success/0 failed/0 test warnings, engine/runner exits 0, raw log zero warnings/errors/ensure/fatal. 16.82 s, sampled working/private 2.979/2.835 GiB. Report/summary Saved/AutomationReports/<name>/index.json and run-summary.json; log Saved/Logs/PF<name>.log. Bounded Trello test task Done; full M7/M8 manual and packaged Server gates remain open. No repeated live tests or rendered performance claim.

## 2026-10-08 M8 native automation verdict tooling

Added Scripts/RunNativeAutomation.ps1 with unique report/log/summary output, bounded owned-process timeout and sampled memory. Reject failed/missing/empty/incomplete/warned reports regardless of Unreal exit 0; reject unsafe inputs and a second Editor. Retained real failure/success inspection returned runner exits 1/0. Fresh Automation_M8ReportGate_20261008_042918633_adb3d3f7 passed StartupFailurePreservesSave 1/1, engine/runner exits 0, no test/raw-log warnings/errors/ensure/fatal; 2.999/2.856 GiB working/private, 18.16 s.

Intentional unmatched selection Automation_M8EmptyReportGate_20261008_043016202_b207d390: no index.json, Unreal exit 0, runner correctly failed with exit 1; one expected no-tests-match log error; 2.964/2.886 GiB, 16.27 s. Parser, null-count/duplicate-ID/nonzero-exit rejection, missing/outside-report/unsafe-filter checks passed. See AUTOMATION_VERIFICATION.md for invocation and limitations. No C++ changed or repeated build claimed. Bounded Trello tooling task Done; manual M7/M8 gates remain open.

## 2026-10-08 M8 startup restoration save-safety fix

Source review and PF.Persistence.StartupFailurePreservesSave reproduced a real gap: after unsupported payload or authored-layout startup failure, login refused but capture/save published a newer empty/default-world generation. Initial report M8StartupFailureReproduced_1791432857773: 0 passed/1 failed, 12 assertion errors, engine exit 0. Preserved failure evidence; disposable generated test slots only.

Added a five-line bStartupBlocked capture guard (Save calls Capture), with an explicit diagnose/restart refusal. No save version, asset/map or new gameplay change. Editor builds passed: test-first 20.52 s, fixed 19.16 s; Development Game passed 28.52 s. No compiler warnings. Focused retry M8StartupFailureVerified_1791432964361 passed 1/1; M8StartupGuardRegression_1791433050190 passed all eight: CorruptPlayerData, FileGenerations, PlayerRoundTrip, PlayerValidation, ServerPlayerAdapter, StartupFailurePreservesSave, WorldRecords, WorldRuntime under PF.Persistence. Zero test errors/warnings; exits 0.

Normal PF.Persistence.Live one-client create/restart passed on both server and client (four reports): M8GuardCreate_1791433114412Server/Client; M8GuardRestart_1791433114412Server/Client. Used L_PrimalFrontier_OpenWorld, same port/identity profile across restart, unique AutomationM8 slot, -nullrhi, -PFExpectedPlayers=1 and existing opt-in automation/TestExit arguments. This is automated gameplay/authority/restart evidence, not human walking/overnight/rendered play. Live working memory create 1.713/1.811 GiB, restart 1.717/1.790 GiB; focused native working/private 2.958/2.825 GiB. No FPS/stutter sample. Known installed-engine ToolsetRegistry PythonTestRunner startup error remains in live logs; test events are clean. Packaged Server remains blocked by the engine distribution.

Evidence: Saved/AutomationReports/<name>/index.json and Saved/Logs/PF<name>.log; Saved/Logs/PFM8StartupGuardGameBuild.log. Changed files: Source/PrimalFrontier/Persistence/PFWorldPersistence.cpp, Tests/PFWorldPersistenceTests.cpp and CURRENT_STATE/PERSISTENCE_M8/MILESTONES/DECISIONS/TRELLO_SYNC docs. Trello bounded AI task Done; M7/M8 parent manual acceptance remains open.

## 2026-10-08 M9/M17 independent audio feedback planning

Completed AUDIO_FEEDBACK_PLAN.md: minimal action/UI/creature/footstep/ambience coverage, server outcome versus local presentation, missing-cue safety, duplicate/replay refusal, dedicated-server guards, concurrency/range/memory targets, source/rights intake, visual fallbacks and future tests. Reviewed the real crafting/building/interaction/vitals/creature/settings routes and Epic's current 5.8 concurrency/attenuation documentation. Clarified PROGRESSION_PLAN.md against source: current exposure has no heat/cold category, and jump rather than sprint has the existing stamina hook.

Document link/whitespace consistency checks only. No sound/source/assets/settings modified; no new Editor build, automation, audibility playtest or runtime performance measurement is claimed. The bounded audio-plan task is Done, not M17 acceptance. M7/M8 manual and asset provenance gates remain open; latest gameplay test evidence is unchanged. Changed docs: AUDIO_FEEDBACK_PLAN.md, AUDIO_COVERAGE.md, PROGRESSION_PLAN.md and state/milestone/decision/roadmap/Trello records.

## 2026-10-08 M12/M13 independent progression planning

Completed the bounded Trello planning task in PROGRESSION_PLAN.md, with XP sources/caps, a provisional ten-level economy (2,700 XP, 27 earned points; 20 points across six proposed unlocks), prerequisites, respec/co-op/discovery policy, separate adaptation/loadout rules and authority/save migration/test requirements. Existing portable food processing/basic construction remains baseline knowledge. The proposal is original and data-driven; no new gameplay, catalog assets, dependencies or save-format changes. Corrected the food document's outdated offline policy to match M8.

Verification is document/source consistency against current C++ catalog defaults and M8 save records, not automated gameplay or a playtest. No C++ changed, so no new Editor build is required or claimed. Latest gameplay evidence remains M8FinalRegression and the recorded live reports. M12/M13 implementation and acceptance remain To Do; M7/M8 manual gates remain open. Changed docs: PROGRESSION_PLAN.md, TECH_TREE_DIRECTION.md, ADAPTATION_DIRECTION.md, FOOD_AND_PRESERVATION.md and the state/milestone/decision/roadmap/Trello records.

## 2026-10-08 M9 independent asset-pipeline planning and read-only audit

The user authorized continuing eligible independent tasks without further prompts. M7/M8 manual acceptance remains pending; this is a planning/audit checkpoint, **not a full M9 acceptance or first-art gameplay pass**. Added Scripts/AuditExistingAssets.py using supported Unreal Asset Registry APIs, with unique generated output; it never loads/saves asset objects. No import, asset move/rename, redirector fix, world change, rendering setting or gameplay source changed.

Isolated UnrealEditor-Cmd Python commandlet with -nullrhi -unattended -NoSaveConfig exited **0**. Generated report explicitly says InventoryCompleted: **7,268 registry assets, 7,270 package files, 44,799,493,793 disk bytes (41.723 GiB), 13 redirectors, zero registry packages without matching files**. Git status confirms no binary asset/map change. Log has no warning/error/ensure/fatal lines. Working/private process memory **1.820/1.720 GiB**; no rendered FPS, stutter, runtime residency or VRAM measurement. Texture Dimensions tags identify 1,014 at 4096 and 94 at 8192 maximum dimensions; no textures were changed.

Evidence: Saved/AutomationReports/M9AssetInventory_20261008T030703Z_8a96b8c959134bbdb6908726df977477/inventory.json and summary.md; Saved/Logs/PFM9AssetInventory.log. No C++ changed in this increment, so no repeated build/gameplay automation is claimed or required for the docs/read-only script. The M8 clean-working-set builds and 26-test/live reports remain the most recent gameplay verification.

ASSET_PIPELINE_M9.md records bounded intake/provenance, naming, folder/material/texture/LOD/collision rules, reversible wrappers/references, redirector review, measurement and Git/LFS policy. ASSET_SOURCES.md now clearly identifies its old claims as an unverified shortlist. The possible Modular Rural Cabins source was checked on Fab; local pack version/acquisition/exact license and compatibility remain unconfirmed. No candidate is approved merely because it is free. M9 remains Doing with a Personal source/approval dependency; first-art integration also waits for M7/M8 manual acceptance.

The independent audio preference/coverage task is Done: native local volume/mix routes exist and PF.Settings.Preferences passed in M8FinalRegression; registry has three project SoundClasses/one SoundMix and only one template weapon SoundWave. Authored survival sound coverage and audibility are unverified. AUDIO_COVERAGE.md records exact evidence/limits. Trello was updated immediately after both task completions. No sound source/asset/settings changed.

Changed files for this checkpoint: Scripts/AuditExistingAssets.py; Docs/ASSET_PIPELINE_M9.md, AUDIO_COVERAGE.md, ASSET_SOURCES.md, CURRENT_STATE.md, MILESTONES.md, ROADMAP_STATUS.md, DECISIONS.md, GIT_WORKFLOW.md, PRIMAL_AGENT_TOOLS.md and TRELLO_SYNC.md. The legacy Trello setup docs from the other chat remain untracked/preserved.

## 2026-10-08 M8 world persistence and reconnect verification

**M8 implementation and automated verification advanced; milestone acceptance is still pending.** M7's sustained manual open-world route/overnight result and M8's rendered gather/craft/build/storage/save/close/restart/reconnect playtest remain unverified. The user authorized independent M8 work while deferring M7. No M9 work starts from this checkpoint.

Implemented bounded V1 whole-world records, server-only save/load, startup restoration and development reconnect identities. Saves include player vitals/location/inventory, structure health/support/ownership/storage/door state, authored resource depletion/respawn, creature identities/health/home/corpse timers, spawner residents and pickups, plus world time. Food ages offline; expired batches are omitted; repeated restoration replaces inventories. A failed player restoration cannot overwrite the saved player with an empty default spawn. Login/restore uses a deferred PostLogin step; disconnect captures the pawn before destruction. Commands PF.SaveWorld [slot], PF.LoadWorld [slot] and read-only PF.TestPersistence now call real APIs and reject clients. No map/assets were saved or changed.

Builds: PrimalFrontierEditor **passed (6.33 s)**; PrimalFrontier Development **passed (69.65 s)**, no compiler warnings. Installed engine reports **5.8.3**, not the originally requested 5.8.2. PrimalFrontierServer was **blocked before compilation (0.73 s)**: "Server targets are not currently supported from this engine distribution." The existing Server target is retained; uncooked Editor -server is the tested dedicated-server route. A post-commit working-set build is still pending.

Final regression: **26 passed, 0 failed, 0 test warnings**, engine exit 0. Report: Saved/AutomationReports/M8FinalRegression/index.json; log: Saved/Logs/PFM8FinalRegression.log. Exact Success tests:

- PF.Building.PlacementAndStorage
- PF.Crafting.Gathering
- PF.Crafting.Transactions
- PF.Creatures.Lifecycle
- PF.Input.Gamepad
- PF.Interaction.TargetAndPickup
- PF.Inventory.Transactions
- PF.Inventory.WorldTransfers
- PF.Persistence.CorruptPlayerData
- PF.Persistence.FileGenerations
- PF.Persistence.PlayerRoundTrip
- PF.Persistence.PlayerValidation
- PF.Persistence.ServerPlayerAdapter
- PF.Persistence.WorldRecords
- PF.Persistence.WorldRuntime
- PF.PrimalAgentTools.CommandArguments
- PF.PrimalAgentTools.MissingSystemsAreBlocked
- PF.PrimalAgentTools.TeleportAndRuntimeReset
- PF.Settings.Preferences
- PF.Survival.Component
- PF.Survival.Environment
- PF.Survival.Lifecycle
- PF.Survival.Needs
- PF.World.Clock
- PF.World.OpenWorldAsset
- PF.World.Resources

Focused prerequisites also passed: M8WorldFixture (WorldRecords and WorldRuntime: 2); M8WorldHookRegression (12); M8DisconnectVerified (WorldRuntime: 1). Actual one-client Create/Restart produced four passing PF.Persistence.Live reports: M8CreateOneVerifiedServer, M8CreateOneVerifiedClient, M8RestartOneVerifiedServer, M8RestartOneVerifiedClient. Actual two-client Create/Restart produced six passing PF.Persistence.Live reports: M8CreateTwoVerifiedServer, M8CreateTwoVerifiedClient1, M8CreateTwoVerifiedClient2, M8RestartTwoVerifiedServer, M8RestartTwoVerifiedClient1, M8RestartTwoVerifiedClient2. All ten have zero failures/test warnings and engine exits 0, under Saved/AutomationReports/<name>/index.json; matching logs are Saved/Logs/PF<name>.log.

Live tests use L_PrimalFrontier_OpenWorld, separate uncooked dedicated server and one/two clients with NullRHI. They perform real gathering, timed tool craft, foundation/storage placement, file save/load, client authority refusals and replicated owner/vitals/items/time. A separate server restart restores the crafted tool, health, storage, ownership and a fibre marker changed after the manual save and captured on disconnect. Fixture teleports and time overrides are **not manual traversal/overnight evidence**.

Preserved failures: first world fixture build lacked the pickup deadline argument (fixed; two tests then passed). Original M8RestartOneServer failed five assertions; client failed with timeout/network aftermath (2 errors/39 warnings). Credential RPC ran before the engine attached the client connection; moving it to deferred PostLogin and capturing departure before pawn destruction fixed the fresh verified runs. Initial M8CreateTwoServer/Client1/Client2 each timed out at a test prerequisite: the test filtered stone nodes to a region containing only one, but required two. Removed that test-only filter; no map change; rebuilt and all six fresh create/restart reports passed. Older failed reports remain evidence. Engine exit 0 alone is not a test verdict.

Final regression sampled working/private memory **2.896/2.769 GiB**. Two-client create sampled server/client1/client2 working sets **1.717/1.808/1.816 GiB**, restart **1.722/1.796/1.795 GiB**. NullRHI has no rendered FPS or stutter measurement; it does not resolve the earlier New Editor Window PIE 14–17 FPS complaint. This host reports about 31.93 GiB installed RAM; the 16 GB target remains a budget, not a measured minimum-spec pass. Known uncooked HLOD, engine Toolsets Python/editor-widget startup and teardown warnings remain outside test events; final regression log has no warning/error/ensure/fatal lines.

Changed areas: Persistence/PFWorldSaveData, PFWorldSaveFormat, PFWorldPersistence, PFLocalPlayer; player-save adapter; GameMode/controller lifecycle; resource/creature/spawner/pickup snapshot helpers; built-in Json/JsonUtilities module dependencies; DefaultEngine.ini LocalPlayer class; plugin adapters/live tests and native world tests; documentation. No unrelated Unreal assets, external art, rendering settings, new gameplay system or MCP server changed.

Remaining: clean-working-set build and scoped commit/push; then M7 manual route/overnight and M8 manual persistence playtest. Production account authentication, globally rollback-atomic restoration, save migration, cooked streaming/HLOD and packaged-server certification are outside this verified development scenario. See PERSISTENCE_M8.md and PLAYTEST.md.

Post-commit c3d221f clean-working-set Editor build passed in **20.76 s** and Development Game in **31.45 s**, no compiler warnings; both game and plugin compiled as unity translation units. Trello audit read back all 53 cards, corrected stale status/roadmap detail and added native M7/M8 evidence gates. Task titles use (Personal)/(AI); manual gates stay open. The user's later authorization permits independent planning tasks beyond this checkpoint, not fabricated manual passes or gated gameplay/art expansion.

## Historical checkpoints

# Milestone 7 — in progress 2026-09-30

## 2026-10-08 current-state documentation and M8 ownership preparation

Maintain [CURRENT_STATE.md](CURRENT_STATE.md) after every meaningful change and build/test outcome, per the user's instruction. It distinguishes completed work, passed evidence, local integration work, outstanding gates and the next development step. Requested adaptations and optional active/passive biological modifiers are documented as future M12/M13 planning only.

The M8 stable structure-owner keys/rebinding compiled. A unity-build C4459 name collision was fixed without changing the player-save format. `M8CurrentStateRegression/index.json`: **12 passed, 1 failed, 0 test warnings**. The building fixture's simulated handover changed only Builder and left the persistent owner key unchanged; updated the fixture and added replacement-PlayerState and wrong-identity checks. Final Editor build **16.26 s**, no compiler warnings. Focused `M8BuildingIdentityVerified/index.json`: **PF.Building.PlacementAndStorage passed, 0 failures/test warnings**, exit 0. Matching final log has no warning/error/ensure/fatal lines. Exact initial passes and paths: CURRENT_STATE.md / PERSISTENCE_M8.md. Only the failed test was rerun after the fixture fix.

Retry sampled working/private memory **2.940/2.801 GiB**, NullRHI; initial monitor's Int32 overflow invalidated its peak and is documented. No new manual or live-network persistence pass is claimed. World-codec/restoration/reconnect source is still being integrated; save/load commands remain unavailable. The large LFS push completed and remote master at `24bace4` plus archive tag were verified; this does not push remaining uncommitted source. M7 and M8 remain incomplete.

## 2026-10-08 M8 server adapter/file checkpoint

The user authorized continuing through remaining milestones until an immediate user action is required. Continue M8 incrementally while retaining the deferred M7 manual gate. Added server-only player capture/restoration and public stable ownership ID; validate identity, capacity and safe capsule/ground location before replacement. Food ages by elapsed offline seconds; repeated restoration does not duplicate items. Added bounded A/B file generations with pending-write verification, active-save preservation and explicit corrupt-generation recovery/refusal. Whole-world persistence, reconnect and PF save/load commands remain pending.

Compiler failure: DOREPLIFETIME's required OutLifetimeProps name, fixed. Editor rebuild **18.19 s**, final fixture rebuild **5.70 s**, no compiler warnings. `M8PlayerRuntime/index.json`: eight Success test states (seven clean, adapter had native missing-mesh socket warnings); after matching the known meshless fixture warning, `M8PlayerAdapterVerified/index.json` records **1 passed, 0 failed, 0 test warnings**, engine exit 0. Exact tests/logs and limits in `PERSISTENCE_M8.md`. Peak initial process working/private **2.949/2.806 GiB**. No asset/settings edit or Computer Use; no manual/restart/multiplayer persistence pass claimed yet.

## 2026-10-08 M8 preparation with M7 manual gate deferred

The user explicitly reported that the M7 route/overnight playtest has not passed and asked to work around it. **M7 remains unverified and incomplete.** This supersedes the earlier strict sequencing for a bounded, data-only M8 foundation; it does not certify gameplay or authorize final art/world expansion.

Current increment: added a versioned native C++ player-record codec, trusted catalog/capacity validation, bounded corrupt-data rejection and in-memory round-trip tests. No save-file I/O, runtime capture/restoration, authenticated reconnect identity, structure/world persistence, autosave or gameplay change. PF.SaveWorld, PF.LoadWorld and PF.TestPersistence remain explicitly unavailable. See `PERSISTENCE_M8.md` for the format and limitations.

- `PrimalFrontierEditor` built successfully in **12.53 s**; test-fixture correction rebuilt successfully in **5.58 s**, no compiler warnings. Installed engine reports **5.8.3**, not the requested 5.8.2; no engine change was made.
- Initial run `Saved/Logs/PFM8PlayerFormat.log`: **PF.Inventory.Transactions, PF.Inventory.WorldTransfers, PF.Persistence.CorruptPlayerData and PF.Persistence.PlayerRoundTrip each completed Success**. PF.Persistence.PlayerValidation then crashed (process exit 3) at the new fixture's `TArray::Add` alias check because it added an element from the same container. Corrected only the fixture to copy first, and guarded a failed-output assertion against secondary out-of-bounds access. No codec/gameplay fix was needed.
- Final-source run `Saved/AutomationReports/M8PlayerFormatVerified/index.json`: **PF.Persistence.CorruptPlayerData, PF.Persistence.PlayerRoundTrip and PF.Persistence.PlayerValidation all Success; 3 passed, 0 failed, 0 test warnings, exit 0**. Matching `Saved/Logs/PFM8PlayerFormatVerified.log` has no warning/error/ensure/fatal lines. Existing inventory results above are from the initial run, not a claim that its complete queue passed. The older `PrimalFrontier.log` remains an October 7 normal shutdown, not this run's evidence.
- Final peak sampled working set/private memory: **2.991/2.848 GiB**. NullRHI data tests do not measure rendered FPS, traversal or stuttering. No manual/multiplayer save/restart test is possible yet because live persistence is absent. No Computer Use, asset or rendering-setting edits. Changed files: `Persistence/PFPlayerSaveFormat.h/.cpp`, `Tests/PFPlayerSaveFormatTests.cpp`, this document, `PERSISTENCE_M8.md`, `ARCHITECTURE.md`, `DECISIONS.md` and `ROADMAP_STATUS.md`.

Remaining M8 progression: server capture/restore and atomic file replacement/backups; stable player/structure identity; world/storage/resource records; restart/reconnect and duplication tests; then the full playable persistence gate. Each increment needs its own verification. The deferred M7 manual route/overnight result is still required before claiming the complete greybox loop passes.

## 2026-10-08 isolated runtime streaming checkpoint

**M7 still awaits the user's sustained route/overnight confirmation; M8 has not started.** Added only an opt-in automated streaming probe and documentation. No gameplay implementation, map, imported asset or project setting changed in this checkpoint. No Computer Use was needed.

- New `PF.World.Streaming` runs in an isolated Standalone Editor binary on `L_PrimalFrontier_OpenWorld`, with `-PFRunWorldStreamingTests`; NullRHI is supported. It harvests a real wood node through server validation, freezes movement for a deliberate distant test probe, waits for all four landmark actors to unload, then returns and waits for their actual reload. It checks active-world identity/counts of all 15 resources, unchanged harvested hit state, the same unique clock remaining at night, and camp ground collision. Player movement/location/look, needs rates and clock tuning are restored after the test. It saves no assets/maps. This is a streaming/state regression, not manual travel or overnight evidence, and not a new console command.
- Editor build passed in **33.62 s**; strengthened active-world identity assertions rebuilt in **8.62 s**, no compiler warnings. `Saved/AutomationReports/M7StreamingProbe/index.json` and final-source `M7StreamingVerified/index.json` each report **PF.World.Streaming: 1 passed, 0 failed, 0 test warnings**, exit status 0. Final log: `Saved/Logs/PFM7StreamingVerified.log`. No C++ changes followed the verified run.
- Sampled final peak working set/private memory: **1.79/1.76 GiB**. NullRHI does not measure rendered FPS or stuttering. Earlier 19-test regression and actual one-/two-client integration results are separate evidence below; this new Standalone probe does not replace them.
- Log investigation: existing experimental Toolsets Python errors and TEDS warnings remain. The uncooked `-game` launch also warns that converter-created HLOD builder settings cannot initially load `/Script/WorldPartitionHLODUtilities`/builder classes. These warnings also occur in the prior `PFM7OpenPolicyClient1.log`; its editor module loads after the warnings. Engine HLOD builder settings are editor-only data. This is consistent with uncooked editor-module load order; cooked HLOD generation/loading is not certified. No new gameplay error, ensure or RHI crash occurred in the probe. The older `PrimalFrontier.log` is not this run's evidence.

## 2026-10-08 settings and World Partition checkpoint

**M7 remains in progress; M8 has not started.** Settings and the open-world candidate are implemented. The New Editor Window PIE presentation bottleneck and sustained manual route/overnight gate remain open.

- Four settings tabs cover game, graphics, audio and accessibility. Apply/Cancel use an isolated draft; unconfirmed display changes revert after 15 real-time seconds. Rendered checks observed all tabs, draft cancellation, applied HUD scaling, defaults and timeout rollback. Physical gamepad feel and authored audio audibility remain unverified. Motion blur/depth of field default off; Medium quality/High textures/75% render scale are initial defaults. Gameplay authority is unchanged.
- Fixed Editor scalability persistence: retain the game profile instead of the separate Editor viewport profile. Final Editor build: **29.68 s, succeeded, no compiler warnings**. `Saved/AutomationReports/M7SettingsPersistence/index.json`: **PF.Settings.Preferences passed**, no failures/test warnings. `PFM7PIEWaitRetry.log` confirms FOV=90, blur=0, renderScale=75 at startup and PIE BeginPlay.
- `Saved/AutomationReports/M7SettingsRegression/index.json`: **19 passed, 0 failed, 0 test warnings**, TestExit status 0. Exact tests: PF.Building.PlacementAndStorage; PF.Crafting.Gathering; PF.Crafting.Transactions; PF.Creatures.Lifecycle; PF.Input.Gamepad; PF.Interaction.TargetAndPickup; PF.Inventory.Transactions; PF.Inventory.WorldTransfers; PF.PrimalAgentTools.CommandArguments; PF.PrimalAgentTools.MissingSystemsAreBlocked; PF.PrimalAgentTools.TeleportAndRuntimeReset; PF.Settings.Preferences; PF.Survival.Component; PF.Survival.Environment; PF.Survival.Lifecycle; PF.Survival.Needs; PF.World.Clock; PF.World.OpenWorldAsset; PF.World.Resources. Matching log: `Saved/Logs/PFM7SettingsRegression.log`.
- Created **400 x 400 m** `L_PrimalFrontier_OpenWorld` independently through Editor Python and the native WorldPartitionConvertCommandlet. Runtime streaming is enabled; existing survival camp plus eight distant primitive landmarks. `ConfigureOpenWorldMilestone7.py` keeps 68 core actors loaded, preventing clock/resource/spawner reset on travel before M8. Save/reload verification and map check passed in `PFM7OpenWorldPolicy.log`; the commandlet's existing CrowdManager/Recast teardown warning remains. No existing template map or imported art was changed.
- After the streaming policy change, `M7OpenPolicyServer1/index.json` and `M7OpenPolicyClient1/index.json` each passed **PF.World.Live**, zero failures/test warnings and exit status 0. Then `M7OpenPolicyTwoServer/index.json`, `M7OpenPolicyTwoClient1/index.json` and `M7OpenPolicyTwoClient2/index.json` each passed the same test with two actual connected clients, zero failures/test warnings and exit status 0. Sampled working set/private memory: server **1.72/1.62 GiB**, clients **1.80/1.77** and **1.81/1.77 GiB**. These are NullRHI connections, not rendered performance certification. Matching logs are `Saved/Logs/PF<report-name>.log`.
- New Editor Window PIE still shows **15-16 external FPS**. Transient Slate throttling/editor VSync checks did not resolve it. `Saved/Profiling/PFM7PIEWaitRetry.utrace`, exported by Unreal Insights to `PFM7PIETimers.csv`, interval 42-61 seconds: 142 frames averaging **133.802 ms**, render synchronization **122.795 ms**, and 284 D3D12 presentations averaging **64.284 ms** each. World tick averages **0.840 ms** across both worlds. Inclusive scopes overlap; do not add them. Presentation dominates the measured delay; a particular driver/overlay cause is not proven.
- **Verified workaround: Selected Viewport PIE, then F11 for immersive view.** Same arena and persisted Medium/High-texture/75% preferences, blur and DoF off, VSync off and unlimited FPS. `Saved/Profiling/CSV/Profile(20261008_073547).csv`: **1,200 stationary unpaused frames**, mean **9.2758 ms / 107.81 effective FPS**, p95 **12.0916 ms**, p99 **13.6713 ms**, maximum **29.4932 ms**, no frame above 33.33 ms. Native read-only viewport query confirms **2560x1392** (the CSV's stored SystemResolution is not the actual PIE viewport). External observations were approximately 88-120 FPS. This launch-mode comparison is not a fix for New Editor Window, a controlled same-size GPU benchmark or a traversal certification. Sampled process **4.54 GiB working set / 6.90 GiB private**. `Saved/Logs/PFM7SelectedViewport.log` confirms settings, capture, viewport dimensions and normal shutdown; existing CrowdManager teardown and outstanding telemetry-request shutdown warnings remain, with no new gameplay error.
- The longer Selected Viewport trace is `Saved/Profiling/PFM7SelectedViewport.utrace` (about 2.6 GB). Insights closed normally (exit 0; `Normal Slate Window Closed`) before creating the requested timer export and logged memory-tag/metadata analysis errors during teardown in `PFM7SelectedInsights.log`. Treat that export as unavailable; the independent native CSV above is the performance evidence. Avoid long traces on the minimum-memory machine.
- First tracing launch failed before PIE: `Saved/Logs/PFM7PIEWait.log`, DXGI_ERROR_DEVICE_REMOVED at D3D12Viewport.cpp:537, reason **0x887A002B / DXGI_ERROR_ACCESS_DENIED**, EndDrawingViewport breadcrumbs. Local video memory: 1129 MB used / 7188 MB budget. Identical retry reached PIE and exited normally. No gameplay-code crash or VRAM exhaustion is established. Project RHI settings were not changed; the intermittent failure is unresolved.
- Earlier standalone stationary sample `Saved/Profiling/CSV/Profile(20261007_140420).csv`: 1200 frames, 2560x1440 output/75% scale, mean **5.9505 ms / 168.05 effective FPS**, p95 **7.5029 ms**, max **14.4905 ms**. Different launch mode/map: this does **not** prove the user's PIE slowdown is fixed or certify traversal. Trace PIE memory: **4.56 GiB working set / 6.50 GiB private**, system overlay about 21.7 GB used on the observed 32 GB machine. Actual adapter remains RTX 4060; see TEST_MACHINE.md.
- Reconciled the GitHub Desktop stash against its original 12-file delta and the merged branch. Most work was already recovered; restored missing world notes only. Archived original stash as `archive/m7-stash-20261008` before dropping it. Did not overwrite later work. User asset imports were committed unchanged/LFS-tracked separately as **349de07**. New footer: `Co-authored-by: Codex GPT-6.1 Sol <codex@openai.com>`. Push completion is separate from local commit success.

## 2026-10-07 settings, performance and open-world follow-up — in progress

Before M8, add local game/audio/graphics/accessibility preferences, a pause-menu settings screen, motion blur off by default, safe display confirmation and automated settings coverage. Compare uncapped performance at the same resolution; the earlier 960x540 result is not a native-resolution comparison. Then create a separate World Partition open-world map using supported Unreal tools, retaining L_M7SurvivalArena as a regression fixture. Verify loading, traversal and the existing survival loop before changing the recommended playtest map. No M8 persistence or external-art work is included.

Initial large-window PIE observation reproduced 16 FPS on the external overlay. Engine CSV `Saved/Profiling/CSV/Profile(20261007_084047).csv` contains 600 frames, averaging about 124.49 ms/frame with 113.27 ms in game-thread event waits. This is a timing/wait clue, not proof of a GPU bottleneck or a completed optimization. Initial settings compilation found C4458 (local `Slot` shadows UWidget::Slot); renamed it and the rebuild succeeded. Further settings and rendered verification remain pending.

## 2026-10-06 merged-source world extension

The full supplied roadmap is archived in [FULL_PROJECT_ROADMAP.md](FULL_PROJECT_ROADMAP.md), with current execution scope and the open-world requirement in [ROADMAP_STATUS.md](ROADMAP_STATUS.md). M7 remains in progress; M8 has not started. The current map is a small continuous integration world, not the final open-world scale or a streaming test.

The user/Claude merge at `6d3ee12` committed the imported packs and source documentation/refactors, but removed the in-progress M7 extension from the working tree. Recovered only the missing extension script/test from local commit `bc18287` and reapplied the small additions onto the merged source. Preserved centralized catalog paths, named request codes, the capacity fix and pickup display names. Older pre-merge passes do not certify this combined state.

- Merged baseline Editor build passed in 32.09 s. Extension build passed in **15.44 s**, no compiler warnings, installed **UE 5.8.3 CL 58210709**. No engine version change was performed.
- `Scripts/ExtendWorldMilestone7.py` saved only the M7 map through supported Unreal APIs, adding primitive woodland/rocks/roofed ruin/colour-marked beach and water edges, six nodes (15 total, five resource kinds), readable zone labels and rebuilt navigation. Existing simple materials were reused; user-imported art and template assets were untouched. The script refuses an already-extended world. Catalog entries add fibre and finite water portions; inventory hints now say eat/drink.
- `Saved/AutomationReports/M7MergedResources/index.json`: **3 passed, 0 failed, 0 test warnings**: PF.World.Resources, PF.Crafting.Gathering, PF.Inventory.Transactions. Water collection/depletion, authority, exact one-portion consumption, thirst-only recovery and refusal at full thirst are covered.
- `M7MergedServer1/index.json` and `M7MergedClient1/index.json`: **PF.World.Live passed once in each process**, zero test warnings/failures. Then `M7MergedTwoServer/index.json`, `M7MergedTwoClient1/index.json`, `M7MergedTwoClient2/index.json`: **one pass each**, zero test warnings/failures. These are actual NullRHI connections. Added navigation destinations for woodland, rocks and ruin; both owners receive fibre/water; an invalid consume quantity leaves water unchanged; the valid drink RPC spends one portion and restores thirst from 10 to 45. Gathering setup uses server APIs; drinking uses actual client RPCs. No rendered multiplayer certification is implied.
- Two-client sampled working set/private memory: server **1.70/1.60 GiB**, clients **1.76/1.71 GiB each**. NullRHI does not measure rendered FPS/stutter. Matching named logs are under `Saved/Logs/PF<report-name>.log`. Map-generation evidence is `PFM7MergedExtension.log`.
- Final merged-source `M7MergedRegression/index.json`: **17 passed, 0 failed, 0 test warnings**. Exact tests: PF.Building.PlacementAndStorage; PF.Crafting.Gathering; PF.Crafting.Transactions; PF.Creatures.Lifecycle; PF.Input.Gamepad; PF.Interaction.TargetAndPickup; PF.Inventory.Transactions; PF.Inventory.WorldTransfers; PF.PrimalAgentTools.CommandArguments; PF.PrimalAgentTools.MissingSystemsAreBlocked; PF.PrimalAgentTools.TeleportAndRuntimeReset; PF.Survival.Component; PF.Survival.Environment; PF.Survival.Lifecycle; PF.Survival.Needs; PF.World.Clock; PF.World.Resources. No subsequent C++ or asset changes.
- Rendered `Saved/Logs/PFM7MergedManual.log`: normal E gathered two water portions at 04:56:42 UTC; Tab displayed water x2; Q sent an accepted action=2/quantity=1 at 04:56:52, leaving water x1 and visibly increasing thirst. E gathered fibre at 04:57:42; bag then showed water x1/fibre x2 and weight 0.7 kg. These checks used BugItGo positioning followed by walk, plus PF.SetThirst 30 to isolate recovery; no item grants. The new eat/drink hints fit. Inspected initial beach/woodland/rock visuals and the roofed ruin; its initially dark exposure settled visibly. Normal window close at 04:59:16 UTC; no new gameplay error, ensure or RHI crash.
- Inspected engine screenshots `Saved/Screenshots/WindowsEditor/ScreenShot00015.png` (water node/woodland) and `ScreenShot00016.png` (settled ruin interior). Shot omits HUD, so these are geometry evidence; inventory counts were observed in Computer Use snapshots and supported by transaction logs. Command export `Saved/AutomationReports/PF_M7MergedManual_20261006T045713_E784A0354A094A93C347F0A39C41D3F6.json` records the thirst setup/export, not a full gameplay verdict.
- Rendered 960x540, t.MaxFPS=0, r.VSync=0, launch-only smoothing disabled: informal warm observations about **146-192 FPS**, with screenshot capture hitches (one observed update about 93 FPS). Sampled process **3.58 GiB working set / 5.54 GiB private**. No new controlled CSV sample, native 1440p, full travel or 16 GB minimum-spec certification. User overlays/background apps remained running.
- Existing experimental engine Toolsets Python startup errors (`ToolsetDefinition`/`PythonTestRunner`) remain in game/server launches; the map commandlet has the known CrowdManager/Recast teardown warning. Do not count these as new gameplay failures or describe the complete logs as warning-free.

Manual full-route/overnight survival and physical controller feel remain separate gates. Do not proceed to M8 based on these partial results.

## 2026-10-06 verification checkpoint

Implemented the small integrated arena, server clock, pause menu, pickup alignment/feedback and controller bindings. **M7 is not declared complete:** the sustained manual walking route between zones remains unverified. Computer Use can send short movement presses but cannot reliably hold movement through the route. Point-to-point manual checks below used explicit fixture teleports, and automated navigation paths do not replace that remaining manual check. Do not begin M8 until this gate is resolved. Physical gamepad delivery/feel also remains unverified; only mappings/action paths are tested.

- Final `PrimalFrontierEditor Win64 Development -gather` build succeeded, up to date in **1.22 s**, no compiler warnings. Installed engine remains **5.8.3 CL 58210709**. No engine update was performed.
- Current-source `Saved/AutomationReports/M7InputRegression/index.json`: **16 successes, zero failures/warnings**. Exact tests: PF.Building.PlacementAndStorage; PF.Crafting.Gathering; PF.Crafting.Transactions; PF.Creatures.Lifecycle; PF.Input.Gamepad; PF.Interaction.TargetAndPickup; PF.Inventory.Transactions; PF.Inventory.WorldTransfers; PF.PrimalAgentTools.CommandArguments; PF.PrimalAgentTools.MissingSystemsAreBlocked; PF.PrimalAgentTools.TeleportAndRuntimeReset; PF.Survival.Component; PF.Survival.Environment; PF.Survival.Lifecycle; PF.Survival.Needs; PF.World.Clock. No C++ changes followed this regression.
- `M7InputTwoServer/index.json`, `M7InputTwoClient1/index.json`, `M7InputTwoClient2/index.json`: **PF.World.Live passed once in each process**, zero failures/warnings, including local menu open/resume without pausing multiplayer. Prior one-client world and pickup RPC passes are recorded below. These are actual NullRHI network runs, not rendered multiplayer performance tests.
- Rendered `Saved/Logs/PFM7LoopManual.log`: actual E input gathered six wood/two stone, timed tool craft completed at 22:43:39 UTC, and a normal placement click created the foundation at (-800,0,10) at 22:45:04. Bag showed wood x1/tool x1 afterwards. No item grants were used. BugItGo positioned the player near fixtures, followed by walk to restore collision. One placement click shifted the view because of mouse capture and was correctly refused; realigning the view allowed placement.
- In that same session, the exposure HUD rose to 60% inside the hazard and health fell; after exiting, exposure returned to zero and health stopped falling from exposure. Needs/health were restored using PF commands to isolate these tests. The prowler visibly chased/attacked and killed the player; the log confirms automatic respawn at (250,-1100,98.15), Health=100/Stamina=100, at 22:47:50 UTC. This session does not prove a manual creature kill; that was separately verified in M6. The session exited normally at 22:51:31 UTC.
- Rendered `Saved/Logs/PFM7DayNightManual.log`: PF.SetTimeOfDay 22 and 9 both passed; inspected night and restored daytime lighting. Auto-exposure makes the settled brightness difference subtle; the greybox intentionally has no sky art. Replayed the final pause layout: the entire footer and single-line Confirm end session label fit, first click stayed in the menu, second click exited normally at 23:48:11 UTC. Solo pause was logged with worldPaused=1. All times here are UTC on October 5 (local October 6).
- Inspected `Saved/Screenshots/WindowsEditor/ScreenShot00014.png`: settled night view, visible resources, ground and survival HUD. `ScreenShot00013.png` confirms wood x1/tool x1 but contains teleport motion blur; it is not a clean foundation/environment screenshot. Prior pickup evidence is ScreenShot00012.png. UI snapshots also verified the final confirmation menu.
- Controlled stationary daytime sample from the safe start, one rendered client at **960x540**, HUD/stats enabled, t.MaxFPS=0, r.VSync=0, launch-only smoothing disabled: `Saved/Profiling/CSV/Profile(20261006_094644).csv`, **6,000 frames**, all retained. Mean **5.7728 ms / 173.23 effective FPS**, median **5.7366 ms**, p95 **6.7835 ms**, p99 **7.6053 ms**, maximum **26.8474 ms**, zero frames over 33.33 ms. Physical memory **3499.86–3518.24 MiB**, virtual-used peak **5576.75 MiB**; external process sample **3.43 GiB working set / 5.43 GiB private**. No pauses, teleports or engine screenshots during capture. Background apps and external overlays remained open. This short sample does not establish native 1440p, minimum-spec, travel or large-world performance.
- No new gameplay error, ensure or RHI crash was found in these passing runs. Existing experimental Toolsets Python startup errors and TEDS warnings remain; CrowdManager/Recast warning occurs during normal teardown. The hazard's own floating label can be viewed from its back and is hard to read from the southern approach; its zone marker and HUD exposure still work. `Saved/Logs/PrimalFrontier.log` now belongs to the user's later Editor session; current test evidence uses the named PFM7 logs.
- User added untracked Adventures_Pack, Bike, DynamicFalling, Modular_Rural_Cabin and Polyphoria content (about 5.92 GiB, including associated Bike external actors/objects). These are unrelated to the M7 code/map checkpoint and are preserved, unstaged and unused by this arena. Future asset sourcing is authorized when a concrete need exists; no new asset acquisition is required for this verification.

Use [PLAYTEST.md](PLAYTEST.md) for opening the correct map, keyboard/controller controls, pickup troubleshooting, recipes, building, food expiry and a self-test checklist. Generated evidence stays local/untracked. Commit this as an M7 verification checkpoint, without claiming the remaining manual route gate passed.

## 2026-10-04 pause, pickup and controller checkpoint

The user requested pause/pickup fixes and controller support before further expansion. Added a local UMG pause menu (P/Esc/Menu, Resume, confirmed End session), Standalone-only simulation pause, contextual gamepad survival controls and a complete `PLAYTEST.md` guide. The existing Enhanced Input context supplies left/right sticks and A jump; template input/assets were not edited. Inventory/craft/build overlays are now mutually exclusive. Camera origin matches the server eye; pickup/resource prompts and validation share a bounded trace with small-target tolerance and obstruction checks. E prioritizes a nearby item/resource even while building preview is open. No M8 persistence has started.

- Initial focused `M7InteractionCore/index.json`: Gathering, Inventory.WorldTransfers and World.Clock passed; Interaction.TargetAndPickup failed three assertions because its transient controller was not marked local. Fixed that fixture, not authority rules; `M7InteractionRetry/index.json` then passed 1/1, zero warnings. `M7InputCore/index.json` passed 2/2 (`PF.Input.Gamepad`, `PF.Interaction.TargetAndPickup`), zero warnings. Gamepad test loads actual template mappings and exercises bound pickup, split, drop and mutually exclusive overlays; it does not simulate physical device/OS delivery.
- Editor builds passed: fixture correction 14.71 s, gamepad/pause additions 24.40 s, pause confirmation layout fix 6.35 s, no compiler warnings. `-gather` was needed to discover newly added C++ tests in the UBT makefile.
- `M7PauseServer/index.json` and `M7PauseClient/index.json`: PF.World.Live passed 1/1 each, zero warnings, including local menu open/resume without pausing the multiplayer world. `M7PickupServer/index.json` and `M7PickupClient/index.json`: PF.Inventory.Live passed 1/1 each, zero warnings; real client split/drop/pickup RPCs conserve quantity/deadline, reject invalid requests, expire food and retain non-food items through death/respawn.
- Rendered `Saved/Logs/PFM7PausePickupManual.log`: observed P menu, stable solo needs for about 30 seconds, mouse Resume and successful jump after resuming. Used BugItGo only to position near the existing wood node (walk restored); actual E gathered two wood, G dropped one, B enabled preview and E picked up the drop. Bag returned to exactly wood x2. End session required two clicks and exited normally. The first confirmation label wrapped poorly; corrected auto-wrap/panel height and rebuilt, with visual replay still required. Physical controller play remains unverified.
- Inspected `Saved/Screenshots/WindowsEditor/ScreenShot00012.png`: wood x2, pickup feedback, interaction prompt and first-person HUD. Command export: `Saved/AutomationReports/PF_M7PausePickupManual_20261004T080522_1844A7C94AFCC05F1499708B2DE21962.json`. This export is command evidence, not a replacement for manual observations.
- Rendered 960x540 with t.MaxFPS=0, r.VSync=0 and launch-only smoothing disabled: sampled 3.44-3.56 GiB working set / 5.52 GiB private; observed roughly 165-190 FPS during warm interaction, with a capture hitch. This is an informal observation, not a controlled M7 CSV benchmark or native-1440p certification. Multiplayer pickup server/client working sets were 1.69/1.79 GiB under NullRHI.
- Known experimental engine Toolsets Python startup errors persist; normal rendered exit also logged the existing CrowdManager/Recast teardown warning. No RHI crash or gameplay ensure occurred. `Saved/Logs/PrimalFrontier.log` is older M5 evidence; current runs use the explicit PFM7 logs above.

The complete M7 rendered travel/craft/build/danger/day-night gate and controlled performance sample remain pending; do not call M7 complete or begin M8 based only on this input checkpoint. The earlier map orientation fix used named Python Rotator arguments and saved only L_M7SurvivalArena. `M7FinalServer` and `M7FinalClient` passed after that repair, including horizontal PlayerStart orientation assertions. The light-priority fix removed competing directional-light warnings in the subsequent rendered session.

Checkpoint: final clock/live-test Editor build passed in 5.79 s; the preceding 14.01 s build also compiled the committed M6 code in unity after its tag-symbol fix. `M7ClockCore/index.json` has 4 successes and `M7Regression/index.json` has **14 successes, zero failures/warnings** (the 13 M6 regression tests plus `PF.World.Clock`). `PFM7Setup.log` confirms only the new M7 map was saved; the known navigation-manager warning occurs during commandlet cleanup, not live testing.

First `M7Server1/index.json` failed gather/craft/build assertions because the harness invoked a local-input helper on a dedicated-server controller. The first client was stopped without a completed verdict after that server exited. The harness now calls the same authoritative node Gather API with a freshly derived view; gameplay validation is unchanged. `M7Server1Retry/index.json` and `M7Client1Retry/index.json` each passed `PF.World.Live`, followed by `M7TwoServer/index.json`, `M7TwoClient1/index.json`, `M7TwoClient2/index.json`, all **one success, zero failures/warnings per process**. Matching logs use `Saved/Logs/PF<report-name>.log`. Tests cover connected paths through zones and onto the rise, map exposure entry/exit, six wood/two stone gathered without grants, timed tool crafting, foundation placement from remaining resources, replicated night/day and building observations, and rejection of client clock changes. These server-driven integration checks supplement, rather than replace, earlier client RPC tests and the pending manual gate. Sampled two-client working set/private memory: server 1.69/1.55 GiB, clients 1.76/1.68 and 1.78/1.69 GiB. NullRHI does not measure rendering.

Start from verified/pushed M6 `0c2d935`. Create only a new small `L_M7SurvivalArena` with safe starting ground, distributed resource nodes, a dangerous creature area, an exposure hazard and primitive height variation. Reuse verified gameplay and catalogs. Add one replicated server world clock with a low-cost moving sun/night fill and a real PF.SetTimeOfDay adapter. No World Partition is needed for this bounded arena. Gate: compile, focused clock and map/network tests, one client before two, rendered travel/gather/craft/build/danger/day-night play, uncapped sample, logs and documentation. Do not begin M8 until that gate passes.

# Milestone 6 — verified 2026-09-30

**Gate passed for the bounded greybox creature scenario.** Editor build, 13-test regression, one- and two-client live navigation/combat/loot tests, rendered first-person observation and controlled kill/loot playtest passed. Installed UE_5.8 now reports **5.8.3, CL 58210709** in Build.version and generated reports; these final results are not a certification of 5.8.2. No engine update was performed by this work. Automatic progression to M7 is authorized.

User authorized automatic progression through M8, retaining a separate compile/automation/manual/network gate for each milestone and no progression past a failure. Begin M6 from clean `5702d4b`. Add two original primitive creature definitions, capped spawn points, server navigation and sight/range perception, idle/patrol/flee/chase/attack/death states, finite loot and first-person attack. Use a separate small M6 map. Verify data/spawn/authority/damage/loot tests, actual navigation and one-/two-client replication, then uncapped rendered play before committing M6 and proceeding to M7. Keep the technology tree and final art deferred. Updated machine reference is `TEST_MACHINE.md`.

M6 implementation and automated evidence:

- Baseline Editor build passed in 72.45 s after engine build-version invalidation. First M6 compile failed C3535 on TObjectPtr deduction in the developer adapter; fixed with an explicit APawn pointer, rebuilt in 6.47 s. Live-test link initially failed LNK2019 for NavigationSystem symbols; added the direct built-in module dependency, then rebuilt successfully. Final live-harness build passed in 6.75 s, no compiler warnings.
- `Saved/AutomationReports/M6Core/index.json`: 3 passed, zero failures/warnings (`PF.Creatures.Lifecycle`, `PF.PrimalAgentTools.CommandArguments`, `PF.PrimalAgentTools.MissingSystemsAreBlocked`). Creature lifecycle tests cover data validation, living-player perception, sight obstruction, flee/chase decisions, damage authority, one-time death loot with expiry and the eight-creature cap.
- Initial `PFM6Setup.log` failed because the batch commandlet retained the navigation async-loading lock. First retry unlocked it after synchronous asset completion but had not processed queued bounds. Added a navigation tick before the explicit build; `Saved/Logs/PFM6SetupFinal.log` confirms successful navigation generation and only the new M6 catalog/map saved. Its CrowdManager warning occurs during commandlet cleanup after the successful save; no matching warning occurs in the live server/client runs. Retry explicitly resumes only the newly created incomplete M6 destinations and refuses duplicate M6 fixtures.
- First network client report `M6Client1/index.json` failed its hostile-movement assertion while `M6Server1/index.json` passed: the client recorded its baseline after the creature had already approached. Fixed the harness to use the known server fixture origin and reject unrelated pre-fixture actors. Gameplay is unchanged. `M6Server1Verified/index.json` and `M6Client1Verified/index.json` then each passed `PF.Creatures.Live`, zero failures/warnings. Two-client `M6TwoServer/index.json`, `M6TwoClient1/index.json`, `M6TwoClient2/index.json` also each passed, zero failures/warnings. Matching logs use `Saved/Logs/PF<report-name>.log`.
- Live coverage: a complete navigation path detours around the primitive obstacle; passive and hostile physically move; hostile windup deals server damage; the attacked client kills it through the owning attack RPC and recovers exactly three food through the interaction RPC; both clients observe movement/death. Direct client damage/spawn and the client developer spawn command are rejected. Server integrity and per-process reports export successfully; intentional authority probes remain in command history.
- `Saved/AutomationReports/M6Regression/index.json`: **13 passed, 0 failed/warnings**, `PF.Creatures.Lifecycle` plus all 12 M5 regression tests listed below. `Saved/Logs/PFM6Regression.log` records 13 tests and TestExit status 0 at 2026-09-29 04:08:43 UTC. No new gameplay errors/ensures/RHI failures in successful runs. Existing engine experimental Toolsets Python startup errors remain in game/server runs.
- Two-client sampled working set/private bytes: server **1.68/1.57 GiB**, clients **1.78/1.74 GiB** and **1.78/1.75 GiB**; free physical memory **12,376,516 KiB** of **33,477,984 KiB**. These NullRHI runs do not measure rendering performance.

## M6 final rendered gate and limits

- Computer Use observed forager flee/idle/patrol and hostile approach/attack, server health loss, death and respawn at normal game speed in `L_M6Creatures`. In `Saved/Logs/PFM6Manual.log`, the controlled kill replay temporarily used `slomo 0.2` to accommodate inspection between inputs: tool clicks dealt 35, 35 and 30 at 09:06:42, 09:06:49 and 09:06:57 UTC; E recovered three food with a live expiry counter. Restored `slomo 1` before export/exit. All network combat tests ran at normal speed. The final normal-speed manual attempt landed one hit before the observer's input delays allowed the player to die; it is not counted as a successful normal-speed manual kill.
- Close-range labels obscured the early rendered encounter. Fixed only their distance-scaled display size, rebuilt Editor successfully in **16.56 s**, reran `PF.Creatures.Lifecycle`: `Saved/AutomationReports/M6Final/index.json`, **1 passed, zero failures/warnings**, TestExit status 0 at 09:09:38 UTC. Replayed the rendered encounter in `PFM6ManualFinal.log`; close and distant labels are readable. No gameplay authority changed after the successful network runs.
- Inspected `Saved/Screenshots/WindowsEditor/ScreenShot00010.png` (tool, food x3 with 286 s freshness, Health 84) and `ScreenShot00011.png` (final forager label/state, primitive arena, first-person tool/HUD). `ScreenShot00009.png` contains a blurred teleport transition and is not the final visual evidence. `PF.Help`, `PF.TestCreatureAI`, spawn/reset and export passed. Reports: `Saved/AutomationReports/PF_M6Manual_20260929T090810_42E8BAFC4C82605F4C58F5B779F25690.json` and `PF_M6ManualFinal_20260929T091504_16866F2E42296165BDDFF7AB2093EA1A.json`. Final session exited normally at 10:29:30 UTC; no Unreal test process remains.
- Uncapped, normal-speed 960x540 D3D12 sample: `Saved/Profiling/CSV/Profile(20260929_191504).csv`, **6,000 frames**, stationary viewing one forager after reset. Engine queries confirm t.MaxFPS=0, r.VSync=0; frame smoothing disabled for this launch. Mean **6.2051 ms / 161.16 effective FPS**, median **6.0102 ms**, p95 **7.8937 ms**, p99 **9.0303 ms**, maximum **451.5793 ms**; one frame exceeded 33.33 ms during capture start. External frame limiters were not changed or verified. This is a short warm greybox sample, not a 1440p/full-world benchmark.
- Final rendered process sampled **3.46 GiB working set / 5.40 GiB private**, with **14,139,800 KiB** free of **33,477,984 KiB** physical memory. CSV physical used memory spans **3,527.72–3,545.09 MiB**, virtual used **5,513.21–5,527.14 MiB**. Initial cold M6 startup compiled existing engine shaders and stuttered; warm replay remained usable. Runtime D3D12 reports RTX 4060 with 7,956 MB dedicated memory, differing from the user-supplied 4060 Ti 16 GB reference. See `TEST_MACHINE.md`.
- Known engine Toolsets Python startup errors and TEDS registration warnings remain. No new gameplay errors, ensures or RHI crashes in passing final runs. Packaged Server/Shipping, advanced dynamic navigation, large creature counts and rendered two-client performance remain unverified.
- Changed: eight creature/navigation/spawner/catalog C++ files; creature lifecycle/live tests; attack controller and HUD; gameplay/plugin module dependencies and PF command adapters; new setup script, creature catalog and M6 map; architecture, decisions, milestone, tooling and machine docs. Existing Content/template assets are unchanged. Generated evidence stays local/untracked. Next is the small M7 survival arena; art integration stays deferred, and the user will be notified before using their assets.

# Milestone 5 — verified 2026-09-29

Authorized after M4 from clean `e7ffd71`. Baseline Editor build passed in 27.07 seconds. Implement only greybox building: data-defined foundation/wall/floor/ceiling/door/storage, grid preview and rotation, authoritative placement/cost/collision/support validation, ownership, health/demolition and transactional storage. Use a separate small M5 map and preserve existing assets. Compile and run focused automation, then regression, manual shelter/storage play and one-client before two-client replication. For rendered tests use session-only `t.MaxFPS 0`, VSync off and frame smoothing disabled, recording frame time/FPS and memory without increasing graphics quality. Update evidence and decisions; stop before creatures/M6.

**Gate passed for the supported greybox building scenario.** Editor compilation, 12-test regression, final three-test rerun, rendered first-person shelter/storage playtest, and real one-/two-client NullRHI replication passed. Stop before M6. Packaged Server/Shipping builds, persistence and rendered two-client performance are not claimed.

## M5 changes and verification

- Added `Source/PrimalFrontier/Building/PFBuildingCatalog`, `PFBuildPiece`, `PFBuildingComponent` and `PFBuildingHUD` header/implementation pairs. Added atomic `PFInventoryComponent::TransferTo`, controller input/RPC integration and the HUD build hint. Six data-defined primitive pieces, grid/rotation preview, server-derived placement, collision/support/cost/ownership checks, door interaction, damage/demolition and private storage are described in `BUILDING_M5.md`.
- Added `Source/PrimalFrontier/Tests/PFBuildingTests.cpp`, plugin `Private/Tests/PFBuildingLiveTests.cpp`, real `PF.TestBuildingPlacement` and guarded `PF.ResetBuildings`. Added `Scripts/SetupBuildingMilestone5.py`; supported Editor Python created only new `Content/PrimalFrontier/Building/DA_BuildingCatalog.uasset` and `Content/PrimalFrontier/Maps/L_M5Building.umap`. Setup evidence: `Saved/Logs/PFM5Setup.log`. Existing binary/template assets are unchanged. Architecture, decisions, plugin documentation and test-machine reference are updated.
- Intermediate compilation failed C3535 on `auto*` deduction from TObjectPtr and C4458 on a local Instigator name; corrected explicit pointer access and naming, then rebuilt successfully. The final C++ build succeeded in **5.94 s**, without compiler warnings, after a visual preview/HUD adjustment. UBT evidence: `C:/Users/jackh/AppData/Local/UnrealBuildTool/Log.txt` (rotating log).
- `Saved/AutomationReports/M5Core/index.json`: **1 passed, 0 failed/warnings**, `PF.Building.PlacementAndStorage`. `M5Regression/index.json`: **12 passed, 0 failed/warnings**. Exact tests: `PF.Building.PlacementAndStorage`, `PF.Crafting.Gathering`, `PF.Crafting.Transactions`, `PF.Inventory.Transactions`, `PF.Inventory.WorldTransfers`, `PF.PrimalAgentTools.CommandArguments`, `PF.PrimalAgentTools.MissingSystemsAreBlocked`, `PF.PrimalAgentTools.TeleportAndRuntimeReset`, `PF.Survival.Component`, `PF.Survival.Environment`, `PF.Survival.Lifecycle`, `PF.Survival.Needs`. Logs: `Saved/Logs/PFM5Core.log`, `PFM5Regression.log`.
- The first live attempt (`PFM5Server1.log`, `PFM5Client1.log`) stalled because its transfer immediately followed storage-open and hit the intentional RPC cooldown. It was stopped without an acceptance result. Added a 0.7 s harness wait; gameplay throttling is unchanged. Rebuilt successfully and replayed.
- `PF.Building.Live` then passed once in **each of five processes**, all with zero failures/warnings: `M5Server1Verified/index.json`, `M5Client1Verified/index.json`, followed by `M5TwoServer/index.json`, `M5TwoClient1/index.json`, `M5TwoClient2/index.json` under `Saved/AutomationReports`. Matching logs are `Saved/Logs/PF<report-name>.log`. Real clients used owned RPCs to place foundation/wall/ceiling/storage, checked replicated support/owner/cost, deposited food preserving its exact deadline, rejected occupied demolition, and observed damage to 75 health. Both clients observed the other player's structures while storage contents remained private. Direct client mutation, unknown definitions and client reset were rejected. PF live client exports intentionally contain NOT AUTHORITY results; their command-history aggregates are not the automation verdict.
- Final binary rerun on September 29: `Saved/AutomationReports/M5Final/index.json` reports **3 passed, 0 failed/warnings/not-run**: `PF.Building.PlacementAndStorage`, `PF.Inventory.Transactions`, `PF.Inventory.WorldTransfers`. This invocation wrote to **`Saved/Logs/PrimalFrontier.log`**, not the requested custom log name; the log records three tests and TestExit status 0 at **2026-09-29 03:45:36 UTC**. No warning/error lines were found in that final log. No C++ changes followed this run.

## M5 rendered playtest and performance

- Computer Use controlled one rendered first-person `L_M5Building` session at 960x540. Developer fixture setup supplied wood/food and positioned the player; actual B/N/T/click/E/U/O/H/J controls performed the building interactions. Placed a foundation, three rotated walls, a floor used as a roof, a door and storage. Duplicate/unsupported placement showed invalid preview/refusal. Opened the door, deposited and withdrew wood, observed occupied-storage demolition refusal, damaged empty storage, demolished it and rebuilt it. Ceiling placement and unauthorized ownership actions were separately exercised by automation/network tests. Controls remained responsive; this is a small shelter test, not a large construction stress test.
- `Saved/Logs/PFM5ManualVerified.log` records placement, accepted door/storage interactions, successful deposit/withdraw, rejected occupied demolition, owner damage, demolition and rebuild. `PF.TestBuildingPlacement` passed over **7 structures**, `PF.Help` passed and report export passed. Export: `Saved/AutomationReports/PF_M5Manual_20260928T221751_B0FA584B467FC7257F4009A99422F6DC.json`. Opened and visually inspected `Saved/Screenshots/WindowsEditor/ScreenShot00008.png`: visible door opening/storage, building overlay, needs HUD and FPS/unit stats. Health 42 reflects needs drain during the extended test; hunger/thirst were reset before capture. The session exited normally at 22:29:46 UTC. An earlier overnight `PFM5Manual.log` session was incomplete and is not counted as acceptance evidence.
- Console queries confirm **t.MaxFPS=0** and **r.VSync=0**; startup disabled frame smoothing only for this session. Engine-generated `Saved/Profiling/CSV/Profile(20260929_081751).csv` contains **6,000 frames**, stationary facing the seven-piece shelter with build HUD/preview and stats enabled. All frames retained: mean **6.0123 ms / 166.33 effective FPS**, median **5.7462 ms**, p95 **7.5312 ms**, p99 **8.8820 ms**, maximum **439.7173 ms**. Exactly one frame exceeded 33.33 ms, the first captured frame immediately after the screenshot/capture start; that association is not a root-cause diagnosis. Remaining live observations were generally about 146–197 FPS. This short sample does not establish native 1440p or open-world performance.
- CSV physical memory spans **3,467.95–3,583.34 MiB** (~3.39–3.50 GiB); virtual-used peak **5,506.54 MiB** (~5.38 GiB). External process sample was **3.50 GiB working set / 5.38 GiB private bytes**. Two-client NullRHI samples were server **1.63/1.48 GiB**, clients **1.71/1.63** and **1.71/1.62 GiB** working set/private. Windows reports **33,477,984 KiB visible physical memory (~31.9 GiB)**. User's updated 32 GB hardware reference is in `TEST_MACHINE.md`; observed GPU adapter name differs from the supplied 4060 Ti model. Earlier 16 GB assumptions are superseded for this machine, but 16 GB minimum-spec performance is unverified.
- Known existing game/server startup diagnostics remain: experimental Toolsets Python missing `ToolsetDefinition`/`PythonTestRunner`, TEDS widget registration warnings, rendered `r.MotionVectorSimulation` thread-safety warning and occasional Unreal Trace Server 0x000020b7 warning in network clients. No new gameplay errors, ensures or RHI crashes were found in the passing M5 logs. Unavailable non-Windows SDK notices from final startup do not affect the Win64 tests.
- Limits: cooperative owner-only damage/access; no group permissions/PvP tuning, save/reconnect restoration or preservation equipment. `PF.ResetBuildings` client rejection is network-tested; its successful populated-world reset branch has not been manually exercised. Generated evidence stays local/untracked. Recommended next milestone is the separately authorized M6 primitive creatures; a distinct 1440p benchmark can establish native-display performance before visual expansion.

# Milestone 4 — verified 2026-09-28

**Gate passed for the supported greybox gathering/crafting loop.** Editor build, 11-test regression, focused final tests, rendered first-person playtest, and real one-/two-client NullRHI authority/replication runs pass. Stop after M4; do not begin M5 automatically. Packaged Server/Shipping builds and rendered two-client performance are not verified.

Authorized after M3's completed gate, starting from clean `fe30fb6`. Baseline Editor build passed, up to date in 1.61 s. Implement only a small gathering/crafting loop: data-driven primitive wood/stone/food nodes with depletion and respawn; first-person server traces; one primitive gathering tool; editable tool/cook/dry recipes with duration and cancellation; atomic inventory conversion and stale-ingredient rejection; placeholder keyboard crafting UI and real PF validation hooks. Use `L_M4Gathering`, preserve template assets and keep the technology tree deferred.

Gate plan: compile Editor after each meaningful C++ slice; run focused transaction/gathering/crafting tests plus regression; create only the project-owned M4 fixtures through supported Editor APIs; rendered solo first-person gathering/tool/cooking/cancel playtest; one-client then two-client NullRHI authority/replication checks; inspect logs/reports/screenshots and record memory/stutter; update architecture, decisions and evidence. Stop after M4; do not begin building or M5.

## M4 implementation and evidence

- Added `Source/PrimalFrontier/Crafting/PFCraftingCatalog`, `PFCraftingComponent`, `PFResourceNode` and `PFCraftingHUD` (.h/.cpp pairs): editable resource/recipe data, finite primitive nodes, server-derived camera interaction, a timed single-job queue, cancellation and placeholder UI. Added atomic inventory conversion, tool/cooked/dried item definitions, owned controller RPCs, replicated primitive tool presentation and a native aim marker. Detailed contract: `GATHERING_CRAFTING_M4.md`.
- Added `Tests/PFCraftingTests.cpp` and plugin `Private/Tests/PFCraftingLiveTests.cpp`; updated `PFCommands.cpp` with real `PF.TestGathering`, `PF.TestCrafting`, `PF.Craft` and `PF.CancelCraft` hooks. Updated architecture, decisions, food and plugin documentation. `Scripts/SetupGatheringMilestone4.py` creates only `Content/PrimalFrontier/Crafting/DA_CraftingCatalog.uasset` and `Content/PrimalFrontier/Maps/L_M4Gathering.umap` through supported Editor APIs. No existing Content asset or template asset changed; no external art or new plugin dependency.
- Editor builds passed after each slice. One intermediate C4458 failure (local `Slot` hiding `UWidget::Slot`) was fixed by renaming the local to `Placement`; corrected build passed in 6.12 s. Native aim-marker compile passed in 6.35 s. Final `PrimalFrontierEditor Win64 Development` build on September 28 succeeded, up to date in 2.03 s, with no compiler warnings. UBT log: `C:/Users/jackh/AppData/Local/UnrealBuildTool/Log.txt` (rotating file).
- `Saved/AutomationReports/M4Core/index.json`: 2 passed, 0 failed/warnings. `M4Regression/index.json`: 11 passed, 0 failed/warnings. Exact regression tests: `PF.Crafting.Gathering`, `PF.Crafting.Transactions`, `PF.Inventory.Transactions`, `PF.Inventory.WorldTransfers`, `PF.PrimalAgentTools.CommandArguments`, `PF.PrimalAgentTools.MissingSystemsAreBlocked`, `PF.PrimalAgentTools.TeleportAndRuntimeReset`, `PF.Survival.Component`, `PF.Survival.Environment`, `PF.Survival.Lifecycle`, `PF.Survival.Needs`. Logs: `Saved/Logs/PFM4Core.log`, `PFM4Regression.log`.
- Final binary focused rerun: `Saved/AutomationReports/M4Final/index.json`, `PF.Crafting.Gathering` and `PF.Crafting.Transactions`, 2 passed, 0 failed/warnings. `Saved/Logs/PFM4Final.log` records two tests performed and `RequestExitWithStatus(1, 0, ...)` at 2026-09-28 08:48:04 UTC. Coverage includes capacity, insufficient/moved/expired ingredients, cancellation, duplicate completion, unknown recipes, forged deadlines, death cancellation, resource depletion/respawn, range/aim and authority rejection.
- Real network test `PF.Crafting.Live` passed once in each process: `Saved/AutomationReports/M4Server1/index.json`, `M4Client1/index.json`, then `M4TwoServer/index.json`, `M4TwoClient1/index.json`, `M4TwoClient2/index.json`; every report has 1 success, 0 failed/warnings. Logs use matching `Saved/Logs/PF<report-name>.log` names. Each client gathered through its owned RPC, crafted a tool, depleted and observed respawn of its node, cancelled/restarted cooking, and verified output quantities. Direct client mutations and unknown recipe requests were rejected. The server checked resulting inventories and PF integrity hooks. The live harness has a completed stage before disconnect-sensitive lookups and allows server linger for client shutdown.
- Network command exports: `Saved/AutomationReports/PF_M4LiveClient_20260927T063444_*.json`, `PF_M4LiveServer_20260927T063444_*.json`, and corresponding `20260927T063614` client/server files. Client command histories include intentional NOT AUTHORITY failures; their aggregate status is not the automation verdict.
- Rendered manual first-person session `Saved/Logs/PFM4Manual.log`: insufficient-resource recipe refusal; hand gathering wood 2/4/6; depleted-node rejection and respawn; stone gathering; tool crafting with exact costs; visible primitive first-person tool; tool harvesting two remaining stone hits in one action. Final aim check revealed that an external overlay crosshair was offset from the viewport center. Added a native centered marker and replayed the food interaction. `PFM4ManualFinal.log` records four gathered food and cancellation without consuming ingredients.
- September 28 rendered replay `Saved/Logs/PFM4FoodFinal.log`: restored already-tested tool/fuel using developer setup, but gathered the food through E. Four gathered food + three wood became one raw food, one cooked food and one dried food, with no wood remaining. Cook took 6 s and dry took 10 s; the UI showed independent freshness deadlines. Q consumed the selected cooked item, reduced occupied slots from four to three, restored Food to 100 and recovered Water. This replay used one rendered standalone client at 960x540/30 FPS cap and closed normally at 08:47:26 UTC.
- `PF.TestGathering`, `PF.TestCrafting`, `PF.Help` and export passed in that rendered process. Evidence: `Saved/AutomationReports/PF_M4FoodFinal_20260928T084634_E51FEE714A7293F84AEC0FB94782C788.json`, aggregate `Passed`. Inspected engine screenshot `Saved/Screenshots/WindowsEditor/ScreenShot00007.png`: readable recipes, tool, native aim marker, cooked/dried output quantities and freshness. `ScreenShot00006.png` is an earlier tool/UI capture with a blurred teleport transition, not the final food evidence.
- Known engine startup issues: game/server launches emit experimental StateTreeToolset/ToolsetRegistry Python errors about missing `ToolsetDefinition`/`PythonTestRunner`, and TEDS widget registration warnings. Rendered replay also emitted a render-thread safety warning for `r.MotionVectorSimulation`. These do not fail the gameplay tests; no new gameplay error, ensure or RHI crash was found in the reviewed runs. Baseline `Saved/Logs/PrimalFrontier.log` remains dated September 23 with normal exit; current launches use the explicit log paths above. Existing starter-content reference findings remain outside M4.
- Performance: two-client NullRHI samples were about 1.63/1.72/1.72 GiB working set for server/client/client (1.48/1.61/1.64 GiB private). Final rendered process sampled 3.36 GiB working set / 5.23 GiB private; Windows free physical memory was 15,068,460 KiB and visible total 33,477,984 KiB. This host reports about 31.9 GiB, so these samples do not certify 16 GB operation. Startup stalled briefly; active play settled at the requested 30 FPS cap. Stutter alone was not treated as gameplay failure; no sustained benchmark is claimed.
- Limitations: cooking/drying is portable and consumes wood fuel; placeable stations belong to future building work. Tool equipping is automatic while carried, with no durability or equipment slots. Storage preservation, technology unlocks, persistence and world expansion remain deferred. All evidence under `Saved` stays local and untracked. Next milestone is greybox building only after explicit authorization.

# Milestone 3 — verified 2026-09-26

**Gate passed for the supported greybox scenario.** Editor build, regression, real one-/two-client NullRHI automation, rendered first-person pickup/split/drop/recovery, food consumption/freshness, developer hooks and actual reconnect observation pass. Reconnect deliberately starts empty until M8 persistence. No packaged Shipping/Server target or rendered two-client result is claimed. All disposable test processes are closed. Stop here; do not begin M4 automatically.

User authorized M3 after M2 passed. Begin from clean `8be2a14`. Implement a small data-asset catalog, owner-replicated inventory on PlayerState, capacity/weight limits, transactional insert/remove/split/drop/pickup, finite batch food freshness and consumption, keyboard placeholder UI and non-Shipping PF grant/remove hooks. Inventory survives pawn respawn; reconnect starts empty until M8 persistence, with no client-supplied restore state. Use a separate primitive M3 map and preserve existing assets. Gate: Editor build, focused regression/duplication/authority tests, rendered first-person inventory playtest, one then two NullRHI clients, reconnect observation, logs/reports/memory and documentation. Do not begin M4 or technology progression in this step.

Implementation and checkpoint evidence (2026-09-25; pending items below are resolved by the final gate):

- Baseline build succeeded in 24.43 s. First inventory compile failed C2065 because Unreal replication macros require `OutLifetimeProps` as the parameter name; corrected both new classes, rebuilt successfully in 13.87 s. HUD float-literal C4305 warnings were corrected with float suffixes. Subsequent builds passed without compiler warnings. UBT initially skipped newly added test translation units; regenerating its makefile picked them up, and compiled test registration was verified in the actual reports.
- `Saved/AutomationReports/M3Transactions/index.json` initially failed three expiry assertions because one large fixture tick was clamped. Replaced it with thirty 0.1 s ticks and explicitly asserted that world time crossed the deadline. `M3TransactionsVerified/index.json`: 1 passed, 0 failed/warnings. Engine exit status alone is insufficient: the initial failed run still exited with 0.
- `M3Regression/index.json`: 9 passed, 0 failed/warnings. Exact tests: `PF.Inventory.Transactions`, `PF.Inventory.WorldTransfers`, `PF.Survival.Component`, `.Needs`, `.Environment`, `.Lifecycle`, `PF.PrimalAgentTools.CommandArguments`, `.MissingSystemsAreBlocked`, `.TeleportAndRuntimeReset`. Log: `Saved/Logs/PFM3Regression.log`.
- `PFM3Setup.log` confirms only the new catalog and M3 map were saved through supported Editor APIs; existing Content files remain unchanged.
- First live run (`PFM3Server1.log`, `PFM3Client1.log`) was stopped after identifying a harness timing issue: its drop RPC immediately followed split and hit the intentional 0.15 s rate limit. Added a 0.5 s client-test wait; gameplay throttling is unchanged. The stopped run has no acceptance result.
- Corrected real-network runs passed `PF.Inventory.Live`: `M3Server1Verified/index.json` and `M3Client1Verified/index.json`, each 1 success, 0 failures/warnings, exit status 0 at 05:50:53 UTC. Two-client reports `M3TwoServer/index.json`, `M3TwoClient1/index.json`, `M3TwoClient2/index.json` also each have 1 success, 0 failures/warnings. Every client split through its owned controller RPC, dropped food, picked it up through server interaction, retained the original food deadline, observed batch expiry, and retained nonperishable inventory through death/respawn. Both clients verified the other PlayerState exists but its private inventory is not replicated.
- Engine-generated command exports are `Saved/AutomationReports/PF_M3Live_20260925T055043_*.json` and `PF_M3Live_20260925T055347_*.json`. The client exports contain intentionally rejected grant probes; their aggregate command-history failures do not mean the automation failed.
- Final HUD build succeeded in 6.69 s without compiler warnings. Initial rendered check found text too small at 960x540; enlarged the panel/font and enabled wrapping. The repeated visual check shows readable controls, Wood x5 after E, Wood x3 plus x2 after X, one world drop and inventory quantity four after G, then quantity five after E recovery. `PFM3ManualVerified.log` records these server mutations. Food UI and reconnect observations are still pending.
- Two-client NullRHI processes sampled 1.63/1.72/1.73 GiB working set and 1.47/1.63/1.64 GiB private bytes, with 11,822,960 KiB physical memory free. Windows reports about 31.9 GiB visible physical memory. Rendered solo frames observed the requested 30 FPS cap; this is a short sample, not a sustained performance certification.
- Rendered process sampled 3.18 GiB working set / 5.02 GiB private bytes, with 12,343,224 KiB free physical memory. Known engine startup Python `ToolsetDefinition`/`PythonTestRunner` errors and TEDS registration warnings remain. No new gameplay errors, ensures or RHI crashes were found in the passing tests. No persistent M3 screenshot has been captured yet; Computer Use captures supplied the partial manual observations.

Changed files: new `Source/PrimalFrontier/Inventory/PFItemCatalog`, `PFInventoryComponent`, `PFInventoryPlayerState`, `PFItemPickup`, `PFInventoryHUD` header/implementation pairs; new `Tests/PFInventoryTests.cpp`, `PFInventoryWorldTests.cpp`; updated survival GameMode, controller and HUD; updated PrimalAgentTools `PFCommands.cpp`, `PFCommandTests.cpp`, new `PFInventoryLiveTests.cpp`; new `Scripts/SetupInventoryMilestone3.py`, `Content/PrimalFrontier/Items/DA_ItemCatalog.uasset` and `Maps/L_M3Inventory.umap`; documentation under `Docs/` and the plugin. Existing binary assets are unchanged. See `INVENTORY_M3.md` for controls and persistence limits.

## M3 final gate, 2026-09-26 (local time; log timestamps are UTC)

- Resumed from clean committed checkpoint `340c538`. No C++ or Content assets changed in this continuation. `PrimalFrontierEditor Win64 Development` rebuilt successfully in **23.35 seconds**, with no compiler warnings. UBT rebuilt the module unity units after its working-set change. Rotating build evidence: `C:/Users/jackh/AppData/Local/UnrealBuildTool/Log.txt`.
- Focused final run: `Saved/AutomationReports/M3FinalInventory/index.json` reports **2 passed, 0 failed, 0 warnings, 0 not run**: `PF.Inventory.Transactions` and `PF.Inventory.WorldTransfers`. `Saved/Logs/PFM3FinalInventory.log` records TestExit status 0 at 2026-09-25 20:17:15 UTC. No warning/error lines were found in this run. The prior nine-test regression and five successful `PF.Inventory.Live` process reports were inspected again; their results remain as recorded above. No gameplay code changed after those network tests.
- Computer Use rendered solo test on `L_M3Inventory`: teleported to the existing food fixture, lowered needs through server-only PF hooks, pressed E to collect food x4, and observed its freshness counter in the inventory. Pressing Q consumed exactly one, reduced weight from 0.8 to 0.6 kg, restored food/water reserves and left the same batch countdown running (260 s before, 253 s after). HUD food/water rose from approximately 17/15 to 51/23 despite ongoing drain. Log: `Saved/Logs/PFM3FoodFinal.log`, accepted consume request at 20:13:44 UTC.
- `PF.GiveItem Item_Wood 25` and `PF.RemoveItem Item_Wood 5` passed; final wood quantity was 20. `PF.GiveItem Item_Stone 100` correctly failed capacity validation without inserting stone: final inventory remained food x3 plus wood x20, 2/8 slots and 10.6/30 kg. `PF.ExportTestReport M3FoodFinal` passed. Export: `Saved/AutomationReports/PF_M3FoodFinal_20260925T201414_223D5306403E2B0666641A8806198DC2.json`. Its aggregate **Failed** and `InventoryRejected` error are the deliberately oversized grant probe, not an unexpected gameplay failure.
- Real reconnect: a single rendered client joined a NullRHI dedicated Editor server at `127.0.0.1:7786`, collected wood x5 (1 slot, 2.5 kg), then issued `open 127.0.0.1:7786`. It successfully rejoined with 0/8 slots and 0 kg. The collected world pickup stayed absent, demonstrating the server world was not reset. `Saved/Logs/PFM3ReconnectServer.log` records first join at 20:15:02 UTC, inventory insertion at 20:15:12, connection teardown and second successful join at 20:15:42. Client evidence: `Saved/Logs/PFM3ReconnectClient.log`. This verifies the intentionally nonpersistent M3 behavior, not item restoration.
- Engine screenshots were opened and visually inspected: `Saved/Screenshots/WindowsEditor/ScreenShot00004.png` shows food x3 with its countdown, wood x20 and needs HUD; `ScreenShot00005.png` shows empty inventory after reconnect. The earlier manual pickup/split/drop/recovery remains recorded above. Both rendered clients exited normally. The owned headless server was stopped after testing; automation exited with status 0.
- Rendered solo sampled **3.34 GiB working set / 5.25 GiB private bytes**. Multiplayer client sampled **3.28 / 5.30 GiB**, headless server **1.64 / 1.48 GiB**, with **11,680,232 KiB free physical memory**. Current Windows visible memory is **33,477,984 KiB (~31.9 GiB)**, so these observations do not certify a 16 GB configuration. Playtest samples were generally 28–31 FPS at the requested 30 FPS cap; screenshot captures briefly showed 20 FPS. No sustained benchmark or rendered two-client RHI result is claimed.
- Known startup Toolsets Python missing `ToolsetDefinition`/`PythonTestRunner` errors and TEDS registration warnings remain in game/server logs. Apart from the deliberate capacity rejection, no new gameplay errors, ensures or RHI crashes were found. The ordinary `Saved/Logs/PrimalFrontier.log` still ends with the September 23 normal editor exit; this continuation used the specifically named logs above.
- Saved the user's eight reference PNGs unchanged under `Docs/References/SurvivalGames`; source/copy SHA-256 comparisons all matched. Added a reference index and linked technology direction; recorded the documentation-only boundary in `DECISIONS.md`. They guide readable quantities/freshness, first-person framing and future original progression without importing runtime art or starting a tech tree.

Files changed in this continuation: `Docs/MILESTONES.md`, `Docs/DECISIONS.md`, `Docs/TECH_TREE_DIRECTION.md`, and `Docs/References/SurvivalGames/README.md` plus eight PNGs. Generated logs/reports/screenshots remain untracked. No unrelated template assets changed. Recommended next step is a separately authorized M4 gathering/crafting slice; M3 retains placeholder keyboard UI and no persistence, equipment or cooking.

# Milestone 2 — verified 2026-09-25

**Gate passed for the supported greybox scenario:** Editor build, seven regression tests, focused expiry deadline test, rendered solo manual playtest, one-client and two-client NullRHI network checks. No packaged Shipping/Server build or rendered two-client RHI result is claimed. Stop here; Milestone 3 has not begun.

Continuation authorized after the recorded Milestone 1 gate. Baseline Editor build succeeded (up to date, 1.98 seconds); Git was clean at `ee05bfe`. Plan: extend replicated vitals with hunger/thirst and exposure, add deterministic server drain/threshold effects and validated placeholder recovery, extend HUD/PF hooks, then run focused regression, solo manual playtest and low-memory network tests. Use a separate primitive M2 map. Do not start Milestone 3 before this gate passes.

Commits for this work include the milestone number and `Co-authored-by: Codex GPT-6 Astra <codex@openai.com>`. Existing Git identity remains the primary author; unrelated changes are excluded.

## M2 implementation and evidence

Changed gameplay files under `Source/PrimalFrontier/Survival`: `PFPlayerSurvivalComponent.h/.cpp`, `PFSurvivalPlayerController.h/.cpp`, `PFSurvivalHUD.h/.cpp`; added `PFSurvivalHazard.h/.cpp` and `PFRecoveryPickup.h/.cpp`. Changed `Tests/PFSurvivalComponentTests.cpp`; added `PFSurvivalNeedsTests.cpp` and `PFSurvivalEnvironmentTests.cpp`. Tooling changes: `PrimalAgentToolsRuntime/Private/PFCommands.cpp` and new `Private/Tests/PFSurvivalNeedsLiveTests.cpp`. Added `Scripts/SetupSurvivalMilestone2.py` and the single new `Content/PrimalFrontier/Maps/L_M2Survival.umap`. Documentation covers architecture, decisions, milestone evidence, Git attribution, command interface, M2 mechanics, food research and future technology progression. No existing Content asset is modified.

- Editor builds succeeded without compiler warnings: initial needs slice 35.03 s, integration 120.29 s (memory pressure while an Editor reopened), fixture refinement 6.36 s, live test 7.41 s. The reopened All Saved editor was closed normally before linking.
- `Saved/AutomationReports/M2Needs/index.json`: 3 passed, 0 failed/warnings. `M2Regression/index.json`: 7 passed, 0 failed/warnings: `PF.Survival.Component`, `.Needs`, `.Environment`, `.Lifecycle`, `PF.PrimalAgentTools.CommandArguments`, `.MissingSystemsAreBlocked`, `.TeleportAndRuntimeReset`. Logs: `PFM2Needs.log`, `PFM2Regression.log`.
- Computer Use manual play on `L_M2Survival`: observed food/water drain, E consumed the ration and restored reserves to 100, empty reserves reduced Health to zero, death HUD appeared, and respawn reset reserves. Real hazard overlap displayed Exposure 100%, caused damage/death; a later controlled entry/exit retained the same living pawn at Health 87 and restored Exposure 0. The exit observation temporarily used `slomo 0.1` and restored `slomo 1`; no settings or map were saved. Logs: `PFM2Manual.log`, `PFM2ManualExit.log`.
- `PF.Help`, `PF.SetHunger`, `PF.SetThirst`, `PF.SetExposure`, `PF.RecoverNeeds` passed in the actual game. Fixed recovery changed 20/30 to 55/65 before ongoing drain. Reports: `PF_M2ManualExit_20260922T231620_AC98172A40D388872990DBA71048263D.json`, `PF_M2ManualHooks_20260922T231651_6503A18F42786CA5B23CB7A568FAFC89.json` under `Saved/AutomationReports`.
- Engine screenshot artifacts: `Saved/Screenshots/WindowsEditor/ScreenShot00001.png` and `ScreenShot00002.png`. The latter captured the teleport transition before the next exposure update and is motion blurred; it is not evidence of the final zero-exposure HUD. The live Computer Use capture and exposure log at 2026-09-22 23:16:21 UTC confirm exit.
- Setup issue resolved: `SaveMap` created M2 but left M1 active. Restored only this session's unintended M1 write from Git; populated the new M2 map using Editor APIs. Corrected setup to `NewLevelFromTemplate` plus an active-map guard. `PFM2SetupRepair.log` confirms only M2 was saved; Git reports no modifications to existing Content assets.
- Manual gameplay sampled 29–30 FPS; startup/capture stalls were transient. System overlay ranged approximately 13.1–15.4 GB RAM across the two days; resumed rendered process was 2.30 GiB working set / 5.21 GiB private bytes. With two NullRHI network processes starting, available physical RAM fell to about 0.77 GiB. No inference of gameplay failure is made from stutter alone.

- Food-expiration refinement builds passed (26.34 s, 7.05 s and final fixture build 6.22 s). `M2ExpiryRegression/index.json` repeats the seven regression tests with 7 passed, 0 failed/warnings. `M2ExpiryDeadline/index.json` passes the focused environment test with cleanup deliberately disabled until after the expired-consumption rejection check.
- `PFM2FoodManual.log`: observed the ration's visible freshness countdown in the rendered game; accelerated the disposable session with `slomo 10` and observed all three pickups expire and disappear (server expiration logs 2026-09-22 23:25:31 UTC). `ScreenShot00003.png` shows the countdown. This is accelerated expiration verification, not a normal-speed performance benchmark.
- Original one-client NullRHI network gate passed: `M2Server1/index.json` and `M2Client1/index.json`, each 1 success, 0 failures/warnings for `PF.Survival.NeedsLive`. Both reached TestExit with status 0. This run predates the replicated food-expiration assertion; final results follow below. A later `PFM2ExpiryServer1` setup was intentionally stopped before connecting a client while the user used the rendered game; it has no acceptance result.
- User-requested Palworld/ARK food research and original proposed spoilage/preservation rules are in [FOOD_AND_PRESERVATION.md](FOOD_AND_PRESERVATION.md). Only M2 world-food expiration is implemented; cooking, inventory and storage remain behind their later gates.

## Final gate, 2026-09-25

- `PrimalFrontierEditor Win64 Development` build succeeded, up to date in 2.11 s, no compiler warnings. Rotating evidence: `C:/Users/jackh/AppData/Local/UnrealBuildTool/Log.txt`.
- Final one-client expiry/needs run: `Saved/AutomationReports/M2ExpiryServer1Verified/index.json` and `M2ExpiryClient1Verified/index.json`, each **1 passed, 0 failed, 0 warnings**. Corresponding logs: `PFM2ExpiryServer1Verified.log`, `PFM2ExpiryClient1Verified.log`; both exit status 0 at 05:19:10 UTC.
- Two-client run on localhost port 7783: `Saved/AutomationReports/M2TwoClientServer/index.json`, `M2TwoClient1/index.json`, `M2TwoClient2/index.json`, each **1 passed, 0 failed, 0 warnings** for `PF.Survival.NeedsLive`; all exit status 0 at 05:21:54 UTC. Corresponding logs have prefix `PF` and suffix `.log` under `Saved/Logs`. Both clients observed PlayerIds 256 and 257 at needs 40/50/exposure .25, recovery 75/85/0, empty needs with Health 90, death Health 0, and replacement pawns at 100/100/100. Fresh food and subsequent replicated removal assertions passed on every process. Direct and PF command client mutations were rejected.
- PrimalAgentTools final exports: `PF_M2LiveServer_20260925T052144_E4015694434DDB4B0ECAD0A2BDE72692.json`, `PF_M2LiveClient_20260925T052144_808CE59F4A097DEA5DC73A8F7DA2EA42.json`, `PF_M2LiveClient_20260925T052144_E789E26E4B8F957379E39788CC019AA4.json`. Client command-history aggregate failures are the deliberate `NotAuthority` probes; the automation independently verifies rejection and passes.
- Known pre-existing startup issues remain in engine Toolsets Python (`ToolsetDefinition` and `PythonTestRunner` missing in game/server mode) and TEDS widget registration. These are not new gameplay failures. No new gameplay errors, ensures or RHI crashes were found in the successful final network runs. The ordinary `PrimalFrontier.log` was also checked; its last recorded editor session exited normally on September 23.
- On this resumed host Windows reports 33,477,984 KiB visible physical memory (about 31.9 GiB), unlike the earlier 16 GB runs. The three NullRHI processes sampled 1.63/1.73/1.73 GiB working set (1.46/1.62/1.62 GiB private bytes), with 12,327,252 KiB physical memory free. Thus the final two-client result is not a certification of the earlier 16 GB environment. Headless runs do not measure rendering stutter; prior rendered solo observations remain above.
- The user's technology references are retained in `TECH_TREE_DIRECTION.md`: accessible ordinary unlock points, a separate challenge-earned path, early essentials and later preservation. No technology tree was implemented.

All test processes finished. Remaining limits: no inventory, player cooking/drying, preservation containers or persistent freshness yet; the world ration is a finite fixture with a 300 simulation-second test lifetime. Recommended next milestone is M3's authoritative item/inventory data, carrying freshness through pickup/drop/stack operations. Do not start it automatically in this step.

# Milestone 1 — verified 2026-09-21

**Gate passed for the supported one-client greybox scenario. Stop here; Milestone 2 has not begun.** The final Editor build, five focused regression tests, actual server/client replication test, and manual first-person playtest passed. This is not a packaged Shipping or two-client acceptance result.

## Changes

- Added separate `PFSurvivorCharacter`, `PFSurvivalGameMode`, `PFSurvivalPlayerController` and `PFSurvivalHUD` headers/implementations under `Source/PrimalFrontier/Survival`. Integrated the existing replicated Health/Stamina component; server damage, jump cost, delayed recovery, death and PlayerStart respawn work through the same authoritative APIs.
- Exported the existing C++ character class for cross-module use; its camera/input implementation and template assets remain unchanged.
- Updated PrimalAgentToolsRuntime's build dependency and `PFCommands.cpp`; added `PFSurvivalLiveTests.cpp` and `Source/PrimalFrontier/Tests/PFSurvivalLifecycleTests.cpp`. `PF.SetHealth`, `PF.SetStamina`, `PF.Damage`, `PF.Kill` and `PF.Respawn` use real gameplay APIs. Commands are discoverable, logged, reported and non-Shipping. Mutations reject clients and ambiguous multiple-player targeting.
- Added `Scripts/SetupSurvivalMilestone1.py`, three project-owned Blueprint compositions in `Content/PrimalFrontier/Survival`, and `Content/PrimalFrontier/Maps/L_M1Survival.umap`. The map contains four cube actors, one PlayerStart and one directional light. It uses existing template presentation/input assets and engine primitives only.
- Updated architecture, decisions, this milestone log and developer command documentation; added `Docs/SURVIVAL_M1.md`. Hunger and Thirst remain interfaces only.

## Build and automated evidence

`Build.bat PrimalFrontierEditor Win64 Development -Project=C:/UnrealProjects/PrimalFrontier/PrimalFrontier.uproject -WaitMutex -NoHotReloadFromIDE -MaxParallelActions=2` succeeded. Final compilation/link: **12.56 seconds, no compiler warnings**. Evidence: `C:/Users/jackh/AppData/Local/UnrealBuildTool/Log.txt` (rotating build log).

`Saved/AutomationReports/M1Regression/index.json`: **5 succeeded, 0 failed, 0 warnings, 0 not run**:

1. `PF.Survival.Component`: initial values, damage, stamina/recovery, invalid input, authority guards, death and reserved interfaces.
2. `PF.Survival.Lifecycle`: actual native GameMode possession, grounded PlayerStart, damage/jump cost, client mutation rejection, three timed death/respawn cycles.
3. `PF.PrimalAgentTools.CommandArguments`: command registration, concise help, arguments, PF.Help execution and recording.
4. `PF.PrimalAgentTools.MissingSystemsAreBlocked`: unimplemented integrations remain explicit blockers.
5. `PF.PrimalAgentTools.TeleportAndRuntimeReset`: existing authority/collision behavior unchanged.

Log: `Saved/Logs/PFM1Regression.log`. The focused lifecycle retry also passed with NullRHI in `M1LifecycleVerified/index.json` and `PFM1LifecycleVerified.log`.

`PF.Survival.Live` passed independently on a NullRHI dedicated server process and one rendered client over localhost port 7781. Each report has **1 succeeded, 0 failed, 0 warnings**:

- `Saved/AutomationReports/M1Server/index.json`; `Saved/Logs/PFM1Server.log` (success at 03:45:31.916 UTC).
- `Saved/AutomationReports/M1Client/index.json`; `Saved/Logs/PFM1Client.log` (success at 03:45:31.906 UTC).

The client observed initial 100/100, authoritative Health 75/Stamina 20, death at 0/0, replacement possession and 100/100 after respawn. All five client mutation commands and direct component mutation probes were rejected. Both processes reached TestExit without a crash. This uses the Editor executable's `-server` mode; a packaged Server target is not claimed.

PrimalAgentTools exports:

- `Saved/AutomationReports/PF_M1Manual_20260921T034349_02F73ADD47C7FCA616EC5C939AF0E44F.json`
- `Saved/AutomationReports/PF_M1LiveAuthority_20260921T034521_E400187743BCB85B32A8FEADFFA05A6B.json`
- `Saved/AutomationReports/PF_M1LiveClient_20260921T034521_F9B92423418C1B381676A38C87F1D028.json`

The client command-history export deliberately has aggregate status **Failed**, because its five forbidden mutation probes are recorded as failed commands with `NotAuthority`. The automation report separately verifies those rejections and passes. Do not interpret the history export as a successful mutation or silently discard its failures.

## Manual playtest and assets

Computer Use inspected the actual standalone and connected-client windows at 960x540. The standalone test showed the first-person template mesh/camera, movement (debug X changed from -400.00 to -397.72 on W), mouse look, and jump reducing Stamina from 100 to 80. `PF.Damage 25` changed the visible Health bar to 75. `PF.SetStamina 0` emptied the bar and it recovered to 100. `PF.Kill` showed the death message and automatic respawn restored Health/Stamina and the initial view. Connected-client play showed server-driven 75/20, death, and respawn on the HUD; jump input also responded.

Persistent screenshot, generated by Unreal's `shot showui` and inspected: `Saved/Screenshots/WindowsEditor/ScreenShot00000.png`. Manual log: `Saved/Logs/PFM1Manual.log`. Asset creation log: `Saved/Logs/PFM1Setup.log`.

All **609 pre-existing Content files** retain their pre-task SHA256 aggregate `AC7D247B10EF3AB88337210954C410EF2C779CEEE96FB7E1FE3D12CC11AD9B16`. Only the four new M1 assets were created. Existing map relocation/deletions and unrelated Phase 10 work were preserved.

## Failures resolved, warnings and limits

- A C++ local name initially shadowed UWidget::Slot; renamed and rebuilt successfully.
- The resumed build initially failed LNK1104 because the user-opened Editor locked the DLL. Closed the All Saved editor normally, then the build passed.
- The first lifecycle run incorrectly required exact PlayerStart Z despite capsule grounding. It now checks start XY and bounded vertical adjustment. A subsequent expected-warning regex needed correction. The final rerun passes; the narrowly matched native fixture warnings reflect missing presentation meshes, not a failure in the actual Blueprint character.
- Asset setup reported a hierarchy-change warning when reparenting the new GameMode copy. The new composition loaded and passed the real possession/respawn tests; templates were not rewritten.
- Existing engine startup issues remain: StateTreeToolset/ToolsetRegistry Python scripts expect editor-only `ToolsetDefinition`/`PythonTestRunner` in game/server launches, TEDS widget-factory registration warnings, and the `r.MotionVectorSimulation` render-thread warning. The server also recovered a stale Zen lock. No new survival gameplay errors, ensures or RHI crashes occurred in the passing runs. `Saved/Logs/PrimalFrontier.log` was inspected; the M1 launches intentionally use separate named logs.
- Observed rendered gameplay held the requested **30 FPS** cap in sampled frames. Screenshot capture briefly displayed 1 FPS before returning to 30; no sustained gameplay stall was observed in this short test. This is not a long-duration performance certification.
- Standalone process: about **2.82 GB working set / 5.10 GB private bytes**. During the network test, rendered client: **2.37 / 5.11 GB**; NullRHI server: **1.64 / 1.47 GB**. Whole-system overlay ranged about **14.5–15.7 GB RAM**; OS available physical RAM reached **0.95 GB**. Process private bytes are not physical residency. Two rendered clients were not practical; no new two-client result is claimed.
- Existing historical generated files exceed GitHub's file-size limit. Do not rewrite history or stage unrelated changes as part of this milestone.

Recommended next step: user review of this first-person foundation. Start Milestone 2 only after explicit approval. All test sessions were closed.

---

# Greybox milestone plan — 2026-09-21

Current authorization: implement and verify **Milestone 1 only**, then stop and report. Milestones 2–8 below are a roadmap, not permission to bypass a gate.

1. Player foundation: integrate the existing Health/Stamina component with an isolated survivor character, first-person input, server damage, death, PlayerStart respawn, placeholder HUD and PF developer hooks. Build, automated tests, manual solo playtest, then one client/server test are required.
2. Hunger/thirst and environmental survival, after an explicit continuation.
3. Item data and authoritative inventory.
4. Gathering and crafting.
5. Greybox building and ownership.
6. Passive/hostile greybox creatures.
7. Small greybox survival arena.
8. Versioned persistence and multiplayer/reconnect acceptance; stop permanently after this milestone.

Each milestone must compile, pass focused automated tests, pass a manual playable test, validate server authority, and record logs, useful screenshots, memory/stutter observations and decisions before progression. Use NullRHI for nonvisual checks and one rendered client at a time. Use existing template assets and primitives only. Preserve unrelated Git and asset changes.

Audit: Phase 9 reports contain three passing tests, Phase 10 editor reports six passing tests, and the Phase 10 standalone live test passed. Client 2's rendered D3D12 residency crash remains an incomplete multiplayer result, not a gameplay pass. The current baseline build now succeeds after normally closing the saved editor (12.10 seconds, two build workers); no new gameplay integration has been verified yet.

## Earlier checkpoint

## First-person survival foundation: in progress, blocked at first build

Scope for this milestone is Health, Stamina, server-authoritative changes, death/respawn, a minimal first-person HUD and focused tests. Hunger and Thirst remain planned interfaces. Inventory, crafting, creatures, building, persistence and world expansion are not authorized by this milestone.

The initial source slice adds `UPFPlayerSurvivalComponent` with replicated Health/Stamina, server-only mutation guards, clamping, stamina recovery, death-state Gameplay Tags and notifications. `PF.Survival.Component` contains assertions for initial values, damage, stamina, authority rejection and death. Hunger/Thirst tags reserve names only; `SupportsStat` explicitly returns false for them. GameplayTags and SlateCore were added to the game module dependencies (SlateCore is for the planned placeholder HUD).

This component is **not yet attached to a player**. Character integration, respawn, HUD, PrimalAgentTools integration, project-owned Blueprint copies and live replication tests remain unimplemented. Existing first-person/template assets have not been changed by this work.

Verification attempted:

- `Build.bat PrimalFrontierEditor Win64 Development -Project=C:/UnrealProjects/PrimalFrontier/PrimalFrontier.uproject -WaitMutex -NoHotReloadFromIDE -MaxParallelActions=2`
- UHT, `PFPlayerSurvivalComponent.cpp` and `PFSurvivalComponentTests.cpp` compiled; no compiler warnings appeared in this attempt.
- Target build **FAILED** with `LNK1104` because running UnrealEditor PID 35724 held `UnrealEditor-PrimalFrontier.dll`, `UnrealEditor-PrimalAgentToolsRuntime.dll` and `UnrealEditor-PrimalAgentTools.dll` open. UBA first reported the file-in-use errors and then retried linking without UBA; the same lock remained.
- Build evidence: `C:/Users/jackh/AppData/Local/UnrealBuildTool/Log.txt`, result `Failed (OtherCompilationError)`, elapsed 78.57 seconds. This generated log may be replaced by the next build.
- New tests run: **none**. The failed link prevents testing current source; previous Phase 10 passes do not verify this new component.
- Existing `Saved/Logs/PrimalFrontier.log` contains the `r.MotionVectorSimulation` render-thread warning. No new editor or multiplayer process was launched.
- Resource observation: UBT reported 15.93 GB physical RAM and 32.01 GB committed at build start. Compilation was limited to two workers. No gameplay performance or stutter assessment was made.

Stopped after the build failure without expanding implementation. Next step: close the existing editor after handling any unsaved work, retry the same incremental build, and run only `PF.Survival.Component` with NullRHI before continuing this milestone. No Git staging or commits were performed.

---

\# Milestones



\## Milestone 0 — Toolchain



\- \[ ] Unreal project opens

\- \[ ] Editor target compiles

\- \[ ] Git and Git LFS are configured

\- \[ ] Logs can be collected

\- \[ ] Development test map exists

\- \[ ] Automated test workflow exists



\## Milestone 1 — Vulnerable Survivor



\- \[ ] First-person character

\- \[ ] Health

\- \[ ] Stamina

\- \[ ] Hunger

\- \[ ] Thirst

\- \[ ] Death and respawn

\- \[ ] Two-player multiplayer test



\## Milestone 2 — Hunter



\- \[ ] Resource gathering

\- \[ ] Inventory

\- \[ ] Primitive tools

\- \[ ] Basic weapon

\- \[ ] Passive creature

\- \[ ] Hostile creature

\- \[ ] Basic combat



\## Milestone 3 — Builder



\- \[ ] Crafting

\- \[ ] Foundation placement

\- \[ ] Walls and doors

\- \[ ] Storage

\- \[ ] Building persistence

\- \[ ] Server validation



\## Milestone 4 — Creature Tamer



\- \[ ] Companion creature prototype

\- \[ ] Ownership

\- \[ ] Follow and stay commands

\- \[ ] Creature persistence

\- \[ ] Basic creature utility



\## Milestone 5 — Settlement Leader



\- \[ ] Cooperative permissions

\- \[ ] Improved building pieces

\- \[ ] Crafting stations

\- \[ ] Settlement storage

\- \[ ] Resource and creature management



\## Milestone 6 — Industrial Survivor



\- \[ ] Advanced resources

\- \[ ] Processing systems

\- \[ ] Industrial crafting tier

\- \[ ] Advanced structures

\- \[ ] Dangerous Eryndor regions



\## Milestone 7 — Ancient Researcher



\- \[ ] Ancient sites

\- \[ ] Research progression

\- \[ ] Ancient technology

\- \[ ] First Frontier preparation



\## Milestone 8 — Dimensional Explorer



\- \[ ] First Frontier

\- \[ ] Frontier hazards

\- \[ ] Frontier creatures

\- \[ ] Frontier resources

\- \[ ] Dimensional progression



\## Milestone 9 — Master of the Frontiers



\- \[ ] Multiple Frontiers

\- \[ ] Endgame systems

\- \[ ] High-tier technology

\- \[ ] Optional third-person mode

\- \[ ] Optional PvP evaluation

