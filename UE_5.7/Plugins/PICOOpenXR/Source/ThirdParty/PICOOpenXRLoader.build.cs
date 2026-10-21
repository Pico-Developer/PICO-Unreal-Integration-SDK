// Copyright 2023 PICO Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class PICOOpenXRLoader : ModuleRules
{
    public PICOOpenXRLoader(ReadOnlyTargetRules Target) : base(Target)
    {
        Type = ModuleType.External;

        string SourceDirectory = "$(PluginDir)/Source/ThirdParty/PICOOpenXRLoader/";

        PublicIncludePaths.Add(SourceDirectory + "include");

        if (Target.Platform == UnrealTargetPlatform.Android)
        {
            RuntimeDependencies.Add(Path.Combine(ModuleDirectory, "PICOOpenXRLoader/libs/arm64-v8a/libopenxr_loader_pico.so"));
            // AndroidPlugin
            {
                string PluginPath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);
                AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(PluginPath, "PICOMobile_APL.xml"));
            }
        }
    }
}