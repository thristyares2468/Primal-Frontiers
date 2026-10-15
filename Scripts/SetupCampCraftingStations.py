"""Add three stateless camp fixtures through Editor APIs; preserve all existing actor identities."""
import json
import os
from pathlib import Path
import unreal

MAP = "/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld"
PREFIX = "M12_CampStation_"
SPECS = [("Workbench", [700, -500, 0], 1.25), ("Refinery", [1100, -500, 0], 1.5)]
report = Path(os.environ["PF_CAMP_STATION_SETUP_REPORT"])
if report.exists():
    raise RuntimeError("Refusing reused report")
levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)


def load():
    if not levels.load_level(MAP):
        raise RuntimeError("Expected map unavailable")
    descs = unreal.WorldPartitionBlueprintLibrary.get_actor_descs()
    if isinstance(descs, tuple):
        ok, descs = descs
        if not ok:
            raise RuntimeError("Partition descriptors unavailable")
    unreal.WorldPartitionBlueprintLibrary.load_actors([d.guid for d in descs])
    return actors.get_all_level_actors()


def xyz(v):
    return [v.x, v.y, v.z]


def snapshot(rows):
    data = []
    for actor in rows:
        if actor.get_actor_label().startswith(PREFIX):
            continue
        rot = actor.get_actor_rotation()
        row = dict(name=actor.get_name(), label=actor.get_actor_label(), cls=actor.get_class().get_name(),
                   guid=unreal.GuidLibrary.conv_guid_to_string(actor.get_editor_property("actor_guid")),
                   at=xyz(actor.get_actor_location()), scale=xyz(actor.get_actor_scale3d()),
                   rotation=[rot.pitch, rot.yaw, rot.roll], tags=[str(t) for t in actor.get_editor_property("tags")],
                   spatial=actor.get_editor_property("is_spatially_loaded"))
        if row["cls"] == "PFResourceNode":
            row["definition"] = str(actor.get_editor_property("resource_id"))
        if row["cls"] == "PFCreatureSpawner":
            row["definition"] = str(actor.get_editor_property("creature_id"))
            row["policy"] = actor.get_editor_property("spawn_policies").get_path_name()
        data.append(row)
    return sorted(data, key=lambda x: x["name"])


rows = load()
before = snapshot(rows)
if len(before) != 97:
    raise RuntimeError("Unexpected baseline; no edits")
found = [a for a in rows if a.get_actor_label().startswith(PREFIX)]
if found and len(found) != 3:
    raise RuntimeError("Partial owned fixture; preserve and inspect")
cls = unreal.load_class(None, "/Script/PrimalFrontier.PFCraftingStation")
if not cls:
    raise RuntimeError("Compile station class first")
guids = {}
if not found:
    for name, at, speed in SPECS:
        station = actors.spawn_actor_from_class(cls, unreal.Vector(*at))
        if not station.rename("PF_Camp_" + name):
            raise RuntimeError("Stable fixture name unavailable")
        station.set_actor_label(PREFIX + name)
        station.set_editor_property("station_type", unreal.PFCraftingStation.workbench_type() if name == "Workbench" else unreal.PFCraftingStation.refinery_type())
        station.set_editor_property("speed_multiplier", speed)
        station.set_editor_property("is_spatially_loaded", False)
        station.set_editor_property("tags", [unreal.Name("PF.M12.CampStationTest")])
    sign = actors.spawn_actor_from_class(unreal.TextRenderActor, unreal.Vector(350, -650, 160), unreal.Rotator(pitch=0, yaw=-90, roll=0))
    sign.set_actor_label(PREFIX + "Sign")
    sign.set_editor_property("is_spatially_loaded", False)
    text = sign.get_component_by_class(unreal.TextRenderComponent)
    text.set_text("CAMP CRAFTING STATIONS ->\nWorkbench 125% / Refinery 150% metal refining\nC: select recipe + Enter; R: cancel\nStay within 2.5m and clear sight until complete\nPortable recipes remain available away from stations")
    text.set_world_size(18)
    text.set_collision_profile_name("NoCollision")
if snapshot(actors.get_all_level_actors()) != before:
    raise RuntimeError("Existing actors changed before save")
for name, at, speed in SPECS:
    station = next(a for a in actors.get_all_level_actors() if a.get_name() == "PF_Camp_" + name)
    if xyz(station.get_actor_location()) != at or abs(station.get_editor_property("speed_multiplier") - speed) > .001:
        raise RuntimeError("Fixture differs; no overwrite")
    guids[name] = unreal.GuidLibrary.conv_guid_to_string(station.get_editor_property("actor_guid"))
if not found and not levels.save_current_level():
    raise RuntimeError("Map save failed")
# Loading the already-current map can retain its pre-save partition descriptor cache.
# Change to the unsaved engine Entry world first, then verify a genuinely loaded map.
if not levels.load_level("/Engine/Maps/Entry"):
    raise RuntimeError("Cannot leave map for reload verification")
rows = load()
if snapshot(rows) != before:
    raise RuntimeError("Reload changed existing actors")
for name, at, speed in SPECS:
    station = next(a for a in rows if a.get_name() == "PF_Camp_" + name)
    if unreal.GuidLibrary.conv_guid_to_string(station.get_editor_property("actor_guid")) != guids[name]:
        raise RuntimeError("Fixture identity changed on reload")
    if xyz(station.get_actor_location()) != at or abs(station.get_editor_property("speed_multiplier") - speed) > .001:
        raise RuntimeError("Fixture data changed on reload")
report.parent.mkdir(parents=True, exist_ok=True)
report.write_text(json.dumps(dict(map=MAP, old_actor_count=97, existing_actors_unchanged=True,
                                 new_actors=0 if found else 3, total_actors=len(rows), fixture=SPECS,
                                 guids=guids, reload_verified=True), indent=2), encoding="utf-8")
unreal.log("[PrimalAgentTools] Camp stations setup PASS; old97 preserved; saved fixtures verified")
