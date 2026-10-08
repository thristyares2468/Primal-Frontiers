// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class PrimalFrontier : ModuleRules
{
	public PrimalFrontier(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// Engine modules used by the game. Why each non-template one is here:
		//   AIModule, NavigationSystem  - creature AIController + navmesh pathing (M6)
		//   UMG, Slate, SlateCore       - native placeholder HUD widgets (M1+)
		//   GameplayTags                - life/creature/world state and item categories
		//   StateTree*                  - used only by the template Variant_Shooter AI
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

		// Built-in bounded world-save metadata conversion (M8); no external plugin.
		PrivateDependencyModuleNames.AddRange(new string[] { "Json", "JsonUtilities" });

		// Variant_Horror / Variant_Shooter are Epic template samples, not part of the
		// survival game. They still compile into this module; removing them (code,
		// include paths and Content/Variant_*) is a safe future cleanup.
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
