// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class PICOXRMR : ModuleRules
{
    public PICOXRMR(ReadOnlyTargetRules Target) : base(Target)
    {
        PrivateIncludePathModuleNames.AddRange(
            new string[]
            {
                "InputDevice",
                "HeadMountedDisplay",
                "ImageWrapper",
                "Engine"
            });

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "InputCore",
                "Slate",
                "SlateCore",
                "RHI",
                "VulkanRHI",
                "RenderCore",
                "MediaAssets",
                "HeadMountedDisplay",
                "PICOXRHMD",
                "PICOXRInput",
                "PXRPlugin",
                "ProceduralMeshComponent",
                "MRMesh",
                "JsonUtilities",
                "Json"
            });

        PrivateIncludePaths.AddRange(
            new string[] {
                "PICOXRHMD/Private",
                "PICOXRInput/Private",
            });
    }
}

