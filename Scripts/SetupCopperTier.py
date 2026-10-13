"""Guarded append-only Copper catalogs + one marked nonblocking near-camp fixture."""
import json
import os
from pathlib import Path
import unreal

MAP = "/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld"
PREFIX = "M12_Copper_"
AT = [0, -850, 60]
levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
actors_api = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
report = Path(os.environ["PF_COPPER_SETUP_REPORT"])
if report.exists():
    raise RuntimeError("Refusing reused setup report")

def tags(container):
    return sorted(str(unreal.GameplayTagLibrary.get_tag_name(t)) for t in container.get_editor_property("gameplay_tags"))

def item_state(entry):
    fields = ["display_name", "stack_limit", "weight", "shelf_life_seconds", "food_recovery",
              "water_recovery", "gathering_hits", "melee_damage", "creature_hit_reduction"]
    row = {field: str(entry.get_editor_property(field)) for field in fields}
    row["category"] = str(unreal.GameplayTagLibrary.get_tag_name(entry.get_editor_property("category")))
    return row

def recipe_state(entry):
    return dict(name=str(entry.get_editor_property("display_name")), output=str(entry.get_editor_property("output")),
                quantity=entry.get_editor_property("output_quantity"), duration=entry.get_editor_property("duration"),
                category=str(unreal.GameplayTagLibrary.get_tag_name(entry.get_editor_property("category"))),
                ingredients=[(str(i.get_editor_property("item_id")), i.get_editor_property("quantity"))
                             for i in entry.get_editor_property("ingredients")])

def xyz(v):
    return [v.x, v.y, v.z]

def load_rows():
    if not levels.load_level(MAP):
        raise RuntimeError("Expected map missing")
    descriptors = unreal.WorldPartitionBlueprintLibrary.get_actor_descs()
    if isinstance(descriptors, tuple):
        ok, descriptors = descriptors
        if not ok:
            raise RuntimeError("Cannot read actor descriptors")
    unreal.WorldPartitionBlueprintLibrary.load_actors([d.guid for d in descriptors])
    return actors_api.get_all_level_actors()

def old_snapshot(rows):
    out = []
    for a in rows:
        if a.get_actor_label().startswith(PREFIX):
            continue
        rotation = a.get_actor_rotation()
        row = dict(name=a.get_name(), guid=unreal.GuidLibrary.conv_guid_to_string(a.get_editor_property("actor_guid")), cls=a.get_class().get_name(),
                   label=a.get_actor_label(), at=xyz(a.get_actor_location()), scale=xyz(a.get_actor_scale3d()),
                   rotation=[rotation.pitch, rotation.yaw, rotation.roll], spatial=a.get_editor_property("is_spatially_loaded"))
        if row["cls"] == "PFResourceNode":
            row["definition"] = str(a.get_editor_property("resource_id"))
        if row["cls"] == "PFCreatureSpawner":
            row.update(creature=str(a.get_editor_property("creature_id")), biome=str(unreal.GameplayTagLibrary.get_tag_name(a.get_editor_property("biome"))),
                       policy=a.get_editor_property("spawn_policies").get_path_name(), enabled=a.get_editor_property("use_spawn_policy"))
        out.append(row)
    return sorted(out, key=lambda r: r["name"])

rows = load_rows()
before = old_snapshot(rows)
if len(before) != 85:
    raise RuntimeError("Unexpected baseline actor count; no edits")
items = unreal.load_asset("/Game/PrimalFrontier/Items/DA_ItemCatalog")
craft = unreal.load_asset("/Game/PrimalFrontier/Crafting/DA_CraftingCatalog")
if not items or not craft:
    raise RuntimeError("Missing catalogs; no edits")
native_items = list(unreal.get_default_object(items.get_class()).get_editor_property("items"))
native_recipes = list(unreal.get_default_object(craft.get_class()).get_editor_property("recipes"))
native_resources = list(unreal.get_default_object(craft.get_class()).get_editor_property("resources"))
if (len(native_items), len(native_recipes), len(native_resources)) != (17, 11, 6):
    raise RuntimeError("Compile the expected Copper definitions before authoring")
old_items = list(items.get_editor_property("items"))
old_recipes = list(craft.get_editor_property("recipes"))
old_resources = list(craft.get_editor_property("resources"))
for entries in [old_items, old_recipes, old_resources]:
    ids = [str(e.get_editor_property("id")) for e in entries]
    if len(ids) != len(set(ids)):
        raise RuntimeError("Duplicate saved ID; no edit")
item_before = {str(e.get_editor_property("id")): item_state(e) for e in old_items}
recipe_before = {str(e.get_editor_property("id")): recipe_state(e) for e in old_recipes}
item_changed = False
for native in native_items:
    identifier = str(native.get_editor_property("id"))
    existing = next((e for e in old_items if str(e.get_editor_property("id")) == identifier), None)
    if existing is None:
        old_items.append(native)
        item_changed = True
    elif identifier in ("Item_Tool", "Item_BoundTool"):
        current_families = tags(existing.get_editor_property("harvest_families"))
        current_grade = existing.get_editor_property("harvest_grade")
        desired_families = tags(native.get_editor_property("harvest_families"))
        desired_grade = native.get_editor_property("harvest_grade")
        if current_families == desired_families and current_grade == desired_grade:
            continue
        if current_families or current_grade != unreal.PFHarvestGrade.NONE:
            raise RuntimeError("Unreviewed custom tool capability; preserve it")
        existing.set_editor_property("harvest_families", native.get_editor_property("harvest_families"))
        existing.set_editor_property("harvest_grade", desired_grade)
        item_changed = True
