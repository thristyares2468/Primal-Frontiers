# M14 accessible camp ledges — October 13, 2026

**Layout superseded later October13:** lane A and its sign have moved immediately beside spawn at the user's request. See [SPAWN_TRAVERSAL_M14.md](SPAWN_TRAVERSAL_M14.md) and current G10 for locations and fresh verification. The original positions/screenshots/results below are retained as historical evidence.

The normal open world now has four solid primitive climbing ledges and a readable instruction sign south-east of the starting camp. This repairs the user's **CLIMB NOT TESTED — no suitable geometry** obstacle. It does not complete all of M14 or supply a human climb pass.

Level: `/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld`. Two lanes each contain an 80 cm and a 110 cm ledge. A lane uses cubes centred at `(1000,-2300,40)` and `(1600,-2300,55)`; B uses Y `-3000`. Each top is 3 × 3.6 m. The sign is at `(600,-1900,180)`, facing west. PlayerStarts remain at X ±250, Y -1100. All coordinates are Unreal centimetres.

The additions use Engine Cube and existing `MI_M7Rock`, remain loaded near camp, and are stateless geometry. No new resource/spawner, simulation ID, material asset, dependency, external art, dimension, rendering quality or gameplay rule was added. The area sits south of the existing creature navigation footprint; no navigation rebuild was made. The first-person character, template assets and save schema remain unchanged.

## Asset and implementation scope

Supported Editor Python `AddOpenWorldTraversal.py` adds only the four tagged ledges and sign. It refuses unexpected/partial existing courses, verifies geometry/text and persistent GUIDs after disk reload, compares all 76 pre-existing actor configurations and performs no save on a valid repeat. `AuditOpenWorldTraversal.py` is read-only. Git contains the existing `.umap` change and exactly five new external actor packages; no old external actor package changed.

`PFAuthoredClimbLiveTests.cpp` registers opt-in `PF.Traversal.AuthoredLive`. It loads the actual saved actor tags, locations, scales and collision profiles; **it creates no replacement geometry**. Owners issue real jump/forward movement, the server verifies physical landing and one jump-plus-climb stamina cost on both heights, and existing saved-move sprint intent acknowledges stages. Direct client mantle mutation is refused and no inventory is granted. Needs drain/recovery, initial placement and health stage markers are fixture isolation, not a complete normal survival route or climb damage. The test is development-only and excluded from Shipping.

Existing ecology/controls runners gain `AuthoredClimb`, exact selector/opt-in flag and OpenWorld path. Headless runs use process-only 120 FPS pacing; rendered 720p requests uncapped FPS/VSync off. Private profiles/saves/settings are hash-guarded. No persistent user world is loaded or overwritten by automation.

## Verification

Paths are project-relative. Report paths below are under `Saved/AutomationReports/`; corresponding unique logs are under `Saved/Logs/`.

| Check | Result | Evidence |
| --- | --- | --- |
| Initial current Editor build | PASS 31.39 s | `PFM14LedgeBaselineBuild.log` |
| Authored-test Editor build | PASS 14.99 s | `PFM14AuthoredEditorBuild.log` |
| Final screenshot-only fixture Editor build | PASS 6.10 s | `PFM14FinalEditorBuild.log` |
| Development Win64 game build | PASS 30.29 s | `PFM14LedgeGameBuild.log` |
| Supported Editor reload/repeat checks | PASS, old76 unchanged, five stable GUIDs, repeat mutations0 | `M14LedgeSetup_20261013/canonical.json`, `repeat.json`; `PFM14LedgeCanonical.log`, `PFM14LedgeRepeat.log` |
| Native AutomaticClimb / Rules / World.Resources | PASS 3/3, 17.23 s | `Automation_M14LedgesNative_20261013_023620453_1dc38e57/index.json`, `run-summary.json` |
| Authored climb, server + one NullRHI client | PASS 2/2 role records, 44.32 s | `M14AuthoredClimb1_20261013_023724343_b33ca2d4/run-summary.json` |
| Authored climb, server + two NullRHI clients, confirmed75ms outgoing lag /1% loss each role | PASS 3/3 role records, 53.27 s | `M14AuthoredClimb2_20261013_023852163_c18b0b60/run-summary.json` |
| Final rendered standalone720p/HUD1.5, actual saved ledges/sign | PASS 1/1, 44.42 s | `M11AuthoredClimb720_20261013_024733262_0d1cde31/run-summary.json` |
| Ordinary authored ecology Save / separate new-server restart, one client | PASS 4/4 role/phase records | `M13AuthoredLive1_20261013_024219602_6e23dc48/run-summary.json` |
| Same new-server restart, two NullRHI clients75ms/1% | PASS 6/6 role/phase records | `M13AuthoredLive2_20261013_024911470_3596f81e/run-summary.json` |

