// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class PICOSpatialRuntimeSettings : ModuleRules
{
    public PICOSpatialRuntimeSettings(ReadOnlyTargetRules Target) : base(Target)
    {
        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
            }
        );
    }
}
