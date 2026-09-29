# PrimalAgentTools

UE 5.8.2 C++ developer plugin. A non-Shipping runtime module owns command registration, argument validation, history and server-side character teleporting. The editor module provides asset checks and unsaved fixtures. No MCP server, listener, remote execution interface or Python scripts.

See [DEVELOPER_COMMANDS.md](DEVELOPER_COMMANDS.md) for the current commands, integration blockers and multiplayer verification. Milestone 2 adds real hunger/thirst/exposure and recovery adapters; [SURVIVAL_M2.md](../../Docs/SURVIVAL_M2.md) describes their gameplay and test contracts. All tooling commands remain excluded from Shipping.

## Enable and use

Milestone 5 adds `PF.TestBuildingPlacement` for runtime structure integrity and `PF.ResetBuildings` for scoped, empty M5 test structures. See [building contract](../../Docs/BUILDING_M5.md) for placement, ownership, storage and test limits. These commands remain excluded from Shipping.

Milestone 3 implements `PF.GiveItem ItemId Quantity` and `PF.RemoveItem ItemId Quantity` for the sole authoritative player. See [INVENTORY_M3.md](../../Docs/INVENTORY_M3.md) for capacity, food freshness, gameplay controls and live replication tests. There is no client grant RPC.

The project enables this plugin for Editor, Game, Client and Server targets except Shipping. Build `PrimalFrontierEditor Win64 Development`, then restart the editor to load the newly built module. Open **Output Log** and enter commands in its console field. All plugin messages contain `[PrimalAgentTools]`. These are synchronous commands: validation can take time while assets load.

| Command | Behavior |
| --- | --- |
| `PF.ValidateAssets [/Game[/Subfolder]]` | Run Unreal's Data Validation subsystem on saved assets, including external objects. Default `/Game`. Report invalid, skipped, warning, and unvalidated counts. No fix-up or save. |
| `PF.CheckNaming` | Check saved assets only under `/Game/PrimalFrontier`. Empty scope and unknown class rules are `NeedsAttention`, not a clean pass. |
| `PF.CheckReferences [/Game[/Subfolder]]` | Inspect saved hard/soft package dependencies, missing packages, redirectors, and references through redirectors. Default `/Game`. Never fix redirectors or rename assets. |
| `PF.ResetAutomation` | Create or reset the four owned fixture actors in the **currently open** `L_Automation` map. Repeat calls reuse actors. |
| `PF.PlaceTestActor` | Create or reset only the known `PF_TestCube`. Repeated calls do not add duplicates. |
| `PF.CaptureScreenshot [label]` | Read a rendered level-editor viewport and synchronously write a PNG. Default label `Viewport`. Fails explicitly in PIE, commandlets, NullRHI, or without a viewport. |
| `PF.ExportResults [label]` | Write a JSON snapshot of this editor session's command history, with per-command status, issue codes, asset paths, counts, and artifact paths. Default label `Results`. |

Output is restricted to **`Saved/AutomationReports`**. Labels allow 1-64 ASCII letters, digits, `_`, and `-`; they cannot be paths. Each output has a UTC timestamp and GUID, so repeated calls preserve earlier reports. `schemaVersion` is 2. Every execution includes `testName`, UTC `timestamp`, `map`, `result`, and structured `details`. Unavailable gameplay systems return `NOT IMPLEMENTED`, never `Passed`. Status is `Failed` if any error exists, `NeedsAttention` for warnings/incomplete coverage, otherwise `Passed`. An empty export is `NoResults`. Export success means the file was written; the report's aggregate status still reflects failed checks.

## Scenario contract

Open the existing `/Game/Maps/L_Automation`, select its persistent level, and stop PIE before changing fixtures. Wait for the editor to finish its initial frames: scenario commands reject first-frame startup `-ExecCmds` mutations, which can trigger a UE 5.8 Scene Outliner/Slate visibility ensure. Use the automation suite for batch scenario work. `/Game/PrimalFrontier/Maps/L_Automation` is also accepted for future project-owned content. The plugin never loads a different map, moves the existing map, or overwrites it. If neither map exists, create a blank map in Unreal and save it at the canonical path before running the command.

| Role / label | Native actor | Location (cm) | Rotation (degrees) | Scale |
| --- | --- | --- | --- | --- |
| `PF_TestCube` | StaticMeshActor / Engine BasicShapes Cube | 0, 0, 100 | 0, 0, 0 | 1, 1, 1 |
| `PF_TestFloor` | StaticMeshActor / Engine BasicShapes Cube | 0, 0, -25 | 0, 0, 0 | 20, 20, 0.5 |
| `PF_TestStart` | PlayerStart | -400, 0, 100 | 0, 0, 0 | 1, 1, 1 |
| `PF_TestLight` | PointLight | 0, 0, 500 | 0, 0, 0 | 1, 1, 1 |

Fixtures use fixed transforms, movable components, disabled mesh physics, BlockAll mesh collision, and a white point light with intensity 3000 and attenuation radius 2000 cm. A point light avoids competing with an existing directional sun. Existing unrelated actors, lighting, world settings, and gameplay are preserved. Thus the fixture is deterministic; this does not guarantee deterministic simulation or identical screenshots of arbitrary pre-existing map content.

Actors have an editor ownership tag `PrimalAgentTools.Scenario.v1` and a role tag. These are editor metadata, not gameplay state; gameplay continues to use Gameplay Tags. Only exact native actor classes with the matching ownership/role are modified. Name collisions, duplicate roles, wrong classes, wrong maps, nonpersistent current levels, and PIE are refused before mutation. Actors are always spatially loaded and marked editor-only, so fixtures are excluded from cooking. Never manually remove ownership tags or make owned actors spatially loaded. Reset does not delete unrelated actors or extras from other fixture versions.

