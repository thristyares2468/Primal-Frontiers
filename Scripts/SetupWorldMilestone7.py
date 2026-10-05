"""Create a new bounded greybox arena; never overwrite an existing destination."""
import unreal

MAP = "/Game/PrimalFrontier/Maps/L_M7SurvivalArena"
if unreal.EditorAssetLibrary.does_asset_exist(MAP):
    raise RuntimeError("M7 destination exists; inspect instead of overwriting")
levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
if not levels.new_level_from_template(MAP, "/Game/PrimalFrontier/Maps/L_M1Survival"):
    raise RuntimeError("Could not create M7 map")
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
if world.get_path_name().split(".")[0] != MAP:
    raise RuntimeError("Wrong destination world")
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
# Clear geometry/lights in this new copy only, retaining GameMode composition.
for actor in actors.get_all_level_actors():
    if isinstance(actor, (unreal.StaticMeshActor, unreal.Light, unreal.PlayerStart)):
        actors.destroy_actor(actor)
cube = unreal.load_asset("/Engine/BasicShapes/Cube.Cube")
def block(label, at, scale):
    actor = actors.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(*at))
    actor.static_mesh_component.set_static_mesh(cube)
    actor.static_mesh_component.set_collision_profile_name("BlockAll")
    actor.set_actor_scale3d(unreal.Vector(*scale))
    actor.set_actor_label("M7_" + label)
    return actor
def native(name, label, at):
    actor = actors.spawn_actor_from_class(unreal.load_class(None, "/Script/PrimalFrontier." + name), unreal.Vector(*at))
    actor.set_actor_label("M7_" + label)
    return actor
def sign(label, text, at):
    actor = actors.spawn_actor_from_class(unreal.TextRenderActor, unreal.Vector(*at), unreal.Rotator(pitch=0,yaw=-90,roll=0))
    component = actor.get_component_by_class(unreal.TextRenderComponent)
    component.set_text(text)
    component.set_world_size(35)
    actor.set_actor_label("M7_Sign_" + label)

block("Ground", (0,2000,-25), (60,70,0.5))
for name, at, scale in [("West",(-3000,2000,100),(0.4,70,2)),("East",(3000,2000,100),(0.4,70,2)),("South",(0,-1500,100),(60,0.4,2)),("North",(0,5500,100),(60,0.4,2))]:
    block(name, at, scale)
for step in range(4):
    block("Rise"+str(step),(-1800,2800,10+20*step),(18-3*step,18-3*step,0.2))
block("DangerLandmark", (1500,4800,200), (1.5,1.5,4))
for x in [-250,250]:
    start = actors.spawn_actor_from_class(unreal.PlayerStart,unreal.Vector(x,-1100,100),unreal.Rotator(pitch=0,yaw=90,roll=0))
    start.set_actor_label("M7_Start"+str(x))
sign("Start","SAFE START - gather nearby, build in the clearing",(-1200,-900,180))
sign("Resources","RESOURCE ZONE - wood / stone / food",(-1000,1800,180))
sign("Danger","DANGER - prowler and exposure ahead",(-1000,3600,200))
for index,(rid,at) in enumerate([
    ("Node_Wood",(-400,-600,60)),("Node_Stone",(0,-500,60)),("Node_Food",(400,-600,60)),
    ("Node_Wood",(-800,1900,60)),("Node_Wood",(-400,2300,60)),("Node_Stone",(400,2000,60)),
    ("Node_Food",(800,1800,60)),("Node_Food",(-700,4200,60)),("Node_Stone",(700,4600,60))]):
    node=native("PFResourceNode","Resource"+str(index),at)
    node.set_editor_property("resource_id",rid)
    node.set_actor_rotation(unreal.Rotator(pitch=0,yaw=90,roll=0),False)
hazard=native("PFSurvivalHazard","Exposure",(0,3400,180))
hazard.set_actor_scale3d(unreal.Vector(3,1.5,1))
hazard.set_editor_property("intensity",0.6)
native("PFWorldClock","Clock",(0,2000,500))
nav=native("PFNavigationBounds","Navigation",(0,2000,0))
if not nav.build_navigation():
    raise RuntimeError("M7 navigation build/projection failed")
for name, creature, at in [("Forager","Creature_Forager",(-1500,1700,60)),("Prowler","Creature_Prowler",(1300,4400,60))]:
    spawn=native("PFCreatureSpawner",name,at)
    spawn.set_editor_property("creature_id",creature)
if not levels.save_current_level():
    raise RuntimeError("M7 map save failed")
unreal.log("[PrimalWorld] Saved new L_M7SurvivalArena: 60x70m, nine resource nodes, two spawns, one hazard, baked navigation and server clock.")
