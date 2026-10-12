"""M13: configure only the existing two open-world points; preserve actor IDs.

Use supported Editor asset/property/save APIs. Never touch regression maps,
private saves, actor positions, navigation or other assets. Safe to rerun.
"""
import json
import os
from pathlib import Path
import unreal

MAP = "/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld"
ASSET = "/Game/PrimalFrontier/Creatures/DA_OpenWorldSpawnPolicies"
levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)

def points():
    if not levels.load_level(MAP):
        raise RuntimeError("Open-world load failed")
    world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
    if world.get_path_name().split(".")[0] != MAP:
        raise RuntimeError("Wrong world")
    descs = unreal.WorldPartitionBlueprintLibrary.get_actor_descs()
    if isinstance(descs, tuple):
        ok, descs = descs
        if not ok:
            raise RuntimeError("Descriptor query failed")
    unreal.WorldPartitionBlueprintLibrary.load_actors([d.guid for d in descs])
    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
    found = [a for a in actors if a.get_class().get_name() == "PFCreatureSpawner"]
    clocks = [a for a in actors if a.get_class().get_name() == "PFWorldClock"]
    if len(found) != 2 or len(clocks) != 1 or clocks[0].get_editor_property("is_spatially_loaded"):
        raise RuntimeError("Expected exactly two resident points and one resident clock")
    expected = {"M7_Forager": ("Creature_Forager", [-1500, 1700, 60]),
                "M7_Prowler": ("Creature_Prowler", [1300, 4400, 60])}
    result = {}
    for a in found:
        label = a.get_actor_label()
        v = a.get_actor_location()
        if label not in expected or label in result:
            raise RuntimeError("Unexpected or duplicate point")
        creature, position = expected[label]
        if str(a.get_editor_property("creature_id")) != creature or [v.x, v.y, v.z] != position or a.get_editor_property("is_spatially_loaded"):
            raise RuntimeError("Placement/identity/lifetime differs from audited map")
        result[label] = a
    return result

found = points()
before = {label: a.get_path_name() for label, a in found.items()}
catalog = unreal.load_asset(ASSET) if unreal.EditorAssetLibrary.does_asset_exist(ASSET) else None
if catalog is None:
    factory = unreal.DataAssetFactory()
    factory.set_editor_property("data_asset_class", unreal.load_class(None, "/Script/PrimalFrontier.PFCreatureSpawnCatalog"))
    catalog = unreal.AssetToolsHelpers.get_asset_tools().create_asset("DA_OpenWorldSpawnPolicies", "/Game/PrimalFrontier/Creatures", None, factory)
if not catalog or catalog.get_class().get_name() != "PFCreatureSpawnCatalog":
    raise RuntimeError("Unexpected existing policy asset; do not overwrite")
policies = catalog.get_editor_property("policies")
if len(policies) != 3 or str(policies[0].entries[0].creature_id) != "Creature_Forager" or str(policies[2].entries[0].creature_id) != "Creature_Prowler":
    raise RuntimeError("Policy layout differs from validated native defaults")
biomes = {"M7_Forager": policies[0].biome, "M7_Prowler": policies[2].biome}
for label, a in found.items():
    old = a.get_editor_property("spawn_policies")
    if old and old != catalog:
        raise RuntimeError("Existing custom policy; refuse replacement")
if not unreal.EditorAssetLibrary.save_loaded_asset(catalog):
    raise RuntimeError("Policy asset save failed")
for label, a in found.items():
    a.set_editor_property("spawn_policies", catalog)
    a.set_editor_property("biome", biomes[label])
    a.set_editor_property("use_spawn_policy", True)
if not levels.save_current_level():
    raise RuntimeError("Point configuration save failed")
found = points()
for label, a in found.items():
    checks = dict(identity=a.get_path_name() == before[label], enabled=a.get_editor_property("use_spawn_policy"),
                  policy=a.get_editor_property("spawn_policies").get_path_name() == ASSET + ".DA_OpenWorldSpawnPolicies",
                  biome=unreal.GameplayTagLibrary.equal_equal_gameplay_tag(a.get_editor_property("biome"), biomes[label]))
    if not all(checks.values()):
        raise RuntimeError("Disk reload check failed for " + label + ": " + str(checks))
out = Path(os.environ["PF_ECOLOGY_REPORT"])
out.parent.mkdir(parents=True, exist_ok=True)
out.write_text(json.dumps(dict(map=MAP, policy=ASSET, points=before, reload_verified=True), indent=2), encoding="utf-8")
unreal.log("[PrimalAgentTools] Authored ecology configured and reloaded; existing two point identities/positions preserved")
