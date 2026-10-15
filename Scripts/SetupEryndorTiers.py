"""Append-only saved catalogs and an exact nine-node, nonblocking home-world test row."""
import json
import os
from pathlib import Path
import unreal

MAP = "/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld"
PREFIX = "M12_Eryndor_"
MINERALS = ["Coal", "Flint", "Iron", "Diamond", "Gold", "Silver", "Quartz", "Obsidian", "Titanium"]
report = Path(os.environ["PF_ERYNDOR_SETUP_REPORT"])
if report.exists():
    raise RuntimeError("Refusing reused report")
levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)


def load():
    if not levels.load_level(MAP):
        raise RuntimeError("Expected saved level unavailable")
    descs = unreal.WorldPartitionBlueprintLibrary.get_actor_descs()
    if isinstance(descs, tuple):
        ok, descs = descs
        if not ok:
            raise RuntimeError("Cannot read partition descriptors")
    unreal.WorldPartitionBlueprintLibrary.load_actors([d.guid for d in descs])
    return actors.get_all_level_actors()


def xyz(v):
    return [v.x, v.y, v.z]


def snapshot(rows):
    out = []
    for a in rows:
        if a.get_actor_label().startswith(PREFIX):
            continue
        rot = a.get_actor_rotation()
        d = dict(name=a.get_name(), guid=unreal.GuidLibrary.conv_guid_to_string(a.get_editor_property("actor_guid")),
                 cls=a.get_class().get_name(), label=a.get_actor_label(), at=xyz(a.get_actor_location()),
                 scale=xyz(a.get_actor_scale3d()), rot=[rot.pitch, rot.yaw, rot.roll],
                 spatial=a.get_editor_property("is_spatially_loaded"), tags=[str(t) for t in a.get_editor_property("tags")])
        if d["cls"] == "PFResourceNode":
            d["definition"] = str(a.get_editor_property("resource_id"))
        if d["cls"] == "PFCreatureSpawner":
            d["definition"] = str(a.get_editor_property("creature_id"))
            d["policy"] = a.get_editor_property("spawn_policies").get_path_name()
        out.append(d)
    return sorted(out, key=lambda x: x["name"])


rows = load()
before = snapshot(rows)
if len(before) != 87:
    raise RuntimeError("Unexpected existing actors; no edits")
items = unreal.load_asset("/Game/PrimalFrontier/Items/DA_ItemCatalog")
craft = unreal.load_asset("/Game/PrimalFrontier/Crafting/DA_CraftingCatalog")
if not items or not craft:
    raise RuntimeError("Missing saved catalogs")
catalogs = [(items, "items", 36), (craft, "recipes", 21), (craft, "resources", 15)]
prepared = []
for asset, prop, count in catalogs:
    saved = list(asset.get_editor_property(prop))
    native = list(unreal.get_default_object(asset.get_class()).get_editor_property(prop))
    ids = [str(x.get_editor_property("id")) for x in saved]
    if len(native) != count or len(ids) != len(set(ids)):
        raise RuntimeError("Compile expected definitions / duplicate IDs")
    # Keep complete existing structs, including custom tuning/icons, intact.
    additions = [e for e in native if str(e.get_editor_property("id")) not in ids]
    prepared.append((asset, prop, saved + additions, len(additions)))

found = [a for a in rows if a.get_actor_label().startswith(PREFIX)]
if found and len(found) != 10:
    raise RuntimeError("Partial fixture; preserve and inspect")
