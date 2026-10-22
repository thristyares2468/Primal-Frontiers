"""Add owned-station definitions only; never edit a map or existing piece rows."""
import unreal

path = "/Game/PrimalFrontier/Building/DA_BuildingCatalog"
catalog = unreal.load_asset(path)
if not catalog or not isinstance(catalog, unreal.PFBuildingCatalog):
    raise RuntimeError("Expected existing PFBuildingCatalog")
rows = list(catalog.get_editor_property("pieces"))
old_ids = ["Build_Foundation", "Build_Wall", "Build_Floor", "Build_Ceiling", "Build_Door", "Build_Storage", "Build_Window", "Build_Stairs"]
new_ids = ["Build_Workbench", "Build_Refinery"]
ids = [str(row.get_editor_property("id")) for row in rows]
if ids not in (old_ids, old_ids + new_ids):
    raise RuntimeError("Refuse custom/partial building catalog; preserve owner rows")
changed = ids == old_ids
if changed:
    defaults = unreal.new_object(unreal.PFBuildingCatalog)
    additions = list(defaults.get_editor_property("pieces"))[8:]
    if [str(row.get_editor_property("id")) for row in additions] != new_ids:
        raise RuntimeError("Native station definitions unavailable")
    catalog.set_editor_property("pieces", rows + additions)
    if not unreal.EditorAssetLibrary.save_loaded_asset(catalog):
        raise RuntimeError("Owned station catalog save failed")
if [str(row.get_editor_property("id")) for row in catalog.get_editor_property("pieces")] != old_ids + new_ids:
    raise RuntimeError("Owned station catalog readback mismatch")
unreal.log("[PrimalCrafting] Owned station catalog PASS: ten choices; old eight rows retained; changed=" + str(changed))
