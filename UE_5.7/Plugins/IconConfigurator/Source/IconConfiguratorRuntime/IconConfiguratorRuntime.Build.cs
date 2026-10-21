using System.IO;
using UnrealBuildTool;

public class IconConfiguratorRuntime : ModuleRules
{
	public IconConfiguratorRuntime(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core" });
		PrivateDependencyModuleNames.AddRange(new string[] { "CoreUObject", "Engine" });

		// Mount the UPL for Android Manifest injection.
		// Only activate when the user has already run Apply in the editor (resources generated).
		if (Target.Platform == UnrealTargetPlatform.Android)
		{
			string PluginRoot = Path.GetFullPath(Path.Combine(ModuleDirectory, "..", ".."));
			string ProjectRoot = Path.GetFullPath(Path.Combine(PluginRoot, "..", ".."));
			string LauncherIcon = Path.Combine(ProjectRoot, "Build", "Android", "res",
				"mipmap-xxxhdpi", "ic_spatial_launcher.png");

			if (File.Exists(LauncherIcon))
			{
				string UplPath = Path.Combine(PluginRoot, "Resources", "IconConfigurator_UPL_Android.xml");
				AdditionalPropertiesForReceipt.Add("AndroidPlugin", UplPath);
			}
		}
	}
}
