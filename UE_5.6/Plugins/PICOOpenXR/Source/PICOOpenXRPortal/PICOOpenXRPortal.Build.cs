// Copyright® PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class PICOOpenXRPortal : ModuleRules
{
    public PICOOpenXRPortal(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        
        PrivateIncludePaths.Add("PICOOpenXRMR/Private");

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core", "Settings", "UnrealEd", "SettingsEditor", "PICOOpenXRHMD", "GameProjectGeneration"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "InputCore",
                "PropertyEditor",
                "Slate",
                "SlateCore",
                "Json",
                "UMG",
                "ToolWidgets",
                "UnrealEd", 
                "Blutility",
                "AndroidRuntimeSettings",
                "Projects",
                "Settings",
                "ToolMenus",
                "DeveloperSettings",
                "GameProjectGeneration",
                "PICOOpenXRRuntimeSettings",
                "Json",
                "JsonUtilities",
                "PICOOpenXRMR",
                "PICOOpenXRHMD",
                "PICOOpenXRInput",
                "PICOOpenXRMovement", "PICOOpenXRHandTracking","PICOOpenXRLoader",
            }
        );
        
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            string AppLogSdkDirectory = Path.Combine(ModuleDirectory, "AppLogSDK");
            PublicIncludePaths.Add(Path.Combine(AppLogSdkDirectory, "Include"));
            PublicAdditionalLibraries.Add(Path.Combine(AppLogSdkDirectory, "Lib", "applogrs.dll.lib"));
            PublicDelayLoadDLLs.Add("applogrs.dll");
            RuntimeDependencies.Add(Path.Combine(AppLogSdkDirectory, "Bin", "applogrs.dll"));
        }
    }
}
