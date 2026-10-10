# M12 — Earned progression through death and respawn

## Scope

This is a bounded technical verification, not full M12 acceptance. No production gameplay, assets, map, reward magnitudes, respawn delay or personal save/settings files change. Existing earned native and rendered fixtures now exercise ordinary lethal damage and the normal three-second server respawn timer. Manual combat feel, physical controller, rendered persistence replay and sustained performance remain unverified.

## Start here

Close other Unreal processes. Build `PrimalFrontierEditor` before running the native case:

```powershell
& ./Scripts/RunNativeAutomation.ps1 -TestFilter 'PF.Progression.EarnedUpgrade' -Label M12EarnedRespawn
```

No authored level is required for that native disposable Game-world fixture. It genuinely gathers resources and completes default-duration recipes; it never seeds XP or grants items. At 185 XP, one learned Field Tools unlock and one remaining point:

1. Capture the full inventory (stack IDs, item IDs, quantities and original expiration times), first-craft ledger, knowledge and aged reward windows.
2. Start a repeat cord craft, apply normal lethal server `TakeDamage`, and refuse actual gathering with the dead pawn.
3. Advance normal world ticks beyond the unchanged default respawn delay. Require a replacement pawn with the same PlayerState/progression component.
4. Require craft cancellation without consumption/output duplication, exact inventory conservation and unchanged earned XP/knowledge/points/ledger.
5. Require unchanged category counts and durations reduced by elapsed active game time, never refreshed.
6. Gather fibre normally with the earned bound tool after respawn: finite yield still succeeds, but the exhausted category cannot grant XP again.

Run relevant independent regressions:

```powershell
& ./Scripts/RunNativeAutomation.ps1 -TestFilter 'PF.Progression.Component+PF.Progression.GatherEvents+PF.Survival.Lifecycle' -Label M12RespawnRegression
```

After native success, run the existing isolated rendered route:

```powershell
& ./Scripts/RunControlsAutomation.ps1 -TestCase EarnedCombat -Resolution 720 -HUDScale 1.5
```

Level: `/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld`, standalone `-game`, D3D12 SM6, 1280×720, uncapped (`t.MaxFPS 0`), VSync off. The runner owns disposable identity/report paths and guards all unrelated saves and personal settings.

The real route reaches 205 XP through bounded successful gathering and unique timed crafts, learns Field Tools, crafts protection, fights the default Prowler with normal melee and recovers its one food batch. It then applies ordinary lethal damage, inspects the actual death/0-health HUD, awaits ordinary replacement, compares all earned inventory/progression/window fields, inspects the restored 100-health HUD and reopens the crafting menu to confirm 205 XP, one point and learned access. No forced respawn, timer shortcut or seeded rewards. Controlled positioning and synthetic input do not certify traversal, human combat feel or hardware-controller operation.

## Evidence recorded so far

- Initial Editor build failed: `Saved/Logs/PFM12EarnedRespawnEditorBuild.log`, 14.92 s, exit 6. The new native test attempted array equality for `FPFItemStack`, which has no equality operator. Only the fixture was corrected to compare complete fields/cardinality.
- Editor retry passed cleanly in 5.81 s: `Saved/Logs/PFM12EarnedRespawnRetryEditorBuild.log`.
- `PF.Progression.EarnedUpgrade` passed 1/1: `Saved/AutomationReports/Automation_M12EarnedRespawn_20261010_042319304_2b2f5cdc/index.json`; raw `Saved/Logs/PFAutomation_M12EarnedRespawn_20261010_042319304_2b2f5cdc.log`. Exact selector, exit 0, no test warnings/errors, no timeout; raw severity/assert/fatal/ensure search clean. 16.57 s, sampled working/private 2.957/2.828 GiB.
- Component, GatherEvents and Survival.Lifecycle passed 3/3: `Saved/AutomationReports/Automation_M12RespawnRegression_20261010_042453978_7c1deb2f/index.json`; corresponding `Saved/Logs/PFAutomation_M12RespawnRegression_20261010_042453978_7c1deb2f.log`. Exact selectors, exit 0, raw/test severity 0, no timeout/assert/fatal/ensure. 16.55 s, working/private 2.974/2.805 GiB.
- Rendered extension Editor builds passed cleanly: `PFM12EarnedRespawnRenderedEditorBuild.log` (15.28 s) and `PFM12EarnedRespawnFinalEditorBuild.log` (7.20 s). The new menu expectation uses the existing singular point and 45-to-next formatting.
- Initial rendered assertions passed 1/1: `M12EarnedCombat720_20261010_042713479_28495d49`, 109.42 s, working/private 3.083/4.884 GiB; exact selector and all exits 0, no test warnings/errors/timeout. All 286 unrelated personal saves and default settings hashes were unchanged. Raw normalized warnings matched the prior 26-line baseline, with no assertion/fatal/ensure. Death/knowledge images were inspected; the respawn screenshot captured the menu because it opened before the requested screenshot flushed. A fixture-only wait now separates those operations; that pass is retained, but artifact acceptance requires replay. Capture build passed cleanly in 7.54 s: `PFM12EarnedRespawnCaptureEditorBuild.log`.
- Final rendered replay passed 1/1: `Saved/AutomationReports/M12EarnedCombat720_20261010_043006241_41861d27/index.json`; raw `Saved/Logs/M12EarnedCombat720_20261010_043006241_41861d27.log`. 109.88 s, sampled working/private 3.090/5.289 GiB; exact selection, engine/verdict/runner exits 0, no test warnings/errors/timeout. All 287 unrelated saves and actual default settings hashes unchanged.
- All nine final PNGs actually inspected under `Saved/AutomationReports/ControlsUI/M12EarnedCombat720_20261010_043006241_41861d27`: earned_level_two, earned_knowledge, earned_bound_tool, earned_guard, guarded_creature_hit, earned_creature_loot, earned_death, earned_respawn and earned_respawn_knowledge. Death reads 0/100 with server-respawn message; separate respawn image reads 100/100 and the menu retains 205 XP, 45 to next level and one knowledge point. Full selected recipe row remains visible.
- Both rendered raw logs retain exactly the prior 26 warnings and 14 experimental engine Python error lines, with zero normalized severity differences and no assertion/fatal/ensure. These are existing EditorDataStorageUI factories, uncooked HLOD settings, console/scalability priority and experimental Python definitions; not new gameplay errors. Ordinary `PrimalFrontier.log` remains an older launch log; unique current logs above are authoritative for these runs.
- Development Game passed cleanly in 23.81 s: `Saved/Logs/PFM12EarnedRespawnGameBuild.log`; final Editor capture build 7.54 s clean. Initial native compile failure and screenshot-evidence improvement are retained above. No new multiplayer death/respawn certificate is inferred: network fixtures and gameplay remain unchanged. Prior actual gather/restart authority evidence is retained in `GATHER_EVENTS_M12.md`.

## Limits

Installed engine reports 5.8.3, while the requested baseline is 5.8.2. This host exposes 31.93 GiB RAM; historical 16 GiB minimum support remains unverified. Reported memory is sampled process working/private memory, not a GPU-memory measurement or an FPS guarantee. Existing personal acceptance cards stay open. No extra personal task is created for this automated verification.