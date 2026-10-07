"""Create empty project-owned audio routing assets; imports no audio or art."""
import unreal

root = "/Game/PrimalFrontier/Audio"
tools = unreal.AssetToolsHelpers.get_asset_tools()
for name, cls, factory in [
    ("SC_Music", unreal.SoundClass, unreal.SoundClassFactory),
    ("SC_Effects", unreal.SoundClass, unreal.SoundClassFactory),
    ("SC_UI", unreal.SoundClass, unreal.SoundClassFactory),
    ("SMX_Preferences", unreal.SoundMix, unreal.SoundMixFactory),
]:
    path = root + "/" + name
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        asset = unreal.load_asset(path)
        if not isinstance(asset, cls):
            raise RuntimeError("Unexpected existing asset: " + path)
        continue
    asset = tools.create_asset(name, root, cls, factory())
    if not asset or not unreal.EditorAssetLibrary.save_loaded_asset(asset):
        raise RuntimeError("Audio routing creation failed: " + path)
unreal.log("[PrimalSettings] Audio routing assets verified. No external audio imported.")
