# M12 — optional owner-bound world progression archive

This is the earlier archive-only checkpoint. The later PLAYER_PROGRESSION_M12.md integration deliberately replaces the temporary runtime V2 refusal and normal V1 writer described below, after actual component/legacy/reconnect/native/live gates. Historical archive evidence remains intact.

October9,2026. This bounded native compatibility step accepts explicit world V2 metadata while retaining the normal runtime V1 writer. No live XP, player component, rewards, recipe restriction or UI added. Full M12 and the Personal M8/M11 gates remain open. Trello https://trello.com/c/dgafRfoo.

## Contract

FPFWorldPlayerRecord has an optional Progression string. Normal V1 encoding omits that reflected field entirely; strict reads insert the explicit empty legacy default. A V1 record containing nonempty progression refuses rather than silently losing it. Existing V1 player bytes, reconnect credentials, inventory/storage batches, structure IDs/support/ownership, timestamps and food freshness retain their existing rules. Existing inventory is not proof of crafting; legacy defaults never award retrospective XP.

An explicitly selected world V2 requires a separate owner-bound progression envelope for every player. PackProgression prepends the exact player GUID to the verified16KiB PFXP envelope, then Base64 encodes it. Reads bound the encoding/decoded allocation, compare the expected owner and validate the inner version/length/CRC/known catalogs/record accounting before committing output. Owner binding plus the outer save-file checksum prevents accidental swapping/corruption, not forgery/authentication; only trusted server save files become state. No arbitrary object/class paths or unknown-name interning added. A second player's zero state remains independent.

DecodeValidated constructs and validates a complete world candidate before replacing any output. Future world versions now start at3; unknown/corrupt/future inner progression, absent/nonstring/oversized V2 fields, malformed player objects, invalid XP and swapped owners refuse atomically. Current runtime UPFWorldPersistence deliberately refuses even valid V2 before actor mutation: `World V2 progression restoration is not integrated; load refused to preserve saved data`. This protects saved progression until a separately tested PlayerState integration can restore/capture it. Normal runtime capture stays V1; no user file migration/rewrite performed. Metadata acceptance is not gameplay restoration acceptance.

## Verification

Editor PASSED22.27s, no compiler warnings: `Saved/Logs/PFM12WorldProgressionEditorBuild.log`. Native5/5 PASSED `Automation_M12WorldProgression_20261009_111526323_3675c124`,33.5s,working/private2.961/2.805GiB,raw/test warnings/errors0,engine/runner0,no timeout/fatal/ensure. Exact tests PF.Progression.WorldCompatibility,Codec,Records,PF.Persistence.WorldRecords,RejectedLoadPreservesWorld. No build/test failure in this slice so far.

Native compatibility verifies V1 field omission/defaults; independent owner/XP/knowledge/ledger roundtrip/derived points/no reaward; existing health/location/tool/stack IDs/two food items12.5seconds freshness/storage identity/ownership/support/credential/time preserved. Negative fixtures cover future world/inner versions, owner swap, absent/nonstring/oversized fields, nonobject player, inner checksum, checksum-valid invalid XP/unknown knowledge and failed encode/decode outputs preserved. The real-file refusal fixture adds valid-but-unintegrated V2 to existing version/map/layout/unsafe-ground cases; running pawn, location, health, inventory/storage IDs/quantities, structure actors/support/owner, clock/resource, active slot and existing file generation/bytes remain intact. A subsequent V1 save still works. Native records need no saved level; transient runtime refusal fixture is L_Automation only.

One-client baseline Create/Restart4/4 PASSED `M8Live1_20261009_111620756_b4260025`: PF.Persistence.Live on server/client each phase. Exits0/test warnings/errors0/no fatal/ensure, working/private1.721–1.821/1.570–1.735GiB. Exact level `/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld`; uncooked dedicated server/NullRHI clients. These runs verify unchanged V1 authority/save/restart/private inventory/storage ownership contracts, not live progression.

Two-client75ms/1%loss baseline Create/Restart6/6 PASSED `M8Live2_20261009_111810709_5d820e56`,PF.Persistence.Live on server/Client1/Client2 each phase. Exits0/test severity0/no fatal/ensure,working/private1.718–1.819/1.564–1.742GiB. Owner and foreign-storage roles confirmed; Client2 did not crash. Game build passed (details below). Reports under `Saved/AutomationReports/<run>/index.json` (native and phase/role directories), `run-summary.json` (aggregate). Raw logs `Saved/Logs/PF<run>[Create|Restart][Server|Client1|Client2].log`. Native raw log `PFAutomation_M12WorldProgression_20261009_111526323_3675c124.log`.

All10network raw logs have24warnings and14Python-error lines per process:21EditorDataStorage factory-registration warnings,3HLOD missing-script/class warnings; two experimental StateTreeToolset/ToolsetRegistry Python traces for absent ToolsetDefinition/PythonTestRunner. Normalized unique warning/error lines compared with `PFM12ProtectionLive2_20261009_103613412_3d04833aCreateServer.log`:0new severity lines in every process. These are retained baseline findings; raw logs are not clean. Native raw log reviewed separately, severity0.

NullRHI memory is not rendered performance/minimum16GB certification. Actual host31.93GiB, installed engine reports5.8.3 despite requested5.8.2; packaged Server target unsupported by installed distribution. No screenshots/FPS/manual UI/gameplay claimed for this metadata step.

Game Development build PASSED34.88s without compiler warnings: `Saved/Logs/PFM12WorldProgressionGameBuild.log`. Editor/Game logs reviewed; no failed build/test in this bounded slice. Changed files: PFWorldSaveData.h,PFWorldSaveFormat.h/.cpp,PFWorldPersistence.cpp,PFWorldPersistenceTests.cpp,newPFProgressionWorldCompatibilityTests.cpp and evidence documents. No assets/maps/config/private saves changed. Technical gate complete; Trello evidence/Done and scoped Sol publication follow.

## Replay and next gate

Close other Unreal processes, then from the project root:

```powershell
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunNativeAutomation.ps1 -TestFilter 'PF.Progression.WorldCompatibility+PF.Progression.Codec+PF.Progression.Records+PF.Persistence.WorldRecords+PF.Persistence.RejectedLoadPreservesWorld' -Label M12WorldProgression
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunPersistenceAutomation.ps1 -Players 1
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunPersistenceAutomation.ps1 -Players 2 -SimulateLagLoss
```

Runners use disposable Automation slots/profiles; never replace personal saves. Next eligible bounded task: server-owned PlayerState progression component with private replication and explicit V1 defaults/V2 capture/restore only after full validation; verify repeated load, death/respawn and one-/two-client restart before reward or knowledge/UI hooks. Remove runtime V2 refusal only when actual restore cannot discard progression. Existing baseline recipes remain unrestricted. No M13/adaptation/art expansion inferred.
