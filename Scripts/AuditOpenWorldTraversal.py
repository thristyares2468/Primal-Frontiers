"""Read-only supported Editor audit for M14 stateless traversal additions."""
import json
import os
from pathlib import Path
import unreal

MAP = "/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld"
levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
if not levels.load_level(MAP):
    raise RuntimeError("Cannot load expected open-world map")
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
if world.get_path_name().split(".")[0] != MAP:
    raise RuntimeError("Unexpected world; refusing traversal audit")
descs = unreal.WorldPartitionBlueprintLibrary.get_actor_descs()
if isinstance(descs, tuple):
    ok, descs = descs
    if not ok:
        raise RuntimeError("World Partition descriptors unavailable")
unreal.WorldPartitionBlueprintLibrary.load_actors([d.guid for d in descs])
rows = []
for actor in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors():
    v, scale, rot = actor.get_actor_location(), actor.get_actor_scale3d(), actor.get_actor_rotation()
    row = dict(label=actor.get_actor_label(), name=actor.get_name(),
               cls=actor.get_class().get_name(), position=[v.x, v.y, v.z],
               scale=[scale.x, scale.y, scale.z], yaw=rot.yaw,
               spatial=actor.get_editor_property("is_spatially_loaded"))
    mesh = actor.get_component_by_class(unreal.StaticMeshComponent)
    if mesh:
        row.update(mesh=mesh.get_editor_property("static_mesh").get_path_name() if mesh.get_editor_property("static_mesh") else None,
                   collision=str(mesh.get_collision_profile_name()))
    if row["cls"] == "PFResourceNode":
        row["resource"] = str(actor.get_editor_property("resource_id"))
    if row["cls"] == "PFCreatureSpawner":
        row.update(creature=str(actor.get_editor_property("creature_id")),
                       biome=str(unreal.GameplayTagLibrary.get_tag_name(actor.get_editor_property("biome"))),
                   policy_enabled=actor.get_editor_property("use_spawn_policy"),
                       policy=actor.get_editor_property("spawn_policies").get_path_name() if actor.get_editor_property("spawn_policies") else None)
    rows.append(row)
report = Path(os.environ["PF_TRAVERSAL_REPORT"])
report.parent.mkdir(parents=True, exist_ok=True)
report.write_text(json.dumps(dict(map=MAP, mutations=0, actors=sorted(rows, key=lambda r: r["name"])), indent=2), encoding="utf-8")
unreal.log("[PrimalAgentTools] Read-only traversal audit complete: " + str(report))
