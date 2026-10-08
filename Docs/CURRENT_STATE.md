# Primal Frontier — current state

Updated: October 8, 2026. Maintain this file after every meaningful code, content, settings or documentation change and after each build/test result. A change being implemented does not mean its milestone has passed.

## Current game

Primal Frontier is a first-person, server-authoritative greybox survival prototype. The intended game is open world. The current open-world candidate is `L_PrimalFrontier_OpenWorld`, a 400 x 400 m World Partition map. `L_M7SurvivalArena` and the earlier milestone maps remain small regression fixtures.

Implemented systems include health, stamina, hunger/thirst, damage/death/respawn, food batches and spoilage, replicated inventory, pickup/drop, gathering, timed crafting, primitive building/storage/ownership, passive/hostile placeholder creatures and a world clock. Pause, controller bindings and settings menus exist. See [PLAYTEST.md](PLAYTEST.md) for controls and opening the game.

Existing user-imported asset packs are preserved; the active greybox uses primitives and simple project materials. Final art, third-person gameplay and production-scale world expansion have not started.

## Completed work and verification

- M1–M6 implementation and their recorded verification are in [MILESTONES.md](MILESTONES.md). Earlier passes are historical evidence, not certification of every later source change.
- M7 settings regression: 19 tests passed, no failures/test warnings, in `Saved/AutomationReports/M7SettingsRegression/index.json`. One-server/two-client NullRHI world tests passed in `M7OpenPolicyTwoServer`, `M7OpenPolicyTwoClient1` and `M7OpenPolicyTwoClient2`.
- Open-world streaming probe passed in `M7StreamingVerified/index.json`. Sustained manual walking between zones and overnight survival have **not passed**. M7 remains incomplete; the user authorized independent M8 work while deferring this gate.
- M8 player save codec, authoritative player restore and checksummed A/B file generations are implemented and committed. `M8PlayerFormatVerified/index.json`: three persistence tests passed. `M8PlayerRuntime/index.json`: eight Success states, one with known meshless-fixture warnings. Final `M8PlayerAdapterVerified/index.json`: one passed, no failures/test warnings.
- M8 stable structure-owner keys and owner rebinding are covered by the passing `M8BuildingIdentityVerified/index.json` fixture. This proves component ownership checks, not an actual network reconnect. The five existing persistence tests and seven other regressions passed in `M8CurrentStateRegression/index.json`; its building test initially failed and was repaired/retried separately below.

Reports above are under `Saved/AutomationReports`; matching `PF<report-name>.log` files are under `Saved/Logs`. Detailed evidence and limitations: [PERSISTENCE_M8.md](PERSISTENCE_M8.md).

## Work in progress

M8 world-save records, storage/resource/creature/pickup restoration and development reconnect identity remain source changes in the working tree. The Editor build passed after fixing a unity-build name collision; their world/reconnect behavior has **not yet been verified**. Stable structure ownership has a separate passing native fixture. GameMode lifecycle integration, developer save/load commands and world/restart/reconnect tests are still being completed. Do not rely on these new APIs as a working save system yet.

The requested adaptation and active/passive biological-modifier systems are **planning only**, provisionally M12 progression/M13 ecology. [ADAPTATION_DIRECTION.md](ADAPTATION_DIRECTION.md) records research, original proposals and future authority/save requirements. The reference games' names, creatures, UI and art will not be copied.

## Known limitations

- M7 manual route/overnight and the full M8 playable save/restart/reconnect gate remain unverified.
- New Editor Window PIE previously showed roughly 15–16 FPS; that presentation bottleneck remains unresolved. A stationary Selected Viewport sample performed much better, but does not prove travel performance. NullRHI has no rendered FPS evidence.
- Installed engine reports 5.8.3, although the project request targets 5.8.2. The installation has not been changed.
- The large imported-asset Git LFS push finished. Remote `master` and the archived stash tag were verified on October 8. Uncommitted M8 integration source remains local; new commits still need remote verification before reporting synchronization.

## Next development step

Add focused world-codec/capture/restore tests before activating M8 GameMode lifecycle hooks and developer save/load commands. Compile `PrimalFrontierEditor` and resolve failures within M8 before expanding scope. Then verify one real client/server, restart/reconnect and two clients with NullRHI where practical; rendered/manual gates must retain their actual status.

This development pass ends after M8. Later roadmap work is listed in [FULL_PROJECT_ROADMAP.md](FULL_PROJECT_ROADMAP.md) and [ROADMAP_STATUS.md](ROADMAP_STATUS.md). Notify the user when a specific asset gap reaches implementation.

## Latest change

Added this current-state document and the instruction to maintain it. The first build failed in 18.32 s with a unity-build C4459 name collision. Renaming the codec constant to `PlayerSaveMagic` preserved the format; rebuild passed in **16.96 s**, without compiler warnings.

`M8CurrentStateRegression/index.json`: **12 passed, 1 failed, no test warnings**. The building fixture handed a chest to another PlayerState by changing only the old `Builder` pointer; the newly authoritative persistent owner GUID still belonged to the original player. Four downstream assertions failed. Updated the fixture to transfer both fields and added stable-ID replacement/rebinding checks. Final Editor rebuild passed in **16.26 s**, without compiler warnings. `M8BuildingIdentityVerified/index.json`: **1 passed, 0 failed, 0 test warnings**, engine exit 0; the matching log has no warning/error/ensure/fatal lines.

The 12 earlier passes were `PF.Crafting.Gathering`, `PF.Crafting.Transactions`, `PF.Creatures.Lifecycle`, `PF.Interaction.TargetAndPickup`, `PF.Inventory.Transactions`, `PF.Inventory.WorldTransfers`, `PF.Persistence.CorruptPlayerData`, `PF.Persistence.FileGenerations`, `PF.Persistence.PlayerRoundTrip`, `PF.Persistence.PlayerValidation`, `PF.Persistence.ServerPlayerAdapter` and `PF.Survival.Lifecycle`. The failed `PF.Building.PlacementAndStorage` is the sole test rerun after the fixture fix. The regression memory wrapper hit an Int32 overflow; its reported peak is invalid. The corrected Int64 monitor on the building retry sampled **2.940 GiB working set / 2.801 GiB private memory**. NullRHI supplies no rendered FPS/stutter evidence. Engine TestExit status was 0 even for the initial failed test: the JSON report is the verdict.
