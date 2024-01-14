using UnrealBuildTool;

public class FabulousDualSense : ModuleRules
{
	public FabulousDualSense(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_3;

		bEnableNonInlinedGenCppWarnings = true;

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