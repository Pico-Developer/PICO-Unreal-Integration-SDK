// Copyright 2023 PICO Inc. All Rights Reserved.

#include "PICO_PassthroughModule.h"

DEFINE_LOG_CATEGORY(LogPassthroughPICO);

void FPICOOpenXRPassthroughModule::StartupModule()
{
	PICOOpenXRPassthrough.Register();
}

void FPICOOpenXRPassthroughModule::ShutdownModule()
{
	PICOOpenXRPassthrough.Unregister();
}

IMPLEMENT_MODULE(FPICOOpenXRPassthroughModule, PICOOpenXRPassthrough)
