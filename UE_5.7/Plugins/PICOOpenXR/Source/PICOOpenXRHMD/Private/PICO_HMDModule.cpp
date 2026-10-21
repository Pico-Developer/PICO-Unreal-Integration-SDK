// Copyright 2023 PICO Inc. All Rights Reserved.

#include "PICO_HMDModule.h"
#include "PICOOpenXRRuntimeSettings.h"
#include "ShaderCore.h"
#include "HAL/IConsoleManager.h"
#include "Interfaces/IPluginManager.h"

void FPICOOpenXRHMDModule::StartupModule()
{
	HMDPICOExtension.Register();

#if PLATFORM_ANDROID
	if (IConsoleVariable* DisableOpenXROnAndroidWithoutOculusCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("xr.DisableOpenXROnAndroidWithoutOculus")))
	{
		const bool bDisableOpenXROnAndroidWithoutOculus = UPICOOpenXRRuntimeSettings::GetBoolConfigByKey(TEXT("bDisableOpenXROnAndroidWithoutOculus"));
		DisableOpenXROnAndroidWithoutOculusCVar->Set(bDisableOpenXROnAndroidWithoutOculus, ECVF_SetByPluginHighPriority);
	}
#endif

	FString PluginShaderDir = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("PICOOpenXR"))->GetBaseDir(), TEXT("Shaders"));
	AddShaderSourceDirectoryMapping(TEXT("/Plugin/PICOOpenXR"), PluginShaderDir);
}

void FPICOOpenXRHMDModule::ShutdownModule()
{
	HMDPICOExtension.Unregister();
}

IMPLEMENT_MODULE(FPICOOpenXRHMDModule, PICOOpenXRHMD)
