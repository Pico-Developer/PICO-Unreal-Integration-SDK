// Copyright 2023 PICO Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class PICOOpenXRInput : ModuleRules
{
    public PICOOpenXRInput(ReadOnlyTargetRules Target) : base(Target)
    {
        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                    "Engine",
                    "HeadMountedDisplay",
                    "XRBase",
                    "InputCore",
                    "OpenXRHMD",
                    "PICOOpenXRHMD",
                    "Slate",
                    "SlateCore",
                    "PICOOpenXRLoader",
                    "PICOOpenXRRuntimeSettings"
            }
            );
    }
}
