# M12 — trusted harvesting capabilities

October 14, 2026. Bounded backend contract; verification in progress. This does not deliver the eight-tier content ladder.

## Contract

`FPFItemDefinition` has a native Gameplay Tag family container and a material grade; `FPFResourceDefinition` has a required family/minimum grade. Pick, Axe and Sickle are exact native tags. Mechanical grades are Wood, Stone, Copper, Iron, Diamond, Titanium and Deepforge. Grade is access capability; `GatheringHits` remains speed and cannot increase a node's total yield. Tech is reserved but explicitly refused: powered Tech harvesting is NOT IMPLEMENTED until real charge validation exists.

Empty families/None grade is the explicit legacy tool default. Empty/None resource requirements are unrestricted. Existing item IDs, speed values, nodes, recipes, native/saved catalogs, actor identities and all save schemas are unchanged. Existing general stone/bound tools do not silently acquire access to future gated nodes. Typed families require a valid mechanical grade, positive bounded gathering benefit and Tool category; unrelated/unknown tags, malformed combinations and unsupported grade values fail catalog lookup.

`UPFInventoryComponent::ResolveHarvest` is a read-only query usable for authoritative validation or future advisory owner UI. It selects the highest gathering speed among compatible, valid, fresh tools actually in that bag. Under-grade/wrong-family/expired/removed/foreign tools confer no access; no cached selection or client-supplied bonus exists. Failed queries preserve the output hit value and all inventory state, including expired stacks.

`APFResourceNode::Gather` resolves access before any mutation or progression preparation. An access failure logs a readable requirement with `[PrimalAgentTools]`, changes no hits, cooldown, items, stamina, respawn or XP. Normal server ownership/aim/life/capacity/cooldown and finite-yield rules remain. No new target/tier/yield RPC is added. No physical feature is exposed yet, so backend tests use disposable worlds instead of inventing a normal-map fixture.

## Tests and limits

## Actual owned-request network gate

Final Game build PFM12HarvestLiveGameBuild.log PASS29.30s without compiler warnings. Editor final PFM12HarvestAckBuild.log PASS6.79s. Final two-role memory: server1.726working/1.632privateGiB, client1 1.803/1.784, client2 1.794/1.779. All five final role logs reviewed; no new severity against existing baseline. No Unreal process left after owned cleanup. The backend/native and live tasks close only their bounded technical gates.

`PF.Harvest.Live`, opt-in `-PFRunHarvestLiveTests -nullrhi -PFExpectedPlayers=1|2`, runs only on disposable `/Game/PrimalFrontier/Maps/L_M7SurvivalArena` with an uncooked dedicated server. Server clones item catalogs at runtime, temporarily treats existing Item_Tool as Stone then Iron Pick, and existing faster Item_BoundTool as Iron Axe. Synthetic Stone nodes require Iron Pick. These trusted boundary fixtures do not add earned materials or change saved data assets. Existing controller RPC is invoked through Unreal reflection on the owning client; access is not widened and no test gameplay RPC is introduced.

Actual client phases: ready; denied under-tier/wrong family; accepted exact tool with immediate duplicate request refused; denied after removal; exact tool restored for finite last hit; final owner/private state acknowledged. Each accepted action earns one bounded gather reward, total6 Stone/10XP per lane. Direct client node/grant mutations fail, foreign inventories/progression remain private. Consume of one test water acknowledges ordered RPCs. The first stalled run found the acknowledgement itself was sent inside the inventory cooldown; per-phase0.5s observation fixes only the fixture. NullRHI runs use120FPS to avoid unbounded saved-move queues; rendered performance remains uncapped and unmeasured here.

Reproduce one before two:

```powershell
./Scripts/RunEcologyAutomation.ps1 -TestCase Harvest -Players 1 -Port 17987 -TimeoutSeconds 180
./Scripts/RunEcologyAutomation.ps1 -TestCase Harvest -Players 2 -SimulateLagLoss -Port 17987 -TimeoutSeconds 180
```

