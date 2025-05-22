// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class IPICOXRMotionTrackingModule : public IModuleInterface
{

public:
	static inline IPICOXRMotionTrackingModule& Get()
	{
		return FModuleManager::GetModuleChecked<IPICOXRMotionTrackingModule>("PICOXRMotionTracking");
	}

	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("PICOXRMotionTracking");
	}
};
