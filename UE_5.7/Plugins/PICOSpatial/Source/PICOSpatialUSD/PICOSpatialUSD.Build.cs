// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class PICOSpatialUSD : ModuleRules
{
    public PICOSpatialUSD(ReadOnlyTargetRules Target) : base(Target)
    {
        PrivateDependencyModuleNames.AddRange(
            new string[] {
                "Core",
                "CoreUObject",
                "Engine",
                "PICOSpatialRuntimeSettings",
            }
        );

        if (Target.Type == TargetType.Editor)
        {
            bUseRTTI = true;
            PrivateDependencyModuleNames.Add("UnrealUSDWrapper");
        }
    }
}
