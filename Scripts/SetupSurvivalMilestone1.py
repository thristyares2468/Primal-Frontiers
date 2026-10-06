"""Run once in Unreal Editor Python. Creates project-owned M1 composition only.

Milestone 1 (f7ed11d). Creates:
  * /Game/PrimalFrontier/Survival/BP_Survivor, BP_SurvivalController and
    BP_SurvivalGameMode: copies of the First Person template Blueprints reparented
    onto the C++ classes APFSurvivorCharacter, APFSurvivalPlayerController and
    APFSurvivalGameMode. The GameMode Blueprint is pointed at the other two.
  * /Game/PrimalFrontier/Maps/L_M1Survival: a primitive test map (floor, jump step,
    look landmark, back wall, PlayerStart, light) using the new GameMode.

Template assets are never modified. Refuses to run if any destination exists.
Run from the editor: Tools > Execute Python Script, or `py Scripts/SetupSurvivalMilestone1.py`.
"""
import unreal

ROOT = "/Game/PrimalFrontier/Survival"
destinations = [ROOT + "/BP_Survivor", ROOT + "/BP_SurvivalController", ROOT + "/BP_SurvivalGameMode"]
MAP = "/Game/PrimalFrontier/Maps/L_M1Survival"
if any(unreal.EditorAssetLibrary.does_asset_exist(p) for p in destinations + [MAP]):
    raise RuntimeError("M1 content already exists. Refusing to overwrite assets; inspect before rerunning.")

def composition(source, destination, parent):
    blueprint = unreal.EditorAssetLibrary.duplicate_asset(source, destination)
    if not blueprint:
        raise RuntimeError("Could not duplicate " + source)
    unreal.BlueprintEditorLibrary.reparent_blueprint(blueprint, unreal.load_class(None, parent))
    unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
    return blueprint

pawn = composition("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter", destinations[0], "/Script/PrimalFrontier.PFSurvivorCharacter")
controller = composition("/Game/FirstPerson/Blueprints/BP_FirstPersonPlayerController", destinations[1], "/Script/PrimalFrontier.PFSurvivalPlayerController")
mode = composition("/Game/FirstPerson/Blueprints/BP_FirstPersonGameMode", destinations[2], "/Script/PrimalFrontier.PFSurvivalGameMode")
mode_default = unreal.get_default_object(mode.generated_class())
mode_default.set_editor_property("default_pawn_class", pawn.generated_class())
mode_default.set_editor_property("player_controller_class", controller.generated_class())
unreal.BlueprintEditorLibrary.compile_blueprint(mode)
for asset in [pawn, controller, mode]:
    if not unreal.EditorAssetLibrary.save_loaded_asset(asset, only_if_is_dirty=False):
        raise RuntimeError("Could not save " + asset.get_path_name())

levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
if not levels.new_level(MAP):
    raise RuntimeError("Could not create M1 map")
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
cube = unreal.load_asset("/Engine/BasicShapes/Cube")
def block(label, location, scale):
    actor = actors.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(*location))
    actor.set_actor_label(label)
    actor.static_mesh_component.set_static_mesh(cube)
    actor.set_actor_scale3d(unreal.Vector(*scale))
    return actor
block("M1_Floor", (0, 0, -25), (30, 30, 0.5))
block("M1_JumpStep", (200, 0, 20), (2, 3, 0.4))
block("M1_LookLandmark", (650, 400, 150), (2, 2, 3))
block("M1_BackWall", (1100, 0, 150), (0.5, 25, 3))
start = actors.spawn_actor_from_class(unreal.PlayerStart, unreal.Vector(-400, 0, 120))
start.set_actor_label("M1_PlayerStart")
light = actors.spawn_actor_from_class(unreal.DirectionalLight, unreal.Vector(0, 0, 500), unreal.Rotator(-45, -30, 0))
light.set_actor_label("M1_Light")
light.light_component.set_editor_property("intensity", 5.0)
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
world.get_world_settings().set_editor_property("default_game_mode", mode.generated_class())
if not levels.save_current_level():
    raise RuntimeError("Could not save M1 map")
unreal.log("[PrimalSurvival] Created M1 survivor/controller/GameMode and six-actor primitive map. Template assets untouched.")
