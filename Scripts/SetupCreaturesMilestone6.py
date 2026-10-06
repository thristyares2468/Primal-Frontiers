"""Create only new M6 definitions and small navigation arena via supported APIs.

Milestone 6 (0c2d935). Creates /Game/PrimalFrontier/Creatures/DA_CreatureCatalog
(UPFCreatureCatalog) and /Game/PrimalFrontier/Maps/L_M6Creatures from L_M5Building
with an arena floor, a path obstacle (the creature live test checks that navigation
detours around it), the PlayerStart moved to the arena, an APFNavigationBounds that
builds the navmesh, and forager/prowler APFCreatureSpawner actors.

Pass -PFResumeM6Setup on the editor command line to finish a setup that was
interrupted after the catalog/map were created (it still refuses duplicate M6_ actors).
"""
import unreal

MAP = "/Game/PrimalFrontier/Maps/L_M6Creatures"
CATALOG = "/Game/PrimalFrontier/Creatures/DA_CreatureCatalog"
resume = "-PFResumeM6Setup" in unreal.SystemLibrary.get_command_line()
if not resume and any(unreal.EditorAssetLibrary.does_asset_exist(p) for p in [MAP, CATALOG]):
    raise RuntimeError("M6 assets exist; inspect instead of overwriting")
factory = unreal.DataAssetFactory()
factory.set_editor_property("data_asset_class", unreal.load_class(None, "/Script/PrimalFrontier.PFCreatureCatalog"))
catalog = unreal.load_asset(CATALOG) if resume else unreal.AssetToolsHelpers.get_asset_tools().create_asset("DA_CreatureCatalog", "/Game/PrimalFrontier/Creatures", None, factory)
if not catalog or not unreal.EditorAssetLibrary.save_loaded_asset(catalog):
    raise RuntimeError("Creature catalog save failed")
levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
if not (levels.load_level(MAP) if resume else levels.new_level_from_template(MAP, "/Game/PrimalFrontier/Maps/L_M5Building")):
    raise RuntimeError("M6 map creation failed")
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
if world.get_path_name().split(".")[0] != MAP:
    raise RuntimeError("Wrong map")
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
if any(a.get_actor_label().startswith("M6_") for a in actors.get_all_level_actors()):
    raise RuntimeError("M6 fixture actors already exist; refusing duplicate setup")
cube = unreal.load_asset("/Engine/BasicShapes/Cube.Cube")
for label, position, scale in [("M6_Arena", (0,4400,-25), (40,24,0.5)), ("M6_PathObstacle", (0,4400,75), (1,6,1.5))]:
    actor = actors.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(*position))
    actor.static_mesh_component.set_static_mesh(cube)
    actor.set_actor_scale3d(unreal.Vector(*scale))
    actor.static_mesh_component.set_collision_profile_name("BlockAll")
    actor.set_actor_label(label)
for actor in actors.get_all_level_actors():
    if isinstance(actor, unreal.PlayerStart):
        actor.set_actor_location(unreal.Vector(-1400,4000,100), False, False)
        actor.set_actor_rotation(unreal.Rotator(0,0,0), False)
nav = actors.spawn_actor_from_class(unreal.load_class(None, "/Script/PrimalFrontier.PFNavigationBounds"), unreal.Vector(0,2200,0))
if not nav.build_navigation():
    raise RuntimeError("M6 navigation failed to build/project")
for label, creature_id, position in [("M6_ForagerSpawn", "Creature_Forager", (-800,4300,60)), ("M6_ProwlerSpawn", "Creature_Prowler", (800,4700,60))]:
    actor = actors.spawn_actor_from_class(unreal.load_class(None, "/Script/PrimalFrontier.PFCreatureSpawner"), unreal.Vector(*position))
    actor.set_editor_property("creature_id", creature_id)
    actor.set_editor_property("catalog", catalog)
    actor.set_actor_label(label)
if not levels.save_current_level():
    raise RuntimeError("M6 map save failed")
unreal.log("[PrimalCreatures] Saved new M6 catalog/map with navigation only.")
