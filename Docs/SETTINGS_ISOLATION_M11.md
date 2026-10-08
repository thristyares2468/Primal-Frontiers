# M11 disposable settings configuration verification

October9,2026. Tooling prerequisite PASSED for later Apply/display testing. No actual Apply/save/display acceptance yet. All Personal gameplay gates remain unverified.

RunControlsAutomation.ps1 gives EVERY rendered case a unique -GameUserSettingsINI="<its report>/GameUserSettings.ini" alongside the existing unique identity profile. It snapshots actual project Saved/Config GameUserSettings.ini and Scalability.ini file paths/presence and SHA256 read-only before/after each launch; any difference forces RunnerExitCode1 even if the engine/report passed. Summary distinguishes EngineExitCode,report VerdictExitCode and RunnerExitCode. Hashes and setting contents are not printed; no user's config is copied/edited/deleted.

Installed engine source supports the destination override in Core/Private/Misc/ConfigCacheIni.cpp FConfigCacheIni::GetDestIniFilename. UE5.8 known globals such as GGameUserSettingsIni identify config-cache branches,not filesystem paths (GetConfigFilename also returns the key for a known branch). SettingsCancelLive resolves FConfigBranch::IniPath via supported FindBranch,requires it to match its own generated report file BEFORE UI actions,and uses that path for fixture existence/content checks.

First guard incorrectly compared GGameUserSettingsIni as a filename. Retained failure M11Settings720_20261008_231342919_8fb4158b: SettingsCancelLive Fail2errors,engine0/report+runner1,no timeout,22.31s;working/private3.058/5.311GiB. One actual default config remained unchanged. This was a verification lookup bug,not evidence the override modified user settings. Fixed only lookup/preservation assertions; no gameplay/settings implementation change. Earlier SETTINGS_CANCEL_M11.md file-preservation claim was insufficient for the same reason and now explicitly corrected; its live preference/renderer/navigation/inventory results remain valid.

Initial Editor19.43s/Game24.92s builds passed without compiler warnings (M11SettingsIsolationEditorBuild_20261009.log,M11SettingsIsolationGameBuild_20261009.log). Corrected lookup Editor5.74s/Game13.56s passed (M11SettingsIsolationKeyFixEditorBuild_20261009.log and M11SettingsIsolationKeyFixGameBuild_20261009.log). PowerShell AST parse/diff whitespace check passed.

Final PF.UI.SettingsCancelLive results:

| Report directory under Saved/AutomationReports | Result | Seconds | Sampled working/private GiB |
| --- | --- | --- | --- |
| M11Settings720_20261008_231626963_4937082b | 1/1 Success,engine/report/runner0 | 30.22 | 3.030/5.266 |
| M11Settings1440_20261008_231745531_7a974649 | 1/1 Success,engine/report/runner0 | 30.50 | 3.209/5.121 |

Both actual config-branch path guards passed and both summaries report DefaultConfigFileCount1,DefaultConfigsUnchanged=true: the existing WindowsEditor/GameUserSettings.ini path/presence/hash stayed unchanged,no default config added. No personal config contents/hashes printed or edited. Four category PNGs per run inspected under Saved/AutomationReports/ControlsUI/<run>/{game_draft,graphics_draft,audio_draft,accessibility_draft}.png. The long Graphics list scrolls at these sizes; this is not all-rows-visible/maximum-HUD-scale acceptance.

Test events have0errors/0warnings. Both raw logs retain21 widget+3 HLOD warnings and14 installedPython error lines PLUS2 LogConsoleManager startup warnings: SetByScalability tries r.MotionBlurQuality/r.DepthOfFieldQuality after SetByGameSetting,so lower-priority requests are ignored and both values remain0. Investigated as engine CVar precedence preserving current blur/DOF-off choices; not suppressed and not a clean-log claim. No other severity,ensure,fatal,crash or VSM overflow. Unique logs Saved/Logs/<run>.log; no sustainedFPS/stutter or16GB-minimum certification from sampled host31.93GiB memory. No multiplayer replay needed for this local fixture/launcher change.

Replay: close existing Editor; powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunControlsAutomation.ps1 -TestCase Settings -Resolution 720 -HUDScale 1.5; only after pass repeat -Resolution 1440. Map /Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld,one rendered standalone process,normal quality,uncapped,-NoSaveConfig,unique report/profile/config. Registered PF.UI.SettingsCancelLive edits only drafts,Defaults/Cancel/focus/teardown; never Apply. RequestedHUDScale does not prove applied scaling. Inspect generated four category screenshots,report/default-config flag,unique raw log and sampled memory. No multiplayer rerun needed for a launcher/local fixture change.

Next independent task ONLY after isolation passes: actual non-resolution Apply/persistence and display confirm/revert tests with an explicit guarded fixture and disposable writes. -NoSaveConfig currently remains; do not claim actual config writes,display transitions or physical controller/audio certification from this isolation replay. Manual guides: TRELLO_TEST_GUIDES.md.
