"""One narrow correction to the new M7 map's Python positional Rotator usage."""
import unreal
MAP="/Game/PrimalFrontier/Maps/L_M7SurvivalArena"
levels=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
if not levels.load_level(MAP):
    raise RuntimeError("M7 map missing")
world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
if world.get_path_name().split(".")[0]!=MAP:
    raise RuntimeError("Wrong map")
actors=unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
starts=[a for a in actors if isinstance(a,unreal.PlayerStart) and a.get_actor_label().startswith("M7_Start")]
signs=[a for a in actors if isinstance(a,unreal.TextRenderActor) and a.get_actor_label().startswith("M7_Sign_")]
nodes=[a for a in actors if a.get_class().get_name()=="PFResourceNode" and a.get_actor_label().startswith("M7_Resource")]
if (len(starts),len(signs),len(nodes))!=(2,3,9):
    raise RuntimeError("Unexpected fixture inventory; no changes made")
unreal.log("[PrimalWorld] Positional Rotator(0,90,0)="+str(unreal.Rotator(0,90,0)))
for actor in starts+nodes:
    actor.set_actor_rotation(unreal.Rotator(pitch=0,yaw=90,roll=0),False)
for actor in signs:
    actor.set_actor_rotation(unreal.Rotator(pitch=0,yaw=-90,roll=0),False)
if not levels.save_current_level():
    raise RuntimeError("M7 orientation save failed")
unreal.log("[PrimalWorld] Repaired only two starts, three signs and nine node rotations in M7.")
