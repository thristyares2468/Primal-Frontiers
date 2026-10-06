// PrimalFrontierServer.Target.cs: dedicated server target named in AGENTS.md (PrimalFrontierServer).
// Same module set as the game target, without client-only rendering/audio/UI. Building a
// Server target needs a source-built engine; the Epic Games Launcher install of UE 5.8
// can still open the editor and build the Game/Editor targets with this file present.
// Live multiplayer tests can keep using "UnrealEditor-Cmd.exe ... -server" until then.

using UnrealBuildTool;
using System.Collections.Generic;

public class PrimalFrontierServerTarget : TargetRules
{
	public PrimalFrontierServerTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Server;
		DefaultBuildSettings = BuildSettingsVersion.V7;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
		ExtraModuleNames.Add("PrimalFrontier");
	}
}
