"""Read-only M13 placement audit through supported Unreal Editor APIs."""
import json
import os
from pathlib import Path
import unreal

MAP = "/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld"
levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
if not levels.load_level(MAP):
    raise RuntimeError("Could not load the existing open-world map")
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
if world.get_path_name().split(".")[0] != MAP:
    raise RuntimeError("Unexpected world; refusing audit")
descs = unreal.WorldPartitionBlueprintLibrary.get_actor_descs()
if isinstance(descs, tuple):
    ok, descs = descs
    if not ok:
        raise RuntimeError("Descriptor query refused")
unreal.WorldPartitionBlueprintLibrary.load_actors([d.guid for d in descs])
rows = []
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
for a in actors.get_all_level_actors():
    if a.get_class().get_name() in ("PFCreatureSpawner", "PFWorldClock"):
        v = a.get_actor_location()
        row = dict(label=a.get_actor_label(), path=a.get_path_name(),
                   cls=a.get_class().get_name(), position=[v.x, v.y, v.z],
                   spatial=a.get_editor_property("is_spatially_loaded"))
        if row["cls"] == "PFCreatureSpawner":
            row.update(creature=str(a.get_editor_property("creature_id")),
                       policy_enabled=a.get_editor_property("use_spawn_policy"),
                       biome=str(a.get_editor_property("biome")),
                       policy=str(a.get_editor_property("spawn_policies")))
        rows.append(row)
result = dict(map=MAP, actors=rows, asset_mutations=0)
out = Path(os.environ["PF_ECOLOGY_REPORT"])
out.parent.mkdir(parents=True, exist_ok=True)
out.write_text(json.dumps(result, indent=2), encoding="utf-8")
unreal.log("[PrimalAgentTools] Open-world ecology read-only audit complete: " + str(out))
