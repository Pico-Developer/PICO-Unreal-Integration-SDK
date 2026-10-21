// Copyright 2023 PICO Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class PICOOpenXRMovement : ModuleRules
{
    public PICOOpenXRMovement(ReadOnlyTargetRules Target) : base(Target)
    {
        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                    "Core",
                    "CoreUObject",
                    "Engine",
                    "LiveLinkAnimationCore",
                    "LiveLinkInterface",
                    "OpenXRHMD",
                    "PICOOpenXRLoader",
                    "PICOOpenXRRuntimeSettings",
                    "EyeTracker",
            }
            );
    }
}