// Copyright 2023 PICO Inc. All Rights Reserved.

using UnrealBuildTool;

public class PICOOpenXREditor : ModuleRules
{
    public PICOOpenXREditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PublicDependencyModuleNames.AddRange(
            new string[] {
                "Core",
                "CoreUObject"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[] {
                "EditorFramework",
                "UnrealEd",
                "PICOOpenXRRuntimeSettings",
                "OpenXRHMD",
                "PICOOpenXRInput",
                "Json",
                "SlateCore",
                "Slate",
                "Engine",
                "RenderCore",
                "Blutility",
                "UMG",
            }
        );
    }
}