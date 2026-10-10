# Successful gathering rewards — M12

October 10, 2026. Bounded technical task: https://trello.com/c/xY9fEfpd. Full M12 and existing Personal acceptance remain open.

## Contract

Actual authoritative resource-node gathering prepares an owned living survivor's progression candidate before inventory conversion and commits only after Grant succeeds. Category derives from the validated server yield catalog. There is no client XP amount, target-player reward or grant RPC. Award 5 XP per successful action, regardless of barehand/tool hit count, at most five credits per category in 1,800 active server-game seconds. Wood, Stone, Fibre, Food and Water have independent budgets. Exhausted/capped budgets allow ordinary finite gathering without extra XP. Grants, dropping, pickup, cancelled/failed crafting and invalid gathering do not award gathering XP.

Existing lazy game-time snapshots age windows; accepted conversion commits the aged candidate. Save/load and reconnect preserve owner-private counts and remaining durations. Separate server restarts rebase the saved durations, without offline aging or fresh budgets. Food freshness still uses its existing independent expiration clock. Normal resource yield, damage, recipe costs/durations, maps and presentation assets are unchanged.

## Evidence

| Gate | Evidence under Saved | Result |
| --- | --- | --- |
| Editor | Logs/PFM12GatherEventsEditorBuild.log; final PFM12GatherEventsFinalEditorBuild.log; capture PFM12GatherEventsCaptureEditorBuild.log | Succeeded 20.49 / 5.70 / 7.00 seconds; no compiler warnings |
| First native | AutomationReports/Automation_M12GatherEvents_20261010_034142909_595c8f5e | GatherEvents 1/1, 16.55 seconds, working/private 2.999/2.837 GiB |
| Final native | AutomationReports/Automation_M12GatherEventsFinal_20261010_034325819_200d5e19 | Exact 7/7, 18.15 seconds, 2.958/2.809 GiB |
| Rendered | AutomationReports/ControlsUI/M12EarnedCombat720_20261010_034455160_b666a1ba | EarnedCombatLive 1/1, 105.64 seconds, 3.192/5.414 GiB |
| One client, separate restart | AutomationReports/M8Live1_20261010_034730959_27f89bfc | PF.Persistence.Live 4/4 roles, engine/verdict exits 0, no timeout |
| Two clients, separate restart | AutomationReports/M8Live2_20261010_035338265_5d887ea6 | PF.Persistence.Live 6/6 roles, 75-ms outgoing lag/1% loss, exits 0, no timeout |
| Game Development Win64 | Logs/PFM12GatherEventsGameBuild.log | Succeeded 25.91 seconds; no compiler warnings |

Final native selectors: PF.Progression.GatherEvents, EarnedUpgrade, GatherClock, Component, RecipeAccess; PF.Crafting.Gathering and Transactions. Exact selection reconciliation, clean test severity and zero engine/strict exits; no timeout/assert/fatal/ensure. Native raw logs have zero warnings/errors. All four current network raw logs equal the prior successful network baseline: 24 startup warnings and 14 experimental Python definition-error lines, with no new severity/assert/fatal/ensure. Unique Logs/PFM8Live1_<run>*.log files are current evidence; ordinary PrimalFrontier.log predates this run.

GatherEvents exercises actual owned node traces/inventory conversions: full bag, cooldown, wrong aim, unknown resource, dead/foreign/client roles, depleted nodes, category caps, all five independent budgets, one reward per three-hit tool action, grants/drop/recovery and active expiry. The fixture explicitly enlarges its bag, isolates movement/needs and controls boundary world time; tool/cap boundary seeds are disclosed. These are deterministic native authority checks, not human pacing or rendered traversal.

EarnedUpgrade starts empty at 0 XP with no grants or seeded progression. Six Wood, one Stone, two Food and four Fibre actions supply 12 wood/2 stone/4 food/8 fibre and 60 XP. Tool/Cook/Dry/Cord/Club first completions reach 160 XP/level 2/3 points; repeat Cord adds no XP. FieldTools costs two points; BoundTool completion reaches 180 XP. A fifth Fibre action with that tool yields six fibre and one 5-XP credit, reaching 185 XP. Normally crafted guard reaches 205 XP; real default creature damage reduces 8 to 6, actual melee costs/cooldown apply, and one three-food loot batch retains its original expiry after normal pickup.

All six rendered PNGs inspected: earned_level_two, earned_knowledge, earned_bound_tool, earned_guard, guarded_creature_hit and earned_creature_loot. Selected summary/details/controls passed bounds assertions at 1280x720, HUD scale 1.5. Existing selected guard row is partly clipped at the bottom of the recipe list: separate M11 layout follow-up, not full-list acceptance. Synthetic fixture positions/ticks do not certify navigation, human combat feel or controller use. Rendered raw log matches the prior baseline exactly: 26 startup warnings/14 experimental Python lines, no new severity/assert/fatal/ensure. 268 personal save files and default settings were unchanged by read-only hash guards.

Network fixtures use actual gathering and timed crafts: storage owner 70 XP/two distinct crafts/Wood4-Stone1-Food1 windows; second player 40 XP/one craft/Wood3-Stone1. Direct client restore/capture and invalid requests refuse; remote records expose zero private progression. Manual load conserves the checkpoint; new-process restart compares the restored record against the real decoded saved window counts/durations, then checks aged capture. Trusted post-save inventory marker grants earn no XP. Disposable unique fixture slots/profiles only.

## Replay and limits

Native level: NONE; disposable Game worlds. Rendered/network level: /Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld. Start here: close competing Unreal processes, build PrimalFrontierEditor, then run the exact seven plus-separated selectors with Scripts/RunNativeAutomation.ps1. Run Scripts/RunControlsAutomation.ps1 -TestCase EarnedCombat -Resolution 720 -HUDScale 1.5; inspect reports, six PNGs, raw logs and hash guards. Run Scripts/RunPersistenceAutomation.ps1 -Players 1 before -Players 2 -SimulateLagLoss. Require exactly four/six PF.Persistence.Live role reports covering Create and separate Restart, private counts and duration conservation. Build PrimalFrontier Development Win64 and review compiler/raw logs before technical completion.

All ten one-/two-client role logs match the same network baseline; no new severity/assert/fatal/ensure. One-client server working/private peaks 1.724/1.571 GiB, client 1.808/1.738 GiB; two-client server 1.724/1.573 GiB, individual clients at most 1.816/1.742 GiB. These are process peaks, not whole-machine totals or sustained frame-rate measurements. No failed build/test in this slice.

One rendered process, uncapped FPS/VSync 0; networking NullRHI. Installed engine is 5.8.3 versus requested 5.8.2, host RAM 31.93 GiB. No sustained FPS, minimum-16-GB, human/controller/manual save-loop or full-milestone certificate. Packaged Server target remains unsupported by the installed engine distribution; uncooked dedicated server is the tested path. No external art, map, private-save, settings or dependency changes. Bounded technical gate passed; next eligible M11 follow-up is the clipped recipe-list row. Full M12 remains open.
