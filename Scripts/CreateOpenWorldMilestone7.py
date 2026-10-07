"""Create an isolated greybox open-world source map, then run UE's WP converter.

Never changes the M7 regression map or overwrites an existing destination.
Run WorldPartitionConvertCommandlet on the new map after this script succeeds.
"""
import unreal

MAP = "/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld"
SOURCE = "/Game/PrimalFrontier/Maps/L_M7SurvivalArena"
if unreal.EditorAssetLibrary.does_asset_exist(MAP):
    raise RuntimeError("Destination exists; inspect it instead of overwriting")
levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
if not levels.new_level_from_template(MAP, SOURCE):
    raise RuntimeError("Could not create independent open-world source map")
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
if world.get_path_name().split(".")[0] != MAP:
    raise RuntimeError("Unexpected destination world")
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
cube = unreal.load_asset("/Engine/BasicShapes/Cube.Cube")
rock = unreal.load_asset("/Game/PrimalFrontier/World/Greybox/MI_M7Rock")
grass = unreal.load_asset("/Game/PrimalFrontier/World/Greybox/MI_M7Grass")

for actor in list(actors.get_all_level_actors()):
    name = actor.get_actor_label()
    if name in {"M7_West", "M7_East", "M7_South", "M7_North"}:
        actors.destroy_actor(actor)  # only the new map's arena walls
    elif name == "M7_Ground":
        # Continuous 400 x 400 m collision floor. Decorative cells stream separately.
        actor.set_actor_location(unreal.Vector(0, 2000, -25), False, False)
        actor.set_actor_scale3d(unreal.Vector(400, 400, 0.5))
        actor.static_mesh_component.set_material(0, grass)

def block(name, at, scale, material):
    actor = actors.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(*at))
    actor.set_actor_label("OW_" + name)
    actor.static_mesh_component.set_static_mesh(cube)
    actor.static_mesh_component.set_material(0, material)
    actor.static_mesh_component.set_collision_profile_name("BlockAll")
    actor.set_actor_scale3d(unreal.Vector(*scale))
    return actor

# Distant primitive landmarks make streamed travel observable without large assets.
for index, (x, y) in enumerate([(-12000, 0), (12000, 0), (-12000, 14000), (12000, 14000)]):
    block("Landmark" + str(index), (x, y, 600), (8, 8, 12), rock)
    sign = actors.spawn_actor_from_class(unreal.TextRenderActor, unreal.Vector(x, y-500, 250), unreal.Rotator(pitch=0,yaw=-90,roll=0))
    sign.set_actor_label("OW_Sign" + str(index))
    text = sign.get_component_by_class(unreal.TextRenderComponent)
    text.set_text("FRONTIER LANDMARK " + str(index+1) + " - return south to the survival camp")
    text.set_world_size(60)

# Visible boundary cliffs rather than invisible arena walls; retain a small test footprint.
for name, at, scale in [
    ("WestRidge",(-19900,2000,250),(2,400,5)),
    ("EastRidge",(19900,2000,250),(2,400,5)),
    ("SouthRidge",(0,-17900,250),(400,2,5)),
    ("NorthRidge",(0,21900,250),(400,2,5)),
]:
    block(name, at, scale, rock)
if not levels.save_current_level():
    raise RuntimeError("Open-world source map save failed")
unreal.log("[PrimalWorld] Created independent 400x400m greybox source map with the existing survival camp. World Partition conversion is still required.")
