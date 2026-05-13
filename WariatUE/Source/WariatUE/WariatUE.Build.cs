// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class WariatUE : ModuleRules
{
	public WariatUE(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"ChaosVehicles",
			"PhysicsCore",
			"UMG",
			"Slate",
			"RHI",
			"RenderCore"
		});

		PublicIncludePaths.AddRange(new string[] {
			"WariatUE",
			"WariatUE/SportsCar",
			"WariatUE/OffroadCar",
			"WariatUE/Variant_Offroad",
			"WariatUE/Variant_TimeTrial",
			"WariatUE/Variant_TimeTrial/UI",
			//Path.GetFullPath(Path.Combine(ModuleDirectory, "../../../WariatCommon"))
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });
		
		PrivateDefinitions.Add("WARIAT_CODE_UE5=1");

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
