// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class PICOXRInput : ModuleRules
{
    public PICOXRInput(ReadOnlyTargetRules Target) : base(Target)
    {
        PrivateIncludePathModuleNames.AddRange(
            new[]
            {
                "InputDevice",			// For IInputDevice.h
				"HeadMountedDisplay",	// For IMotionController.h
				"ImageWrapper",
                "XRBase"
            });

        PrivateDependencyModuleNames.AddRange(
            new[]
            {
                "Core",
                "CoreUObject",
                "ApplicationCore",
                "Engine",
                "InputCore",
                "HeadMountedDisplay",
                "PICOXRHMD",
                "PXRPlugin",
                "Projects",
                "XRBase"
            });
        PrivateIncludePaths.AddRange(
            new[] {
                "PICOXRInput/Private",
                "PICOXRHMD/Private",
            });

    }
}
