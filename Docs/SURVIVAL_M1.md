# Milestone 1 survival foundation

Scope: Health and Stamina only. Hunger/Thirst tags reserve interfaces and remain unsupported. No inventory, persistence, crafting, creatures or third-person mode is included.

## Composition

`APFSurvivorCharacter` extends the existing first-person C++ template without changing its assets. `UPFPlayerSurvivalComponent` owns one replicated vitals struct, validates finite inputs, clamps values, and rejects every mutation without authority. Death is derived from Health and exposed as a Gameplay Tag. The Unreal damage pipeline feeds server-owned Health. Jumping costs 20 Stamina; recovery is 10/second after a 1.5-second delay.

`APFSurvivalGameMode` requires a real PlayerStart and replaces the dead pawn after three seconds. Health and Stamina reset on the replacement pawn. A failed spawn retains the dead pawn for an explicit retry. The controller survives respawn and its placeholder UMG HUD reads the current possessed pawn. Blueprint subclasses can replace presentation through `PresentVitals`.

`Scripts/SetupSurvivalMilestone1.py` creates project-owned Blueprint copies and a six-actor primitive test map at `/Game/PrimalFrontier/Maps/L_M1Survival`. It refuses to overwrite existing destinations. Run through Unreal's `-ExecutePythonScript` editor option. No template assets or default project map settings are rewritten.

## Developer commands

Use the authoritative standalone/server console. Mutations are rejected on clients, including local calls to component setters. No arbitrary client damage RPC exists.

| Command | Meaning |
| --- | --- |
| `PF.Help` | Discover command help and implementation status |
| `PF.Damage 25` | Apply damage through the possessed survivor's Unreal damage pipeline |
| `PF.SetHealth 75` | Set the living survivor's clamped Health |
| `PF.SetStamina 0` | Set clamped Stamina; depletion starts the recovery delay |
| `PF.Kill` | Apply lethal damage |
| `PF.Respawn` | Immediately replace a dead survivor at a PlayerStart |
| `PF.ExportTestReport M1Manual` | Export command results with timestamps, map and details |

These hooks require exactly one possessed player to avoid ambiguous targeting. They belong to the non-Shipping PrimalAgentTools runtime module; gameplay does not depend on the tooling plugin. Reports include explicit rejected results for client authority probes; those are expected rejections, not gameplay passes.

## Verification

- `PF.Survival.Component`: defaults, damage, stamina, recovery, invalid inputs, authority guards and death tags.
- `PF.Survival.Lifecycle`: native transient world, actual GameMode/PlayerStart possession, damage and jump cost, three timed death/respawn cycles. The fixture intentionally has no skeletal presentation asset; its expected head-socket warnings are narrowly matched. Blueprint presentation is verified separately.
- `PF.Survival.Live`: opt-in isolated `-game` or `-server` process with `-PFRunSurvivalLiveTests`. Starts at 100/100, applies 25 damage and sets 20 Stamina after 20 seconds, holds that replication sample for 15 seconds, kills, then respawns after eight seconds. A client must independently observe every transition and reject mutation commands. Timeout is a failure. The temporary recovery/respawn tuning affects only this disposable test session.

Use NullRHI for nonvisual tests and one rendered window for manual first-person testing. An editor-only unit pass does not prove wire replication or a playable HUD. See `MILESTONES.md` for actual results and evidence, rather than treating this procedure as a pass.
