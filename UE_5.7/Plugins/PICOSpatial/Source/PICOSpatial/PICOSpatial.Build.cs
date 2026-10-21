// Copyright 2023 PICO Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class PICOSpatial : ModuleRules
{
    public PICOSpatial(ReadOnlyTargetRules Target) : base(Target)
    {
        PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private", "EntityProxy"));

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                    "Core",
                    "CoreUObject",
                    "Engine",
                    "Projects",
                    "RHI",
                    "RenderCore",
                    "UMG",
                    "ProceduralMeshComponent",
                    "GeometryCore",
                    "GeometryFramework",
                    "DynamicMesh",
                    "GeometryScriptingCore",
                    "SpatialSDKLoader",
                    "PICOSpatialRuntimeSettings",
                    "Niagara"
            }
            );

        // PICOSpatialUSD is an Editor-only module (see .uplugin). Do not pull it into Game/Android builds.
        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.Add("PICOSpatialUSD");
        }

        PublicIncludePathModuleNames.AddRange(
            new[]
            {
                "Launch",
            }
            );

        if (Target.Platform == UnrealTargetPlatform.Win64 || Target.Platform == UnrealTargetPlatform.Android
            || Target.IsInPlatformGroup(UnrealPlatformGroup.Linux))
        {
            PublicDependencyModuleNames.Add("VulkanRHI");

            if (!bUsePrecompiled || Target.LinkType == TargetLinkType.Monolithic)
            {
                PublicDependencyModuleNames.Add("Vulkan");
            }
        }
    }
}