Final one-client `M12Harvest1_20261013_211151288_3abeafce` PASS2/2,48.62s, server1.729/1.635GiB working/private and client1.791/1.779. Final two-client `M12Harvest2_20261013_211327579_a866ce05` PASS3/3,55.61s,75ms/1%; exact per-role memory in run-summary.json. Both private-save/settings guards pass, no test warnings/errors/fatal/ensure/timeout. All five final role logs introduce zero normalized severity lines against the previously inspected startup baseline; raw21warnings/14known ToolsetDefinition/PythonTestRunner errors per role remain. Actual .log/index.json/run-summary paths are Saved/Logs/PF<Run><Role>.log and Saved/AutomationReports/<Run>[<Role>]. No disk restart, new mineral crafting, rendered prompt/physics, manual pacing or packaged-server acceptance inferred.

Retained failures: PFM12HarvestLiveBuild.log C3535/C2440 weak pointer deduction and C2248 protected RPC access; fixture `.Get()` plus reflected existing RPC repair buildPASS5.86s. Initial `M12Harvest1_20261013_210835439_df6d06c7` failed both role tests at phase9 and logged3 saved-move warnings; private/default guards still passed. Ack buildPASS6.79s, new-phase delay and headless120 guard repaired it. No production authority, cooldown or inventory rules relaxed. Subsequent native source was unchanged from d8245ec.

Final backend checkpoint: fixture-only Editor rebuild `PFM12HarvestFixtureBuild.log` PASS 12.18s; Game `PFM12HarvestGameBuild.log` PASS 39.17s, no compiler warnings. Failed selector replay `Automation_M12GatherFixtureReplay_20261013_210101442_0ffa2c10` PASS 1/1, 19.09s, raw severity 0, sampled 2.998GiB working/2.881GiB private. Other five compatibility selectors passed in the initial run: PF.Crafting.Gathering, PF.Crafting.Transactions, PF.World.Resources, PF.Persistence.WorldRuntime and PF.Progression.WorldCompatibility. Together with the three new selectors, nine distinct relevant native tests have passed. Ordinary PrimalFrontier.log checked alongside the unique native logs; no new fatal/ensure. No live networking/rendering acceptance yet.

Compatibility first run `Automation_M12HarvestCompatibility_20261013_205830343_12eb09c6`: 5/6 passed; GatherEvents failed its cap-boundary gather. Root cause was a latent fixture interaction with the previous stat feature: the fixture retained temporary hard-weight enforcement after its full-bag probe, while progression restore reapplies the normal 30kg threshold. Its accumulated 33.5kg then correctly refused under that artificial hard-container policy. Restore normal soft player-bag enforcement after the probe; do not change production weight or progression rules. Failed report retained; narrow rebuild/replay pending.

Initial native run: `Automation_M12HarvestContract_20261013_205728976_097d7850`, PASS 3/3, zero warnings/errors, raw severity 0, 43.07s, sampled peak 2.936GiB working/2.835GiB private. Each new selector succeeded; exact selection checked and process exit 0. Matching report/index/run-summary under Saved/AutomationReports and PFAutomation_M12HarvestContract_20261013_205728976_097d7850.log under Saved/Logs. UTC run ID is October13; user-facing Brisbane date is October14. Baseline compatibility and Game build still pending.

New selectors: `PF.Harvest.Definitions`, `PF.Harvest.Selection`, `PF.Harvest.AtomicGather`. They exercise validation/legacy defaults, real bag grants/removals/freshness/family selection and an actual possessed pawn aiming at a synthetic Iron-gated Stone node. This synthetic gate changes only the disposable test world; it is not a shipped Iron pick or ore deposit. Runtime tests check refusal snapshots, immediate same-clock success after refusals, exact compatible speed, finite total yield, cooldown/depletion, XP, client-role and foreign-controller rejection.

Editor build PFM12HarvestContractBuild.log passed in 41.66s with no compiler warning. Native results/evidence follow after execution. No multiplayer/rendering, new resource crafting loop, authored save migration, earned tier pacing or human pass is claimed by these native tests. The existing climb/player-upgrade Personal checks remain open. Permanent node expansion still requires the explicit layout migration in SAVE_COMPATIBILITY_PLAN.md; do not alter private saves to make tests pass.