This final checkpoint has **19 reported executions across five unique selectors**, including repeated roles/phases. They are `PF.Traversal.AutomaticClimb`, `PF.Traversal.Rules`, `PF.World.Resources`, `PF.Traversal.AuthoredLive` and `PF.Creatures.AuthoredLive`. The restart regression proves the ordinary authored ecology/world-save path still restores residents and accepts the unchanged persistent layout; it is not a new certification of every personal save, general migration or all inventory/storage scenarios.

All selected test records/exits pass with zero test warnings/errors, no timeout/fatal/ensure/assertion. All compiler logs are free of compiler warnings. Native raw severity0. Network logs have24 raw warnings/14 known Python startup errors per role, with zero new normalized severity lines versus the previous authored ecology baseline. Rendered has26/14, with zero new lines versus the prior automatic-climb renderer baseline. Conventional `PrimalFrontier.log` was inspected and is the earlier closed editor log; these runs write unique absolute logs.

Peak native working/private memory: 2.957/2.832 GiB. Climb network peak: 1.800/1.792 GiB. Restart network peak: 1.791/1.781 GiB. Final rendered: 3.231/5.434 GiB. Private hash guards preserve432/433/437/440/442 existing files across these final runners; default settings are unchanged. The host reports31.93GiB physical RAM and installed engine5.8.3 versus requested5.8.2. No sustained FPS/minimum16GB or physical-device certificate.1440p remains explicitly deferred; packaged Server remains unsupported by the installed distribution.

The four final inspected PNGs are in `ControlsUI/M11AuthoredClimb720_20261013_024733262_0d1cde31/`: `camp_approach.png`, `climb_80_top.png`, `climb_110_top.png`, `course_sign.png`. They show first-person approach, real landings, stamina100→65→30 and readable sign. Health97/96/95 and damage messages are trusted fixture coordination, not real climb damage. The black sky is the existing greybox presentation. An earlier successful render (`M11AuthoredClimb720_20261013_024040862_3d725107`,45.96s) had three inspected PNGs but lacked sign proof and captured the approach before HUD refresh; the final capture-only replay corrects that. Native/headless records precede this final rendered-only wait/overview change; their headless timing/path was deliberately retained.

## Retained setup failure

Initial `PFM14LedgeSetup.log` saved the five actors but failed its post-reload guard. Engine exit0 was not accepted as script success. A first GUID-representation retry (`PFM14LedgeSetupRetry.log`) also failed. The guard used Python `str()` for UObject/GameplayTag/Guid wrappers containing memory addresses. Independent before/after audits showed the old76 rows differed only in those process addresses, not recorded configuration/identity. The repaired guard uses asset paths, `GameplayTagLibrary.get_tag_name` and `GuidLibrary.conv_guid_to_string`; canonical reload and repeat both pass. Prior temporary-name explanations were hypotheses, not established map changes. No failed gameplay test or compiler gate occurred in this step.

## AI replay

No other Unreal instance may be running; use fresh report/profile slots and available ports. Editor setup report paths must be fresh, supplied through `PF_TRAVERSAL_REPORT`; replay the supported scripts rather than edit binary packages.

```powershell
./Scripts/RunNativeAutomation.ps1 -TestFilter 'PF.Traversal.AutomaticClimb+PF.Traversal.Rules+PF.World.Resources' -Label M14LedgesNative
./Scripts/RunEcologyAutomation.ps1 -TestCase AuthoredClimb -Players 1 -Port 17998
./Scripts/RunEcologyAutomation.ps1 -TestCase AuthoredClimb -Players 2 -Port 17998 -SimulateLagLoss
./Scripts/RunControlsAutomation.ps1 -TestCase AuthoredClimb -Resolution 720 -HUDScale 1.5
./Scripts/RunPersistenceAutomation.ps1 -Players 1 -AuthoredEcology -Port 17989
./Scripts/RunPersistenceAutomation.ps1 -Players 2 -AuthoredEcology -Port 17989 -SimulateLagLoss
```

The next human action is only **G10 CLIMB** in [TRELLO_TEST_GUIDES.md](TRELLO_TEST_GUIDES.md). Previous twelve passes need no repeat. No fullM14/world expansion, new tiers, dimensions or art milestone is accepted by these technical results. Further world expansion waits for the actual climb/route observation; independently actionable future prerequisites retain their separate scopes.
