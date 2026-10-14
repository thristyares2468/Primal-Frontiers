# M14 - Full-height stamina climbing

**Historical prepaid implementation, superseded October14 by [CONTINUOUS_CLIMB_M14.md](CONTINUOUS_CLIMB_M14.md).** User CLIMN_STAMINA PASS accepts the old requested4mF only. Current F attaches, W/S/A/D moves,4/s hold and12/s movement drain; zero drops. Do not use the older cost/preflight instructions below for the current build. New manual gate G16/WALL_CLIMB.

The October13 request supersedes the old45-120cm ledge limit. F from grounded wall contact, or W + Space during a rising paid jump, requests a full climb. The server finds the wall/top and checks a walkable landing and swept capsule/overhead clearance. No target, height or cost is accepted from the client. Death, menus, encumbrance and cooldown still refuse.

Cost is15 stamina through1.2m, then10 per additional metre. W + Space additionally pays the ordinary20-stamina jump. The entire cost must be affordable before launch; a refusal spends no climb stamina. A100-stamina survivor can therefore use F on roughly9.7m, or jump-contact climbing on roughly7.7m. Greater stamina capacity increases reach; there is no arbitrary gameplay wall-height ceiling. Coordinate/finite-value safety limits remain. The legacy serialized `MaxClimbHeight` property now means base-cost height and is displayed accordingly.

The greybox movement is a swept ascent and mantle using Unreal CharacterMovement, with no wall-hanging, crawling animation or arbitrary collision bypass. Horizontal drive is budgeted for the interval above the wall, not the whole blocked ascent. Landing, cancellation, new jumps and death clear the bounded drive. The owning HUD displays the actual height/cost or a specific refusal.

## Saved fixtures

Level `/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld`. Turn right/east at normal camp spawn. Existing80/110cm blocks are retained. New4m wall at `(1850,-1100,200)`,10m wall at `(2450,-1100,500)`, and sign at `(1550,-850,200)`. Multiplayer lane B4m wall is `(2250,-3000,200)` to avoid the existing110cm block. Existing Engine Cube/greybox rock material only. No resource/spawner identity or configuration changed.

Supported `Scripts/AddClimbBudgetFixtures.py` validates exact known geometry, saves/reloads stable GUIDs and preserves all81 old actor configurations. A narrowly guarded relocation of the newly created lane B wall is allowed from its initial obstructed position; repeated execution is read-only. Setup report `Saved/AutomationReports/M14ClimbBudgetSetup_20261013/setup.json`, relocation `relocate.json`.

## Verification in progress

Editor builds pass. Native `PF.Traversal.Contact` now includes80/110/400/800cm physics landings and exact costs. Initial8m manual case failed to land safely on top; the above-top horizontal timing repair passed its focused replay (`Automation_M14ClimbBudgetTiming_20261013_053744418_15422ca3`, raw severity0).

Saved-map actual-key `PF.Traversal.AuthoredLive` uses optional `-FullHeightClimb` on the isolated runners. One-client/server manual and automatic runs passed2 records each:

- `M14HeightAuthoredClimb1_20261013_054939796_9a5898ed` F
- `M14HeightAuthoredClimb1_20261013_055043795_5bb05808` W + Space

Two impaired-network runs failed before the second climb: initially a hard10cm reposition assumption was suspected; replay showed the older110cm lane-B block physically obstructed the approach. Retain `M14HeightAuthoredClimb2_20261013_055153900_6c7f9364` and `M14HeightAuthoredClimb2_20261013_055621771_621bccd1`. The fixture now accepts only the bounded clear approach corridor, requires real single-attempt contact inputs and keeps clients connected until all server lanes complete. The new lane-B4m wall was relocated through supported Editor APIs with stable GUIDs/old81 unchanged. Final two-client/render/native/Game/repeat/log gates pending. No user pass or fullM14 acceptance inferred.

## Final technical evidence

Native `Automation_M14FullHeightFinal_20261013_061158678_39628487` PASS3/3 (Contact, AutomaticClimb, Rules),19.23s, raw severity0, peak3.010working/2.857privateGiB. Two-client75ms/1% final `M14HeightAuthoredClimb2_20261013_060854577_42002133` PASS3/3,58.33s, no test warnings. Raw24warnings/14known experimental-Python error lines per role; normalized severity comparison to previous baseline adds zero lines. One720p F/HUD1.5 rendered `M14Height720_20261013_061011129_af65037c` PASS1/1,53.12s, peak3.521working/5.438privateGiB, no new normalized severity.4m landing and15-stamina refusal PNGs inspected. Screenshot request/camera teleport ordering corrected for a final capture-only replay. Uncapped requested, not an FPS benchmark or1440p/hardware certification.

Game build `PFM14FullHeightGameBuild.log` PASS50.37s; Editor incremental builds all pass, latest capture build alsoPASS. Script repeat report `repeat.json` mutations0/stable4GUIDs/old81 unchanged. Private/default-settings guards passed every run. Installed Engine Build.version is5.8.3 in UE_5.8, not requested5.8.2; no engine modification. PackagedServer remains unsupported by installed distribution.

Additional failed fixtures retained: `060044202_75c098ff` exposed unrelated4.3 sprint drain from the Shift readiness cue; disable sprint boost/drain ONLY in this isolated test, alongside existing needs/recovery isolation. `060529811_b09d8d21` had all physical landings/server costs pass but owner completion95/94 snapshots coalesced; distinct replicated view-yaw acknowledgement now gates all-role exit. Tests still use actual single W/F/Space inputs,20-stamina jump and full height cost, no substitute geometry/climb calls, owner-visible refusals and client authority checks. These are not production sprint fixes or human acceptance. Current separate stat-upgrade objective remains pending.

## Stamina-upgrade integration and final landing completion

The later10m rank1-capacity120 test found that Unreal could perch the rounded capsule on a walkable edge before the feet reached the top, clearing the old drive too soon. Preserve the bounded approved drive until a validated full landing target is reached, through normal swept walking/step-up, with a1s completion margin. The owner RPC carries only the server-approved target; no client target/cost input is introduced. Death/cancel/newjump/reached landing/expiry clear it. Failure/replay details are in PLAYER_STATS_M12.md.

Final native8/8 Automation_M12StatsFinalNative_20261013_071721184_16f26d6a PASS/raw0: low80/110cm,4m/8m physics plus10m after upgraded stamina, full top atX107.888/Z1098.05 and drive0/cost102.8. Actual rendered saved10m F passed M12StatsUI720_20261013_073212143_3c179b6c,46.54s/3.412working5.457privateGiB; stats_10m_climb.png inspected. Current one-client2/2 M14HeightAuthoredClimb1_20261013_072158028_c8824f54 and two-client3/3 M14HeightAuthoredClimb2_20261013_073755369_90197f8c PASS with75ms/1%, normal saved80/4m rawW+Space and exact costs. No private/default mutation. G10 now tests4m F; G14 separately checks an earned stat point. Old fixed-height instructions are superseded, not accepted as a human pass.
