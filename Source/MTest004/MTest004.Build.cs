// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MTest004 : ModuleRules
{
	public MTest004(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "GameplayTags" });
	}
}
