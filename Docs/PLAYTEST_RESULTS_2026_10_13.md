# User playtest results — October 13, 2026

These are direct user reports against the named test batch, not new AI launches. The batch guide targets `/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld`, one normal standalone session. The reply did not separately state resolution, frame rates, durations or device specifications; none are inferred.

| Result name | Reported result | Accepted scope |
| --- | --- | --- |
| ECOLOGY_ENCOUNTERS | PASS | First-person passive/hostile encounters, reaction, chase and escape from the provided test. |
| ECOLOGY_DAWN | PASS | Dawn behavior and resident retention from the provided test. |
| BUILD_ROTATION | PASS | Ahead-facing wall placement and R rotation. |
| BUILD_PIECES | PASS | Eight-piece picker, supported pieces and traversal/invalid-placement checks from the provided test. |
| MENU_NAVIGATION | PASS | Contextual B/Esc closure, pause and return to normal input. |
| CRAFTING_UI | PASS | Centered item-picture/category crafting view, actual basic craft/cancel and restored input. |
| SPRINT_WEIGHT | PASS | Sprint eligibility, weight slowdown/refusal and unloading recovery. |
| JUMP_WEIGHT | PASS | User repeated the weight test with jumping; jump threshold accepted. |
| GLIDE | PASS | Glide activation, descent, stamina and stopping/landing from the provided test. |
| CREATURE_LOOT | PASS | Named corpse payouts, expiry and repeat-loot refusal from the provided test. |
| WORLD_SPACING | PASS | Resource/hazard/landmark/encounter spacing from the provided test. |
| WORLD_RENAME | PASS | Menu rename and same-world restoration from the provided test. |
| CLIMB | NOT TESTED | User could not complete it because suitable geometry was unavailable. The batch originally described manual F; subsequent automatic jump-contact implementation has separate AI technical evidence. Neither has a human climb pass. |
| CONTROLLER | UNTESTED | No physical device or hot-plug acceptance. |
| AUDIO | UNTESTED | No listening, route, mute or output-device acceptance. |

There are **12 named passes and three untested checks**. Existing accepted chest/filter, solo Save/Load, Pause, Settings and M7 route remain accepted and require no repetition.

M13's supported greybox ecology encounter gate is now accepted alongside its prior native, rendered and one-/two-client restart evidence. This resolves that dependency for a bounded M14 greybox step. Optional predator/prey behavior, taming, adaptations, final art, sustained FPS and production-scale ecology are not certified or implemented by this reply.

M11's reported keyboard/mouse building/menu/crafting checks are accepted. Broader guide portions not reported individually—expiry/removal selection, standalone display rollback, optional earned progression pacing, physical controller and audio—are not silently converted to passes. These remain separate from independently actionable AI work. M8 supported solo persistence additionally has rename acceptance; rendered multiplayer and packaged Server remain separate. The user explicitly deferred 1440p.

## Next development

Automatic jump-contact climb has completed its technical gate: see [AUTOMATIC_CLIMB_M1.md](AUTOMATIC_CLIMB_M1.md). Provide accessible greybox ledge geometry in a separately scoped M14 traversal step, using primitives and supported Editor APIs, before requesting another human climb observation. Preserve existing resource/spawner identities and save layout; adding persistent actors requires compatibility work first. No additional immediate user action is required to record these results.

The user confirmed that “proxy chat” means **proximity voice chat**. It is recorded as future M15 multiplayer work in [PROXIMITY_VOICE_PLAN.md](PROXIMITY_VOICE_PLAN.md), not implemented or verified. Physical controller/audio stay on their existing Personal cards; do not create repeat assignments for the passed tests.
