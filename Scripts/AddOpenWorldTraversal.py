"""M14: add only stateless primitive ledges south-east of the spawn camp.

Supported Editor APIs; refuse custom/partial courses and preserve all old actor
identities/transforms/catalog assignments. Repeat is verification-only.
"""
import json
import os
from pathlib import Path
import unreal

MAP = "/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld"
PREFIX = "M14_Traversal_"
SPECS = [("80_A", (1000, -2300, 40), (3, 3.6, .8)),
         ("110_A", (1600, -2300, 55), (3, 3.6, 1.1)),
         ("80_B", (1000, -3000, 40), (3, 3.6, .8)),
         ("110_B", (1600, -3000, 55), (3, 3.6, 1.1))]
SIGN_AT = (600, -1900, 180)
SIGN = "CLIMB LEDGES: 80cm / 110cm\nFace a WEST edge; hold forward + jump (35 stamina)\nF: manual climb (15). Tall / blocked walls refuse."
levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
editor_actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

def load():
    if not levels.load_level(MAP):
        raise RuntimeError("Could not load existing open-world map")
    world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
    if world.get_path_name().split(".")[0] != MAP:
        raise RuntimeError("Refusing unexpected map")
    descs = unreal.WorldPartitionBlueprintLibrary.get_actor_descs()
    if isinstance(descs, tuple):
        ok, descs = descs
        if not ok:
            raise RuntimeError("Descriptor query failed")
    unreal.WorldPartitionBlueprintLibrary.load_actors([d.guid for d in descs])
    return editor_actors.get_all_level_actors()

def xyz(v):
    return [v.x, v.y, v.z]

def snapshot(actors):
    rows = []
    for a in actors:
        if a.get_actor_label().startswith(PREFIX):
            continue
        r = dict(name=a.get_name(), label=a.get_actor_label(), cls=a.get_class().get_name(),
                 at=xyz(a.get_actor_location()), scale=xyz(a.get_actor_scale3d()),
                 yaw=a.get_actor_rotation().yaw, spatial=a.get_editor_property("is_spatially_loaded"))
        if r["cls"] == "PFResourceNode":
            r["resource"] = str(a.get_editor_property("resource_id"))
        if r["cls"] == "PFCreatureSpawner":
            r.update(creature=str(a.get_editor_property("creature_id")),
                     biome=str(unreal.GameplayTagLibrary.get_tag_name(a.get_editor_property("biome"))),
                     enabled=a.get_editor_property("use_spawn_policy"),
                     policy=a.get_editor_property("spawn_policies").get_path_name())
        rows.append(r)
    return sorted(rows, key=lambda r: r["name"])

def verify(actors):
    found = {a.get_actor_label(): a for a in actors if a.get_actor_label().startswith(PREFIX)}
    if len(found) != 5:
        raise RuntimeError("Expected exactly four ledges and one sign")
    for name, at, scale in SPECS:
        a = found.get(PREFIX + name)
        if not a or a.get_class().get_name() != "StaticMeshActor":
            raise RuntimeError("Missing/unexpected ledge " + name)
        c = a.static_mesh_component
        if (xyz(a.get_actor_location()) != list(at) or
            any(abs(x-y) > .0001 for x, y in zip(xyz(a.get_actor_scale3d()), scale)) or
            a.get_actor_rotation().yaw != 0 or a.get_editor_property("is_spatially_loaded") or
            str(c.get_editor_property("static_mesh").get_path_name()) != "/Engine/BasicShapes/Cube.Cube" or
            str(c.get_collision_profile_name()) != "BlockAll" or
            not a.actor_has_tag(unreal.Name("PF.M14.Ledge" + name))):
            raise RuntimeError("Unexpected custom ledge configuration " + name)
    a = found.get(PREFIX + "Sign")
    if not a or a.get_class().get_name() != "TextRenderActor" or xyz(a.get_actor_location()) != list(SIGN_AT):
        raise RuntimeError("Unexpected sign")
    if str(a.get_component_by_class(unreal.TextRenderComponent).get_editor_property("text")) != SIGN:
        raise RuntimeError("Unexpected sign text")
    # Newly created WP actors receive an external-package object name at save.
    # Their persistent ActorGuid, unlike that temporary name, must not change.
    return {name: unreal.GuidLibrary.conv_guid_to_string(a.get_editor_property("actor_guid")) for name, a in found.items()}

actors = load()
before = snapshot(actors)
existing = [a for a in actors if a.get_actor_label().startswith(PREFIX)]
mutations = 0
if existing:
    identities = verify(actors)  # never overwrite a custom/partial course
else:
    cube = unreal.load_asset("/Engine/BasicShapes/Cube.Cube")
    rock = unreal.load_asset("/Game/PrimalFrontier/World/Greybox/MI_M7Rock")
    if not cube or not rock:
        raise RuntimeError("Existing primitive/material unavailable")
    for name, at, scale in SPECS:
        a = editor_actors.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(*at))
        a.set_actor_label(PREFIX + name)
        a.set_editor_property("is_spatially_loaded", False)
        a.set_editor_property("tags", [unreal.Name("PF.M14.Ledge" + name)])
        c = a.static_mesh_component
        c.set_static_mesh(cube)
        c.set_material(0, rock)
        c.set_collision_profile_name("BlockAll")
        a.set_actor_scale3d(unreal.Vector(*scale))
        mutations += 1
    sign = editor_actors.spawn_actor_from_class(unreal.TextRenderActor, unreal.Vector(*SIGN_AT), unreal.Rotator(pitch=0, yaw=180, roll=0))
    sign.set_actor_label(PREFIX + "Sign")
    sign.set_editor_property("is_spatially_loaded", False)
    text = sign.get_component_by_class(unreal.TextRenderComponent)
    text.set_text(SIGN)
    text.set_world_size(24)
    mutations += 1
    identities = verify(editor_actors.get_all_level_actors())
    if snapshot(editor_actors.get_all_level_actors()) != before:
        raise RuntimeError("Old actor configuration changed before save")
    if not levels.save_current_level():
        raise RuntimeError("Course save failed")
actors = load()
if verify(actors) != identities or snapshot(actors) != before:
    raise RuntimeError("Disk reload changed course or pre-existing actor identities/configuration")
out = Path(os.environ["PF_TRAVERSAL_REPORT"])
out.parent.mkdir(parents=True, exist_ok=True)
out.write_text(json.dumps(dict(map=MAP, mutations=mutations, course=identities,
                              old_actor_count=len(before), old_actors_unchanged=True,
                              reload_verified=True, ledges=SPECS, sign=SIGN_AT), indent=2), encoding="utf-8")
unreal.log("[PrimalAgentTools] Authored open-world traversal saved/reloaded; old actors unchanged; mutations=" + str(mutations))
