using UnrealBuildTool;

public class FabulousDualSense : ModuleRules
{
	public FabulousDualSense(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_3;

		bEnableNonInlinedGenCppWarnings = true;

		PrivateDependencyModuleNames.AddRange(new[]
		{
			"Core", "CoreUObject", "Engine", "ApplicationCore", "InputCore",
			"InputDevice", "SlateCore", "Slate", "DeveloperSettings", "DualSenseWindows"
		});
	}
}