"""Create M4 greybox fixtures; extend only the project-owned item catalog."""
import unreal

MAP = "/Game/PrimalFrontier/Maps/L_M4Gathering"
CRAFT = "/Game/PrimalFrontier/Crafting/DA_CraftingCatalog"
ITEMS = "/Game/PrimalFrontier/Items/DA_ItemCatalog"
if any(unreal.EditorAssetLibrary.does_asset_exist(p) for p in [MAP, CRAFT]):
    raise RuntimeError("M4 destinations exist; inspect before rerunning, never overwrite")
items = unreal.load_asset(ITEMS)
if not items:
    raise RuntimeError("Existing M3 item catalog missing")
existing = list(items.get_editor_property("items"))
ids = {str(i.get_editor_property("id")) for i in existing}
defaults = unreal.get_default_object(items.get_class()).get_editor_property("items")
for definition in defaults:
    if str(definition.get_editor_property("id")) not in ids:
        existing.append(definition)
items.set_editor_property("items", existing)
if not unreal.EditorAssetLibrary.save_loaded_asset(items):
    raise RuntimeError("Item catalog save failed")
factory = unreal.DataAssetFactory()
factory.set_editor_property("data_asset_class", unreal.load_class(None, "/Script/PrimalFrontier.PFCraftingCatalog"))
catalog = unreal.AssetToolsHelpers.get_asset_tools().create_asset("DA_CraftingCatalog", "/Game/PrimalFrontier/Crafting", None, factory)
if not catalog or not unreal.EditorAssetLibrary.save_loaded_asset(catalog):
    raise RuntimeError("Crafting catalog save failed")
levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
if not levels.new_level_from_template(MAP, "/Game/PrimalFrontier/Maps/L_M1Survival"):
    raise RuntimeError("New M4 level failed")
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
if world.get_path_name().split(".")[0] != MAP:
    raise RuntimeError("Wrong active map")
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
node_class = unreal.load_class(None, "/Script/PrimalFrontier.PFResourceNode")
for resource_id, y in [("Node_Wood", 0), ("Node_Stone", 350), ("Node_Food", -350)]:
    node = actors.spawn_actor_from_class(node_class, unreal.Vector(-220, y, 130))
    node.set_editor_property("resource_id", resource_id)
    node.set_actor_label("M4_" + resource_id)
if not levels.save_current_level():
    raise RuntimeError("M4 map save failed")
unreal.log("[PrimalCrafting] Saved M4 map/catalog and appended new M4 item definitions; template assets unchanged.")
