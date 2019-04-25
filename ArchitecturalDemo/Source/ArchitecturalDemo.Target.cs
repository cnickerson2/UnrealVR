// Chris Nickerson -- TestingGrounds -- Unreal C++ Development

using UnrealBuildTool;
using System.Collections.Generic;

public class ArchitecturalDemoTarget : TargetRules
{
	public ArchitecturalDemoTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;

		ExtraModuleNames.AddRange( new string[] { "ArchitecturalDemo" } );
	}
}
