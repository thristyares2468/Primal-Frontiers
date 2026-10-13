"""Read saved resource identities/catalog entries for a future reviewed layout edge.
No asset/property/map save, no private persistence files or gameplay mutation.
"""
import json
import os
from pathlib import Path
import unreal

MAP = "/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld"
report = Path(os.environ["PF_LAYOUT_AUDIT_REPORT"]).resolve()
root = (Path(unreal.Paths.project_saved_dir()) / "AutomationReports").resolve()
if report.parent != root or report.exists():
    raise RuntimeError("Require a unique immediate project-local automation report directory")
levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
actors_api = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
if not levels.load_level(MAP):
    raise RuntimeError("Expected saved open-world map unavailable")
descs = unreal.WorldPartitionBlueprintLibrary.get_actor_descs()
if isinstance(descs, tuple):
    ok, descs = descs
    if not ok:
        raise RuntimeError("Actor descriptors unavailable")
unreal.WorldPartitionBlueprintLibrary.load_actors([d.guid for d in descs])
rows = actors_api.get_all_level_actors()

def xyz(v):
    return [v.x, v.y, v.z]

def actor_row(a):
    return dict(name=a.get_name(), label=a.get_actor_label(), cls=a.get_class().get_name(),
                guid=unreal.GuidLibrary.conv_guid_to_string(a.get_editor_property("actor_guid")),
                location=xyz(a.get_actor_location()), scale=xyz(a.get_actor_scale3d()),
                spatial=a.get_editor_property("is_spatially_loaded"))

resources, starts, spawners = [], [], []
for actor in rows:
    cls = actor.get_class().get_name()
    if cls == "PFResourceNode":
        row = actor_row(actor)
        row["definition"] = str(actor.get_editor_property("resource_id"))
        resources.append(row)
    elif cls == "PFCreatureSpawner":
        spawners.append(actor_row(actor))
    elif cls == "PlayerStart":
        starts.append(actor_row(actor))
if not resources or len(resources) > 128 or len(starts) != 2:
    raise RuntimeError("Unexpected baseline resource/start count")
if len({r["name"] for r in resources}) != len(resources):
    raise RuntimeError("Duplicate persistent resource name")
catalogs = {}
for kind, path, property_name in [
    ("items", "/Game/PrimalFrontier/Items/DA_ItemCatalog", "items"),
    ("recipes", "/Game/PrimalFrontier/Crafting/DA_CraftingCatalog", "recipes"),
    ("resources", "/Game/PrimalFrontier/Crafting/DA_CraftingCatalog", "resources"),
]:
    asset = unreal.load_asset(path)
    if not asset:
        raise RuntimeError("Missing expected project catalog")
    entries = []
    for entry in asset.get_editor_property(property_name):
        row = dict(id=str(entry.get_editor_property("id")))
        if kind == "items":
            families = entry.get_editor_property("harvest_families")
            row.update(hits=entry.get_editor_property("gathering_hits"),
                       grade=str(entry.get_editor_property("harvest_grade")),
                       families=sorted(str(unreal.GameplayTagLibrary.get_tag_name(tag))
                                       for tag in families.get_editor_property("gameplay_tags")))
        elif kind == "recipes":
            row.update(output=str(entry.get_editor_property("output")),
                       duration=entry.get_editor_property("duration"))
        else:
            requirement = entry.get_editor_property("harvest_requirement")
            row.update(yield_item=str(entry.get_editor_property("yield_item")),
                       requirement=dict(family=str(unreal.GameplayTagLibrary.get_tag_name(requirement.get_editor_property("family"))),
                                        minimum_grade=str(requirement.get_editor_property("minimum_grade"))))
        entries.append(row)
    if len({r["id"] for r in entries}) != len(entries):
        raise RuntimeError("Duplicate catalog ID")
    catalogs[kind] = entries
result = dict(map=MAP, read_only=True, actor_count=len(rows), descriptor_count=len(descs),
              resources=sorted(resources, key=lambda r: r["name"]), starts=starts,
              spawners=spawners, catalogs=catalogs,
              limit="Authored layout audit only; no migration/new ore or runtime/playtest proof")
result["candidate_only"] = dict(location=[0, -850, 60], created=False,
                               reason="Between camp starts and existing Stone node; runtime clearance and authoring still required")
report.mkdir()
(report / "layout.json").write_text(json.dumps(result, indent=2), encoding="utf-8")
unreal.log("[PrimalAgentTools] Harvest layout read-only audit PASS resources=%d starts=%d actors=%d" %
           (len(resources), len(starts), len(rows)))
