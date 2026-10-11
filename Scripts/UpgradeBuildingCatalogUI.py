"""Idempotent supported Editor edit of the project-owned building catalog only.
Keeps existing IDs/kinds/tuning and appends missing native greybox choices.
"""
import unreal

PATH = "/Game/PrimalFrontier/Building/DA_BuildingCatalog"
catalog = unreal.load_asset(PATH)
expected = unreal.load_class(None, "/Script/PrimalFrontier.PFBuildingCatalog")
if not catalog or not expected or catalog.get_class() != expected:
    raise RuntimeError("Unexpected/missing project building catalog; no edit")
pieces = list(catalog.get_editor_property("pieces"))
ids = [str(p.get_editor_property("id")) for p in pieces]
if len(ids) != len(set(ids)):
    raise RuntimeError("Duplicate existing building IDs; no edit")
defaults = list(unreal.get_default_object(expected).get_editor_property("pieces"))
if len(defaults) != 8:
    raise RuntimeError("Expected newly compiled eight-piece native catalog; no edit")
unreal.log("[PrimalBuilding] Catalog before: " + ", ".join(ids))
changed = False
for definition in defaults:
    identifier = str(definition.get_editor_property("id"))
    if identifier not in ids:
        pieces.append(definition)
        ids.append(identifier)
        changed = True
    elif identifier in ("Build_Ceiling", "Build_Storage"):
        existing = pieces[ids.index(identifier)]
        if str(existing.get_editor_property("name")) in ("Ceiling", "Storage"):
            existing.set_editor_property("name", definition.get_editor_property("name"))
            changed = True
if changed:
    catalog.set_editor_property("pieces", pieces)
    if not unreal.EditorAssetLibrary.save_loaded_asset(catalog):
        raise RuntimeError("Building catalog save failed")
readback = unreal.load_asset(PATH).get_editor_property("pieces")
if not all(str(d.get_editor_property("id")) in [str(p.get_editor_property("id")) for p in readback] for d in defaults):
    raise RuntimeError("Building choices missing after save")
unreal.log("[PrimalBuilding] Catalog upgrade PASS: " + ", ".join(ids) + "; changed=" + str(changed))
