"""One-time, scoped M7 extension. No imported art, template edits or actor removal."""
import unreal

MAP = "/Game/PrimalFrontier/Maps/L_M7SurvivalArena"
ROOT = "/Game/PrimalFrontier/World/Greybox"
levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
if not levels.load_level(MAP):
    raise RuntimeError("M7 map missing")
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
if world.get_path_name().split(".")[0] != MAP:
    raise RuntimeError("Unexpected world")
existing_actors = actors.get_all_level_actors()
if any(a.get_actor_label().startswith("M7V2_") for a in existing_actors):
    raise RuntimeError("M7 extension already exists; inspect instead of overwriting")
nodes = [a for a in existing_actors if a.get_class().get_name() == "PFResourceNode"]
navs = [a for a in existing_actors if a.get_actor_label() == "M7_Navigation"]
if len(nodes) != 9 or len(navs) != 1:
    raise RuntimeError("Unexpected original M7 fixture inventory")

# Append only the two new definitions, preserving all existing designer tuning.
for path, field, wanted in [
    ("/Game/PrimalFrontier/Items/DA_ItemCatalog", "items", {"Item_Fibre", "Item_Water"}),
    ("/Game/PrimalFrontier/Crafting/DA_CraftingCatalog", "resources", {"Node_Fibre", "Node_Water"}),
]:
    asset = unreal.load_asset(path)
    if not asset:
        raise RuntimeError("Catalog missing: " + path)
    entries = list(asset.get_editor_property(field))
    ids = {str(d.get_editor_property("id")) for d in entries}
    defaults = unreal.get_default_object(asset.get_class()).get_editor_property(field)
    for definition in defaults:
        key = str(definition.get_editor_property("id"))
        if key in wanted and key not in ids:
            entries.append(definition)
            ids.add(key)
    if not wanted.issubset(ids):
        raise RuntimeError("Rebuild C++ catalogs before this migration")
    asset.set_editor_property(field, entries)
    if not unreal.EditorAssetLibrary.save_loaded_asset(asset):
        raise RuntimeError("Catalog save failed")

# One simple opaque material, five colour instances, no textures or effects.
tools = unreal.AssetToolsHelpers.get_asset_tools()
edit = unreal.MaterialEditingLibrary
parent = unreal.load_asset(ROOT + "/M_M7Greybox") if unreal.EditorAssetLibrary.does_asset_exist(ROOT + "/M_M7Greybox") else None
if not parent:
    parent = tools.create_asset("M_M7Greybox", ROOT, unreal.Material, unreal.MaterialFactoryNew())
    colour = edit.create_material_expression(parent, unreal.MaterialExpressionVectorParameter)
    colour.set_editor_property("parameter_name", "Tint")
    colour.set_editor_property("default_value", unreal.LinearColor(0.3, 0.3, 0.3, 1))
    if not edit.connect_material_property(colour, "RGB", unreal.MaterialProperty.MP_BASE_COLOR):
        raise RuntimeError("Material connection failed")
    edit.recompile_material(parent)
    if not unreal.EditorAssetLibrary.save_loaded_asset(parent):
        raise RuntimeError("Material save failed")
materials = {}
for name, rgb in {"Sand": (0.5,0.36,0.18), "Grass": (0.07,0.2,0.055), "Rock": (0.2,0.23,0.27), "Water": (0.025,0.18,0.35), "Timber": (0.18,0.07,0.025)}.items():
    path = ROOT + "/MI_M7" + name
    instance = unreal.load_asset(path) if unreal.EditorAssetLibrary.does_asset_exist(path) else None
    if not instance:
        instance = tools.create_asset("MI_M7" + name, ROOT, unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())
        edit.set_material_instance_parent(instance, parent)
        # UE 5.8's setter always returns false despite applying the parameter.
        # Verify the observable value instead of trusting that return flag.
        edit.set_material_instance_vector_parameter_value(instance, "Tint", unreal.LinearColor(*rgb,1))
        actual = edit.get_material_instance_vector_parameter_value(instance, "Tint")
        if any(abs(a-b)>0.00001 for a,b in zip((actual.r,actual.g,actual.b),rgb)):
            raise RuntimeError("Material tint readback failed")
        if not unreal.EditorAssetLibrary.save_loaded_asset(instance):
            raise RuntimeError("Material instance save failed")
    materials[name] = instance

