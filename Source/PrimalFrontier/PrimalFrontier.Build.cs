// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class PrimalFrontier : ModuleRules
{
	public PrimalFrontier(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"NavigationSystem",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
			"SlateCore",
			"GameplayTags"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"PrimalFrontier",
			"PrimalFrontier/Variant_Horror",
			"PrimalFrontier/Variant_Horror/UI",
			"PrimalFrontier/Variant_Shooter",
			"PrimalFrontier/Variant_Shooter/AI",
			"PrimalFrontier/Variant_Shooter/UI",
			"PrimalFrontier/Variant_Shooter/Weapons"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
