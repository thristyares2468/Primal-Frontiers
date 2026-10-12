"""Add known alternate IDs to the existing policy only; no map/point changes."""
import json
import os
from pathlib import Path
import unreal

ASSET = "/Game/PrimalFrontier/Creatures/DA_OpenWorldSpawnPolicies"
asset = unreal.load_asset(ASSET)
if not asset or asset.get_class().get_name() != "PFCreatureSpawnCatalog":
    raise RuntimeError("Expected existing authored policy asset")
policies = list(asset.get_editor_property("policies"))
if len(policies) != 3:
    raise RuntimeError("Unexpected policy domains")
expected = [(0, "Ecology.Biome.Shore", "Creature_Forager", "Creature_Scrabbler", 1, 1),
            (2, "Ecology.Biome.Ridge", "Creature_Prowler", "Creature_DuskPouncer", 0, 1)]
for index, biome, original, addition, day, night in expected:
    policy = policies[index]
    if str(unreal.GameplayTagLibrary.get_tag_name(policy.biome)) != biome:
        raise RuntimeError("Unexpected biome ordering")
    entries = list(policy.entries)
    if not entries or str(entries[0].creature_id) != original:
        raise RuntimeError("Original policy entry missing; do not overwrite")
    if any(str(e.creature_id) not in (original, addition) for e in entries):
        raise RuntimeError("Custom entries present; refuse replacement")
    present = [e for e in entries if str(e.creature_id) == addition]
    if len(present) > 1:
        raise RuntimeError("Duplicate alternate entry")
    if not present:
        entry = unreal.PFCreatureSpawnEntry()
        entry.set_editor_property("creature_id", addition)
        entry.set_editor_property("day_weight", day)
        entry.set_editor_property("night_weight", night)
        entries.append(entry)
    elif present[0].day_weight != day or present[0].night_weight != night:
        raise RuntimeError("Custom alternate weights present; preserve rather than overwrite")
    policy.set_editor_property("entries", entries)
asset.set_editor_property("policies", policies)
if not unreal.EditorAssetLibrary.save_loaded_asset(asset):
    raise RuntimeError("Policy save failed")
out = Path(os.environ["PF_ECOLOGY_REPORT"])
out.parent.mkdir(parents=True, exist_ok=True)
out.write_text(json.dumps(dict(asset=ASSET, points_changed=0, maps_changed=0,
                              shore=["Creature_Forager", "Creature_Scrabbler"],
                              ridge_night=["Creature_Prowler", "Creature_DuskPouncer"]), indent=2), encoding="utf-8")
unreal.log("[PrimalAgentTools] Weighted original variants added without spawn-point or save-layout changes")
