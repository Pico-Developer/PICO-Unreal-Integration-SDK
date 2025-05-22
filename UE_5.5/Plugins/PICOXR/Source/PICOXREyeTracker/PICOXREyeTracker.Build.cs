// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

namespace UnrealBuildTool.Rules
{
	public class PICOXREyeTracker : ModuleRules
	{
		public PICOXREyeTracker(ReadOnlyTargetRules Target) : base(Target)
		{
			if (Target.Platform == UnrealTargetPlatform.Win64 ||
				Target.Platform == UnrealTargetPlatform.Android)
			{
				PrivateIncludePaths.AddRange(
					new string[] {
						"PICOXRHMD/Private",
					});

				PublicDependencyModuleNames.AddRange(
					new string[]
					{
						"InputDevice",
						"EyeTracker",
						"PXRPlugin",
						"PICOXRHMD",
                    }
				);

				PrivateDependencyModuleNames.AddRange(
					new string[]
					{
						"Core",
						"CoreUObject",
						"Engine",
						"InputCore",
					}
				);
            }
        }
	}
}
