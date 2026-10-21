// Copyright 2023 PICO Inc. All Rights Reserved.

#pragma once

#include "PICO_IPassthroughModule.h"
#include "PICO_Passthrough.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPassthroughPICO, Log, All);

class FPICOOpenXRPassthroughModule : public IPICOOpenXRPassthroughModule
{
public:
	static inline FPICOOpenXRPassthroughModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FPICOOpenXRPassthroughModule>("PICOOpenXRPassthrough");
	}

	FPICOOpenXRPassthrough& GetPassthroughPlugin() { return PICOOpenXRPassthrough; }
	virtual void StartupModule() override;

	virtual void ShutdownModule() override;

public:
	FPICOOpenXRPassthrough PICOOpenXRPassthrough;
};