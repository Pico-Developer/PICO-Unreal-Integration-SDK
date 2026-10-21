// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "PICO_IMRModule.h"
#include "PICO_MRExtension.h"

class FPICOOpenXRMRModule : public IPICOOpenXRMRModule
{
public:
	static inline FPICOOpenXRMRModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FPICOOpenXRMRModule>("PICOOpenXRMR");
	}

	virtual void StartupModule() override;

	virtual void ShutdownModule() override;
	
	FMRModuleExtensionPICO& GetMRModuleExtension() { return MRModuleExtension; }
	
private:
	FMRModuleExtensionPICO MRModuleExtension;
};