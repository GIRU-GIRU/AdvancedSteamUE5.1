// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class AdvancedSteamUtils : ModuleRules
{
	public AdvancedSteamUtils(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"OnlineSubsystem",
			"OnlineSubsystemUtils",
			"OnlineSubsystemSteam",
			"Networking",
			"Sockets",
			"Voice",
			"Json",
			"JsonUtilities",
			"HTTP",
		});

		PrivateDependencyModuleNames.AddRange(new[]
		{
			"Sockets",
			"Networking",
			"Voice",
			"Steamworks",
		});


	}
}