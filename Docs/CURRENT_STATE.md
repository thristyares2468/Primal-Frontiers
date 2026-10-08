# Primal Frontier — current state

Updated October 8, 2026. Maintain after each meaningful change and build/test result; implementation and automated evidence do not replace manual acceptance.

## Current game

First-person server-authoritative greybox survival: Health/Stamina/Hunger/Thirst, damage/death/respawn, food spoilage, inventory/pickup/drop, gathering/crafting, primitive building/storage/ownership, passive/hostile creatures and time of day. Pause, settings and controller bindings exist. The open-world candidate is **L_PrimalFrontier_OpenWorld**, a 400 x 400 m World Partition map. L_M7SurvivalArena and earlier maps remain regression fixtures. Existing user-imported assets are preserved; current tests use primitives. See [PLAYTEST.md](PLAYTEST.md).

## Completed and verified

- M1–M6 historical supported checkpoints: [MILESTONES.md](MILESTONES.md).
- M7 automation/streaming and one-/two-client NullRHI world checks passed. Sustained walking and overnight survival **have not passed**; M7 remains Doing.
- M8 bounded versioned player/world saves, A/B file generations, authoritative restoration, stable ownership and development reconnect are implemented. PF.SaveWorld/PF.LoadWorld and read-only PF.TestPersistence use real server APIs.
- Editor rebuild passed in 6.33 s; post-commit clean-working-set Editor build passed in 20.76 s. Development Game passed in 69.65 s; clean-working-set Game build passed in 31.45 s. No compiler warnings.
- **M8FinalRegression: 26 passed, 0 failed, 0 test warnings**, exit 0. Exact test names are in [MILESTONES.md](MILESTONES.md) and Saved/AutomationReports/M8FinalRegression/index.json; log Saved/Logs/PFM8FinalRegression.log.
- Actual one-client create/restart: four PF.Persistence.Live reports passed. Actual two-client create/restart: six reports passed. All zero failures/test warnings. Restored tool, health, structures/storage/ownership and the item captured on disconnect after manual save. Evidence and previous repaired failures: [PERSISTENCE_M8.md](PERSISTENCE_M8.md).

## Known limits and remaining gates

M7 manual route/overnight and M8 rendered gather/craft/build/storage/save/close/restart/reconnect acceptance remain **unverified**. NullRHI fixture teleports cannot certify these. Controller feel/hot-plugging also need a physical device test. Independent M9 planning/audits are permitted; no gated art/gameplay integration starts from them.

PrimalFrontierServer is blocked by the installed engine distribution: "Server targets are not currently supported from this engine distribution." Uncooked Editor -server works in the tested scenario; packaged server remains unverified. Installed engine is 5.8.3 although 5.8.2 was requested.

Development reconnect is a trusted LAN GUID capability/profile, not production account authentication. Keep private identity/save data out of Git and reports. No timed autosave or automatic server-exit save: manually save before quitting. Active-slot client logout checkpoints are tested. Food ages offline; other world timers pause. Failed restore/corrupt generation refuses unsafe overwrite; migrations remain future work.

Final regression working/private memory: 2.896/2.769 GiB. Two-client NullRHI working sets: create 1.717/1.808/1.816 GiB, restart 1.722/1.796/1.795 GiB. No rendered FPS/stutter measurement was made. Host has about 31.93 GiB RAM; these runs do not certify a 16 GB minimum. Earlier slow New Editor Window PIE presentation remains unresolved. Known engine Toolsets Python/uncooked HLOD startup warnings are documented separately from clean test events.

## Next step and coordination

October 8 independent progression planning: [PROGRESSION_PLAN.md](PROGRESSION_PLAN.md) defines provisional XP/points, original unlock prerequisites, respec/co-op/discovery rules and separate lasting adaptations/equipped modifiers. Baseline tools/cooking/drying/shelter remain available; no gameplay, data assets, save versions or settings changed. Document/source consistency checked; no new build or gameplay test is claimed. Corrected FOOD_AND_PRESERVATION.md to match implemented M8 offline food aging. Programming's bounded planning card is complete; full M12/M13 remains To Do. M7/M8 manual gates remain open. Next independent task: audio feedback planning; art/gameplay integration remains gated.

M8 checkpoint is pushed and remote verified at e04d332. Under the user's independent-continuation instruction, M9 inventory/pipeline planning now has a completed task: UE Python commandlet exit 0, 7,268 registry assets/7,270 packages, 41.723 GiB disk, 13 unchanged redirectors, no missing registry package files, zero asset objects loaded/saved by the script. Process working/private memory 1.820/1.720 GiB, NullRHI. Rules and source research: [ASSET_PIPELINE_M9.md](ASSET_PIPELINE_M9.md); exact unique report/log there. Registry lists 1,014 4K and 94 8K textures, not proof they are runtime-resident or causing low FPS.

The audio-preferences-versus-coverage audit is also Done: three classes/one mix exist; only one template SoundWave was found, no authored survival audio coverage is verified. See [AUDIO_COVERAGE.md](AUDIO_COVERAGE.md). No gameplay, art, maps, audio settings, downloads or imports changed. Remaining M9 candidate approval needs actual source/version/license mapping; the public Modular Rural Cabins page was checked, but local acquisition/version and exact license remain unconfirmed. M7/M8 manual acceptance still gates art/gameplay integration. Next user action remains the M7 Selected Viewport + F11 route/overnight playtest; M8 rendered persistence follows. Future implementation tasks remain dependency-blocked.

Trello: audited all six To Do → Doing → Done boards, initially 53 cards and now 54 after adding the completed M9 inventory subtask. Final readback verified every Personal/AI prefix. Corrected stale status, real roadmap objectives/criteria/links and native M7/M8 gate checklists. M7/M8/M9 remain Doing; M0–M6 historical checkpoints Done; M10–M25 planned To Do. On every continue, first choose the next eligible task from Trello and reconcile with the docs. Immediately on completing a task, update its card before starting another. Preserve unrelated cards, dates and assignments. See [TRELLO_SYNC.md](TRELLO_SYNC.md).

Original technology/adaptation and active/passive biological modifier requests are planning only for M12/M13: [ADAPTATION_DIRECTION.md](ADAPTATION_DIRECTION.md). Notify the user of a concrete asset gap when its implementation is authorized. The imported-asset LFS upload/archive tag and M8 checkpoint pushes succeeded; clean-working-set Editor/Game builds passed. M9 planning/audit committed/pushed as 091c407; local/remote master matched 091c407f36559b0b943f8100b32f808e4089bf20 on readback. Trello uses (Personal) for required user actions or (AI) for agent work; AGENTS.md/TRELLO_SYNC.md preserve it. Only the other chat's two legacy Trello setup docs remain untracked; no imported asset changes. Further gameplay/art milestones wait at the recorded Personal acceptance/candidate dependencies.
