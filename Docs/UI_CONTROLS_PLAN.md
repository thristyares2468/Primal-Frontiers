# Controls, reconnect feedback and research UI plan — M11–M13

October 8, 2026. Independent planning complete. **M11 read-only controls/help is now implemented and verified within its bounded scope:** [CONTROLS_HELP_M11.md](CONTROLS_HELP_M11.md). Stable stack selection and survival damage/urgent/possession feedback are implemented; see INVENTORY_SELECTION_M11.md and SURVIVAL_FEEDBACK_M11.md. Normal1440p VSM-budget check now passes the bounded profile fix in SHADOW_BUDGET_M11.md. Full M11 remains in progress; remapping and M12/M13 research/progression views are not implemented. Latest continuation authorization allows independent implementation while Personal tests remain deferred/unverified. Historical baseline below describes the planning checkpoint, not every later change.

## Source-verified baseline

- `PFSurvivalPlayerController.cpp` binds survival keyboard actions with `BindKey`. `PFGamepadInput.cpp` uses the same authoritative handlers with contextual face/D-pad controls. Movement/look/jump use the existing Enhanced Input template context. A remapping feature must address both routes; storing a preferred key alone cannot change these fixed bindings.
- Pause/settings exist. Solo pause stops simulation; multiplayer menus do not pause the world. The settings draft/apply/cancel and display-confirmation behavior is documented in [SETTINGS.md](SETTINGS.md). P is reliable in PIE; Esc may stop PIE. Controls/preferences are local presentation, not world-save authority.
- Native survival HUD has a centered aim marker, interaction/refusal prompt and numeric vitals. It refreshes at 10 Hz, reacquires the current pawn and has Blueprint presentation hooks. HUDScale changes these labels; inventory/crafting/building still use their own placeholder layouts. Uniform panel text scaling/reflow is not established by the existing setting.
- Inventory UI shows capacity, weight, stack selection and batch freshness. It is a non-clickable text panel; split/drop/consume use controls, not drag-and-drop. Overlays are mutually exclusive. [PLAYTEST.md](PLAYTEST.md) is the current authoritative user guide.
- Client credential persistence reports success/failure in logs, omitting the credential. Restore failure returns a reason to the main menu. No complete reconnect/loading or world-save feedback screen exists. Private profiles depend on host spelling/port/profile; no production account system is present.
- Progression/research/adaptations exist only as plans in [PROGRESSION_PLAN.md](PROGRESSION_PLAN.md). No XP, research scanner, unlock transaction, adaptation or equipped-modifier UI backend exists.

Sources: survival controller/gamepad/HUD, inventory/crafting/building HUDs, PFSettingsMenu/GameUserSettings and PFLocalPlayer/WorldPersistence. Historical PF.Input.Gamepad and PF.Settings.Preferences passed in M8FinalRegression; that does not certify physical controller feel or the new plan.

## First implementation slice: read-only controls and feedback

Begin with a small controls/help panel and clearer current-state feedback. Use existing text/primitive styling; no icon pack or external dependency is needed. Preserve the first-person center view and native crosshair. Blueprint/UMG may own presentation/tuning; C++ supplies validated state and action results.

| Surface | Required behavior |
| --- | --- |
| Controls help | Show the actual action/key for the current mode and device family. Explain P versus PIE Esc, pickup range/aim, finite food and pause safety. No claim that scenery is an item. |
| World prompt | Target display name, actual interact action, reach/blocked/full/depleted reason. Building mode still permits nearby pickup; unavailable actions remain explained. |
| Inventory | Selection by stable stack ID, name/quantity, remaining freshness, weight/slots and usable-state reason. Reconcile selection when a stack expires/splits/disappears. |
| Crafting/building | Display real cost/queue/support/owner refusal. Do not show success until the server accepts; cancellation/duplicate input cannot create an optimistic item or structure. |
| Respawn/join | Explicit waiting versus restored state; reacquire the new pawn and PlayerState. Hide stale target/action hints during transition. No locally granted vitals/items. |
| Multiplayer menu | Keep a clear “world continues” cue. Do not imply safety or pause enemies/food. |

Prefer a single reviewed action-description source for help and context prompts, tied to actual input actions/handlers. Avoid another hardcoded table that diverges from gameplay. Device changes update labels without stealing focus or duplicating input. Text-only controller names are sufficient initially; generic original glyphs can follow approved asset intake later.

One input has one meaning. Gamepad X consumes in bag, crafts in crafting, otherwise interacts; RT places in build mode or attacks with overlays closed. A help/settings panel consumes its own input. Closing it must not accidentally fire, consume, split or demolish in the game. Dangerous actions need explicit target/quantity/ownership feedback and deliberate confirmation where appropriate; avoid automatic selection of a different stack after expiry.

## Later remapping and accessibility

