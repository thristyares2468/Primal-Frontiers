# Main menu, world selection and Pause Save — M8/M11

## Current implementation and reference direction

The user's Palworld/Subnautica screenshots and their Florpy Runners menus inform clear navigation, prominent typography, separate single-player/multiplayer screens and an obvious Back action. Their artwork, characters, branding, UI assets and backgrounds are not imported. Primal Frontier uses its own texture-free Slate/UMG theme and existing fonts. Background artwork and thumbnails can follow after source/provenance and performance review; no new asset is required for this slice.

`/Engine/Maps/Entry` is the lightweight main-menu shell. Single player creates a new `/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld` or loads a validated local saved world. Main-menu Settings reuses the current settings interface. The Multiplayer panel currently describes the documented server/client workflow; menu hosting, joining, server discovery and invite codes are not implemented. It does not pretend to connect.

The pause menu has an explicit host-local **Save world** button. Actual save success/refusal is shown separately from reconnect-profile and new/restored-player messages. Saving captures gameplay and publishes a checksummed save generation; it must not load/reset gameplay or modify an editor `.umap`. End session asks for confirmation, then returns to the menu. It does not promise an autosave.

## Safe world selection

Only bounded world-slot identifiers and two existing gameplay maps are accepted. New world refuses any existing generation, including unreadable/corrupt saves. The menu hides identity profiles and validates schema, catalogs and saved survivor count before Load. Multiplayer saves require the existing server flow; solo loading requires one survivor. A selected menu request takes precedence over stale process launch save/load arguments. Startup restore failure continues to block capture/save so default state cannot replace a failed restoration.

Saved worlds can be renamed from Single player: select the world, enter a new display name in the field below Refresh worlds, then choose Rename selected world. Names allow spaces and Unicode, are trimmed, and must contain1–64 characters without control characters. Duplicate display names are refused case-insensitively. Rename requires a valid loadable world and preserves the internal slot, gameplay save bytes/generation and ownership IDs. It changes only the version1 checksummed `Metadata_WorldNames` name registry. Corrupt name metadata is refused without replacement; missing metadata falls back to the original slot label. `Identity_` and `Metadata_` are reserved from New and gameplay Save/Load. Renaming is currently a local solo-menu operation; multiplayer server/world renaming is not exposed.

No world-delete, import, arbitrary map/path, network save/load RPC or new online service is introduced. Tests own uniquely named world/profile/name-registry/settings/report destinations and compare unrelated save files and settings before/after launch. There are no asset changes or new dependencies.

## Reported manual save observation

The October9 user playtest remains failed/unverified. In the retained manual log, two resource-node timers respawned at01:48:49/01:48:53 UTC, followed by `PF.SaveWorld ManualGuide20261009` Passed at01:48:53.417. No subsequent map reload or death/respawn is recorded. This is consistent with a timed resource respawn; it does not establish everything the user observed. The manual launch also omitted `-PFLoadSave`, so restarting that same create-only launch is not a restoration request.

Native live-state conservation passed before menu integration. A separate solo reopen bug was reproduced: a shared endpoint credential from another solo world was rejected by CheckLogin before the existing Login sole-owner adoption. CheckLogin now follows that existing adoption rule only for standalone with one saved survivor; network credential validation and duplicate-connected identity rejection remain. This is not a manual full-loop pass or proof of the cause of the user's Save observation. Personal `ManualGuide20261009` generations are not loaded, overwritten, renamed or removed by the tests.

## Normal play after verification

Start here: launch the game using the project with `-game`, without an explicit map or save/load flags:

```powershell
& 'C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe' 'C:\UnrealProjects\PrimalFrontier\PrimalFrontier.uproject' -game -windowed
```

1. Select Single player, enter a new unused world name, then Create new world. Gameplay uses `L_PrimalFrontier_OpenWorld`.
2. Gather supplies, move and build. Press **P**, then **Save world**. Continue only after the visible `Saved world:` acknowledgment; refusal is a failed save.
3. Select End session, then Confirm end session. The main menu should return.
4. Select Single player, choose the same saved world. To rename it, enter a new display name below Refresh worlds and choose Rename selected world; expect a success message and the renamed selection. Then Load selected world. Check the server-confirmed restored-survivor message, inventory, health, position, structures, storage and ownership. The original console save ID remains unchanged by Rename.
5. For the full manual M8 gate, close the game, relaunch the same menu command, and load that world again. Missing items, duplication, lost ownership, a fresh survivor or false save acknowledgment is a failure.

Resource/food/creature timers may legitimately progress while playing or while a saved world is closed. This is separate from a fresh world or lost construction/inventory. Save/load does not change the editor asset.

## Verified bounded checkpoint — October9

Final Editor/Game builds PASSED19.22/50.32s with no compiler warnings: `Saved/Logs/PFM11MenuLayoutFinalEditorBuild.log` and `PFM11RenameFinalGameBuild.log`. Rename-specific Editor build passed41.97s; added guard fixtures compiled7.18s. No assets changed. Installed engine reports5.8.3;5.8.2 is the requested baseline.

All table reports live under `Saved/AutomationReports/`. Each final engine/report/runner exit was0 and every test recorded zero test warnings/errors.

