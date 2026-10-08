# Trello task workflow

## Every continue/task

1. Read current Trello cards and Docs/CURRENT_STATE.md / MILESTONES.md before selecting work. On continue, select the next eligible bounded task from Trello, respecting milestone acceptance and the current authorized scope. The user's October 8 follow-up permits independent planning/audit tasks while M7/M8 manual acceptance waits; it does not make those gates passed.
2. Reconcile stale cards with actual source/log/report evidence. Move the active bounded task to Doing; describe dependencies, scope, next action and evidence. Do not infer a manual pass from automation or generic Done cards.
3. Immediately when a task completes, update its existing card with changed areas, exact build/test verdicts, report paths, warnings and remaining limits, then move that task Done. Update it before beginning another task. A narrow test can be Done while its parent milestone remains Doing.
4. On failure, retain Doing and state the exact failed assertion/error and next diagnostic. Mark a genuine dependency as blocked in its description. Preserve failed evidence and record a successful retry separately.
5. Refresh Docs/CURRENT_STATE.md after meaningful changes/results and verify remote Trello writes by reading the affected board/card. Preserve unrelated cards, members, assignments and deadlines. Do not create duplicate boards/cards, invent dates or upload private save/profile/credential data.

## User action marker

Start every project card title with **(Personal)** or **(AI)**, as explicitly requested. Personal means the next required action needs the user (manual sustained playtest, physical controller check, or unavailable setup/approval). State the exact action in the description. AI means independently actionable agent work or future agent implementation plans. Reevaluate the prefix when the dependency is resolved; future manual acceptance does not make every future milestone Personal now. Milestone numbers remain in the title. This is next-action ownership, not a claim of historical authorship. The earlier [YOU] marker is superseded.

## Existing boards

- [Main Board](https://trello.com/b/XqitZ1Fg/primal-frontier-main-board): M0–M25; milestone acceptance and current implementation status.
- [Programming](https://trello.com/b/YZIEdyWN/primal-frontier-programming): authoritative code, persistence and later progression.
- [Greyboxing](https://trello.com/b/JA3pN8Ix/primal-frontier-greyboxing): prototype maps/world distribution and traversal.
- [HUD & UI](https://trello.com/b/dhjVbNNj/primal-frontier-hud-ui): first-person presentation, controls, menus.
- [Sound & Audio](https://trello.com/b/Hbm7xohf/primal-frontier-sound-audio): audio preferences/coverage and later authored sound.
- [Testing](https://trello.com/b/WwUL7l6L/primal-frontier-testing): automated checkpoints and separate manual acceptance.

All use To Do → Doing → Done. No Level Design or Game Design Document board is required. Milestone descriptions come from FULL_PROJECT_ROADMAP.md; future cards do not themselves authorize implementation or skipping acceptance. Only independent planning/audit work proceeds under the user's latest continuation instruction while M7/M8 manual gates are open.

## October 8 audit

All six layouts were read. M0–M6 remain historical supported Done checkpoints; M7/M8 remain Doing; M9–M25 remain To Do. Corrected literal newline escapes/generic descriptions, inserted the actual roadmap objectives/playtests/criteria, cross-linked discipline tasks, and moved recorded completed implementations/tests out of To Do. The overlapping M7 generic test card now tracks physical controller feel, leaving the dedicated route/overnight card as its own gate.

Native evidence/gate checklists were added and read back on M7/M8; manual/rendered criteria remain unchecked. NullRHI persistence create/restart is a completed test task; full M8 remains open. Packaged Server build is blocked by the installed engine distribution. All 53 open cards were read back across six boards: no literal newline escapes remain, layout/status matches the audited scope. No memberships, dates, assets or new gameplay were changed for this audit. Git synchronization remains pending.

Readback counts (To Do / Doing / Done): Main 17/2/7; Programming 2/1/2; Greyboxing 2/2/1; HUD & UI 2/1/2; Sound & Audio 5/0/0; Testing 3/2/2. Future cards are plans; authored audio has not been accepted. Legacy TRELLO_BOARD_PLAN.md / TRELLO_SETUP_PROMPT.md are preserved from the other setup chat, excluded from this scoped M8 commit.
