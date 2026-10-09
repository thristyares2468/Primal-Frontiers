# M12 — private player progression and completed-craft persistence

October 9, 2026. Trello: https://trello.com/c/qmZ2ESrC. This is a bounded progression integration, not full M12 acceptance. Personal M8/M11 checks remain open. No art, maps, settings, item definitions or baseline recipe access changed.

## Current behavior

UPFProgressionComponent lives on APFInventoryPlayerState, alongside inventory and crafting. It survives pawn death/respawn and replicates its complete XP/knowledge/credited-recipe record only to the owning client. Level and available points are derived through the verified native curve/catalog, never saved as redundant counters. The component has no tick and no XP grant RPC or Blueprint mutator. Blueprint can read XP, level and points; there is no new progression screen yet.

A recipe gives 20 XP the first time its actual server-owned timed craft completes. The crafting transaction prepares a validated candidate before consuming inputs and commits it only after the existing atomic inventory conversion succeeds. Repeating a recipe still produces its item but never repeats the first-craft award. Cancellation, expired/moved inputs, full output capacity, invalid requests, pickup/drop/storage transfers, and developer grants do not earn XP. The craft ledger survives load/reconnect, so reading a save does not replay an award. No gathering/building/discovery/kill/idle rewards added in this slice. Only the current eight recipes can earn first-craft XP; this is not enough to reach the entire ten-level curve yet.

The existing Tech_FieldTools metadata and pure purchase transaction remain foundations. Players cannot purchase knowledge through an input/RPC/UI yet, and no recipes are restricted. A native test restores a validated 100-XP/learned-knowledge fixture solely to verify persistence and point derivation; it is not proof of a playable purchase flow. No equipment slot, respec, adaptation or discovery credit implemented.

## Save and authority contract

Normal runtime world capture now writes explicit world V2, with owner-bound progression for every connected or offline player identity. Inventory/player bytes remain on their existing V1 codec. Old V1 world files remain readable and start progression at zero with empty ledgers; an owned tool or structure is not proof of a past award. Migration is in memory until an ordinary explicit save or existing disconnect checkpoint publishes a new generation. No private user save was rewritten by development tests; runners use unique disposable Automation slots/profiles. Existing save-file generations/backups, ownership, freshness/offline aging and reserved-slot rules remain in effect.

The old temporary V2-runtime refusal is removed because actual component capture/restore now exists. Full world metadata and progression are validated before actor mutation; each connected player also preflights progression together with inventory/location restoration. Invalid accounting, owner swap, unknown IDs, corrupt/future data and wrong-role restore refuse safely. Startup/pending-restoration guards continue to prevent a default player from overwriting a failed restoration.

Reconnect can happen before the first manual save. Logout now prepares the entire roster checkpoint, including legacy zero defaults, then publishes its V2 schema and owner records together. This avoids a mixed V1 roster with a populated V2 progression field. Existing reconnect credential privacy/timing and real controller teardown ordering remain unchanged. Progression is server-owned even while no pawn exists; death never resets it. Client calls to Capture/Restore fail and preserve outputs/state. There is no client-supplied XP amount or public observer XP/knowledge ledger.

## Retained failures and native verification

All Editor builds passed without compiler warnings: initial 26.66 s (`PFM12ProgressionPlayerEditorBuild.log`), fixture correction 6.10 s (`PFM12ProgressionPlayerRetryEditorBuild.log`), reconnect probe 6.29 s (`PFM12ProgressionReconnectProbeBuild.log`), reconnect fix 6.65 s (`PFM12ProgressionReconnectFixBuild.log`), plugin-only live fixture 6.43 s (`PFM12ProgressionNetworkEditorBuild.log`), under `Saved/Logs/`.

| Run under Saved/AutomationReports | Result | Evidence |
| --- | --- | --- |
| Automation_M12ProgressionPlayer_20261009_113111797_58d2b292 | Failed: Component 3 assertions; other 4 tests passed | 36.94 s; working/private 2.909/2.782 GiB; engine 0, verdict runner 1; raw 0 warnings/4 error lines. Fixture removed 2 of 4 stone, still leaving the reserved 2 available. Legitimate completion/XP shifted later expectations. Fixed only fixture to remove/replenish all 4. |
| Automation_M12ProgressionPlayerRetry_20261009_113315807_46d84228 | Failed-only Component 1/1 passed | 16.35 s; 2.969/2.826 GiB; raw/test severity 0, exits 0. |
| Automation_M12ProgressionReconnectProbe_20261009_113519768_cb1b0af7 | Failed: Component 2 errors | 16.21 s; 3.114/3.045 GiB; engine 0, runner 1; raw 4 error lines. Real pre-first-save reconnect refused `Invalid progression world version`. Fixed only atomic roster publication described above. |
| Automation_M12ProgressionReconnectFix_20261009_113644751_75918954 | Failed-only Component 1/1 passed | 16.12 s; 2.915/2.778 GiB; raw/test severity 0, exits 0. |
| Automation_M12ProgressionPlayerFinal_20261009_113738338_e0dc2fb4 | Final original selection 5/5 passed | 16.64 s; 2.988/2.955 GiB; raw/test severity 0, exits 0. |

