// Copyright (C) Microsoft Corporation. All rights reserved.
// Modified by Rift Bound Games: portable Server staging and attribution. See PF_VENDOR_NOTES.md.

using UnrealBuildTool;

public class PlayFabGSDK : ModuleRules
{
	public PlayFabGSDK(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		RuntimeDependencies.Add("$(PluginDir)/License.txt", StagedFileType.NonUFS);
		RuntimeDependencies.Add("$(PluginDir)/PF_VENDOR_NOTES.md", StagedFileType.NonUFS);
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Core",
				"Json",
				"HTTP"
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
			// PF: NullRHI dedicated server has no D3DCompiler_43 dependency.
			// Do not stage a legacy DLL from a machine-specific Windows path.
	}
}