fixture = []
if not found:
    cls = unreal.load_class(None, "/Script/PrimalFrontier.PFResourceNode")
    material = unreal.load_asset("/Game/PrimalFrontier/World/Greybox/MI_M7Rock")
    if not cls or not material:
        raise RuntimeError("Existing node/material unavailable")
    for index, mineral in enumerate(MINERALS):
        # 4m spacing, west of camp: unobstructed spawn and normal northward route.
        at = [-2600, -1200 + index * 400, 60]
        node = actors.spawn_actor_from_class(cls, unreal.Vector(*at))
        if not node.rename("PF_Eryndor_" + mineral):
            raise RuntimeError("Cannot assign reviewed stable identity")
        node.set_actor_label(PREFIX + mineral)
        node.set_editor_property("resource_id", unreal.Name("Node_" + mineral))
        node.set_editor_property("tags", [unreal.Name("PF.M12.EryndorTest")])
        node.set_editor_property("is_spatially_loaded", False)
        node.set_actor_scale3d(unreal.Vector(.65, .65, .65))
        node.get_component_by_class(unreal.StaticMeshComponent).set_material(0, material)
        fixture.append(dict(name=node.get_name(), mineral=mineral, at=at, guid=unreal.GuidLibrary.conv_guid_to_string(node.get_editor_property("actor_guid"))))
    sign = actors.spawn_actor_from_class(unreal.TextRenderActor, unreal.Vector(-2400, -1450, 180), unreal.Rotator(pitch=0, yaw=-90, roll=0))
    sign.set_actor_label(PREFIX + "Sign")
    sign.set_editor_property("is_spatially_loaded", False)
    text = sign.get_component_by_class(unreal.TextRenderComponent)
    text.set_text("ERYNDOR MINING TEST - WEST OF CAMP\nSouth to north: Coal / Flint / Iron / Diamond / Gold / Silver / Quartz / Obsidian / Titanium\nE gather | C craft + learn | K unlock | P pause\nStone > Iron ore; Iron > Diamond; Diamond > Titanium\nCosts and levels in crafting. Portable greybox refining.\nNo free tools/XP. Advanced dimensional tiers not present.")
    text.set_world_size(20)
    text.set_collision_profile_name("NoCollision")
else:
    for mineral in MINERALS:
        node = next(a for a in found if a.get_actor_label() == PREFIX + mineral)
        if node.get_name() != "PF_Eryndor_" + mineral or str(node.get_editor_property("resource_id")) != "Node_" + mineral:
            raise RuntimeError("Existing fixture differs; preserve it")
        fixture.append(dict(name=node.get_name(), mineral=mineral, at=xyz(node.get_actor_location()), guid=unreal.GuidLibrary.conv_guid_to_string(node.get_editor_property("actor_guid"))))

if snapshot(actors.get_all_level_actors()) != before:
    raise RuntimeError("Old actor snapshot changed")
for asset, prop, entries, additions in prepared:
    if additions:
        asset.set_editor_property(prop, entries)
for asset in [items, craft]:
    if any(a == asset and additions for a, _, _, additions in prepared) and not unreal.EditorAssetLibrary.save_loaded_asset(asset):
        raise RuntimeError("Catalog save failed")
if not found and not levels.save_current_level():
    raise RuntimeError("Level save failed")
reloaded = load()
after = snapshot(reloaded)
if after != before:
    report.parent.mkdir(parents=True, exist_ok=True)
    report.write_text(json.dumps(dict(before=before, after=after, passed=False), indent=2), encoding="utf-8")
    raise RuntimeError("Reload changed old actors")
for d in fixture:
    node = next(a for a in reloaded if a.get_name() == d["name"])
    if unreal.GuidLibrary.conv_guid_to_string(node.get_editor_property("actor_guid")) != d["guid"]:
        raise RuntimeError("Reload changed fixture identity")
for asset, prop, entries, _ in prepared:
    loaded = unreal.load_asset(asset.get_path_name())
    if len(loaded.get_editor_property(prop)) != len(entries):
        raise RuntimeError("Saved catalog count mismatch")
report.parent.mkdir(parents=True, exist_ok=True)
report.write_text(json.dumps(dict(map=MAP, old_actor_count=len(before), old_actors_unchanged=True,
                                  new_actors=0 if found else 10, fixture=fixture,
                                  additions=[n for _, _, _, n in prepared], reload_verified=True), indent=2), encoding="utf-8")
unreal.log("[PrimalAgentTools] Eryndor setup PASS; old87 preserved; fixture additions=" + str(0 if found else 10))
