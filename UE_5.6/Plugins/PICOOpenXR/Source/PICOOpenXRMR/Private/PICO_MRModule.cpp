// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#include "PICO_MRModule.h"

void FPICOOpenXRMRModule::StartupModule()
{
	MRModuleExtension.Register();
}

void FPICOOpenXRMRModule::ShutdownModule()
{
	MRModuleExtension.Unregister();
}

IMPLEMENT_MODULE(FPICOOpenXRMRModule, PICOOpenXRMR)