Exact final tests: PF.Progression.Component, PF.Progression.WorldCompatibility, PF.Persistence.WorldRuntime, PF.Persistence.RejectedLoadPreservesWorld, PF.Crafting.Transactions. No timeout, crash, fatal or ensure in these runs; both failures are retained. Native fixtures need no saved level; transient runtime fixtures use L_Automation. Component tests perform real crafting, cancellation/capacity/input failure, repeated completion, authority refusal/output preservation, controller teardown/reconnect before any disk save, actual V2 files loaded twice, death/respawn and actual V1-to-V2 zero-default migration. Owner-swapped V2 real-file rejection preserves the running world and original active generation. Existing whole-world and craft transaction assertions remain.

Each native report has `index.json` and `run-summary.json`; raw logs are `Saved/Logs/PF<run>.log`. Test report success does not certify human gameplay or rendering.

## Live verification

Exact level: `/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld`. Uncooked dedicated server and NullRHI clients; one client before two. No competing rendered instances. PF.Persistence.Live now includes actual gathered-resource tool crafting (20 XP per player), actual gathered-food cooking (another 20 XP for the storage owner), manual save/load, independent public identities, private bag/storage/progression checks, invalid client requests, disconnect checkpoints and restart into a new server process.

One-client Create/Restart 4/4 passed: `M8Live1_20261009_114018336_4f534f23`. Server and client each phase passed PF.Persistence.Live with engine/verdict exits 0, test severity 0, no fatal/ensure; sampled working/private range 1.719–1.816/1.576–1.731 GiB. The owner receives 40 XP and both recipe IDs after manual load/restart; client Restore refuses without changing XP. Raw logs contain 24 known startup warnings/14 experimental Python-error lines per process; final baseline comparison pending.

Two-client 75 ms lag/1% packet-loss Create/Restart 6/6 passed: `M8Live2_20261009_114214761_a8e1ad78`. Exact PF.Persistence.Live on server/Client1/Client2 each phase; engine/verdict exits 0, test severity 0, no timeout/fatal/ensure/crash. Sampled working/private range 1.721–1.819/1.580–1.731 GiB. Actual storage owner has 40 XP/tool+cook ledger, other player 20 XP/tool ledger after manual load and separate server restart. Each sees remote progression zero/empty, and direct client replacement refuses; Client2 did not crash. All 10 network raw logs retain 24 startup warnings/14 experimental Python-error lines each. Normalized unique severity compared with `PFM8Live2_20261009_111810709_5d820e56CreateServer.log`: zero new warning/error lines in every process. Known 21 widget-factory/3 HLOD warnings and the two experimental Toolsets Python traces remain; raw logs are not clean. Successful native logs reviewed separately: zero severity.

Development Game build passed in 28.01 s without compiler warnings: `Saved/Logs/PFM12ProgressionPlayerGameBuild.log`. Editor/Game logs reviewed. Reports follow `Saved/AutomationReports/<run>[Create|Restart][Server|Client1|Client2]/index.json` and aggregate `<run>/run-summary.json`; raw logs `Saved/Logs/PF<run>[Create|Restart][Server|Client1|Client2].log`. No rendered progression-screen/FPS/controller/audio/manual acceptance claimed. Actual host is 31.93 GiB; minimum 16-GB behavior remains uncertified. Installed engine reports 5.8.3 instead of requested 5.8.2; packaged Server target remains unsupported by this installed distribution.

Changed source: new Progression/PFProgressionComponent.h/.cpp and Tests/PFProgressionRuntimeTests.cpp; Inventory/PFInventoryPlayerState.h/.cpp; Crafting/PFCraftingComponent.cpp; Persistence/PFWorldPersistence.h/.cpp; native world/refusal tests and plugin PFPersistenceLiveTests.cpp. Evidence/current-state/architecture/decision/progression/Trello documents updated. No binary assets, engine/project rendering settings or personal files changed. Technical component/craft/persistence gate passed; bounded Trello completion and scoped Sol publication follow. Full M12 remains incomplete.

## Replay and next step

Close other Unreal sessions, then from the project root:

```powershell
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunNativeAutomation.ps1 -TestFilter 'PF.Progression.Component+PF.Progression.WorldCompatibility+PF.Persistence.WorldRuntime+PF.Persistence.RejectedLoadPreservesWorld+PF.Crafting.Transactions' -Label M12ProgressionPlayerFinal
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunPersistenceAutomation.ps1 -Players 1
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunPersistenceAutomation.ps1 -Players 2 -SimulateLagLoss
```

Next independently actionable work: readable owner progression feedback, server-validated knowledge purchasing and a deliberately tested optional unlock, then bounded gathering/building reward rules before balance acceptance. Do not infer those features from this component. Preserve baseline food/water/tool/shelter access and old items; no retrospective reward or silent recipe lock. Human UI/controller/persistence gates remain on their existing Personal cards rather than creating another redundant task.