meshes = {name: unreal.load_asset("/Engine/BasicShapes/" + name) for name in ["Cube", "Cylinder", "Sphere"]}
def block(label, at, scale, tint, shape="Cube", collision=True):
    actor = actors.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(*at))
    actor.set_actor_label("M7V2_" + label)
    actor.static_mesh_component.set_static_mesh(meshes[shape])
    actor.static_mesh_component.set_material(0, materials[tint])
    actor.static_mesh_component.set_collision_profile_name("BlockAll" if collision else "NoCollision")
    actor.set_actor_scale3d(unreal.Vector(*scale))
    return actor

def sign(label, text, at):
    actor = actors.spawn_actor_from_class(unreal.TextRenderActor, unreal.Vector(*at), unreal.Rotator(pitch=0,yaw=-90,roll=0))
    actor.set_actor_label("M7V2_Sign_" + label)
    component = actor.get_component_by_class(unreal.TextRenderComponent)
    component.set_text(text)
    component.set_world_size(45)

# Flat overlays do not alter movement collision. The original bounded ground remains.
block("Beach", (0,-750,0.5), (59,14,0.01), "Sand", collision=False)
block("WaterEdge", (-2200,-1000,1.5), (12,8,0.01), "Water", collision=False)
block("Grassland", (-1500,1200,0.5), (25,22,0.01), "Grass", collision=False)
block("RockyGround", (1700,1700,0.5), (20,21,0.01), "Rock", collision=False)
block("FreshwaterPool", (1700,600,1.5), (5,5,0.01), "Water", collision=False)
for index, (x,y) in enumerate([(-2450,550),(-2050,1000),(-2500,1600),(-1100,500),(-1000,1350),(-2200,1900)]):
    block("TreeTrunk"+str(index),(x,y,180),(0.65,0.65,3.6),"Timber","Cylinder")
    block("TreeCrown"+str(index),(x,y,410),(2.7,2.7,2.7),"Grass","Sphere")
for index, (x,y,z,sx,sy,sz) in enumerate([(2200,1100,90,3,3,1.8),(1800,2100,130,5,3,2.6),(2400,2450,180,3,4,3.6)]):
    block("Boulder"+str(index),(x,y,z),(sx,sy,sz),"Rock")
# Wide roofed ruin entrance; no doorway narrower than 4 m.
for name,at,scale in [
    ("RuinWest",(-2450,4350,200),(1,9,4)),("RuinEast",(-1450,4350,200),(1,9,4)),
    ("RuinBack",(-1950,4800,200),(11,1,4)),("RuinRoof",(-1950,4350,430),(11,10,0.6)),
    ("RuinPillar",(-2550,3700,240),(1,1,4.8))]:
    block(name,at,scale,"Rock")
sign("Beach", "BEACH / SAFE START - freshwater west", (-1000,-1200,230))
sign("Woodland", "WOODLAND - wood / fibre / forage", (-2100,350,230))
sign("Rocks", "ROCKY AREA - stone / water", (1200,1000,230))
sign("Ruin", "RUIN - shelter / forage", (-2450,3850,230))
sign("Water", "WATER EDGE - collect at labelled source (no swimming)", (-2800,-600,160))

for label, rid, at in [
    ("BeachWater","Node_Water",(-1200,-400,60)),("PoolWater","Node_Water",(1450,600,60)),
    ("FibreA","Node_Fibre",(-1300,900,60)),("FibreB","Node_Fibre",(-1900,1400,60)),
    ("RockStone","Node_Stone",(1850,1500,60)),("RuinFood","Node_Food",(-1950,4450,60))]:
    node = actors.spawn_actor_from_class(unreal.load_class(None,"/Script/PrimalFrontier.PFResourceNode"),unreal.Vector(*at),unreal.Rotator(pitch=0,yaw=90,roll=0))
    node.set_actor_label("M7V2_" + label)
    node.set_editor_property("resource_id",rid)
    node.get_component_by_class(unreal.StaticMeshComponent).set_material(0,materials["Water" if rid=="Node_Water" else "Grass" if rid in ["Node_Fibre","Node_Food"] else "Rock"])

# Rotate only the label component, preserving the hazard's collision footprint.
for actor in existing_actors:
    if actor.get_actor_label()=="M7_Exposure":
        actor.get_component_by_class(unreal.TextRenderComponent).set_world_rotation(unreal.Rotator(pitch=0,yaw=-90,roll=0),False,False)
if not navs[0].build_navigation():
    raise RuntimeError("Extended arena navigation build failed")
if not levels.save_current_level():
    raise RuntimeError("Extended M7 map save failed")
unreal.log("[PrimalWorld] M7 extension saved: five traversal spaces, 15 resource nodes, unchanged two creature spawns and bounds; no imported art.")
