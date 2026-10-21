// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#include "PICO_EnterpriseModule.h"
#include "PICO_EnterpriseInterfaceWrapper.h"

#define LOCTEXT_NAMESPACE "FPICOEnterpriseModule"

void FPICOEnterpriseModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	FInterfaceWrapper::GetInstance()->Initialize();
}

void FPICOEnterpriseModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FPICOEnterpriseModule, PICOEnterprise)