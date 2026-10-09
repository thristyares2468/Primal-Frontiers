# M12 fibre cord and bound stone tool

October9,2026. Bounded tool slice verified; full M12 is not accepted. No external assets, maps, dependencies or save-schema changes. Next independent scope: XP/knowledge or further weapon/armor work, each with separate save compatibility and playtest gates.

## Play the loop

Level: `/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld`. One player, Standalone net mode, Selected Viewport + F11 for PIE; P opens Pause because Esc may stop PIE. Existing labelled fibre patches supply plant fibre via the same E interaction as wood/stone. No new level or binary asset is needed.

1. Gather3wood and2stone; C opens the centered menu. Select Stone gathering tool with Up/Down or click its row; Enter or Craft selected starts its5-second job. Keep ingredients in the bag until completion.
2. Gather at least4plant fibre. In C, select Twist fibre cord. Four fibre become one cord after4seconds. R cancels without consuming inputs or producing output.
3. Keep one stone tool, one cord and2wood. Select Bind stone tool; its8-second job consumes those exact ingredients and creates one bound stone tool.
4. Close C; gather a fresh full resource node with E. Bare hands spend1hit, the stone tool2hits, bound tool3hits per action. Every full node still supplies its configured total; an upgrade speeds gathering without multiplying yield. Existing reach/cooldown/capacity checks apply.
5. Aim at a creature within2.5m and attack with left mouse. Bare hands20damage, stone tool35, bound tool45; each accepted swing costs5stamina and respects the existing0.5-second cooldown. Damage/aim/tool strength are selected by the server. Existing cheap local/remote tool primitives remain visible after upgrading.
6. Drop/storage transfers remove the tool's benefit from the old bag; regaining it restores the benefit. This slice uses the best valid carried gathering/melee benefits per action; it has no equipment slot, durability, repair, XP, technology gate or armor yet.

Menu details show actual costs/duration/weight/stack limits/tool statistics and original procedural pictures. Gamepad D-pad browses, A/X crafts the selected recipe, D-pad left cancels, Y/B closes. Actual hardware/mouse usability remains part of the existing Personal G2/G4 checks, not certified by synthetic Slate inputs.

## Authoritative implementation

`PFItemCatalog` validates GatheringHits0–8 and finite MeleeDamage0–100; non-tool categories cannot declare these effects. `PFInventoryComponent` derives the strongest valid owned fresh benefit without caching. `PFResourceNode` spends a bounded number of remaining hits only after insertion can succeed. Controller melee derives damage from the same server inventory. No client-supplied damage amount/tool grant RPC is added.

`Item_Cord` and `Item_BoundTool` plus `Recipe_Cord`/`Recipe_BoundTool` append native defaults to the real loaded catalogs. Actual catalog tests confirmed them; no serialized asset migration is required. Existing recipes remain available. Existing V1 save records store the stable IDs/batches and recompute tool benefits on restoration; no effect/stat execution is serialized.

## Verification checkpoint

Final Editor6.76s (`PFM12ToolRestartFixtureEditorBuild.log`) and Development Game28.25s (`PFM12ToolGameBuild.log`) PASSED/no compiler warnings. Two-client NullRHI Create/Restart `M12ToolLive2_20261009_090258613_a1df09c0` PASSED6/6 process reports with75ms outgoing lag/1%loss. Create73.14s,Restart33.44s; working/private1.719–1.817/1.562–1.733GiB per process. All tests/engine/report/runner0, foreign owner-private inventories empty, public held-tool flag replicated, exact costs/yield and actual restored IDs/stats/roster count verified. Client2 no crash. Every network process has only known24startupwarnings/14experimental Python lines, no gameplay error/ensure/fatal. One-client phases70.59s/30.21s are recorded in its retained run summary; use that report for exact timings rather than approximate human observations. Earlier pending statements below are chronological checkpoints.

Editor17.81s passed after live fixture addition; later held-tool assertion build failed C2248/private flag, retained `PFM12ToolPresentationEditorBuild.log`12.47s/exit6. Added read-only query; retry17.81s passed `PFM12ToolPresentationEditorRetry.log`, no compiler warnings. Earlier first native fixture compile failed C2664/limits argument; corrected before tests, `PFM12CordTierEditorBuild.log` retained.

