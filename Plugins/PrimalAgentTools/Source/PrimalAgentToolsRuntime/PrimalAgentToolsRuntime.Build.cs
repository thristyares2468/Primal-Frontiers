// PrimalAgentToolsRuntime: the PF.* developer command layer (PFCommands.cpp) and
// result/report types. Loaded in editor, game and server processes so commands work
// in live multiplayer tests, but never built for Shipping. Depends on the game module
// (PrimalFrontier) to call its server-authoritative APIs, and on NavigationSystem for
// the creature/world live tests' navmesh path checks.
using UnrealBuildTool;
public class PrimalAgentToolsRuntime : ModuleRules
{
    public PrimalAgentToolsRuntime(ReadOnlyTargetRules Target) : base(Target)
    {
        if (Target.Configuration == UnrealTargetConfiguration.Shipping)
            throw new BuildException("PrimalAgentToolsRuntime must never be built for Shipping.");
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine" });
        // InputCore is required by development-only live tests of registered keys.
        PrivateDependencyModuleNames.AddRange(new[] { "Json", "PrimalFrontier", "NavigationSystem", "InputCore" });
        // Cooked development servers run native automation too. The controller
        // reads this engine-supplied HTML shell as a loose file when exporting.
        if (Target.Type == TargetType.Server)
            RuntimeDependencies.Add("$(EngineDir)/Content/Automation/Report-Template.html", StagedFileType.NonUFS);
    }
}
