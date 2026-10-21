using UnrealBuildTool;

public class IconConfigurator : ModuleRules
{
	public IconConfigurator(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"CoreUObject",
			"Engine",
			"Slate",
			"SlateCore",
			"EditorStyle",
			"UnrealEd",
			"LevelEditor",
			"InputCore",
			"ToolMenus",
			"AssetTools",
			"AssetRegistry",
			"DesktopPlatform",
			"ImageWrapper",
			"RenderCore",
			"RHI",
			"WorkspaceMenuStructure",
			"HTTP",
			"Json",
			"JsonUtilities",
			"Projects",
		});
	}
}
