﻿using UnrealBuildTool;

public class FabulousDualSense : ModuleRules
{
	public FabulousDualSense(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_4;

		bEnableNonInlinedGenCppWarnings = true;
		// UnsafeTypeCastWarningLevel = WarningLevel.Warning;

		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core", "CoreUObject", "Engine", "DeveloperSettings"
		});

		PrivateDependencyModuleNames.AddRange(new[]
		{
			"ApplicationCore", "InputCore", "InputDevice", "SlateCore", "Slate", "DualSenseWindows"
		});
	}
}