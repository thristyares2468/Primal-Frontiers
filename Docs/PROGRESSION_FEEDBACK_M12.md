# M12 owner progression feedback

October10,2026. Read-only owner XP/level/available points are now in the centered crafting screen, with the selected recipe's one-time reward status. The summary explicitly says point spending is not available yet. Actual cancel/failure earns nothing; actual first successful completion earns20XP; later completions still make items without further XP. At2700XP the view says Level cap; near cap it shows the actual remaining award rather than20. Missing owner/invalid read-only point accounting is unavailable, never a reward or fabricated level.

No new assets, RPCs, grants, save fields, recipe restrictions, external dependencies or knowledge purchases. Level thresholds share FPFProgressionTransactions::ExperienceForLevel rather than a separate UI economy. Refresh uses the existing10Hz crafting view and owner-private component already checked with one/two-client save/reconnect in PLAYER_PROGRESSION_M12.md.

## Verification

Editor build PASSED19.56s and Development Game PASSED24.94s, no compiler warnings: Saved/Logs/PFM12ProgressionFeedbackEditorBuild.log and PFM12ProgressionFeedbackGameBuild.log. No failed build/test in this slice; technical gate passed,full milestone/human acceptance remains open.

Native3/3 PASSED: PF.UI.ProgressionDetails,PF.Progression.Component,PF.Progression.Records. Automation_M12ProgressionFeedback_20261009_191446900_2837ea86,16.67s,working/private2.923/2.787GiB,raw/test severity0,engine/strict0,no fatal/ensure/timeout. Formatter covers all ten boundary thresholds,cap/partial reward,missing owner/invalid accounting,deduplication/no selection and read-only conservation; component/record fixtures retain prior actual craft/reconnect/persistence authority coverage.

Rendered PF.UI.ProgressionFeedbackLive1/1 each at maxHUDScale1.5:

| Resolution | Run | Seconds | Working/private GiB |
| --- | --- | ---: | --- |
|1280x720|M11Progression720_20261009_191525464_90a89cbe|39.94|3.205/4.800|
|2560x1440|M11Progression1440_20261009_191621261_70457efa|37.73|3.347/5.264|

Actual isolated standalone PlayerState/native HUD, real Slate Enter/R/C, timed tool cancellation/first completion/repeat,another uncredited recipe,Pause after close and geometry checks passed. First tool20XP stays20 after repeat; developer input grants/cancel0. Level10 screen uses an explicitly seeded trusted fixture record, not a claim a player earned2700XP or pacing is tested. All engine/strict exits0,test warnings/errors0,no timeout/crash/fatal/ensure. Defaultconfig one file and232/233 unrelated personal save files unchanged by SHA256/presence guards.

All8 screenshots were inspected: first_craft_available.png,first_craft_earned.png,different_recipe.png,level_cap.png for each run. Stored Saved/AutomationReports/ControlsUI/<run>/. Reports Saved/AutomationReports/<run>/index.json and run-summary.json; unique raw logs Saved/Logs/<run>.log. Native report follows the same index/run-summary pattern,raw Saved/Logs/PF<native-run>.log.

Both rendered raw logs retain26knownwarnings:21EditorDataStoragewidget entries,3missing WorldPartitionHLODUtilities/classes and2scalability priority notices preserving blur/DOF0.14Python startup error lines from experimental ToolsetRegistry/StateTreeToolset missing PythonTestRunner/ToolsetDefinition remain. Compared normalized unique warning/error lines against M12Protection1440_20261009_103335442_3f0bea5f.log:0new severity. Raw logs are not clean. Ordinary Saved/Logs/PrimalFrontier.log is stale04:53UTC; use unique matching abslogs above.

## Replay and limits

Level: /Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld. Native fixtures need no manually opened level. Start here: close Unreal to avoid competing instances,then run Scripts/RunNativeAutomation.ps1 -TestFilter 'PF.UI.ProgressionDetails+PF.Progression.Component+PF.Progression.Records' -Label M12ProgressionFeedback. Build Editor first after source changes; stop/fix/replay any failed test.

Run Scripts/RunControlsAutomation.ps1 -TestCase Progression -Resolution 720 -HUDScale1.5 (PowerShell argument is `-HUDScale 1.5`),then1440onlyafterpass. Disposable identity/config and unique artifacts; standalone actualrendering, t.MaxFPS0/r.VSync0 requested by runner. Inspect allfour PNGs,strictreports,personalfileguards and rawlogs. No sustained route FPS/hitch measurement,mouse button hit-testing,physical controller/human acceptance or16GBminimum claim. Host31.93GiB,installedengine5.8.3 despite requested5.8.2.

For the existing Personal UI guideG2, open C on that map. Read level/XP/points; after a first ordinary craft reopen to see+20XP and already-earned text; repeating cannot earnXP again. An existing older save startszeroXP and gives no retrospective credit for carried gear. No new Personal assignment or inferred pass. FullM11/M12 remains incomplete. Next independent scope: server-validated knowledge spending with a useful optional recipe effect and save/reconnect tests; preserve baseline survival access and older gear.

Source: UI/PFProgressionDetails.h/.cpp,Tests/PFProgressionDetailsTests.cpp,Tests/PFProgressionFeedbackLiveTests.cpp,Crafting/PFCraftingHUD.h/.cpp,Progression/PFProgressionRecord.h/.cpp,Scripts/RunControlsAutomation.ps1. Docs/CURRENT_STATE,MILESTONES,PROGRESSION_PLAN,TRELLO_SYNC and existing guide/card updates track handoff. Trello: https://trello.com/c/mKHbaAu0.
