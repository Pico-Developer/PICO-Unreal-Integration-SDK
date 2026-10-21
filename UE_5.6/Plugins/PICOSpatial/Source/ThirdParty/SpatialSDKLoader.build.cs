// Copyright 2023 PICO Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class SpatialSDKLoader : ModuleRules
{
    public SpatialSDKLoader(ReadOnlyTargetRules Target) : base(Target)
    {
        Type = ModuleType.External;
		PublicSystemIncludePaths.Add("$(ModuleDir)/SpatialSDKLoader/include");
		if (Target.Platform == UnrealTargetPlatform.Android)
		{
			PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "SpatialSDKLoader/libs/arm64-v8a/libspatial_pack_loader.so"));
			// Keep this receipt dependency for UBT/staging visibility; SpatialSDK_APL.xml performs
			// the Android APK libs copy for the arm64-v8a package layout.
			RuntimeDependencies.Add(Path.Combine(ModuleDirectory, "SpatialSDKLoader/libs/arm64-v8a/libspatial_pack_loader.so"));
            // AndroidPlugin
            {
                string PluginPath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);
                AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(PluginPath, "SpatialSDK_APL.xml"));
            }
        }
    }
}
