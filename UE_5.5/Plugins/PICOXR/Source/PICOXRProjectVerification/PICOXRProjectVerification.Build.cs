// Copyright® PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class PICOXRProjectVerification : ModuleRules
{
    public PICOXRProjectVerification(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core", "Settings", "UnrealEd", "SettingsEditor", "PICOXRHMD", "GameProjectGeneration"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "Json",
                "UMG",
                "ToolWidgets",
                "UnrealEd", "Blutility","AndroidRuntimeSettings","Projects","Settings","ToolMenus","DeveloperSettings"
            }
        );
    }
}