# Crafting knowledge caption width — M11

October 10, 2026. The bounded technical gate passed. The supported UMG change fills the Learn button's available content width and centers its original wrapped caption. Full M11/M12 and Personal acceptance remain open.

## Change and measurement

Reviewed Field Weapons screenshots exposed excessive caption wrapping inside a wide button. The button's content slot used its default center alignment. `PFCraftingHUD.cpp` now sets that actual `UButtonSlot` to `HAlign_Fill` and centers the `UTextBlock`. Font, caption, purchase/focus/input, server authority, assets and save data are unchanged.

The existing rendered exercises now assert that the real label occupies available button width, stays inside the button, and fits its allocated height. Available width includes both actual style normal padding and actual content-slot padding, matching installed `SButton::GetCombinedPadding`. These assertions accompany real locked, available and learned purchase states; they are not screenshot-only substitutes for purchase validation.

## Verification

Reports are under `Saved/AutomationReports/<run>/index.json`, summaries beside them, unique raw logs under `Saved/Logs/<run>.log`.

| Gate | Evidence | Result | Seconds | Sampled working/private GiB |
| --- | --- | --- | ---: | --- |
| Corrected Editor | PFM11KnowledgeCaptionEditorRetry.log | Passed, no compiler warnings | 34.26 | build host only |
| PF.UI.ProgressionDetails | Automation_M11KnowledgeCaption_20261010_053946910_087be7e6 | 1/1 passed | 67.21 | 3.018 / 2.866 |
| Padding-fixture Editor | PFM11KnowledgeCaptionPaddingEditorBuild.log | Passed, no compiler warnings | 41.02 | build host only |
| PF.UI.ProgressionFeedbackLive, 1280×720/HUD1.5 | M11Progression720_20261010_054721892_f71e8802 | Failed-only replay 1/1 passed | 58.90 | 3.329 / 5.107 |
| PF.UI.ProgressionFeedbackLive, 2560×1440/HUD1.5 | M11Progression1440_20261010_054856534_e30571e1 | 1/1 passed | 58.01 | 3.352 / 5.435 |
| PF.Progression.EarnedWeaponLive, 1280×720/HUD1.5 | M12EarnedWeapon720_20261010_055019340_6f212ffc | 1/1 passed | 141.55 | 3.323 / 5.393 |
| Game Development Win64 | PFM11KnowledgeCaptionGameBuild.log | Passed, no compiler warnings | 42.32 | build host only |

All three final rendered runs have exact selected tests, zero test errors/warnings, all exits zero and no timeout. All thirty final PNGs were inspected under `Saved/AutomationReports/ControlsUI/<run>/`: locked/available/learned knowledge, first/repeat/different recipe feedback, cap and completed craft, plus the existing fourteen earned-route frames. The original purchase/cancellation/conversion checks remain active. The Progression UI-boundary fixtures seed trusted records; they do not prove earned pacing. The separate grant-free earned route verifies the longer Field Weapons caption using actual gathering and crafting, finishing270XP with one point, sixty-damage club, ordinary death/respawn and conservation checks. Controlled actor positioning and AI isolation remain fixture limits.

## Retained failures

- Initial Editor `PFM11KnowledgeCaptionEditorBuild.log` failed in 16.72s with C2039: the alignment setter belongs to `UButtonSlot`, not `UButton`. Used the supported slot setter and concrete header; corrected build passed.
- Initial 720p `M11Progression720_20261010_054128969_c582a094` failed three width assertions, 61.46s, engine exit zero/strict verdict one, no timeout/test warnings, 3.341/5.392 GiB. The actual available-caption PNG was readable and purchase succeeded. The new assertion counted style padding but omitted default slot content padding. Installed `ButtonSlot.cpp` and `SButton.cpp` confirmed the sum. Corrected only measurement; inside-button/text-height checks retained. Rebuild and failed-only720p replay passed. Keep the original failed report and PNGs.

## Log, safety and acceptance limits

Native raw log has zero warning/error/assert/fatal/ensure. All three final rendered raw logs match the known 26-warning/14-experimental-Python-error startup baseline, with no new normalized findings or assert/fatal/ensure. These are not zero-error raw logs. Default `PrimalFrontier.log` remains older than the unique launch logs and is not these runs' evidence. Runner hashes confirmed 308/309/310 unrelated saves and default GameUserSettings/Scalability preferences unchanged.

One rendered process at a time, windowed, uncapped/VSync zero, no personal slots. Host physical RAM is 31.93 GiB, with 37.5 GiB committed at one build start; startup/build times were slower. No sustained route, FPS/stutter, minimum16GB, physical-controller or human-readability certificate is implied. Installed engine is 5.8.3, requested baseline 5.8.2. No redundant network replay is required for this local alignment change.

## Replay

Level: `/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld` for rendered standalone; native view needs no level. With no competing Unreal process, from the project root:

```powershell
./Scripts/RunNativeAutomation.ps1 -TestFilter PF.UI.ProgressionDetails -Label M11KnowledgeCaption
./Scripts/RunControlsAutomation.ps1 -TestCase Progression -Resolution 720 -HUDScale 1.5 -TimeoutSeconds 240
./Scripts/RunControlsAutomation.ps1 -TestCase Progression -Resolution 1440 -HUDScale 1.5 -TimeoutSeconds 240
./Scripts/RunControlsAutomation.ps1 -TestCase EarnedWeapon -Resolution 720 -HUDScale 1.5 -TimeoutSeconds 240
```

Stop on any failed verdict; retain evidence and correct only the failed scope before expanding. Inspect actual captions and purchase feedback in the PNGs as well as exact test records/logs/hash guards. Ordinary controls are unchanged, so the existing G2 Personal guide needs no additional assigned route. Trello: https://trello.com/c/f8PqJjT7.
