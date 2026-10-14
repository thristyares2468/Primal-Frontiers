"""Import only the six original runtime UI images. Never loads/saves a level or catalog."""
from pathlib import Path
import os
import subprocess
import unreal

root = Path(unreal.Paths.project_dir())
sources = root / "ArtSource" / "UI"
imported = 0
verified = 0
for filename, name in [("ExpeditionItems.png", "T_ExpeditionItems"), ("FrontierMenu.png", "T_FrontierMenu"), ("ExpeditionStructures.png", "T_ExpeditionStructures"), ("ExpeditionSymbols.png", "T_ExpeditionSymbols"), ("ExpeditionActions.png", "T_ExpeditionActions"), ("ExpeditionEncounters.png", "T_ExpeditionEncounters")]:
    destination = "/Game/PrimalFrontier/UI/Textures"
    task = unreal.AssetImportTask()
    task.filename = str(sources / filename)
    task.destination_path = destination
    task.destination_name = name
    task.automated = True
    task.replace_existing = False
    task.save = False
    if unreal.EditorAssetLibrary.does_asset_exist(destination + "/" + name):
        existing = unreal.load_asset(destination + "/" + name)
        if not isinstance(existing, unreal.Texture2D):
            raise RuntimeError("Existing path is not a texture: " + name)
        original = Path(existing.get_editor_property("asset_import_data").get_first_filename()).resolve()
        if original != (sources / filename).resolve():
            raise RuntimeError("Refusing unrelated texture source: " + name)
        if name == os.environ.get("PF_UI_REIMPORT_OWN_DRAFT"):
            # Only the newly created, uncommitted draft from this task can be replaced.
            relative = "Content/PrimalFrontier/UI/Textures/" + name + ".uasset"
            tracked = subprocess.run(["git", "ls-files", "--", relative], cwd=root, capture_output=True, text=True, check=True)
            if tracked.stdout.strip():
                raise RuntimeError("Refusing to replace a tracked texture with the draft-only path")
            task.replace_existing = True
        else:
            verified += 1
            continue  # Never reimport/save an established asset.
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    texture = unreal.load_asset(destination + "/" + name)
    if not isinstance(texture, unreal.Texture2D):
        raise RuntimeError("Texture import failed: " + name)
    texture.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_EDITOR_ICON)
    texture.set_editor_property("mip_gen_settings", unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
    texture.set_editor_property("lod_group", unreal.TextureGroup.TEXTUREGROUP_UI)
    texture.set_editor_property("srgb", True)
    if not unreal.EditorAssetLibrary.save_loaded_asset(texture):
        raise RuntimeError("Texture save failed: " + name)
    unreal.log("[PrimalAgentTools] UI texture imported: " + texture.get_path_name())
    imported += 1
unreal.log("[PrimalAgentTools] Expedition UI import PASS: imported={}, verified={}, no map/catalog changes".format(imported, verified))
