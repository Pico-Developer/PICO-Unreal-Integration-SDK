// Copyright 2023 PICO Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class PICOOpenXRPassthrough : ModuleRules
{
    public PICOOpenXRPassthrough(ReadOnlyTargetRules Target) : base(Target)
    {
        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                    "Core",
                    "CoreUObject",
                    "Engine",
                    "OpenXRHMD",
                    "RHI",
                    "RenderCore",
                    "PICOOpenXRLoader",
                    "PICOOpenXRRuntimeSettings"
            }
            );
    }
}