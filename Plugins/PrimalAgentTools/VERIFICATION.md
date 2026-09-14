# Verification — UE 5.8.2, 2026-09-12

| Check | Final result |
| --- | --- |
| `PrimalFrontierEditor Win64 Development` | Passed; no compiler warnings in final build; UBT `Result: Succeeded`, 11.17 seconds |
| `PF.PrimalAgentTools.PolicyAndReports` | Passed: scope guards, prefix policy, editor-only metadata, seven registered commands, safe report labels, JSON failure propagation |
| `PF.PrimalAgentTools.ScenarioIdempotence` | Passed: four fixtures, reuse without duplicates, fixed transform reset, Undo/Redo, ownership-conflict refusal, unrelated actors preserved, editor-only flags, live console dispatch |
| `PF.PrimalAgentTools.ViewportCapture` | Passed: rendered 2060 x 619 PNG; visually inspected cube, floor and PlayerStart; no competing-directional-light warning |
| Automation total | **3 passed, 0 warnings, 0 failed, 0 not run** |
| Startup safety regression | Passed by deliberate refusal: first-frame reset returned `EditorStartingUp`; reset/place commands passed after startup |
| `PF.ValidateAssets /Game` | Passed: 609 discovered, 255 top-level assets valid, 354 external objects included; 0 invalid/warnings/skipped/unable to validate |
| `PF.CheckReferences /Game` | **Project check failed:** 609 packages / 2101 references checked; 18 missing package references, one redirector, one transient-package dependency warning |
| `PF.CheckNaming` | **NeedsAttention:** no saved assets under `/Game/PrimalFrontier`; zero naming coverage |
| Shipping exclusion | Passed at UBT target-graph level: `PrimalFrontier Win64 Shipping`, 502 modules, one binary, zero `PrimalAgentTools` references |
| Saved content preservation | Passed: hashes of all 609 `.uasset`/`.umap` files match before/after testing |
| Acceptance log | No ensures/fatal errors; errors are the deliberate startup refusal and pre-existing missing references |

The Data Validation pass does not negate the independent missing-reference findings. These checks cover different failure modes.

## Evidence

Generated paths below are project-relative; they are not source-control changes:

- `Saved/AutomationReports/EngineAcceptance/index.json`
- `Saved/Logs/PrimalAgentToolsAcceptance.log`
- `Saved/AutomationReports/PF_AcceptanceChecks_20260912T060051_4F5E2BEB487D721AD43A2BB91ED343FB.json`
- `Saved/AutomationReports/PF_StartupGuard_20260912T060044_EC20B87246E6FC36B1CF188E324BEE27.json`
- `Saved/AutomationReports/PF_ScenarioVerification_20260912T060103_0926898D4FF3EF093962A1978524EB76.json`
- `Saved/AutomationReports/PF_ViewportVerification_20260912T060106_90D9AD4B4D8FFA1E152689AAAFA2042A.png`
- `Saved/AutomationReports/PF_ViewportVerification_20260912T060106_4E4C70274E7127C1592E58993FA8F40C.json`
- `Saved/AutomationReports/PFShippingTarget.json`

`AcceptanceChecks` includes the deliberate startup refusal before asset checks; read its per-command entries. `PolicySerialization` files contain a synthetic failure solely to test JSON failure propagation. Neither represents an unexplained plugin failure.

Content integrity: SHA-256 of newline-joined, sorted `relative filename + space + file SHA-256` records was `66CDD7C2F7A9B8BBEF31FC0A994D5A4C01CF020522046318145EB89A5914A590` both before and after. Fixtures were created only in the isolated editor's memory; no map/asset was saved. Unreal generated normal build products, caches, logs and reports. Existing unrelated working-tree changes were retained.

## Existing project findings

Missing references include Manny/Quinn source meshes, editable animation sequences, `MI_Intro_Colorway` referenced by `BP_WobbleTarget`, and `SM_DoorFrame_Edge` referenced by Horror/Shooter external actors. The JSON records all 18 referring packages and missing targets.

The redirector is `/Game/FirstPerson/Lvl_FirstPerson.Lvl_FirstPerson`. The transient dependency belongs to `/Game/Variant_Shooter/Blueprints/AI/ST_Shooter_ShootAtTarget`. No automatic repairs were performed.

The working editor's `Saved/Logs/PrimalFrontier.log` was also inspected. It contained a pre-existing `r.MotionVectorSimulation` render-thread warning. The isolated acceptance log did not contain that warning.

## Issues resolved during implementation

- Compiler API/include mismatches were fixed; final compilation had no warnings.
- A test directional light competed with the existing sun. It was replaced by a point light; final screenshot has no corresponding warning.
- Startup `-ExecCmds` actor mutations produced a Scene Outliner/Slate visibility ensure in an intermediate run. The plugin now refuses first-frame mutation. Final acceptance deliberately exercised the refusal, then passed running-editor command and scenario tests without the ensure.
- Unreal's transient package is reported separately from missing disk assets.

## Manual setup and limits

- Restart the already-open working editor after reviewing/saving existing work to load the new module. The isolated test editor loaded it successfully and exited.
- Open existing `/Game/Maps/L_Automation` before running scenario commands. Save fixtures manually only if persistence is wanted.
- Add project-owned assets under `Content/PrimalFrontier` for real naming-check coverage. Prefix logic is tested, but that scope currently has no saved assets.
- Review the existing missing references, redirector and transient dependency separately.
- Full Shipping cook/package execution was not run. Exclusion was checked through descriptors, build guards, and UBT's actual Shipping target graph.
- Viewport capture used a real renderer. Image creation and visible fixtures were checked, not screenshot comparison, deterministic gameplay, or multiplayer behavior.

## Reproduce

Use the build and isolated editor command in README. The final acceptance run additionally used this `-ExecCmds` value:

```text
PF.ResetAutomation,PF.ExportResults StartupGuard,PF.CheckNaming,PF.CheckReferences,PF.ValidateAssets,PF.ExportResults AcceptanceChecks,Automation RunTests PF.PrimalAgentTools
```

The first reset is deliberately rejected. The automation suite exercises mutations after startup. Inspect both automation results and the full log; do not suppress ensures to make a run pass.
