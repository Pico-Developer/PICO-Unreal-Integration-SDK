// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "Modules/ModuleManager.h"
#include "IHeadMountedDisplayModule.h"

#define PICO_HMD_SUPPORTED_PLATFORMS (PLATFORM_WINDOWS && WINVER > 0x0502)  || PLATFORM_ANDROID

class IPICOXRHMDModule : public IHeadMountedDisplayModule
{
public:

	static inline IPICOXRHMDModule& Get()
	{
		return FModuleManager::LoadModuleChecked< IPICOXRHMDModule >("PICOXRHMD");
	}

	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("PICOXRHMD");
	}
};
