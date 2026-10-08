# M8 post-fix verification — October 8, 2026

Checkpoint source: 31dce6b, including startup save protection (ada477c), rejected-load verification (a2e6548), corpse collision (6761ad3) and doorway/hidden-shape collision (31dce6b). This is automated verification, not full milestone acceptance. Installed engine is 5.8.3; no engine migration or asset change.

Editor and Development Game incremental build checks passed, up to date in 1.13/1.11 seconds. No compile actions or warnings; logs PFM8PostFixUnityEditorBuild.log and PFM8PostFixGameBuild.log. These are build checks, not new unity compilation. The actual collision-fix compilation results are in PERSISTENCE_M8.md.

## Native tests

Reports under Saved/AutomationReports:

- Automation_M8PostFixNative_20261008_052215655_f0fd1818/index.json: 27 passed, 0 failed, 0 test warnings. Engine/runner exits 0; raw log clean. 16.65 s; sampled working/private 2.979/2.832 GiB.
- Automation_M8PostFixCommands_20261008_052331100_6d23b4d7/index.json: 3 passed, 0 failed, 0 test warnings. Engine/runner exits 0; raw log clean. 16.07 s; sampled working/private 2.978/2.826 GiB.

Combined: 30 distinct tests, no missing original M8FinalRegression test. The four additional cases are CorpseLootRestore, RejectedLoadPreservesWorld, StartupFailurePreservesSave and StructureCollisionRestore. Each run also has run-summary.json and a matching Saved/Logs/PF<run>.log. No fatal or ensure found.

| Test | Result | Errors | Warnings |
| --- | --- | --- | --- |
| PF.Building.PlacementAndStorage | Success | 0 | 0 |
| PF.Crafting.Gathering | Success | 0 | 0 |
| PF.Crafting.Transactions | Success | 0 | 0 |
| PF.Creatures.Lifecycle | Success | 0 | 0 |
| PF.Input.Gamepad | Success | 0 | 0 |
| PF.Interaction.TargetAndPickup | Success | 0 | 0 |
| PF.Inventory.Transactions | Success | 0 | 0 |
| PF.Inventory.WorldTransfers | Success | 0 | 0 |
| PF.Persistence.CorpseLootRestore | Success | 0 | 0 |
| PF.Persistence.CorruptPlayerData | Success | 0 | 0 |
| PF.Persistence.FileGenerations | Success | 0 | 0 |
| PF.Persistence.PlayerRoundTrip | Success | 0 | 0 |
| PF.Persistence.PlayerValidation | Success | 0 | 0 |
| PF.Persistence.RejectedLoadPreservesWorld | Success | 0 | 0 |
| PF.Persistence.ServerPlayerAdapter | Success | 0 | 0 |
| PF.Persistence.StartupFailurePreservesSave | Success | 0 | 0 |
| PF.Persistence.StructureCollisionRestore | Success | 0 | 0 |
| PF.Persistence.WorldRecords | Success | 0 | 0 |
| PF.Persistence.WorldRuntime | Success | 0 | 0 |
| PF.PrimalAgentTools.CommandArguments | Success | 0 | 0 |
| PF.PrimalAgentTools.MissingSystemsAreBlocked | Success | 0 | 0 |
| PF.PrimalAgentTools.TeleportAndRuntimeReset | Success | 0 | 0 |
| PF.Settings.Preferences | Success | 0 | 0 |
| PF.Survival.Component | Success | 0 | 0 |
| PF.Survival.Environment | Success | 0 | 0 |
| PF.Survival.Lifecycle | Success | 0 | 0 |
| PF.Survival.Needs | Success | 0 | 0 |
| PF.World.Clock | Success | 0 | 0 |
| PF.World.OpenWorldAsset | Success | 0 | 0 |
| PF.World.Resources | Success | 0 | 0 |

Retained selection failure: Automation_M8PostFixRegression_20261008_052105446_867499d7 ran 33 Success records, including two setup warnings from opt-in ScenarioIdempotence and ViewportCapture. Engine exit 0, strict runner exit 1. Those two operations did not run; no scenario-reset or screenshot pass claimed. Corrected selection above excludes that unrelated editor setup scope. No code/warning-suppression change was needed. Reproduction commands are in AUTOMATION_VERIFICATION.md.

## Live server/client restart

All runs use /Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld, an uncooked dedicated Editor server, localhost port 17984, separate clients, NullRHI/no sound/unattended/NoLiveCoding/NoSaveConfig. Automation RunTests PF.Persistence.Live and TestExit="Automation Test Queue Empty"; PFRunPersistenceLiveTests, unique AutomationM8 slot, PFExpectedPlayers=1 or 2. Restart starts a new server process with the same slot plus PFLoadSave; each client keeps its same private PFIdentityProfile and endpoint. Private profiles/payloads/credentials stay local and out of Git/Trello. The disposable orchestration helper has bounded owned-process cleanup and checks actual report/process/fatal/ensure outcomes.

One client: M8PostFix1_20261008_052403_47d05cCreateServer, CreateClient1, RestartServer and RestartClient1 all passed PF.Persistence.Live (1/1 each), zero test warnings/errors, engine/report exits 0. Create sampled working sets server/client 1.715/1.808 GiB, private 1.626/1.778 GiB; restart working 1.708/1.799 GiB, private 1.609/1.730 GiB. Phases 49.33/37.96 seconds. Real gathering/craft/storage/file save/load, authority refusals, disconnect checkpoint, process restart and reconnect validated; no fixture telemetry is a manual route test.

Two clients: M8PostFix2_20261008_052602_be9b24CreateServer, CreateClient1, CreateClient2, RestartServer, RestartClient1 and RestartClient2 all passed PF.Persistence.Live (1/1 each), zero test warnings/errors, engine/report exits 0. Create sampled working sets server/client1/client2 1.714/1.812/1.814 GiB, private 1.617/1.778/1.771 GiB; restart working 1.717/1.796/1.798 GiB, private 1.614/1.724/1.726 GiB. Phases 51.30/39.58 seconds. Client 2 passed both phases and did not crash. Independent replicated identities and restored server-owned state are verified; corpse/door collision semantics are covered by native tests, not directly exercised by the live fixture.

All ten live reports are Saved/AutomationReports/<full run name>/index.json; matching logs Saved/Logs/PF<full run name>.log. All processes finished without timeout and were inspected. The bounded Testing task is complete; parent manual milestones remain Doing. No project C++/asset/settings change for this checkpoint. Temporary diagnostic helper is outside the repo; local isolated test saves/profiles and original failure reports are retained.

Known startup log conditions on every completed live process: 24 warning lines (21 editor-widget factory registrations, three uncooked WorldPartition HLOD script/settings imports) and 14 Python traceback error lines from installed StateTreeToolset and ToolsetRegistry (missing unreal.ToolsetDefinition / unreal.PythonTestRunner). No new gameplay error, fatal or ensure observed. Automation events are clean; raw live logs are not warning-free. No engine/plugin/rendering-setting workaround is applied.

Saved/Logs/PrimalFrontier.log remains the older October 6 editor log; actual launches use the explicit unique PF log paths above. NullRHI measures no rendered FPS or stuttering, and this 31.93 GiB host does not certify a 16 GB minimum. M7 sustained route/overnight, M8 rendered full loop and packaged Server support remain open/blocked. See PLAYTEST.md for the Personal gates.
