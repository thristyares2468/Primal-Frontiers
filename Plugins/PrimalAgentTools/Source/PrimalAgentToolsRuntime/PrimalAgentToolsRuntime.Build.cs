using UnrealBuildTool;
public class PrimalAgentToolsRuntime : ModuleRules
{
    public PrimalAgentToolsRuntime(ReadOnlyTargetRules Target) : base(Target)
    {
        if (Target.Configuration == UnrealTargetConfiguration.Shipping)
            throw new BuildException("PrimalAgentToolsRuntime must never be built for Shipping.");
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine" });
        PrivateDependencyModuleNames.AddRange(new[] { "Json", "PrimalFrontier" });
    }
}
