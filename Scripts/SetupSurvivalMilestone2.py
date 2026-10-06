"""Create an isolated M2 greybox map through supported Unreal Editor APIs.

Milestone 2 (8be2a14). Copies L_M1Survival to /Game/PrimalFrontier/Maps/L_M2Survival
and adds one APFSurvivalHazard exposure zone and three APFRecoveryPickup rations.
Refuses to overwrite an existing L_M2Survival; M1 assets stay unchanged.
"""
import unreal

MAP = "/Game/PrimalFrontier/Maps/L_M2Survival"
if unreal.EditorAssetLibrary.does_asset_exist(MAP):
    raise RuntimeError("M2 map exists; refusing to overwrite it.")
levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
if not levels.new_level_from_template(MAP, "/Game/PrimalFrontier/Maps/L_M1Survival"):
    raise RuntimeError("Could not create and load separate M2 map")
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
if world.get_path_name().split(".")[0] != MAP:
    raise RuntimeError("Active world is not M2; refusing fixture creation")
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
hazard_class = unreal.load_class(None, "/Script/PrimalFrontier.PFSurvivalHazard")
pickup_class = unreal.load_class(None, "/Script/PrimalFrontier.PFRecoveryPickup")
hazard = actors.spawn_actor_from_class(hazard_class, unreal.Vector(450, -550, 180))
hazard.set_actor_label("M2_ExposureZone")
for index, location in enumerate([(-240, 0, 150), (-240, 250, 150), (-240, -250, 150)]):
    pickup = actors.spawn_actor_from_class(pickup_class, unreal.Vector(*location))
    pickup.set_actor_label("M2_Ration_" + str(index + 1))
if not levels.save_current_level():
    raise RuntimeError("Could not save M2 fixtures")
unreal.log("[PrimalSurvival] Created separate M2 primitive map with exposure zone and three single-use rations. M1 assets unchanged.")
