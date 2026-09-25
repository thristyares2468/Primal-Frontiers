# Development command interface

## Integration and availability

`PrimalAgentToolsRuntime` uses `APlayerController`, `APlayerState::GetPlayerId`, `ACharacter`, its capsule and CharacterMovement, `UWorld` collision queries, `TeleportTo`, and `ForceNetUpdate`. The project's `APrimalFrontierCharacter` and `APrimalFrontierPlayerController` inherit these APIs, so the first-person camera/input setup is preserved. The existing Blueprint game mode selects the project pawn. No character, controller, game mode or binary asset was changed.

Milestone 1 adds an isolated survival character with authoritative Health/Stamina and PlayerStart respawn. The runtime module depends on `PrimalFrontier` to call those validated APIs; gameplay has no dependency on tooling. Project-owned Blueprint copies and a primitive map are separate from the templates. The earlier paragraph describes the Phase 10 teleport integration.

The checkout contains FirstPerson, Shooter and Horror templates. Shooter HP/weapon arrays are not a survival inventory. Item registry, inventory, creature registry/spawner, time, gameplay save/load, gathering, crafting and construction remain explicit stubs. M1 implements Health/Stamina and M2 adds Hunger/Thirst/Exposure; see `Docs/SURVIVAL_M1.md` and `Docs/SURVIVAL_M2.md` for the composition and tests.

The runtime module is available in Development, DebugGame and Test. Shipping excludes the plugin through the project reference and both module descriptors; runtime build rules additionally reject Shipping. Registration and execution are compile guarded. The editor module alone depends on UnrealEd, AssetRegistry and DataValidation. No MCP server, remote command endpoint, RPC or additional external plugin is introduced.

## Commands

All commands have console help, appear in `PF.Help`, and log with `[PrimalAgentTools]`. Arguments are positional and extra arguments fail. Registry identifiers permit only 1–64 ASCII letters, digits, underscore and hyphen; object paths cannot load arbitrary creature classes.

| Command | Implementation |
| --- | --- |
| `PF.Help` | Lists every command, usage and current backend status. |
| `PF.ValidateAssets [/Game[/Subfolder]]` | Editor: real Unreal Data Validation on saved assets. |
| `PF.CheckNaming` | Editor: project prefix checks under `/Game/PrimalFrontier`; empty scope needs attention. |
| `PF.CheckReferences [/Game[/Subfolder]]` | Editor: missing saved package dependencies and redirectors; no fix-up. |
| `PF.ResetTestWorld` | Editor: reset only four owned fixtures in approved `L_Automation`, undoable and unsaved. Runtime gameplay reset is NOT IMPLEMENTED. |
| `PF.PlaceTestActor` | Editor: upsert owned `PF_TestCube` in approved `L_Automation`. |
| `PF.RunSmokeTest` | Editor: combines asset validation, naming and package references. Records each child check; excludes gameplay coverage. |
| `PF.CaptureTestScreenshot [label]` | Editor: synchronous rendered viewport PNG. Runtime/PIE unavailable. |
| `PF.ExportTestReport [label]` | JSON command history to project-local `Saved/AutomationReports`. |
| `PF.Teleport X Y Z [PlayerId]` | Real server-side possessed-character teleport. See safety contract below. |
| `PF.GiveItem ItemId Quantity` | NOT IMPLEMENTED: item registry/inventory absent. Syntax requires `Item_` identifier and positive int32 quantity. |
| `PF.SpawnCreature CreatureId` | NOT IMPLEMENTED: creature registry/spawner absent. Simple registry ID only. |
| `PF.SetHealth Value` | Server-only: set living survivor Health, clamped to its configured maximum. |
| `PF.SetStamina Value` | Server-only: set clamped Stamina; depletion delays recovery. |
| `PF.Damage Amount` | Server-only: positive finite damage through the survivor damage pipeline. |
| `PF.Kill` | Server-only: lethal damage, followed by automatic respawn. |
| `PF.Respawn` | Server-only: immediately respawn a dead survivor at a valid PlayerStart. |
| `PF.SetHunger Value` | M2 server-only food reserve, clamped 0..100. Empty causes starvation. |
| `PF.SetThirst Value` | M2 server-only water reserve, clamped 0..100. Empty causes dehydration. |
| `PF.SetExposure Value` | M2 server-only test exposure 0..1; zero restores volume-only exposure. |
| `PF.RecoverNeeds` | M2 server-only recovery of 35 food/water through the component API. |
| `PF.SetTimeOfDay Hour` | NOT IMPLEMENTED: time API absent. Finite hour from 0 through 23. |
| `PF.SaveWorld` | NOT IMPLEMENTED: gameplay save API absent. Never saves an editor map. |
| `PF.LoadWorld` | NOT IMPLEMENTED: gameplay load API absent. |
| `PF.TestGathering` | NOT IMPLEMENTED: gathering system absent. |
| `PF.TestCrafting` | NOT IMPLEMENTED: crafting system absent. |
| `PF.TestBuildingPlacement` | NOT IMPLEMENTED: construction system absent. |
| `PF.TestCreatureAI` | NOT IMPLEMENTED: survival creature AI absent. |
| `PF.TestMultiplayerReplication` | NOT IMPLEMENTED: no survival acceptance scenario. Transport/teleport verification below is narrower. |
| `PF.TestPersistence` | NOT IMPLEMENTED: gameplay persistence absent. |
| `PF.ResetAutomation` | Compatibility alias of `PF.ResetTestWorld`. |
| `PF.CaptureScreenshot [label]` | Compatibility alias of `PF.CaptureTestScreenshot`. |
| `PF.ExportResults [label]` | Compatibility alias of `PF.ExportTestReport`. |