| Test | Exact report prefix/directory | Passed | Seconds | Sampled working/private GiB |
| --- | --- | --- | --- | --- |
| PF.Input.Gamepad; PF.Persistence.RejectedLoadPreservesWorld; PF.Persistence.StartupFailurePreservesSave; PF.Persistence.WorldRuntime | Automation_M11WorldMenuFinal_20261009_031014929_6efba8a2 |4/4|23.73|3.003/2.907|
| PF.UI.WorldMenuLive720p, rename plus original conservation | M11WorldMenu720_20261009_030843603_e443e451 |1/1|62.15|3.284/4.652|
| PF.UI.WorldMenuLive1440p, final button labels | M11WorldMenu1440_20261009_031331936_5ef2e6f8 |1/1|61.36|3.448/5.714|
| PF.UI.ControlsLive720p | M11Controls720_20261009_031456406_59dda1f9 |1/1|37.40|3.129/5.152|
| PF.UI.ReconnectProfileLive720p | M11Reconnect720_20261009_031549752_c11eef2b |1/1|37.64|2.369/5.122|
| PF.Persistence.Live, one-client Create/Restart server+client | M8Live1_20261009_031704133_94663d4f +CreateServer/CreateClient1/RestartServer/RestartClient1 |4/4|See run-summary|1.723–1.817/1.631–1.787 per process|
| PF.Persistence.Live, two-client Create/Restart server+clients | M8Live2_20261009_031919719_d6a38e7e +CreateServer/CreateClient1/CreateClient2/RestartServer/RestartClient1/RestartClient2 |6/6|See run-summary|1.720–1.825/1.621–1.790 per process|

Rendered menu tests use actual supported Slate keyboard focus and key down/up activation of UButtons, not direct callbacks or OnClicked broadcasts. They cover Main→Settings Cancel→Multiplayer information→Single player→New→P→Save→confirmed End→Rename→Load. Exact gameplay save bytes/generation, actor/player identity and live health/inventory/location are preserved by Save/Rename. Load restores inventory4wood, health65, settled position, one structure ID/health75/ownership and night22, even after another-world credential replaces the isolated endpoint profile. Invalid/duplicate labels and an owned malformed name-registry payload are refused without overwriting. Metadata/profile slots cannot become gameplay saves. Actual default settings and131/133/135/136 unrelated save files respectively were unchanged. Native WorldRuntime verifies repeated real-file restoration, storage/resource state and disconnect checkpointing; network restart tests retain identity, authority/invalid RPC checks, ownership and foreign storage/inventory privacy. These do not prove production authentication.

Screenshots: `Saved/AutomationReports/ControlsUI/<rendered report>/`. Main, Multiplayer, selection/Rename, saved/restored Pause and Controls/reconnect-status PNGs were inspected. The720p rename checkpoint exposed awkward button-label wrapping; final source disables button-label wrapping while allowing per-character wrapping only for long Save feedback. Final720p Controls and1440p WorldMenu layouts fit. No new viewport clipping was observed.

Raw native logs contain zero warning/error/ensure/fatal lines. Rendered menu startup retains21 EditorDataStorageUI warnings and14 engine Python error lines. Six existing uncooked travel messages (three HLOD imports, one missing Recast/Crowd instance, blur/DOF settings precedence) are individually expected by the fixture, rendered as Verbose during automation; no blanket suppression. Other rendered/network runs retain existing24–26 warnings and14Python error lines. No new gameplay error, fatal/ensure or crash in final runs. These known engine/content findings are not fixed here. NullRHI clients prove no rendered D3D12 result. Uncapped UI screenshots are stationary automation, not sustained FPS/stutter measurements. Actual host31.93GiB physical RAM;16GB minimum-spec usability remains unverified.

## Retained failures and limits

Initial build C2039 button alignment was corrected to UButtonSlot; inherited UWidget Slot parameter collision was corrected. Solo owner reopen failed one real assertion in `Automation_M8SoloReopenRepro_20261009_022616554_39bb50ae`; standalone-only fix replay passed1/1 (`Automation_M8SoloReopenFix_20261009_022915490_cb1503ea`). Native Save conservation passed1/1 (`Automation_M8SaveConservation_20261009_020203465_36044a74`).

Earlier WorldMenu720 attempts are retained: `20261009_023000710_28ac880d` and `023820173_8de4fc25` failed timeout; `024313602_cf6875df` timed out without a completed index. Background pointer handling did not reliably activate buttons. The latter's unrelated-save false alarm was a PowerShell owned-filename concatenation fixture error; only its owned test records changed, not ManualGuide. `023453555_0fdbc3d9` completed gameplay assertions but failed nine warnings; fixed runner VSync precedence and menu pawn startup; `024915016_47abffbc` failed one remaining null HUD warning; retained default empty AHUD fixes it. Clean pre-Rename `025249336_68f1a2c7` passed before final Rename runs above. Retaining these reports avoids treating discarded failures as success.

Mouse/physical controller delivery, separate-process rendered menu reload, full human gather/craft/build/store/save/restart and M8/M11 acceptance remain unverified; the user's earlier Save observation remains a failed manual gate. The menu Multiplayer screen is informational only. Packaged Server remains unavailable in this installed engine distribution; uncooked dedicated-server tests are separate. No16GB or traversal performance certification.

Replay with other Unreal processes closed:

```powershell
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunControlsAutomation.ps1 -TestCase WorldMenu -Resolution 720 -TimeoutSeconds 240
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunControlsAutomation.ps1 -TestCase Controls -Resolution 720
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunControlsAutomation.ps1 -TestCase Reconnect -Resolution 720
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunPersistenceAutomation.ps1 -Players 1
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunPersistenceAutomation.ps1 -Players 2
```

These runners own fresh world/profile/name-registry/INI/report destinations. Do not reuse personal worlds for automation. Guide G3 in TRELLO_TEST_GUIDES.md is the simpler human menu replay; no manual save corruption is required.
