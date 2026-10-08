# M11 server player-setup acknowledgment

October9,2026. Bounded verification passed. Human first-person/open-world acceptance remains unverified.

## Behavior

Pause distinguishes the server's new/restored survivor setup from the separate local reconnect-profile write. A reliable owner-client acknowledgment is sent by the existing deferred PostLogin callback only after RestorePlayer succeeds. Unconfirmed remains until acknowledgment; failed setup returns to menu and destroys the controller without a success acknowledgment. No new client gameplay mutation, save/load request, save schema or asset.

RestorePlayer supplies an optional restoration outcome. Transient RestoredLogins records actual successful adapter/load restoration for the current login; saving a fresh record alone never sets it. Login/logout reset that identity's observation, and a successful world Apply rebuilds it for restored connected players. Rejected setup resets the output to false. Pause updates labels without resetting focus/selection/quit confirmation. This server statement does not claim all replicated actors have arrived, that the client's pawn is ready, that world state was saved, or that development credentials are production authentication. It describes setup for this connection, not later manual world loads.

## Current evidence

Requested engine5.8.2; installed5.8.3. Initial Editor build before restoration tracking passed21.25s (M11ServerSetupEditorBuild_20261009.log). Tracked Editor build failed41.09s because a user Editor held both DLLs: UBA9001 then LNK1104; retained M11ServerSetupTrackedEditorBuild_20261009.log. User closed Editor; retry passed2.63s without compiler warnings (M11ServerSetupUnlockedEditorBuild_20261009.log). Development Game passed26.60s without compiler warnings (M11ServerSetupGameBuild_20261009.log).

| Check | Report directory under Saved/AutomationReports | Result | Seconds | Sampled working/private GiB |
| --- | --- | --- | --- | --- |
| Four native persistence regressions | Automation_M11ServerSetup_20261008_230008322_e784d941 | 4/4 passed | 19.70 | 3.014/2.894 |
| PF.UI.ReconnectProfileLive,1280x720 | M11Reconnect720_20261008_230050506_5b0aba76 | 1/1 passed | 30.54 | 3.009/3.991 |
| PF.UI.ReconnectProfileLive,2560x1440 | M11Reconnect1440_20261008_230131363_e6551325 | 1/1 passed | 30.66 | 3.193/4.386 |

Native tests: PF.Persistence.ActiveCraftCancellation, PF.Persistence.OfflineFoodAging, PF.Persistence.RejectedLoadPreservesWorld, PF.Persistence.StartupFailurePreservesSave. ActiveCraftCancellation now checks real fresh login, save-only false restoration, actual Load/reconnect true restoration and rejected null setup clearing stale output; it also retains crafting/ingredient conservation checks. Other three retain actual file/food/refusal safety coverage. All engine/strict exits0; zero test warning/error events. Native raw log also clean.

Six actual screenshots inspected: Saved/AutomationReports/ControlsUI/<rendered run>/{saved,failed,retry}.png. Actual fresh server label remains separate through saved/rejected/retry profile feedback; pause bounds/focus/quit-state and no credential exposure checked. Profile failure is an explicitly expected invalid credential rejection, not a simulated disk-full failure. RequestedHUDScale1.5 metadata does not prove the fixture applied that scale; pause uses fixed typography. An initial mistyped -Resolution720 invocation was rejected by PowerShell before any Unreal launch, corrected to -Resolution 720.

Both rendered raw logs retain known21 LogEditorDataStorageUI+3 LogLinker/HLOD warnings and14 LogPython error lines; no other severity,ensure,fatal,crash or VSM overflow. Test reports are clean despite these startup issues. Logs are unique Saved/Logs/<rendered run>.log and PFAutomation_<native run suffix>.log; PrimalFrontier.log remains unrelated/stale for these launches.

## Real multiplayer restart verification

Scripts/RunPersistenceAutomation.ps1 -Players 1 -Port 17989 -TimeoutSeconds 210 PASSED4/4, then -Players 2 -SimulateLagLoss -Port 17989 -TimeoutSeconds 210 PASSED6/6. All are PF.Persistence.Live Success with engine/strict0,test warnings/errors0 and no timeout. Both use /Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld, isolated profiles/slots, uncooked dedicated server and NullRHI clients. Two-client settings were confirmed by the runner:75ms outgoing lag/1% loss. Existing gather/craft/build/file save/load/restart/ownership/storage,invalid inventory RPC and owner/foreign privacy assertions remain passed.

Exact report directory prefixes:

- M8Live1_20261008_230303515_0044a238: suffixes CreateServer,CreateClient1,RestartServer,RestartClient1. Phase times49.75/38.33s. Server working/private1.715–1.718/1.612–1.623GiB;client1.790–1.807/1.721–1.774GiB.
- M8Live2_20261008_230444609_b49f5187: suffixes CreateServer,CreateClient1,CreateClient2,RestartServer,RestartClient1,RestartClient2. Phase times51.56/40.09s. Server working/private1.717–1.719/1.620–1.630GiB;clients1.798–1.817/1.718–1.781GiB.

Each role's index.json is Saved/AutomationReports/<prefix><suffix>/index.json; aggregate run-summary.json is under the unsuffixed prefix directory. Logs: Saved/Logs/PF<prefix><suffix>.log. All ten raw logs have exactly known21 widget+3 linker/HLOD warnings and14 installedPython error lines,no other warning/error category or ensure/fatal/crash. Each actual Create client logs one sanitized restored=0 acknowledgment; each Restart client one restored=1,with no opposite outcome. Dedicated servers execute no local-client UI callback. No Client2 headless crash occurred.

For replay,close the user's Editor first; use the existing disposable runners and exact filters above. The old retained DLL-lock failure is not a code failure. Do not upload credentials/private saves/full network URLs.

Changed source: survival controller enum/RPC/getters,GameMode deferred callback,Pause labels,WorldPersistence transient outcome tracking,ActiveCraftCancellation native and reconnect-profile/live-network assertions. Documentation: this file,CURRENT_STATE,MILESTONES,DECISIONS,UI_CONTROLS_PLAN,RECONNECT_FEEDBACK andTRELLO_SYNC; user guides also clarify observable server acknowledgment. No assets,save schema,new gameplay dependency or production authentication changed.

Next independent M11 slice: bounded settings Apply/display confirmation isolation and verification; current Cancel evidence does not cover it. Human acceptance stays separate under Personal guide cards G1–G4/G7.

## Limits

Rendered fresh-menu screenshots plus headless restored-client state do not certify a rendered multiplayer restored screen, a network restore-failure injection, sustained walking/overnight, physical controller, production authentication or16GB minimum performance. No FPS/stutter pass inferred from short automation. Packaged Server remains blocked by installedengine distribution; uncooked Editor-server path is separate.