Native regression5/5: `Automation_M12ToolRegression_20261009_084452100_30a41706`,16.67s,3.002/2.869GiB working/private. Names: PF.Crafting.Gathering, PF.Crafting.ToolProgression, PF.Crafting.Transactions, PF.Creatures.Lifecycle, PF.Persistence.WorldRuntime. Focused final presentation replay1/1 `Automation_M12ToolPresentation_20261009_085029903_e5cdd1b8`,16.27s,2.978/2.822GiB. Native logs/test severity0. Actual trace dealt45damage, spent5stamina, duplicate swing refused; transaction/cancellation/capacity/conservation/invalid data/client role/save-codec checks passed.

Final rendered720p/maxscale1.5: `M12Tool720_20261009_085117778_a36102b2`,72.54s,3.076/5.297GiB,PF.Crafting.ToolProgressionLive1/1. It gathers fibre through E path, exercises real Slate browsing/Enter, unknown/duplicate/cancel requests, real4s/8s craft durations, regrowth and finite upgraded yield. Three PNGs under `Saved/AutomationReports/ControlsUI/<run>/`; completed frame explicitly refreshes after completion. Actual settings/166 unrelated saves unchanged. Final1440p/network restart/Game build pending.

Final1440p/maxscale1.5: `M12Tool1440_20261009_085304460_b4e093f0`,72.14s,3.220/5.126GiB,1/1. All six final PNGs inspected. Both final rendered runs have test severity0 and engine/report/runner0; default settings and166/167 unrelated saves unchanged. Raw logs have26warnings (21EditorDataStorageUI,3uncooked HLOD,2blur/DOF priority),14experimental StateTree Python startup error lines; no gameplay error/ensure/fatal. Completion now explicitly shows idle/completed/exact consumed ingredients.

First one-client restart retained FAILED: `M12ToolLive1_20261009_085556336_ca42a712` Create server/client passed; Restart client passed but server1timeout because fixture waited for client-only UI acknowledgment. Fixed test condition only: actual restored server counts, Capture/Validate/original roster count, never invoke RestorePlayer. Editor6.76s clean, `PFM12ToolRestartFixtureEditorBuild.log`. Corrected one-client run `M12ToolLive1_20261009_090100905_c722f015` passed all4 Create/Restart server/client reports; working/private1.719–1.815/1.563–1.752GiB per process. Every test/exits0; all raw24warnings/14Pythonstartup lines/noensurefatal. Two-client lag/loss and Game build pending.

Initial rendered passes retained:720p `M12Tool720_20261009_084534054_2f773275`71.90s,3.147/5.089GiB;1440p `M12Tool1440_20261009_084712898_2bbabecb`71.70s,3.267/5.170GiB. Final replay follows the held-tool presentation fix and completion capture timing correction.

Reports live under `Saved/AutomationReports/<run>/index.json` with runner summaries; logs under `Saved/Logs/` (rendered names `<run>.log`, native/network prefix `PF`). Do not edit/generated reports or personal saves. Startup UI/HLOD/experimental Python/CVar priority findings are reviewed separately from test severity. Uncapped stationary scripted loops are not sustained traversal FPS, physical-controller or human acceptance; host31.93GiB does not certify16GB minimum. Installed engine5.8.3 differs from requested5.8.2. Uncooked dedicated server is separate from unavailable packaged Server target.

Replays with Unreal closed:

```powershell
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunNativeAutomation.ps1 -TestFilter 'PF.Crafting.ToolProgression' -Label M12Tool
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunControlsAutomation.ps1 -TestCase ToolProgression -Resolution 720 -HUDScale 1.5
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunPersistenceAutomation.ps1 -ToolProgression -Players 1
```

Run the1440p case and two clients only after their preceding cases pass. Each runner creates unique evidence/profile/settings/slot destinations, checks strict engine/report outcomes and cleans only owned process handles. Headless test requires AutomationM12 slot; rendered fixture requires explicit controls/tool opt-in flags. Current bounded card: https://trello.com/c/TFNzTtu0. Personal failed M8 replay and full M11 remain open.
