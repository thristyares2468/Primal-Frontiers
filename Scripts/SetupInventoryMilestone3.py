"""Create only new M3 data and greybox fixtures through Unreal Editor APIs.

Milestone 3 (340c538). Creates:
  * /Game/PrimalFrontier/Items/DA_ItemCatalog: a UPFItemCatalog data asset (its item
    definitions come from the C++ constructor defaults).
  * /Game/PrimalFrontier/Maps/L_M3Inventory: a copy of L_M1Survival with three
    APFItemPickup actors (5 wood, 5 stone, 4 food).
Refuses to run if either destination exists.
"""
import unreal

CATALOG = "/Game/PrimalFrontier/Items/DA_ItemCatalog"
MAP = "/Game/PrimalFrontier/Maps/L_M3Inventory"
if unreal.EditorAssetLibrary.does_asset_exist(CATALOG) or unreal.EditorAssetLibrary.does_asset_exist(MAP):
    raise RuntimeError("M3 assets already exist; refusing to overwrite")
catalog_class = unreal.load_class(None, "/Script/PrimalFrontier.PFItemCatalog")
factory = unreal.DataAssetFactory()
factory.set_editor_property("data_asset_class", catalog_class)
catalog = unreal.AssetToolsHelpers.get_asset_tools().create_asset("DA_ItemCatalog", "/Game/PrimalFrontier/Items", catalog_class, factory)
if not catalog or not unreal.EditorAssetLibrary.save_loaded_asset(catalog):
    raise RuntimeError("Catalog creation/save failed")
levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
if not levels.new_level_from_template(MAP, "/Game/PrimalFrontier/Maps/L_M1Survival"):
    raise RuntimeError("M3 map creation failed")
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
if world.get_path_name().split(".")[0] != MAP:
    raise RuntimeError("Wrong active world; refusing changes")
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
pickup_class = unreal.load_class(None, "/Script/PrimalFrontier.PFItemPickup")
for item_id, quantity, y in [("Item_Wood", 5, 0), ("Item_Stone", 5, 250), ("Item_Food", 4, -250)]:
    pickup = actors.spawn_actor_from_class(pickup_class, unreal.Vector(-240, y, 150))
    pickup.set_editor_property("item_id", item_id)
    pickup.set_editor_property("quantity", quantity)
    pickup.set_actor_label("M3_" + item_id)
if not levels.save_current_level():
    raise RuntimeError("M3 map save failed")
unreal.log("[PrimalInventory] Saved M3 catalog and isolated map; existing assets unchanged.")
