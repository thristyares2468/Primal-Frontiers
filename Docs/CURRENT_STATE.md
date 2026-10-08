# Primal Frontier — current state

Updated October 8, 2026. Maintain after each meaningful change and build/test result; implementation and automated evidence do not replace manual acceptance.

## Current game

First-person server-authoritative greybox survival: Health/Stamina/Hunger/Thirst, damage/death/respawn, food spoilage, inventory/pickup/drop, gathering/crafting, primitive building/storage/ownership, passive/hostile creatures and time of day. Pause, settings and controller bindings exist. The open-world candidate is **L_PrimalFrontier_OpenWorld**, a 400 x 400 m World Partition map. L_M7SurvivalArena and earlier maps remain regression fixtures. Existing user-imported assets are preserved; current tests use primitives. See [PLAYTEST.md](PLAYTEST.md).

## Completed and verified

- M1–M6 historical supported checkpoints: [MILESTONES.md](MILESTONES.md).
- M7 automation/streaming and one-/two-client NullRHI world checks passed. Sustained walking and overnight survival **have not passed**; M7 remains Doing.
- M8 bounded versioned player/world saves, A/B file generations, authoritative restoration, stable ownership and development reconnect are implemented. PF.SaveWorld/PF.LoadWorld and read-only PF.TestPersistence use real server APIs.
- Latest Editor build passed in 6.33 s; Development Game build passed in 69.65 s, no compiler warnings. Post-commit working-set verification remains pending.
- **M8FinalRegression: 26 passed, 0 failed, 0 test warnings**, exit 0. Exact test names are in [MILESTONES.md](MILESTONES.md) and Saved/AutomationReports/M8FinalRegression/index.json; log Saved/Logs/PFM8FinalRegression.log.
- Actual one-client create/restart: four PF.Persistence.Live reports passed. Actual two-client create/restart: six reports passed. All zero failures/test warnings. Restored tool, health, structures/storage/ownership and the item captured on disconnect after manual save. Evidence and previous repaired failures: [PERSISTENCE_M8.md](PERSISTENCE_M8.md).

## Known limits and remaining gates

M7 manual route/overnight and M8 rendered gather/craft/build/storage/save/close/restart/reconnect acceptance remain **unverified**. NullRHI fixture teleports cannot certify these. Controller feel/hot-plugging also need a physical device test. No M9 implementation starts automatically.

PrimalFrontierServer is blocked by the installed engine distribution: "Server targets are not currently supported from this engine distribution." Uncooked Editor -server works in the tested scenario; packaged server remains unverified. Installed engine is 5.8.3 although 5.8.2 was requested.

Development reconnect is a trusted LAN GUID capability/profile, not production account authentication. Keep private identity/save data out of Git and reports. No timed autosave or automatic server-exit save: manually save before quitting. Active-slot client logout checkpoints are tested. Food ages offline; other world timers pause. Failed restore/corrupt generation refuses unsafe overwrite; migrations remain future work.

Final regression working/private memory: 2.896/2.769 GiB. Two-client NullRHI working sets: create 1.717/1.808/1.816 GiB, restart 1.722/1.796/1.795 GiB. No rendered FPS/stutter measurement was made. Host has about 31.93 GiB RAM; these runs do not certify a 16 GB minimum. Earlier slow New Editor Window PIE presentation remains unresolved. Known engine Toolsets Python/uncooked HLOD startup warnings are documented separately from clean test events.

## Next step and coordination

Finish the scoped M8 verification commit/push and clean-working-set build. Then the next acceptance action is the user's sustained route/overnight playtest on L_PrimalFrontier_OpenWorld (Selected Viewport + F11). M8's rendered save/restart walkthrough follows it. Stop this development pass at M8; no external art, third person or large-world expansion.

Trello: audited all six To Do → Doing → Done boards and read back 53 open cards. Corrected stale discipline status, inserted real roadmap objectives/criteria and links, and added native M7/M8 gate checklists. M7/M8 remain Doing; M0–M6 historical checkpoints Done; M9–M25 planned To Do. On every continue, first choose the next eligible task from Trello and reconcile with the docs. Immediately on completing a task, update its card before starting another. Preserve unrelated cards, dates and assignments. See [TRELLO_SYNC.md](TRELLO_SYNC.md).

Original technology/adaptation and active/passive biological modifier requests are planning only for M12/M13: [ADAPTATION_DIRECTION.md](ADAPTATION_DIRECTION.md). Notify the user of a concrete asset gap when its implementation is authorized. The previous imported-asset LFS upload and archive tag succeeded; current M8 changes still need commit/push verification.