for identifier, state in item_before.items():
    if item_state(next(e for e in old_items if str(e.get_editor_property("id")) == identifier)) != state:
        raise RuntimeError("Existing item tuning changed")
craft_changed = False
for prop, saved, native in [("recipes", old_recipes, native_recipes), ("resources", old_resources, native_resources)]:
    ids = [str(e.get_editor_property("id")) for e in saved]
    for entry in native:
        if str(entry.get_editor_property("id")) not in ids:
            saved.append(entry)
            craft_changed = True
for identifier, state in recipe_before.items():
    if recipe_state(next(e for e in old_recipes if str(e.get_editor_property("id")) == identifier)) != state:
        raise RuntimeError("Existing recipe tuning changed")

found = [a for a in rows if a.get_actor_label().startswith(PREFIX)]
if found and len(found) != 2:
    raise RuntimeError("Partial Copper fixture; preserve and inspect before authoring")
mutations = 0
if not found:
    node_class = unreal.load_class(None, "/Script/PrimalFrontier.PFResourceNode")
    material = unreal.load_asset("/Game/PrimalFrontier/World/Greybox/MI_M7Rock")
    if not node_class or not material:
        raise RuntimeError("Required existing class/material missing")
    node = actors_api.spawn_actor_from_class(node_class, unreal.Vector(*AT))
    node.set_actor_label(PREFIX + "Vein")
    node.set_editor_property("resource_id", unreal.Name("Node_Copper"))
    node.set_editor_property("is_spatially_loaded", False)
    node.set_editor_property("tags", [unreal.Name("PF.M12.CopperTest")])
    mesh = node.get_component_by_class(unreal.StaticMeshComponent)
    mesh.set_material(0, material)
    node.set_actor_scale3d(unreal.Vector(.65, .65, .65))
    sign = actors_api.spawn_actor_from_class(unreal.TextRenderActor, unreal.Vector(-230, -910, 150), unreal.Rotator(pitch=0, yaw=-90, roll=0))
    sign.set_actor_label(PREFIX + "Sign")
    sign.set_editor_property("is_spatially_loaded", False)
    text = sign.get_component_by_class(unreal.TextRenderComponent)
    text.set_text("MINING TEST: Wood > Stone > Copper\nE: Stone tool required for Copper\nC: refine ore + wood, then craft pick\nFieldTools unlock; U stats are separate")
    text.set_world_size(22)
    text.set_collision_profile_name("NoCollision")
    mutations = 2
else:
    node = next(a for a in found if a.get_actor_label() == PREFIX + "Vein")
    if node.get_class().get_name() != "PFResourceNode" or xyz(node.get_actor_location()) != AT or str(node.get_editor_property("resource_id")) != "Node_Copper":
        raise RuntimeError("Existing fixture differs; preserve it")
if old_snapshot(actors_api.get_all_level_actors()) != before:
    raise RuntimeError("Existing actor identity/configuration changed")
if item_changed:
    items.set_editor_property("items", old_items)
if craft_changed:
    craft.set_editor_property("recipes", old_recipes)
    craft.set_editor_property("resources", old_resources)
if item_changed and not unreal.EditorAssetLibrary.save_loaded_asset(items):
    raise RuntimeError("Item catalog save failed")
if craft_changed and not unreal.EditorAssetLibrary.save_loaded_asset(craft):
    raise RuntimeError("Crafting catalog save failed")
if mutations and not levels.save_current_level():
    raise RuntimeError("Fixture save failed")
guid = unreal.GuidLibrary.conv_guid_to_string(node.get_editor_property("actor_guid"))
node_name = node.get_name()
rows = load_rows()
if old_snapshot(rows) != before:
    raise RuntimeError("Saved reload changed old actors")
loaded = next(a for a in rows if a.get_actor_label() == PREFIX + "Vein")
if unreal.GuidLibrary.conv_guid_to_string(loaded.get_editor_property("actor_guid")) != guid or loaded.get_name() != node_name:
    raise RuntimeError("Saved fixture identity changed")
report.parent.mkdir(parents=True, exist_ok=True)
report.write_text(json.dumps(dict(map=MAP, mutations=mutations, catalogs_changed=[item_changed, craft_changed], old_actors_unchanged=True,
                                  old_actor_count=len(before), copper_name=node_name, copper_guid=guid, location=AT,
                                  item_count=len(old_items), recipe_count=len(old_recipes), resource_count=len(old_resources),
                                  reload_verified=True), indent=2), encoding="utf-8")
unreal.log("[PrimalAgentTools] Copper fixture/catalogs PASS mutations=%d node=%s" % (mutations, node_name))
