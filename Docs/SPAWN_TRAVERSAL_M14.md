# M14 — climbing fixtures beside spawn, October 13, 2026

The existing 80cm and 110cm lane A blocks and their sign now sit immediately east/right of the normal spawn in `/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld`. This replaces the previous trip south-east to find climbing geometry.

- Both PlayerStarts remain at X ±250, Y -1100, Z100, facing north.
- First wall centre: `(650,-1100,40)`, scale `(3,3.6,0.8)`; its west face is2.5-7.5m from the starts.
- Second wall centre: `(1250,-1100,55)`, scale `(3,3.6,1.1)`;6m east of the first.
- Sign: `(500,-850,160)`, facing west. Text starts **CLIMB TEST: beside spawn**.
- Southern lane B stays at `(1000,-3000,40)` and `(1600,-3000,55)`.

Use flat ground at a west face: hold W, briefly press/release Space, keep moving into the face, then release W after landing. Have at least35stamina and no overweight restriction. F remains the manual alternative. These are clear low ledges; walls above120cm and blocked landings remain deliberately refused. No gameplay rule changed.

## Scope and safeguards

`AddOpenWorldTraversal.py` supports exactly one migration from the known published positions/text. It refuses custom/partial courses, calls Modify before moving existing World Partition actors, saves through the Editor, reloads, compares all76 unrelated actor configurations and preserves all five course GUIDs. Valid repeats perform no mutations or saves. Only the map and three existing external actor packages changed; no new actor, mesh, material, resource/spawner record, save schema, dependency, rendering setting or external art was added. Old course coordinates/evidence are retained in LEDGE_TRAVERSAL_M14.md as historical.

`PFAuthoredClimbLiveTests.cpp` now validates the near-spawn coordinates and both normal starts'2-8m clear approach distance. Real jump/forward movement uses the actual saved geometry. The rendered fixture captures the normal spawn view before repositioning for controlled physical tests. Stage health97/96/95, sprint acknowledgements and disabled needs/recovery are test coordination, not normal climb damage or a human playtest.

AGENTS.md now requires a clearly marked reachable nearby test fixture and exact instructions for every new playable feature. Backend-only changes may use automated fixtures; persistence layout changes require an explicit migration plan. This request is a small M14 accessibility repair, not acceptance of the full world expansion milestone.

## Fresh verification

Paths below are relative to the project. All selected test records passed with zero test warnings/errors and process exit0; no timeout, fatal, ensure or assertion. Four distinct selectors produced nine execution records across native/network/rendered runs.

| Check | Result | Evidence |
| --- | --- | --- |
| Supported Editor relocation/reload | PASS,3 moves,76 unrelated configurations unchanged,5 stable GUIDs | Saved/AutomationReports/M14SpawnWall_20261013/retry.json; Saved/Logs/PFM14SpawnWallRetry.log |
| Fresh-process repeat | PASS,0 mutations/saves, same configuration/GUIDs | Saved/AutomationReports/M14SpawnWall_20261013/repeat.json; Saved/Logs/PFM14SpawnWallRepeat.log |
| PrimalFrontierEditor Development Win64 | PASS6.72s, no compiler warnings | Saved/Logs/PFM14SpawnWallEditorBuild.log |
| PrimalFrontier Development Win64 | PASS14.07s, no compiler warnings | Saved/Logs/PFM14SpawnWallGameBuild.log |
| PF.Traversal.AutomaticClimb, PF.Traversal.Rules, PF.World.Resources | PASS3/3,17.07s, raw severity0 | Saved/AutomationReports/Automation_M14SpawnWallNative_20261013_035319983_22f85671/index.json and run-summary.json |
| PF.Traversal.AuthoredLive, uncooked server + one NullRHI client | PASS2/2,44.64s | Saved/AutomationReports/M14AuthoredClimb1_20261013_035344219_5b5cfc7a/run-summary.json |
| Same, server + two NullRHI clients, confirmed75ms outgoing lag/1% loss | PASS3/3,53.24s | Saved/AutomationReports/M14AuthoredClimb2_20261013_035503213_5c9cb136/run-summary.json |
| Same, rendered standalone1280×720, HUD1.5, uncapped requested | PASS1/1,44.42s | Saved/AutomationReports/M11AuthoredClimb720_20261013_035607948_06ad711f/run-summary.json |

All five rendered PNGs were inspected in `Saved/AutomationReports/ControlsUI/M11AuthoredClimb720_20261013_035607948_06ad711f/`: spawn_wall.png, camp_approach.png, climb_80_top.png, climb_110_top.png, course_sign.png. They show the nearby wall/sign, first-person actual landings and stamina100→65→30. Existing black sky remains greybox presentation.

Native peak working/private memory2.941/2.779GiB; network peak1.788/1.781GiB per process; rendered3.125/4.874GiB. The runners preserved447/448/450 existing private files and default settings. No sustained FPS/stuttering or minimum16GB certificate is claimed. Actual host reports31.93GiB RAM; installed engine5.8.3 differs from requested5.8.2.1440p remains deferred; packaged Server distribution limitation remains. No manual controller/audio or fullM14 acceptance is inferred.

Network raw logs contain24 known warnings and14 Python startup error lines per role; rendered26/14. Normalized severity sets contain no new entries against previous authored one-client Server and rendered checkpoints respectively. Existing HLOD/editor widget warnings and experimental Python ToolsetDefinition/PythonTestRunner errors remain baseline issues, not newly introduced gameplay errors. `Saved/Logs/PrimalFrontier.log` was checked: it belongs to the earlier closed editor session (11:39 local); fresh tests use the unique absolute logs referenced by their reports.

Initial `PFM14SpawnWallSetup.log` failed the post-save reload guard: transform changes had not dirtied the existing external actor packages. Engine exit0 was not accepted as success. Explicit Modify before relocation fixed persistence; retry and a separate verification-only process both passed. No gameplay/build test failed. The failed attempt and old successful checkpoint remain available.

This step did not repeat the earlier save/restart suite because persistent actor records, definitions and gameplay source were unchanged. Private hash preservation proves no mutation to personal saves; it does not certify every existing saved building layout. The previous save/restart evidence remains separate.

## Replay and next action

With no Unreal instance running:

```powershell
./Scripts/RunNativeAutomation.ps1 -TestFilter 'PF.Traversal.AutomaticClimb+PF.Traversal.Rules+PF.World.Resources' -Label M14SpawnWallNative
./Scripts/RunEcologyAutomation.ps1 -TestCase AuthoredClimb -Players 1 -Port 17998
./Scripts/RunEcologyAutomation.ps1 -TestCase AuthoredClimb -Players 2 -Port 17998 -SimulateLagLoss
./Scripts/RunControlsAutomation.ps1 -TestCase AuthoredClimb -Resolution 720 -HUDScale 1.5
```

Human next action: current G10 in TRELLO_TEST_GUIDES.md; turn right at spawn and test both heights with W+Space. Reply **CLIMB PASS** or the first issue. Other twelve accepted checks need no repeat. Trello technical task4stnnnKf closes only this repair; Personal7zjNOKJJ and MainM14 remain unverified for real input/usability. No new world expansion starts here.
