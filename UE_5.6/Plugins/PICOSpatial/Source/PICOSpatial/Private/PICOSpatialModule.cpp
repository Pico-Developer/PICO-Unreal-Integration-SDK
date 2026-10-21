// Copyright Epic Games, Inc. All Rights Reserved.

#include "PICOSpatialModule.h"
#include "Interfaces/IPluginManager.h"
#include "PICOSpatial.h"

void FPICOSpatialModule::StartupModule()
{
	FPICOSpatial::Get().Register();
}

void FPICOSpatialModule::ShutdownModule()
{
	FPICOSpatial::Get().Unregister();
}

IMPLEMENT_MODULE(FPICOSpatialModule, PICOSpatial)
