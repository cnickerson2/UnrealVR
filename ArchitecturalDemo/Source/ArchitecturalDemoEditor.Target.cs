// Chris Nickerson -- TestingGrounds -- Unreal C++ Development

using UnrealBuildTool;
using System.Collections.Generic;

public class ArchitecturalDemoEditorTarget : TargetRules
{
	public ArchitecturalDemoEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;

		ExtraModuleNames.AddRange( new string[] { "ArchitecturalDemo" } );
	}
}
