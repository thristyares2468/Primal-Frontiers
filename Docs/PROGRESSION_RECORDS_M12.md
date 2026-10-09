# M12 — bounded XP and knowledge record foundation

October 9, 2026. Pure native transaction foundation, not live player progression. No player component, grant RPC, reward event hook, knowledge menu, recipe restriction or save-format change. Existing recipes remain available. Trello: https://trello.com/c/K9NYY2ad. Full M12/Personal M8/M11 remain open.

## Implemented contract

`FPFProgressionRecord` stores cumulative XP, learned stable knowledge IDs and first-craft reward IDs. It does not redundantly store level or points. `FPFProgressionTransactions` validates a complete candidate then commits once; refusal leaves the caller's record unchanged. These pure C++ functions require a trusted server caller and do not themselves prove a real successful event or enforce networking. There is no Blueprint or client grant function.

The original provisional ten-level curve is100,250,450,700,1000,1350,1750,2200,2700 cumulative XP. Three points per attained level after level1 gives27 at level10. Valid positive XP additions are bounded<=2700 and saturate at2700 without integer overflow; invalid/zero/negative/overflow input and awards at cap refuse. First craft of a validated recipe credits20XP once; at cap its identity is still recorded once, preventing later replay. Failed/cancelled crafting awards nothing in normal gameplay because there is no event hook yet, not because these pure records can prove completion. Native fixtures alone call these transactions.

`UPFProgressionCatalog` is a designer-facing native data asset definition with registered Progression.Knowledge/Tool tags. One default optional metadata entry, Tech_FieldTools (Field tool knowledge), requireslevel2/cost2 and refers to existing Recipe_BoundTool. It does not gate that recipe. No future station, adaptation or invented content fixture is exposed as available. IDs are bounded ASCII Tech_ names;32knowledge definitions/128craft records maximum. Duplicate IDs, unknown references, missing/repeated/self prerequisites, cycles, repeated recipe assignments, invalid levels/prices/tags and prerequisite closures unaffordable at declared minimum level are rejected. Baseline Recipe_Tool/Cook/Dry cannot be assigned a knowledge gate. Learned records require valid level, full prerequisite closure, unique IDs, enough earned points and consistent credited XP. AvailablePoints fails closed for malformed IDs/costs/duplicates/range rather than overflowing.

The wider six-technology/XP sources/respec/discovery plan in PROGRESSION_PLAN.md remains a proposal. This small native catalog intentionally introduces no station or new recipe. Gather reward windows, first-building/discovery events, respec and death/reconnect ownership are unimplemented. Full gameplay cannot reach the tested cap from the record fixture alone.

## Verification and retained failure

- Editor initial16.36s, pretest final5.38s, postfixture correction5.07s passed without compiler warnings. Logs:`Saved/Logs/PFM12ProgressionRecordsEditorBuild.log`, `PFM12ProgressionRecordsFinalEditorBuild.log`, `PFM12ProgressionRecordsRetryEditorBuild.log`.
- First selection FAILED `Automation_M12ProgressionRecords_20261009_104812641_69c2ca62`,18.81s,working/private3.019/2.868GiB,engine3/runner1/no valid final report. New native fixture appended a reference from its own TArray, triggering Array.h:2196 alias assertion at PFProgressionRecordTests.cpp:29. This was a test bug, not RHI or runtime progression. Three preceding regressions completed in the raw log but do not establish a successful final run. Failure artifacts retained; copy the ID before adding the duplicate fixed only the fixture.
- Failed-test-only replay PASSED1/1 `Automation_M12ProgressionRecordsRetry_20261009_105008341_a1b309e1`,16.09s,2.937/2.792GiB.
- Final original selection PASSED4/4 `Automation_M12ProgressionRecordsFinal_20261009_105111554_a7175097`,16.12s,2.918/2.794GiB. Exact tests PF.Progression.Records,PF.Crafting.Transactions,PF.Persistence.PlayerRoundTrip,PF.Persistence.WorldRecords. Both successful runs have engine/runner0,raw/test warnings/errors0,no assertion/ensure/fatal. Actual XP boundaries/cap/multi-level accounting, first-craft identity/capped dedupe, optional/duplicate/prerequisite purchases, corrupt bounds/IDs/cycles/baseline access and existing crafting/save regressions passed.

Reports:`Saved/AutomationReports/<run>/index.json` and `run-summary.json`; logs:`Saved/Logs/PF<run>.log`. No level required; fixture constructs native catalog defaults and isolated plain records. This is not a loaded designer override/UI/rendered/multiplayer playtest. No screenshots or FPS measured. Host31.93GiB; NullRHI sampled memory does not certify16GB. Installed engine5.8.3 rather than requested5.8.2; packaged Server restriction unchanged.

Development Game build PASSED22.88s without compiler warnings:`Saved/Logs/PFM12ProgressionRecordsGameBuild.log`. Successful native log severity and build logs reviewed. Bounded Trello evidence/Done readback follows; broader gates stay open.

Replay after closing other Unreal sessions:

```powershell
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunNativeAutomation.ps1 -TestFilter 'PF.Progression.Records+PF.Crafting.Transactions+PF.Persistence.PlayerRoundTrip+PF.Persistence.WorldRecords' -Label M12ProgressionRecords
```

## Next required integration gate

First implement a separately tested bounded progression codec and deliberate V1 compatibility path, preserving vitals/identities/inventory/freshness/ownership and never generating retrospective XP. Unknown/future/truncated data must refuse without replacing live state or private saves. Only after that gate attach a focused server-owned PlayerState component and real successful craft/gather hooks, owner-only replication, purchase validation, readable UI and one-/two-client restart tests. Baseline survival must remain playable without points. No M13/adaptations advance.

Files: Progression/PFProgressionCatalog.h/.cpp, PFProgressionRecord.h/.cpp, Tests/PFProgressionRecordTests.cpp, current-state/milestone/architecture/decision/progression/Trello evidence docs. No existing gameplay source, assets, settings, recipes or save codecs modified by this bounded task.
