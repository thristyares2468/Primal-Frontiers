# Survival HUD feedback — M11

The HUD reads the current server-owned survival snapshot at10Hz. A same-pawn health decrease produces a1.5-second text cue with two-decimal precision; it does not infer a source/direction or predict damage. A new pawn or missing possession resets observation. Critical health, low/empty food, low/empty water and exposure have independent text warnings, including when control hints are disabled. Death waits for server respawn rather than promising a successful spawn. Missing possession clears old numeric vitals/bars and hides aim/interaction prompts; dead/menu states suppress aim/prompts. Empty status/cue rows collapse. The wrapped480-unit panel honors HUDScale; Blueprint children receive the new PresentStatus availability/status/damage event alongside existing vitals/needs events.

No authority/RPC, replicated field, save data, map, Blueprint asset or dependency changed. The test uses the actual UMG widgets, authoritative needs/damage, automatic GameMode respawn and a temporary possession gap in one isolated rendered standalone world. It refuses a non-fresh inventory fixture.

## Evidence — October8

Editor PASSED20.86s initial,5.93s fixture,6.04s layout and final6.05s precision build; no compiler warnings. Logs: Saved/Logs/M11SurvivalFeedbackBuild_20261008.log, M11SurvivalFeedbackFixtureBuild_20261008.log, M11SurvivalFeedbackFinalBuild_20261008.log and M11SurvivalFeedbackPrecisionBuild_20261008.log. Development Game PASSED27.09s: M11SurvivalFeedbackGameBuild_20261008.log.

| Check | Exact outcome |
| --- | --- |
| Native survival | PF.Survival.Component/Environment/Lifecycle/Needs PASSED4/4; Automation_M11SurvivalHUD_20261008_094801576_db682721;16.09s; working/private2.947/2.812GiB |
| Final720p, scale1.5 maximum, normal shadows | PF.UI.SurvivalFeedbackLive PASSED1/1; M11Survival720_20261008_095644854_7cf56cdb;33.26s;3.040/4.028GiB |
| Normal1440p, scale1 | Strict FAILED:0assertion errors,1VSM warning; M11Survival1440_20261008_095159852_5765757b;33.50s;3.216/4.436GiB |
| Normal1440p repeat | Strict FAILED again:0assertion errors,1VSM warning; M11Survival1440_20261008_095353651_74e4800e;33.35s;3.241/4.459GiB |
| Final1440p, scale1, explicit low-shadow diagnostic | PASSED1/1; M11Survival1440_20261008_095525071_3d95342d;33.39s;3.242/4.465GiB |

All passed reports have engine/strict verdict0 and test warnings/errors0. Native raw logs clean. Rendered passed logs retain24 known widget/HLOD warnings and14 engine Python startup error lines; no crash/ensure/fatal. Failed1440p logs include two additional VSM warning lines (runtime and report copy). Ten final damage/urgent/death/respawn/waiting PNGs inspected. Earlier passed720p runs095108447_7ea7e7ee and094839811_60bf78a3 retained; screenshot review improved wrapping, height, empty rows and tiny-loss precision afterward.

Reports: Saved/AutomationReports/<run>/index.json and run-summary.json. Screenshots: Saved/AutomationReports/ControlsUI/<run>/. Logs: Saved/Logs/<run>.log; native prefix PFAutomation_. Saved/Logs/PrimalFrontier.log is older and does not replace unique current launch logs.

The installed Engine/Config/BaseScalability.ini uses512 physical VSM pages and directional bias0 at low/medium. VirtualShadowMapClipmap.cpp documents bias+1 halves directional resolution. The optional runner switch applies bias1 for stationary/moving directional shadows only for that process; it does not increase the pool, persist settings, suppress warnings or weaken the verdict. Logs confirm both applied CVars. **Normal1440p shadow-budget failure remains unresolved by this HUD change.** Diagnostic success is not a normal-profile pass. Next independent task: address that low/medium profile within its existing memory budget and replay normal1440p.

```powershell
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunControlsAutomation.ps1 -Resolution 720 -TestCase Survival -HUDScale 1.5
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunControlsAutomation.ps1 -Resolution 1440 -TestCase Survival -HUDScale 1 -LowShadowDiagnostic
```

Uncapped t.MaxFPS0/VSync0 confirmed. These stationary UI scenarios are not sustained traversal FPS/stutter measurements or physical-controller/human acceptance. Server mutation APIs are unchanged; no fresh network result is claimed. Manual M7/M8/controller gates remain nonblocking/unverified and full M11 remains incomplete. Bounded task: https://trello.com/c/z43ZUagT.
