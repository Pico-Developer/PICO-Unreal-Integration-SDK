// Copyright® 2015-2023 PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Unreal® Engine, Copyright 1998 – 2023, Epic Games, Inc. All rights reserved. 


#include "PXR_MRCFunctionLibrary.h"
#include "PXR_MRCModule.h"

bool UPICOXRMRCFunctionLibrary::IsMrcActivated()
{
	if (FPICOXRMRCModule::IsAvailable())
	{
		return FPICOXRMRCModule::Get().IsMrcActivated();
	}
	return false;
}

bool UPICOXRMRCFunctionLibrary::GetInGameThirdCameraRT(UTextureRenderTarget2D* & Background_RT, UTextureRenderTarget2D*& Forgound_RT)
{
	if (FPICOXRMRCModule::IsAvailable())
	{
		return FPICOXRMRCModule::Get().GetMRCRT(Background_RT,Forgound_RT);
	}
	return false;
}

void UPICOXRMRCFunctionLibrary::SimulateEnableMRC(bool enable)
{
	if (FPICOXRMRCModule::IsAvailable())
	{
		FPICOXRMRCModule::Get().bSimulateEnableMRC=enable;
	}
}

void UPICOXRMRCFunctionLibrary::EnableForegroundMRC(bool enable)
{
	if (FPICOXRMRCModule::IsAvailable())
	{
		FPICOXRMRCModule::Get().EnableForeground(enable);
	}
}

void UPICOXRMRCFunctionLibrary::SetMRCTrackingReference(USceneComponent* TrackingReference)
{
	if (FPICOXRMRCModule::IsAvailable())
	{
		if (FPICOXRMRCModule::Get().GetMRCState())
		{
			FPICOXRMRCModule::Get().GetMRCState()->CurrentTrackingReference = TrackingReference;
		}
	}
}

void UPICOXRMRCFunctionLibrary::SetMRCUseCustomTrans(const FTransform& CustomTrans, bool UseCutomTrans)
{
	if (FPICOXRMRCModule::IsAvailable())
	{
		if (FPICOXRMRCModule::Get().GetMRCState())
		{
			FPICOXRMRCModule::Get().GetMRCState()->bUseCustomTrans = UseCutomTrans;
			FPICOXRMRCModule::Get().GetMRCState()->CustomTrans = CustomTrans;
		}
	}
}

void UPICOXRMRCFunctionLibrary::GetMRCRelativePose(FTransform& CustomTrans)
{
	if (FPICOXRMRCModule::IsAvailable())
	{
		if (FPICOXRMRCModule::Get().GetMRCState())
		{
			CustomTrans = FPICOXRMRCModule::Get().GetMRCState()->FinalTransform;
		}
	}
}

void UPICOXRMRCFunctionLibrary::UpdateMRCCameraZ()
{
	if (FPICOXRMRCModule::IsAvailable())
	{
		if (FPICOXRMRCModule::Get().GetMRCState())
		{
			FPICOXRMRCModule::Get().GetMRCState()->bUpdateMRCCameraZ = true;
		}
	}
}
