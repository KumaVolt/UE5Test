// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Outlaw : ModuleRules
{
	public Outlaw(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
	PublicDependencyModuleNames.AddRange(new string[] {
		"Core",
		"CoreUObject",
		"Engine",
		"InputCore",
		"EnhancedInput",
		"GameplayAbilities",
		"GameplayTags",
		"GameplayTasks",
		"UMG",
		"CommonUI",
		"CommonInput",
		"NetCore",
		"AIModule",
		"NavigationSystem",
		"Niagara",
		"AnimGraphRuntime",
	});

	PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore", "StateTreeModule", "GameplayStateTreeModule" });

		PublicIncludePaths.Add("Outlaw");

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
