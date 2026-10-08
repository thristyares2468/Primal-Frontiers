# Low/medium shadow budget — M11/M16 prerequisite

Config/DefaultScalability.ini sets stationary/moving directional VSM resolution bias to1 for ShadowQuality0/1. The installed renderer defines +1 as half resolution; this reduces shadow detail/demand while retaining the inherited512-page pool. Higher tiers inherit engine profiles; saved quality/resolution preferences, maps/materials/lights and engine files are unchanged. There is no new shader feature or larger memory allocation.

Two normal1440p survival HUD runs previously failed the strict verdict on one runtime VSM pool-overflow warning each (no assertion errors). The explicit session-only bias1 diagnostic passed. This project profile now applies through normal engine scalability with **no diagnostic switch**.

| Verification | Evidence |
| --- | --- |
| Editor | PASSED19.12s initial and4.71s fixture retry, no compiler warnings; M11ShadowBudgetBuild_20261008.log / M11ShadowBudgetFixtureBuild_20261008.log |
| Development Game | PASSED25.05s, no compiler warnings; M11ShadowBudgetGameBuild_20261008.log |
| Initial native strict failure | Automation_M11ShadowBudget_20261008_100319773_fb0c6620; Preferences clean, ShadowBudget0assertion errors/10ConsoleManager warnings; engine0/verdict1 |
| Corrected native | PF.Settings.Preferences/ShadowBudget PASSED2/2; Automation_M11ShadowRetry_20261008_100508620_aa242e0e;16.05s; working/private2.938/2.786GiB; engine/verdict0, zero test/raw warnings/errors/fatal/ensure |
| Normal1440p, scale1 | PF.UI.SurvivalFeedbackLive PASSED1/1; M11Survival1440_20261008_100548465_3d120bc7;33.65s;3.183/4.412GiB |
| Normal1440p, maximum scale1.5 | Same test PASSED1/1; M11Survival1440_20261008_100640924_df254fd7;33.60s;3.197/4.393GiB |

Native fixture uses the supported sg.ShadowQuality callback at current priority, switches actual tiers0/1/2/3, checks stationary/moving resolution and inherited512/512/2048/4096-page budgets, then restores the original group/values. The first all-group forced fixture collided with deliberate game-setting blur/DOF overrides; corrected only the fixture, without suppressing warnings or clearing unrelated overrides. Failed evidence is retained.

Both final rendered runs: LowShadowDiagnostic=false; engine/strict0, zero test warnings/errors; configured bias1 logged; **zero VSM overflow lines**. Ten PNGs inspected. Known raw24 widget/HLOD warnings and14 engine Python startup lines remain, no crash/fatal/ensure. Uncapped/VSync0 confirmed. This proves these stationary damage/death/respawn/possession scenarios fit the low/medium budget at1440p; it does not prove every view/time/map or sustained traversal FPS/stutter. Full M16 remains planned and Personal observations remain unverified/nonblocking.

Reports: Saved/AutomationReports/<run>/index.json and run-summary.json; screenshots Saved/AutomationReports/ControlsUI/<run>/; logs Saved/Logs/<run>.log, native prefix PFAutomation_. Previous renderer failures095159852_5765757b /095353651_74e4800e are documented in [SURVIVAL_FEEDBACK_M11.md](SURVIVAL_FEEDBACK_M11.md).

```powershell
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunNativeAutomation.ps1 -TestFilter PF.Settings -Label ShadowProfile
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunControlsAutomation.ps1 -Resolution 1440 -TestCase Survival -HUDScale 1.5
```

Bounded task: https://trello.com/c/LMulR1lr. Next M11 slice: keep crafting/building overlays clear of the first-person aim/vitals and make their real feedback readable at supported scales; not yet implemented.
