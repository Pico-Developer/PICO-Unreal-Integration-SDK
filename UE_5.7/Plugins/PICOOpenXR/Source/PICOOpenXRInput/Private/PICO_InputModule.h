// Copyright 2023 PICO Inc. All Rights Reserved.

#pragma once

#include "PICO_IInputModule.h"
#include "PICO_Controller.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPICOOpenXRInput, Log, All);

class FPICOOpenXRInputModule : public IPICOOpenXRInputModule
{
public:
	static inline FPICOOpenXRInputModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FPICOOpenXRInputModule>("PICOOpenXRInput");
	}

	FControllerPICO& GetController() { return ControllerExtension; }

	virtual void StartupModule() override;

	virtual void ShutdownModule() override;

private:
	FControllerPICO ControllerExtension;
};