// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "IPXR_MotionTrackingModule.h"
#include "PXR_MotionTracking.h"

class FPICOXRMotionTrackingModule : public IPICOXRMotionTrackingModule
{
public:
	FPICOXRMotionTrackingModule();

	static inline FPICOXRMotionTrackingModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FPICOXRMotionTrackingModule>("PICOXRMotionTracking");
	}

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

