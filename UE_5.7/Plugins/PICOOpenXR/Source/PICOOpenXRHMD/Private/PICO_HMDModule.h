// Copyright 2023 PICO Inc. All Rights Reserved.

#pragma once

#include "PICO_IHMDModule.h"
#include "PICO_HMD.h"

class FPICOOpenXRHMDModule : public IPICOOpenXRHMDModule
{
public:
	static inline FPICOOpenXRHMDModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FPICOOpenXRHMDModule>("PICOOpenXRHMD");
	}

	FHMDPICO& GetXRPlugin() { return HMDPICOExtension; }
	virtual void StartupModule() override;

	virtual void ShutdownModule() override;

public:
	FHMDPICO HMDPICOExtension;
};