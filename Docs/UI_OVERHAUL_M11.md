# Expedition UI overhaul — M11/M12

## Scope and status

October 15, 2026: original expedition interface implemented and verified in Unreal. New human visual acceptance remains separate from technical verification. The previous accepted gameplay/navigation checks remain historical evidence. Trello: https://trello.com/c/1wNmkHRa.

Inventory uses clickable stable-GUID slots, a selected-item illustration/details pane and mouse Use/Split/Drop actions. Player upgrades remain an inventory tab. Crafting shares illustrated items, recipe categories, ingredient availability and the existing validated craft queue. Building shows eight original structure illustrations; storage illustrates real batches alongside Store/Take, Quick Store, Sort and filters. Main menu uses original environment art and saved-world cards. Pause, controls, settings and HUD share the charcoal/copper palette. HUD vitals are compact with thin bars; aim cues show the actual item/resource yield or creature classification. No blur, 3D preview, additional gameplay system or new plugin.

## Reference research

- [Minecraft's official crafting guide](https://www.minecraft.net/en-us/article/how-craft): recipe categories, searching and explicit missing supplies. Use the availability/detail pattern; retain this game's timed C++ crafting rather than copying Minecraft's spatial recipes.
- [Palworld developer announcements](https://steamcommunity.com/app/1623730/allnews/): Quick Stack behavior and storage convenience. Keep this project's existing server-validated Quick Store and chest filters; no copied artwork, creatures, typography or screen assets.
- [ARK inventory UI discussion on the official forum](https://survivetheark.com/index.php?/forums/topic/167309-new-ui-feedback-and-thoughts/&page=2): player feedback on consistent inventory layouts. This is community usability feedback, not a developer specification. Paired bag/chest panes and explicit actions are our design inference; avoid clutter and unreadable translucent cyan text.

## Art and performance contract

`ArtSource/UI/ExpeditionItems.png`: one original 4x4 atlas, row-major wood, stone, found food, fibre, water, cooked food, dried food, cord, wood tool, stone tool, bound stone tool, copper pick, club, bound club, woven guard, copper ore. Copper ingot uses the structure atlas. Custom configured catalog icons take precedence; unknown items retain a neutral fallback. No catalog/item IDs, gameplay tuning or saves change.

`ArtSource/UI/ExpeditionStructures.png`: 3x3 original atlas, foundation/wall/floor/roof/door/chest/window/stairs/copper ingot. Native schematic fallback remains for missing art. `ExpeditionSymbols.png` and `ExpeditionActions.png`:16+16 original survival/menu/traversal/action symbols. `ExpeditionEncounters.png`:4 classification badges (forager/prowler/scrabbler/dusk pouncer), not portraits or promises of final creature species. Six resources reuse their actual yield-item pictures. **61 sprites plus one title backdrop**, five shared atlases, six Texture2D assets. `FrontierMenu.png`: original static frontier illustration, no foreign game art.

All six images produced with the built-in image generation tool. `Scripts/ImportExpeditionUI.py` imports only new UI Texture2D assets with supported Unreal APIs, refuses unrelated-source overwrite and verifies/skips existing textures without saving them; no level/catalog load/save. One explicitly selected new untracked draft may be replaced using PF_UI_REIMPORT_OWN_DRAFT; it refuses tracked textures. UI compression/no mipmaps, no expensive effects. Packaging explicitly includes `/Game/PrimalFrontier/UI`; packaged coverage is not claimed by an editor run. Sprite gutters use stable verified source dimensions, not asynchronous compiling texture proxy sizes. Glyph frame seams are read-only pixel-audited, with a three-texel inset; source canvases need not be perfectly square. Image boxes retain square aspect instead of stretching structures. Labels/values remain readable without recognizing icons or distinguishing colors.

User clarified the existing Fab models are staged and none currently used. Original illustrations are **provisional**, not a claim of exact Fab mesh likeness. Read-only supported Asset Registry audit: Saved/AutomationReports/M9AssetInventory_20261014T041906Z_adeaa7d0379b4adfaf7efc4de0357dbc/inventory.json. Actual pickup/tool/structure source still uses engine primitives, item definitions have icon soft references but no model assignment. Do not pick a random vendor model or alter world actors just to justify an icon. At asset integration select the actual licensed model/material variant, capture it in Unreal, then replace its item catalog soft icon (existing custom icons already win) or structure atlas cell after visual comparison. Candidate names alone are not visual/provenance/compatibility approval.

Full generation prompts and provenance: ArtSource/UI/README.md. The installed engine reports 5.8.3, while the requested baseline is 5.8.2; unchanged.

## Verification

Final technical coverage: six native selectors and eleven rendered selectors passed, seventeen distinct test records. One720p rendered client at a time, uncapped requested, disposable test profiles/settings/worlds. User-deferred1440p and physical controller remain unverified. Screenshots inspected; tests retain real quantities, expiry, stable identity, authority/refusal and saved-world checks. TargetSpritesLive exercises17actual pickup definitions,6resource yields and4creature classification cues in transient fixtures without granting inventory or editing a level. Native AtlasAssets checks all five imported sprite sources and registered frame boundaries.

Native six-selector proof: `Automation_ExpeditionFinalNative_20261014_050244698_fe98b9a8` — PF.Building.PlacementAndStorage, PF.Input.Gamepad, PF.Input.InventorySelection, PF.UI.AtlasAssets, PF.UI.InventoryDetails, PF.UI.RecipeDetails; all Success/0 test errors/warnings. Final imported-dimension assertion replay `Automation_ExpeditionSourceDimensions_20261014_205941519_51e500f9` passed AtlasAssets1/1 (23.21s, raw severity0).

Rendered evidence, all Success1/1, exit0, no timeout, test errors/warnings0, personal-save/default-config guards unchanged. Paths are `Saved/AutomationReports/<run>/index.json` and `run-summary.json`, matching `Saved/Logs/<run>.log`; PNGs under `Saved/AutomationReports/ControlsUI/<run>`.

| Selector | Final run |
| --- | --- |
| PF.UI.InventorySelectionLive | M11Inventory720_20261014_205813514_9b9111f2 |
| PF.UI.TargetSpritesLive | M11Sprites720_20261014_052344159_14fbc823 |
| PF.UI.PlayerStatsLive | M12StatsUI720_20261014_052647832_9a14d7e8 |
| PF.UI.BuildingMenusLive | M11BuildingMenus720_20261014_052841503_73d7d756 |
| PF.UI.ActionOverlaysLive | M11Overlays720_20261014_052920369_27768138 |
| PF.UI.SettingsCancelLive | M11Settings720_20261014_052953415_1614d541 |
| PF.UI.ControlsLive | M11Controls720_20261014_053025319_badfb2b8 |
| PF.UI.SurvivalFeedbackLive | M11Survival720_20261014_053306971_0cabeca1 |
| PF.UI.WorldMenuLive | M11WorldMenu720_20261014_053342001_57f52018 |
| PF.UI.MenuNavigationLive | M11MenuNav720_20261014_053423489_d21336cf |
| PF.UI.SettingsApplyLive | M11SettingsApply720_20261014_205659438_a1a48ccc |

First builds caught Slot shadowing/TObjectPtr iteration and Slate UV wrapper fixture integration; fixed against installed headers. Atlas source tests first failed4 boundary-band specks, then26 legitimate inside-cell pixels. Renderer excludes3texels; source test checks actual registered seams, never claims an entirely transparent interior gutter. Inventory replays caught real async-proxy cropping, fixed using verified source dimensions; subsequent float32 corner difference0.0000000149 required1e-6 assertion tolerance. SettingsApply initially failed old22-point-base expectation28 versus actual18; updated to redesigned14-point base and replay passed. Failures retained in their unique reports; gameplay acceptance/conservation tests were not disabled.

Latest Editor `Saved/Logs/PFExpeditionApplyFixtureBuild.log` PASS19.45s/no compiler warnings. Game Saved/Logs/PFExpeditionGameBuild.log PASS66.53s/no compiler warnings. All eleven successful runtime logs add0 normalized Warning/Error/Fatal/ensure messages over `M12StatsUI720_20261013_073212143_3c179b6c.log`. Existing EditorDataStorage/HLOD/scalability warnings and experimental ToolsetRegistry Python startup errors remain; this is not a warning-free engine claim. Ordinary PrimalFrontier.log is the earlier closed user session, inspected separately; unique current logs are the test evidence.

Sampled rendered working3.093–3.324GiB/private4.423–5.479GiB; no sustained FPS, stutter, packaged-asset, minimum-spec or physical-device certification. Actual host31.93GiB RAM/RTX4060 8GB; requested reference differs. No existing map, Fab asset, item catalog, layout identity or save schema changed. Six new UI textures and original PNG sources only.

## Layout designs

`ArtSource/UI/Designs/ExpeditionInterfaceConcept.png` is a newly generated original three-screen design sheet: inventory, crafting and building. It is reference-only, not a gameplay screenshot, recipe catalog or promise that the additional illustrative items exist. Runtime uses the actual17-item/eight-piece catalog. Sources remain separate from Content; do not import the sheet as an interactive menu or spawn its illustrated assets. Inventory uses a real grid/details tab; crafting uses centered recipe/details/queue; building uses illustrated four-column selection. The requested hotbar is a separately verified follow-up after art creation.

## One user review after technical verification

Level `/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld`, one normal standalone world. Start here: press Tab at the starting camp. Click an item and verify its picture, quantity and details; press U for upgrades, Esc back to Items, C for crafting, Esc out, B for pictured building pieces, Esc out, then P for Pause and Settings. Existing chest: aim within 2.5 metres and E. Main menu: use End session only after saving any progress you want to keep; inspect Single player saved worlds. Report **UI_HOTBAR PASS** using G17 in Docs/TRELLO_TEST_GUIDES.md or the first unreadable/clipped/misleading control and your resolution/HUD scale. Do not delete/rename a personal world just to test. This visual review is separate from automated transaction verification.

## Changed files across UI and hotbar follow-up

- ArtSource/UI/Designs/ExpeditionInterfaceConcept.png
- ArtSource/UI/ExpeditionActions.png
- ArtSource/UI/ExpeditionEncounters.png
- ArtSource/UI/ExpeditionItems.png
- ArtSource/UI/ExpeditionStructures.png
- ArtSource/UI/ExpeditionSymbols.png
- ArtSource/UI/FrontierMenu.png
- ArtSource/UI/README.md
- Config/DefaultGame.ini
- Content/PrimalFrontier/UI/Textures/T_ExpeditionActions.uasset
- Content/PrimalFrontier/UI/Textures/T_ExpeditionEncounters.uasset
- Content/PrimalFrontier/UI/Textures/T_ExpeditionItems.uasset
- Content/PrimalFrontier/UI/Textures/T_ExpeditionStructures.uasset
- Content/PrimalFrontier/UI/Textures/T_ExpeditionSymbols.uasset
- Content/PrimalFrontier/UI/Textures/T_FrontierMenu.uasset
- Docs/CURRENT_STATE.md
- Docs/DECISIONS.md
- Docs/HOTBAR_M11.md
- Docs/MILESTONES.md
- Docs/PLAYTEST.md
- Docs/TRELLO_SYNC.md
- Docs/TRELLO_TEST_GUIDES.md
- Docs/UI_OVERHAUL_M11.md
- Scripts/ImportExpeditionUI.py
- Scripts/RunControlsAutomation.ps1
- Source/PrimalFrontier/Building/PFBuildingMenus.cpp
- Source/PrimalFrontier/Crafting/PFCraftingHUD.cpp
- Source/PrimalFrontier/Inventory/PFHotbar.cpp
- Source/PrimalFrontier/Inventory/PFHotbar.h
- Source/PrimalFrontier/Inventory/PFHotbarHUD.cpp
- Source/PrimalFrontier/Inventory/PFHotbarHUD.h
- Source/PrimalFrontier/Inventory/PFHotbarInput.cpp
- Source/PrimalFrontier/Inventory/PFInventoryHUD.cpp
- Source/PrimalFrontier/Inventory/PFInventoryHUD.h
- Source/PrimalFrontier/Inventory/PFInventoryInput.cpp
- Source/PrimalFrontier/Progression/PFPlayerStatsMenu.cpp
- Source/PrimalFrontier/Settings/PFSettingsMenu.cpp
- Source/PrimalFrontier/Survival/PFControlsMenu.cpp
- Source/PrimalFrontier/Survival/PFGamepadInput.cpp
- Source/PrimalFrontier/Survival/PFPauseMenu.cpp
- Source/PrimalFrontier/Survival/PFSurvivalHUD.cpp
- Source/PrimalFrontier/Survival/PFSurvivalHUD.h
- Source/PrimalFrontier/Survival/PFSurvivalPlayerController.cpp
- Source/PrimalFrontier/Survival/PFSurvivalPlayerController.h
- Source/PrimalFrontier/Tests/PFHotbarLiveTests.cpp
- Source/PrimalFrontier/Tests/PFHotbarTests.cpp
- Source/PrimalFrontier/Tests/PFInventorySelectionLiveTests.cpp
- Source/PrimalFrontier/Tests/PFInventorySelectionTests.cpp
- Source/PrimalFrontier/Tests/PFProgressionFeedbackLiveTests.cpp
- Source/PrimalFrontier/Tests/PFSettingsApplyLiveTests.cpp
- Source/PrimalFrontier/Tests/PFTargetSpritesLiveTests.cpp
- Source/PrimalFrontier/Tests/PFUIAtlasTests.cpp
- Source/PrimalFrontier/Tests/PFWorldMenuLiveTests.cpp
- Source/PrimalFrontier/UI/PFItemPicture.cpp
- Source/PrimalFrontier/UI/PFItemPicture.h
- Source/PrimalFrontier/UI/PFMainMenu.cpp
- Source/PrimalFrontier/UI/PFMainMenu.h
- Source/PrimalFrontier/UI/PFReadOnlyOverlay.cpp
- Source/PrimalFrontier/UI/PFUISprites.cpp
- Source/PrimalFrontier/UI/PFUISprites.h
- Source/PrimalFrontier/UI/PFUITheme.h

## Publication receipt

Source/assets/evidence c04f62b4684d4d5a38a8965fdab519a38db2abe0 published to origin/master; remote SHA equality and clean working tree verified,6 LFS UI textures8.0MB uploaded,61 scoped files/Sol coauthor. This subsequent handoff receipt is documentation only; no additional source/assets/tests or manual acceptance.
