using UnrealBuildTool;

public class PrimalAgentTools : ModuleRules
{
    public PrimalAgentTools(ReadOnlyTargetRules Target) : base(Target)
    {
        if (Target.Type != TargetType.Editor || Target.Configuration == UnrealTargetConfiguration.Shipping)
        {
            throw new BuildException("PrimalAgentTools is restricted to development editor targets.");
        }

        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PrivateDependencyModuleNames.AddRange(new[]
        {
            "PrimalAgentToolsRuntime", "Core", "CoreUObject", "Engine", "UnrealEd", "AssetRegistry", "DataValidation",
            "Json", "ImageWrapper", "RenderCore", "RHI", "Slate", "SlateCore", "Projects"
        });
    }
}
