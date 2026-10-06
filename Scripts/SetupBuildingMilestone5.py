"""Create only new M5 assets through supported editor APIs; refuse overwrite.

Milestone 5 (5702d4b). Creates /Game/PrimalFrontier/Building/DA_BuildingCatalog
(UPFBuildingCatalog) and /Game/PrimalFrontier/Maps/L_M5Building, a copy of
L_M4Gathering with a large extra floor (M5_BuildingGround) for placement tests.
"""
import unreal

MAP = "/Game/PrimalFrontier/Maps/L_M5Building"
CATALOG = "/Game/PrimalFrontier/Building/DA_BuildingCatalog"
if any(unreal.EditorAssetLibrary.does_asset_exist(p) for p in [MAP, CATALOG]):
    raise RuntimeError("M5 destinations exist; inspect instead of overwriting")
factory = unreal.DataAssetFactory()
factory.set_editor_property("data_asset_class", unreal.load_class(None, "/Script/PrimalFrontier.PFBuildingCatalog"))
catalog = unreal.AssetToolsHelpers.get_asset_tools().create_asset("DA_BuildingCatalog", "/Game/PrimalFrontier/Building", None, factory)
if not catalog or not unreal.EditorAssetLibrary.save_loaded_asset(catalog):
    raise RuntimeError("M5 catalog save failed")
levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
if not levels.new_level_from_template(MAP, "/Game/PrimalFrontier/Maps/L_M4Gathering"):
    raise RuntimeError("M5 map creation failed")
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
if world.get_path_name().split(".")[0] != MAP:
    raise RuntimeError("Wrong active map")
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
floor = actors.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(0, 1800, -25))
floor.static_mesh_component.set_static_mesh(unreal.load_asset("/Engine/BasicShapes/Cube.Cube"))
floor.set_actor_scale3d(unreal.Vector(40, 40, 0.5))
floor.static_mesh_component.set_collision_profile_name("BlockAll")
floor.set_actor_label("M5_BuildingGround")
if not levels.save_current_level():
    raise RuntimeError("M5 map save failed")
unreal.log("[PrimalBuilding] Saved new M5 catalog and map only.")
