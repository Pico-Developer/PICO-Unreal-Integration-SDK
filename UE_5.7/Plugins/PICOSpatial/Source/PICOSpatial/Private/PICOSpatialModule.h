// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "IPICOSpatialModule.h"

class FPICOSpatialModule : public IPICOSpatialModule
{
public:
	static inline FPICOSpatialModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FPICOSpatialModule>("PICOSpatial");
	}

	virtual void StartupModule() override;

	virtual void ShutdownModule() override;
};
