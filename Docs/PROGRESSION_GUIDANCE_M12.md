# M11/M12 — Current progression guidance

## Change

The locked crafting view now explains both implemented ways to earn XP: limited successful gathering and first completion of different recipes. The existing level, point cost, available points, missing prerequisites and learned-state messages are unchanged. This is read-only guidance: it cannot award XP, spend points, unlock recipes, renew reward windows or change save data.

`PROGRESSION_PLAN.md` now starts with a current implementation section. Dated checkpoints and original six-technology/station/adaptation proposals remain historical/future context; they are not claims that those features exist. Current implemented knowledge is Field Tools only. Current owner-private progression/world saves are V2-compatible, not absent as some older planning text describes.

No new assets, maps, input bindings, network paths, reward values, dependencies, human assignments or personal settings/save files change.

## Repeat the technical check

Start here: close competing Unreal processes and build `PrimalFrontierEditor` Development Win64.

1. No level required: `Scripts/RunNativeAutomation.ps1 -TestFilter 'PF.UI.ProgressionDetails' -Label M12ProgressionGuide`. Require exactly one successful view test, no warnings/errors/assert/fatal/ensure. Existing null/invalid-owner, cap, credited recipe, missing prerequisite, insufficient points and learned/refusal coverage remains; fresh locked view explains both bounded gathering and unique craft rewards.
2. Rendered standalone on `/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld`: `Scripts/RunControlsAutomation.ps1 -TestCase Progression -Resolution 720 -HUDScale 1.5`. Require exactly `PF.UI.ProgressionFeedbackLive` Success, all exits0/no timeout/test errors, actual updated hint/button/scroll allocation checks and unchanged personal hashes. Inspect screenshots of fresh lock, real crafting/repeat/cancel, capped record, available purchase, learned purchase and completed optional craft.
3. After720p passes, repeat with `-Resolution 1440 -HUDScale 1.5`. Uncapped/VSync off, one isolated D3D12 process per run. No fully rendered multiple clients.
4. Inspect unique raw log severities against the retained engine baseline and screenshots; build Development Game. Update CURRENT_STATE/MILESTONES and the bounded Trello task immediately after technical success.

This existing UI fixture uses trusted100XP/2700XP seeds to inspect available/cap states. It is not a genuinely earned route, server restart or human/controller/FPS certificate. Actual earning and death/restart evidence is separately documented in GATHER_EVENTS_M12.md and EARNED_RESPAWN_M12.md.

## Evidence

- Editor16.21s and Game23.10s passed cleanly: `Saved/Logs/PFM12ProgressionGuideEditorBuild.log` and `PFM12ProgressionGuideGameBuild.log`. No compile warnings or failed gate in this slice.
- Native `PF.UI.ProgressionDetails` passed1/1: `Saved/AutomationReports/Automation_M12ProgressionGuide_20261010_043738395_214d1aa1/index.json`; raw `Saved/Logs/PFAutomation_M12ProgressionGuide_20261010_043738395_214d1aa1.log`.16.57s,working/private2.952/2.816GiB,exact selector/exit0/no test/raw severity/assert/fatal/ensure/timeout.
- Rendered720p passed1/1: `Saved/AutomationReports/M11Progression720_20261010_043836298_b0dae995/{index.json,run-summary.json}`; corresponding unique raw log.51.75s,working/private3.134/5.048GiB;288personal saves/default settings unchanged.
- Rendered1440p passed1/1: `Saved/AutomationReports/M11Progression1440_20261010_043950978_341ea993/{index.json,run-summary.json}`; corresponding unique raw log.51.81s,working/private3.210/5.625GiB;289personal saves/default settings unchanged.
- Both rendered cases: exact PF.UI.ProgressionFeedbackLive,engine/verdict/runner exits0,no test warnings/errors/timeout. All16PNG actually inspected under `Saved/AutomationReports/ControlsUI/<run>/`: knowledge_level_locked,first_craft_available,first_craft_earned,different_recipe,level_cap,knowledge_available,knowledge_learned,learned_craft_complete. Updated hint fits the real inner scroll, pictures/selected rows/requirements/buttons remain readable and changes do not fabricate a purchase or reward.
- Both unique raw logs match the known26warnings/14experimentalPythonerror baseline with zero normalized severity differences and no assertion/fatal/ensure. Ordinary PrimalFrontier.log remains older; current unique logs are the evidence.

Bounded https://trello.com/c/HxqYElNZ technically complete. FullM11/M12 and all Personal acceptance remain open. No new network certificate: this slice changes only read-only text/fixtures/documentation.

## Limits

Installed5.8.3 versus requested5.8.2; host31.93GiB physical RAM rather than the historical16GiB minimum. Sampled process memory and stationary synthetic interaction do not measure sustained traversalFPS/stutter, actual audio/controller hardware or manual rendered persistence. FullM11/M12 and Personal gates stay open.