Remapping follows the read-only help gate. Preserve old settings until a validated draft is applied; support cancel/default restoration, conflicting actions and required navigation bindings. Ensure keyboard and controller context routes actually use the mapping; do not add a cosmetic menu over fixed BindKey handlers. Keep local key preferences out of server world saves.

Use deterministic focus/navigation and a consistent Back route, with controller-only access to every selectable setting. Test disconnect/reconnect of a real physical controller separately; native binding tests cannot certify hardware. Stick dead zones, independent axis sensitivity, remapping, vibration and device glyphs are future work, not existing features.

Reflow rather than only enlarge fonts. Future tests cover 1280x720 and 2560x1440, full allowed text scales, long names/localized strings and narrow aspect ratios without covering aim/health/actions. Communicate urgent states with text/numbers/shape as well as colour; colour correction is not proof every panel is readable. Test focus restoration after pause/display-confirmation timeout and respawn. Do not advertise screen-reader or full accessibility certification without a real implementation and assistive-device test.

## Reconnect and save feedback contract

| State | Player-facing message / allowed action |
| --- | --- |
| Connecting | Show endpoint display name and wait/cancel; no private capability or full login URL. |
| Restoring | Explain that saved state is being checked; keep gameplay mutations disabled until possession/restore succeeds. |
| Restored | One acknowledgment of server-owned restoration; no replayed gathering/craft/loot feedback. |
| Local profile write failed | “Reconnect details could not be saved on this device.” Explain that later reconnect may fail; log sanitized cause. Never claim persistence succeeded. |
| Unknown/duplicate identity | Explain which condition occurred and permit retry/back. Never silently claim another player's public ID or overwrite their record. |
| Startup/map/version/corrupt-save refusal | Explain server load failure and preserve saves. Give a sanitized diagnostic reference for the host, not a client repair/delete button. |
| World saved | Acknowledge only successful authoritative publication; include safe slot/time if appropriate. Client UI cannot perform an arbitrary save/load RPC. |

A future UI must not turn development GUID capabilities into production authentication. Loading arbitrary slots or deleting profiles/world files remains outside player controls. [SAVE_COMPATIBILITY_PLAN.md](SAVE_COMPATIBILITY_PLAN.md) defines future recovery boundaries; current PF.SaveWorld/PF.LoadWorld remain server developer commands.

## Original progression/research interfaces

Implement these only when their validated C++ backend exists. Use a readable list/detail layout and project-specific labels, not the supplied games' protected UI skins, names or art.

- Technology details show cost, earned/unspent points, level/prerequisites, required station/materials, learned state and exact lock/refusal reason. Baseline tools/food/shelter remain available under the progression plan. Unlocking knowledge is separate from spending recipe ingredients to craft.
- Discovery/research shows known versus incomplete evidence and the next available action. Do not reveal hidden content through an unrestricted catalog dump or pretend a scan granted an unlock before the server result.
- Permanent adaptations and equipped passive/active modifiers are distinct pages/states. Show original effect/tradeoff, slot budget, conflicts, required owned station and change/activation cost. Active cooldown reflects authoritative state; loading or reconnecting cannot reset it.
- Present pending/accepted/refused transactions, dedupe repeated input and keep the second player's independent unlocks private. Recompute views from replicated state, not UI reward animations. Never grant points, effects or save fields in Blueprint presentation.

## Narrow gates

1. Compile Editor and run the smallest model/argument/focus tests. Verify displayed bindings match actual handlers and one press cannot reach two actions; invalid IDs and server refusals remain explicit. Existing gamepad/preferences tests are regression prerequisites, not tests of unimplemented remapping.
2. Render one player on L_PrimalFrontier_OpenWorld: pickup, expired-stack selection, craft/build refusals, waiting/death/respawn, pause/settings and controls help. Compare readability at the proposed resolutions/scales; capture evidence. Physical device navigation/hot-plug remains Personal.
3. Run one client/server, then low-memory two-client checks for accepted/refused results, independent inventories/unlocks and reconnect. NullRHI proves state/events only; inspect rendered feedback separately. No one-client menu may pause the server or duplicate actions.
4. Add one progression/research transaction only after its backend gate. Test affordability/requirements, repeat requests, late replication, save/restart and stale widgets. No broad UI/art pass is necessary to prove one original choice.

Record uncapped frame times, memory and stutter for rendered tests; avoid large widget-per-item grids, every-frame inventory-string rebuilding or unrestricted catalogs. Proposed data/model events and bounded visible lists need measurement before a performance claim. No new FPS/audibility/manual result is implied by this document.

Planning verification is source/document consistency and link/whitespace checks only. Latest runtime evidence remains the M8 corpse/living restoration regression and successful Editor/Game builds. M7 route/overnight, M8 rendered persistence, physical controller, provenance and full M11–M13 acceptance remain open.
