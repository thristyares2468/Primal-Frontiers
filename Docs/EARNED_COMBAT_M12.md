# Earned protection and default creature combat — M12

October10,2026. This bounded task adds an opt-in rendered integration fixture and runner case. Production rules, catalogs, assets, saves and networking are unchanged. It extends the grant-free route in EARNED_UPGRADE_M12.md; manual combat feel, navigation, sustained FPS and full M12 acceptance remain unverified.

## Actual route

1. Start fresh at0XP with an empty inventory. Real finite-node interaction and unchanged timed crafting earn FieldTools and one bound tool:120XP/one remaining point. No XP/item grants or seeded Restore.
2. A disposable Prowler uses the loaded100HP/8damage definition. Call its actual authoritative Think to start the0.6s windup, wait and let Think land8health damage. Only test position/movement/tick are controlled; no damage/health override.
3. Use the earned tool to gather two further fibre/wood nodes normally. The extra Cord consumes4fibre in4s without another reward. WovenGuard consumes8fibre/one cord/2wood in6s, awarding its first20XP:140XP/one point/seven unique recipes,5wood remain. Its carried mitigation is25%, nonstacking.
4. A second actual default windup lands6health damage, total14loss. Actual HUD shows86health and the6damage feedback. Real registered LeftMouse binding sends the existing server melee request:45damage and5stamina per accepted hit, immediate duplicate swing refused. Three accepted hits kill100HP; combat adds no XP.
5. The creature creates exactly one3food perishable batch. Actual view trace/E interaction picks it up with the original expiration deadline. Repeated corpse attacks/pickups cannot create another item or reward. Real Tab/Down bindings select the recovered batch; the actual inventory displays its three-item freshness row and consumable details.

Menu actions use actual Slate key events and the owned server request. Recipe selection, disposable target positions and disabled automatic movement/needs are fixture controls. This is not a human walk/fight, physical mouse/controller or authored zone route certificate.

## Evidence

Final technical gate passed: PF.Progression.EarnedCombatLive1/1, M12EarnedCombat720_20261009_210553405_6071cf4b,118.98s,working/private3.185/5.174GiB. Exact selection, engine/strict/runner0, test warnings/errors0, no timeout.267personal saves/default settings unchanged. All six final PNGs actually inspected; loot picture now shows row8 Found food x3/298s fresh, carried tool/guard and selected consumable details. Actual selected detail text fits allocated height. Final Editor8.64s and Game15.51s passed without compiler warnings: PFM12EarnedCombatCaptureEditorBuild.log and PFM12EarnedCombatCaptureGameBuild.log.

Final raw log has26warnings/14experimental Python error lines,0new normalized severity versus Protection1440 baseline and0fatal/ensure. Literal creature logs show two Hit survivor damage=8; authoritative survival assertions establish8 unprotected versus6 guarded, HUD86health/DAMAGE -6.00. Completion info records three45damage accepted swings/five stamina each and one perishable loot pickup.

Native Protection and WeaponProgression passed2/2 in Automation_M12EarnedCombat_20261009_210122567_d4dff96f,16.27s,working/private3.026/2.863GiB. Exact selection/exits/test/raw severity0, no timeout/fatal/ensure. No opened level required; temporary native Game fixtures.

Initial rendered PF.Progression.EarnedCombatLive passed1/1 in M12EarnedCombat720_20261009_210153376_610ac0e6,103.48s,3.102/5.289GiB. Engine/strict/runner0, exact selection/test severity0;266personal save files and actual default settings unchanged. All six images inspected. The loot picture showed the first inventory page, so the fixture was strengthened to select/check the actual recovered batch before final replay. This was an artifact-evidence improvement, not a failed gameplay assertion. Initial Game build14.62s clean.

Retained compile failure: PFM12EarnedCombatEditorBuild.log,3.04s/exit6,C2248 private PlaceBuilding and C2027 incomplete FInputKeyParams. Fixed only fixture calls to use the existing registered key delegates and supported InputComponent header; no private API exposure/deprecated input type. Editor retry6.88s clean, PFM12EarnedCombatRetryEditorBuild.log. Loot-capture Editor8.64s clean, PFM12EarnedCombatCaptureEditorBuild.log.

## Replay

Start here: close competing Unreal processes, build PrimalFrontierEditor Development Win64.

1. Native: Scripts/RunNativeAutomation.ps1 -TestFilter 'PF.Crafting.Protection+PF.Crafting.WeaponProgression' -Label M12EarnedCombat. Require exactly two clean records and inspect raw log.
2. Rendered: Scripts/RunControlsAutomation.ps1 -TestCase EarnedCombat -Resolution 720 -HUDScale 1.5. One isolated Standalone -game on /Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld. The runner supplies PFRunControlsUITest/PFRunEarnedCombatTest, a fresh profile and a unique bounded evidence label.
3. Require exactly PF.Progression.EarnedCombatLive Success, engine/strict/runner0, no timeout/test warnings, unchanged personal hashes, reviewed raw severity and all six PNGs. Any refusal or discrepancy fails; do not waive assertions. Build Development Game afterward.

Reports: Saved/AutomationReports/<run>/{run-summary.json,index.json}; raw logs Saved/Logs/<run>.log. PNGs Saved/AutomationReports/ControlsUI/<run>/{earned_level_two,earned_knowledge,earned_bound_tool,earned_guard,guarded_creature_hit,earned_creature_loot}.png. Ordinary PrimalFrontier.log was stale; current unique logs are evidence.

Known raw rendered baseline:26warnings/14experimental engine Python error lines, normalized against M12Protection1440_20261009_103335442_3f0bea5f; no new normalized severity/fatal/ensure in the initial run. Native raw logs clean. Actual installed engine5.8.3 differs from requested5.8.2; host31.93GiB physical RAM,16GB minimum unverified. Uncapped/VSync0 requested; stationary fixture timing/memory do not establish walking FPS/stutter. Existing one-/two-client transport/save/privacy checks remain separately documented; this test-only increment adds no multiplayer certificate.

Current early balance observations are measured mechanics, not final tuning approval: earned guard8→6hits; tool45damage requires three hits at5stamina each; finite3hitnodes yield6items with the bound tool; corpse gives one3food batch. No tuning values were changed without evidence. Higher-level XP sources and meaningful mid-game goals remain future; repair/equipped slots/durability/PvP are absent. Trello https://trello.com/c/4TnyuMEm; parent M12 stays Doing.


October10 follow-up: current EarnedCombat runner additionally checks ordinary lethal TakeDamage/default respawn and earned inventory/progression/window conservation, with three lifecycle captures after the original six. Current real gathered route ends205XP; prior120/140XP values above are historical. Exact new evidence and limits: EARNED_RESPAWN_M12.md. No new network/human certificate.