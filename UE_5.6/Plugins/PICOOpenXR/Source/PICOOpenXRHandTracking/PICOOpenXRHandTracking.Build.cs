// Copyright Epic Games, Inc. All Rights Reserved.

namespace UnrealBuildTool.Rules
{
    public class PICOOpenXRHandTracking: ModuleRules
    {
        public PICOOpenXRHandTracking(ReadOnlyTargetRules Target) 
				: base(Target)
        {
            PublicDependencyModuleNames.Add("InputDevice");
            PublicDependencyModuleNames.Add("HeadMountedDisplay");
            PrivateDependencyModuleNames.AddRange(
                new string[]
                {
                    "Core",
                    "CoreUObject",
                    "Engine",
					"XRBase",
                    "InputCore",
					"LiveLinkAnimationCore",
					"LiveLinkInterface",
					"OpenXRHMD",
					"OpenXRInput",
					"Slate",
					"SlateCore",
					"ApplicationCore",
                    "PICOOpenXRRuntimeSettings"
                }
				);

            AddEngineThirdPartyPrivateStaticDependencies(Target, "OpenXR");

            if (Target.bBuildEditor == true)
            {
				PrivateDependencyModuleNames.Add("EditorFramework");
				PrivateDependencyModuleNames.Add("UnrealEd");
				PrivateDependencyModuleNames.Add("InputEditor");
            }
        }
    }
}
