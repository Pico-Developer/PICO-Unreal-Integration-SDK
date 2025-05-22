// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#include "PXR_MRModule.h"
#include "PXR_ProviderManager.h"

#define LOCTEXT_NAMESPACE "FPICOXRMRModule"

FPICOXRMRModule::FPICOXRMRModule()
	: PICOXRHMD(nullptr)
{
}

FPICOXRMRModule::~FPICOXRMRModule()
{
}

void FPICOXRMRModule::StartupModule()
{
	if (GEngine)
	{
		if (GEngine->XRSystem.IsValid() && (GEngine->XRSystem->GetSystemName() == TEXT("PICOXRHMD")))
		{
			PICOXRHMD = static_cast<FPICOXRHMD*>(GEngine->XRSystem.Get());
		}
	}

#if PLATFORM_ANDROID
	if(FPICOProviderManager::Initialize(PICOXRHMD))
	{
		UE_LOG(LogHMD,Log, TEXT("FPICOProviderManager::Initialize success!"))
	}
#endif
	
}

void FPICOXRMRModule::ShutdownModule()
{
	if(FPICOProviderManager::Finalize())
	{
		UE_LOG(LogHMD,Log, TEXT("FPICOProviderManager::Initialize success!"))
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FPICOXRMRModule, PICOXRMR)