// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CPPStudy : ModuleRules
{
	public CPPStudy(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"EnhancedInput",
			"Niagara",
			"AIModule",
			"NavigationSystem",
			"GameplayTasks",
			"UMG",
		});
	}
}