Unknown item/creature IDs cannot be resolved without registries. Well-formed unknown IDs return NOT IMPLEMENTED, not a fabricated “unknown registry entry” validation or successful mutation. Malformed identifiers fail argument validation.

## Authority and teleport safety

Gameplay mutations never select another world or forward a client request. A network client is rejected before mutation, including future stub commands. Teleport requires a running game/PIE world with an authoritative GameMode and possessed ACharacter. On a server with multiple players, supply the authoritative `PlayerState.PlayerId`; omission fails as ambiguous. No arbitrary actor name lookup is supported.

Coordinates must be finite decimal centimetres within ±100000 on every axis (a 1km development envelope). The character capsule must not overlap blocking collision, must remain above KillZ, and must have ground with upward normal Z >= 0.5 within 100m below it. Unreal's collision-aware `TeleportTo` performs the move, then movement velocity is stopped and a net update requested. These checks do not model streaming readiness, gameplay restricted areas or a future map-boundary subsystem; integrate those before broadening this developer envelope.

Reset is limited to editor-owned fixtures documented in README. It refuses the wrong map, PIE, ownership/name conflicts, nonpersistent levels and early editor startup. It does not delete actors, files, saved games or directories, and does not save maps. Runtime reset remains a blocker until a real development-state API exists.

## Reports

Schema 2 records each invocation's exact name (including aliases), UTC timestamp, map or `Unavailable`, result, arguments and structured details. Results distinguish `Passed`, `Failed`, `NeedsAttention` and `NOT IMPLEMENTED`. Report filenames use a validated label, UTC timestamp and GUID. Exports are process-local, so server and clients produce separate reports. Failed checks remain failed even when exporting succeeds. The export request is included in its snapshot; the completed write result and artifact path enter history for the next export.

## Automated and multiplayer procedure

Build `PrimalFrontierEditor Win64 Development` after C++ changes. Build `PrimalFrontier Win64 Development` to check noneditor dependencies. `PF.PrimalAgentTools` runs parser, blocker, serialization, transient-world teleport, fixture idempotence and viewport tests. Run the suite in an isolated editor on `/Game/Maps/L_Automation` with `-PFRunScenarioTests -PFRunViewportTest -RenderOffscreen -NoSaveConfig`, and `-ExecCmds="PF.Help,Automation RunTests PF.PrimalAgentTools" -TestExit="Automation Test Queue Empty"`. The two fixture/viewport flags authorize test-only unsaved modifications and capture; never run those tests in an editor containing unsaved user work.

Live test: launch `UnrealEditor-Cmd.exe PrimalFrontier.uproject /Game/FirstPerson/Lvl_FirstPerson -game -PFRunLiveTests -RenderOffscreen -unattended -NoSaveConfig -ExecCmds="Automation RunTests PF.LiveDeveloperCommands" -TestExit="Automation Test Queue Empty"`. Use an absolute project path. The test waits 30 seconds for players, teleports the real controlled project character, validates its resulting position, exercises all registered commands, waits 20 seconds, then exports a snapshot. Editor-only commands correctly report unavailable in this game process.

For multiplayer, use the same executable/project with `/Game/FirstPerson/Lvl_FirstPerson -server -port=17777 -nullrhi` instead of `-game`. Start two clients with `127.0.0.1:17777 -game -RenderOffscreen`. Give all three `-PFRunLiveTests`, the live-test ExecCmds/TestExit arguments and unique `-log` and `-ReportExportPath` values. Start clients promptly after the server. The server requires exactly two players, targets each by PlayerId, and remains alive 30 seconds after its snapshot. Clients assert that teleport, item grant, save, load and reset requests are rejected with `NotAuthority` and leave their immediate pawn location unchanged.

Compare each process's `[PrimalAgentTools] ReplicationSnapshot PlayerId=... Role=... Location=...` lines by PlayerId. Both clients must see both server positions after settling. Record server/client test results separately from the cross-process position comparison. A real dedicated-server executable additionally requires a Server target and an engine distribution supporting server builds; this checkout currently has only Game and Editor targets. Editor `-server` exercises an uncooked dedicated-server world.

Item grant, registry resolution, creature spawning, gameplay persistence and runtime reset integration tests remain blocked until their actual systems exist. Health/Stamina and M2 needs use real survival APIs. The `PF.Survival` automation and opt-in `PF.Survival.NeedsLive` test cover those implemented systems; unrelated stubs continue to return NOT IMPLEMENTED.
