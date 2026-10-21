// Copyright 2023 PICO Inc. All Rights Reserved.

#include "PICO_InputModule.h"

DEFINE_LOG_CATEGORY(LogPICOOpenXRInput);

void FPICOOpenXRInputModule::StartupModule()
{
	ControllerExtension.Register();
}

void FPICOOpenXRInputModule::ShutdownModule()
{
	ControllerExtension.Unregister();
}

IMPLEMENT_MODULE(FPICOOpenXRInputModule, PICOOpenXRInput)