Each operation is a single Undo transaction. Changes remain unsaved; use Ctrl+Z to undo or save manually after review. A rare spawn failure is reported and earlier work in the transaction can be undone. No plugin class is stored in the map: fixtures are native Engine actors.

## Naming policy

Rules follow existing project/template conventions where observed: `L_` maps, `BP_` Blueprints, `BPI_` interfaces, `UI_` widget Blueprints, `ABP_` animation Blueprints, `SM_` static meshes, `SKM_` skeletal meshes, `SK_` skeletons, `M_` materials, `MI_` instances, `MF_` material functions, `T_` textures, `IA_` input actions and `IMC_` input contexts. Additional v1 defaults include `DA_` data assets, `DT_` data tables, `MPC_`, `RT_`, `PHYS_`, `A_`, `AM_`, `BS_`, `S_`, `SC_`, `NS_`, `NE_`, `CT_`, `E_`, and `F_`. Custom DataAsset subclasses inherit `DA_`; otherwise unknown asset classes need a reviewed rule in `PFAssetChecks.cpp`.

Prefixes are case sensitive and require a nonempty suffix. Names use the same 64-character ASCII limit as labels. Existing template content outside `/Game/PrimalFrontier` is deliberately outside naming enforcement. Nothing is renamed automatically.

## Limits and dependencies

- The only explicit plugin dependency is Epic's **DataValidation** editor plugin, used for supported `UEditorValidatorSubsystem` validation. Other dependencies are engine C++ modules: AssetRegistry, UnrealEd, JSON, rendering/image utilities, and Projects for plugin metadata verification. Editor Scripting Utilities and Python are not necessary for this version.
- Data Validation runs installed native/Blueprint validators and `IsDataValid`. Assets with no applicable validator are not proof of validity. Loading may compile derived data or update in-memory state; this plugin never saves those assets.
- Reference checks use **saved package dependencies**. Transient-package dependencies are reported as a distinct warning, not a missing disk file. Checks do not fully validate object/subobject paths within existing packages, unsaved edits, native script imports, or paths built dynamically at runtime. Pair them with asset validation, map checks, and gameplay/network tests.
- Screenshot success proves pixel readback and PNG creation, not image comparison, visual correctness, network correctness, or a performance benchmark.
- This version does not add gameplay, replication, persistence, or a server target.

## Shipping exclusion

The `.uproject` reference and both module descriptors deny Shipping. The runtime module build rules also reject Shipping and command registration is compile-guarded with `!UE_BUILD_SHIPPING`. The editor module remains Type=Editor and cannot enter a game package. The runtime module has no UnrealEd or DataValidation dependency. Development, DebugGame and Test are allowed; `ECVF_Default` keeps commands available in Test, with explicit world authority checks rather than relying on cheat flags. Fixture actors remain editor-only.


## Milestone 4 integration

`PF.TestGathering` and `PF.TestCrafting` now perform read-only runtime integrity checks on real resource nodes and player recipe catalogs/queues. `PF.Craft RecipeId` and `PF.CancelCraft` use the sole player's authoritative timed craft API. These commands are non-Shipping, logged and exported like the existing interface. `PF.Crafting.Transactions`, `PF.Crafting.Gathering` and the opt-in `PF.Crafting.Live` test verify gameplay separately; command integrity alone does not certify the complete loop. No MCP server is added.

## Verification

See [VERIFICATION.md](VERIFICATION.md) for the completed acceptance run, exact findings, evidence, and remaining manual setup.

Run the narrow suite `PF.PrimalAgentTools` in an **isolated editor process**, not a working editor with unsaved user changes. Scenario and screenshot integration tests require explicit flags; absent flags produce a setup warning and do not exercise those features.

```powershell
& 'C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat' PrimalFrontierEditor Win64 Development '-Project=C:\UnrealProjects\PrimalFrontier\PrimalFrontier.uproject' -WaitMutex -NoHotReloadFromIDE

& 'C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'C:\UnrealProjects\PrimalFrontier\PrimalFrontier.uproject' /Game/Maps/L_Automation -unattended -nop4 -nosplash -nosound -NoSaveConfig -NoLiveCoding -RenderOffscreen -PFRunScenarioTests -PFRunViewportTest '-ExecCmds=Automation RunTests PF.PrimalAgentTools' '-TestExit=Automation Test Queue Empty' '-ReportExportPath=C:\UnrealProjects\PrimalFrontier\Saved\AutomationReports\EngineVerification' '-log=PrimalAgentToolsVerification.log' '-ini:EditorPerProjectUserSettings:[/Script/UnrealEd.EditorLoadingSavingSettings]:bAutoSaveEnable=False'
```

`PolicyAndReports` checks path guards, naming rules, descriptor restrictions, command registration, and report serialization. Its `PolicySerialization` JSON contains a deliberately synthetic error to test failure propagation; it is not a project health report. `ScenarioIdempotence` checks actor reuse, transform reset, ownership-conflict refusal, unrelated actors, and cooking flags. `ViewportCapture` performs real rendered viewport readback after a frame delay.

Inspect `Saved/Logs/PrimalFrontier.log`, the isolated run's `Saved/Logs/PrimalAgentToolsVerification.log`, and exported reports. Existing project asset failures and engine startup errors must be distinguished from plugin test failures. Do not commit generated logs, screenshots, Binaries, Intermediate, Saved, or DerivedDataCache.
