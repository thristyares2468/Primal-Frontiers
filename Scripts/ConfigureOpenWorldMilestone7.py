"""Keep the small survival camp alive while primitive landmarks stream.

Safe to rerun. Changes only L_PrimalFrontier_OpenWorld through Editor APIs.
M8 must define persistent simulation lifetimes before these actors may unload.
"""
import unreal

MAP = "/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld"
levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
if not levels.load_level(MAP):
    raise RuntimeError("Open-world candidate could not be loaded")
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
if world.get_path_name().split(".")[0] != MAP:
    raise RuntimeError("Refusing to change an unexpected world")

def descriptors():
    result = unreal.WorldPartitionBlueprintLibrary.get_actor_descs()
    if isinstance(result, tuple):
        success, result = result
        if not success:
            raise RuntimeError("World Partition descriptor query failed")
    if result is None:
        raise RuntimeError("No World Partition descriptors returned")
    return result

unreal.WorldPartitionBlueprintLibrary.load_actors([d.guid for d in descriptors()])
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
changed = 0
for actor in actors.get_all_level_actors():
    label = actor.get_actor_label()
    # Only stateless, distant landmark blocks/signs are allowed to unload.
    spatial = label.startswith("OW_Landmark") or label.startswith("OW_Sign")
    if actor.get_editor_property("is_spatially_loaded") != spatial:
        actor.set_editor_property("is_spatially_loaded", spatial)
        changed += 1
if not levels.save_current_level():
    raise RuntimeError("Open-world actor policy save failed")
if not levels.load_level(MAP):
    raise RuntimeError("Open-world policy reload failed")
descs = descriptors()
core = [d for d in descs if not str(d.label).startswith(("OW_Landmark", "OW_Sign"))]
if not core or any(d.get_editor_property("is_spatially_loaded") for d in core):
    raise RuntimeError("Core actors must remain loaded after a disk reload")
if sum(str(d.label) == "M7_Clock" for d in descs) != 1:
    raise RuntimeError("Expected exactly one persistent survival clock")
unreal.log("[PrimalWorld] Open-world streaming policy verified after reload: %d core actors remain loaded; %d distant landmarks stream; %d changed." % (len(core), len(descs)-len(core), changed))
