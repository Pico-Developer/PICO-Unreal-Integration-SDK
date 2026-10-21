// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class PICOSpatialEditor : ModuleRules
{
    public PICOSpatialEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PrivateDependencyModuleNames.AddRange(
            new string[] {
                "Core",
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
				"ToolMenus",
                "EditorWidgets",
				"DeveloperToolSettings",
                "PICOSpatial",
                "PICOSpatialRuntimeSettings",
                "GLTFExporter",
				"AssetTools",
                "USDExporter",
                "PICOSpatialUSD",
                "Niagara",
                "NiagaraEditor",
                "Sequencer",
                "UnrealEd",
                "AssetRegistry",
                "EditorScriptingUtilities",
                "EngineSettings",
                "Json"
            }
        );
    }
}
