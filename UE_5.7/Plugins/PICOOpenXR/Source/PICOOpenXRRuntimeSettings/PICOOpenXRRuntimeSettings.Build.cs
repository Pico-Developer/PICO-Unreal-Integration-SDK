// Copyright 2023 PICO Inc. All Rights Reserved.

using UnrealBuildTool;

public class PICOOpenXRRuntimeSettings : ModuleRules
{
    public PICOOpenXRRuntimeSettings(ReadOnlyTargetRules Target) : base(Target)
    {
        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine"
            }
        );
    }
}
