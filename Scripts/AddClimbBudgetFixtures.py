"""Author stateless tall climbing fixtures beside camp; preserve all existing actors."""
import json
import os
from pathlib import Path
import unreal

MAP = "/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld"
PREFIX = "M14_ClimbBudget_"
SPECS = [("4_A", (1850, -1100, 200), (3, 3.6, 4)),
         ("4_B", (2250, -3000, 200), (3, 3.6, 4)),
         ("10_A", (2450, -1100, 500), (3, 3.6, 10))]
SIGN_AT = (1550, -850, 200)
SIGN = "STAMINA CLIMB: 4m / 10m\nW + Space or F: cover FULL climb cost\nRecover stamina; U: player upgrades"
levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
actors_api = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
def load():
    if not levels.load_level(MAP): raise RuntimeError("Cannot load expected map")
    ds = unreal.WorldPartitionBlueprintLibrary.get_actor_descs()
    if isinstance(ds, tuple):
        ok, ds = ds
        if not ok: raise RuntimeError("Cannot query descriptors")
    unreal.WorldPartitionBlueprintLibrary.load_actors([d.guid for d in ds])
    return actors_api.get_all_level_actors()
def xyz(v): return [v.x, v.y, v.z]
def identity(a): return unreal.GuidLibrary.conv_guid_to_string(a.get_editor_property("actor_guid"))
def snapshot(rows):
    out = []
    for a in rows:
        if a.get_actor_label().startswith(PREFIX): continue
        r = dict(id=identity(a),label=a.get_actor_label(),cls=a.get_class().get_name(),
                 at=xyz(a.get_actor_location()),scale=xyz(a.get_actor_scale3d()),
                 yaw=a.get_actor_rotation().yaw,spatial=a.get_editor_property("is_spatially_loaded"))
        if r['cls']=='PFResourceNode': r['resource']=str(a.get_editor_property('resource_id'))
        if r['cls']=='PFCreatureSpawner':
            r.update(creature=str(a.get_editor_property('creature_id')),
                     biome=str(unreal.GameplayTagLibrary.get_tag_name(a.get_editor_property('biome'))),
                     policy=a.get_editor_property('spawn_policies').get_path_name(),
                     enabled=a.get_editor_property('use_spawn_policy'))
        out.append(r)
    return sorted(out,key=lambda r:r['id'])
def verify(rows, allow_legacy=False):
    found={a.get_actor_label():a for a in rows if a.get_actor_label().startswith(PREFIX)}
    if len(found)!=4: raise RuntimeError("Expected exactly three tall walls and one sign")
    for name,at,scale in SPECS:
        a=found.get(PREFIX+name)
        if not a or a.get_class().get_name()!='StaticMeshActor': raise RuntimeError("Missing wall "+name)
        actual=xyz(a.get_actor_location())
        position_ok=actual==list(at) or (allow_legacy and name=='4_B' and actual==[1850,-3000,200])
        if not position_ok or any(abs(x-y)>.001 for x,y in zip(xyz(a.get_actor_scale3d()),scale)) or a.get_editor_property('is_spatially_loaded') or not a.actor_has_tag(unreal.Name('PF.M14.Climb'+name)) or str(a.static_mesh_component.get_collision_profile_name())!='BlockAll':
            raise RuntimeError("Custom wall configuration "+name)
    s=found.get(PREFIX+'Sign')
    if not s or xyz(s.get_actor_location())!=list(SIGN_AT) or str(s.get_component_by_class(unreal.TextRenderComponent).get_editor_property('text'))!=SIGN: raise RuntimeError("Unexpected sign")
    return {name:identity(a) for name,a in found.items()}
rows=load();before=snapshot(rows);mutations=0
if any(a.get_actor_label().startswith(PREFIX) for a in rows):
    ids=verify(rows,allow_legacy=True)
    wall=next(a for a in rows if a.get_actor_label()==PREFIX+'4_B')
    if xyz(wall.get_actor_location())==[1850,-3000,200]:
        wall.modify();wall.static_mesh_component.modify()
        wall.set_actor_location(unreal.Vector(2250,-3000,200),False,True);mutations+=1
        if verify(rows)!=ids or snapshot(rows)!=before: raise RuntimeError('Fixture relocation guard failed')
        if not levels.save_current_level(): raise RuntimeError('Relocation save failed')
else:
    cube=unreal.load_asset('/Engine/BasicShapes/Cube.Cube')
    rock=unreal.load_asset('/Game/PrimalFrontier/World/Greybox/MI_M7Rock')
    if not cube or not rock: raise RuntimeError("Missing existing primitive/material")
    for name,at,scale in SPECS:
        a=actors_api.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector(*at))
        a.set_actor_label(PREFIX+name);a.set_editor_property('is_spatially_loaded',False)
        a.set_editor_property('tags',[unreal.Name('PF.M14.Climb'+name)])
        a.static_mesh_component.set_static_mesh(cube);a.static_mesh_component.set_material(0,rock)
        a.static_mesh_component.set_collision_profile_name('BlockAll');a.set_actor_scale3d(unreal.Vector(*scale))
        mutations+=1
    s=actors_api.spawn_actor_from_class(unreal.TextRenderActor,unreal.Vector(*SIGN_AT),unreal.Rotator(pitch=0,yaw=180,roll=0))
    s.set_actor_label(PREFIX+'Sign');s.set_editor_property('is_spatially_loaded',False)
    t=s.get_component_by_class(unreal.TextRenderComponent);t.set_text(SIGN);t.set_world_size(24);mutations+=1
    ids=verify(actors_api.get_all_level_actors())
    if snapshot(actors_api.get_all_level_actors())!=before: raise RuntimeError("Old actor configuration changed")
    if not levels.save_current_level(): raise RuntimeError("Save failed")
rows=load()
if verify(rows)!=ids or snapshot(rows)!=before: raise RuntimeError("Reload changed identities or old configuration")
p=Path(os.environ['PF_TRAVERSAL_REPORT']);p.parent.mkdir(parents=True,exist_ok=True)
p.write_text(json.dumps(dict(map=MAP,mutations=mutations,old_actor_count=len(before),old_actors_unchanged=True,guids=ids,reload_verified=True),indent=2),encoding='utf-8')
unreal.log('[PrimalAgentTools] Climb-budget fixtures verified; mutations='+str(mutations))
