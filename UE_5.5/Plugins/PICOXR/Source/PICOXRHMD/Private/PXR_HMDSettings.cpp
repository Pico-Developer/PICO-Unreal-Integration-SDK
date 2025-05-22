// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#include "PXR_HMDSettings.h"

#if PICO_HMD_SUPPORTED_PLATFORMS

FGameSettings::FGameSettings() :
	BaseOffset(0, 0, 0)
	, BaseOrientation(FQuat::Identity)
	, CustomOffsetYaw(0.f)
	, PixelDensity(1.0f)
	, PixelDensityMin(0.7f)
	, PixelDensityMax(1.26f)
	, FoveatedRenderingLevel(PxrFoveationLevel::PXR_FOVEATION_LEVEL_NONE)
	, CoordinateType(EPICOXRCoordinateType::Local)
	, bApplyColorScaleAndOffsetToAllLayers(false)
	, ColorScale(PxrVector4f{ 1,1,1,1 })
	, ColorOffset(PxrVector4f{ 0,0,0,0 })
	, bLateLatching(false)
	, bWaitFrameAtGameFrameTail(false)
	, SeeThroughState(0)
{
	Flags.Raw = 0;
	Flags.bHMDEnabled = true;

#if PLATFORM_ANDROID
	Flags.bsRGBEyeBuffer = true;
	Flags.bStereoEnabled = true;
	CurrentShaderPlatform = EShaderPlatform::SP_VULKAN_ES3_1_ANDROID;
#else
	Flags.bsRGBEyeBuffer = false;
	Flags.bStereoEnabled = false;
	CurrentShaderPlatform = EShaderPlatform::SP_PCD3D_SM5;
#endif
	EyeRenderViewport[0] = EyeRenderViewport[1] = FIntRect(0, 0, 0, 0);
	RenderTargetSize = FIntPoint(0, 0);
}

TSharedPtr<FGameSettings, ESPMode::ThreadSafe> FGameSettings::Clone() const
{
	TSharedPtr<FGameSettings, ESPMode::ThreadSafe> NewSettings = MakeShareable(new FGameSettings(*this));
	return NewSettings;
}

void FGameSettings::SetPixelDensity(float NewPixelDensity)
{
	if (IsAdaptiveResolutionEnabled())
	{
		PixelDensity = FMath::Clamp(NewPixelDensity, PixelDensityMin, PixelDensityMax);
	}
	else
	{
		PixelDensity = FMath::Clamp(NewPixelDensity, ClampPixelDensityMin, ClampPixelDensityMax);
	}
}

void FGameSettings::EnableAdaptiveResolution(bool bEnable)
{
	// Return if nothing is changed
	if (bAdaptiveResolutionEnabled == bEnable)
	{
		return;
	}
	bAdaptiveResolutionEnabled = bEnable;
	// Ensure pixel density abides by the new min / max pixel densities
	SetPixelDensity(PixelDensity);
}

bool FGameSettings::IsAdaptiveResolutionEnabled() const
{
#ifndef PICO_CUSTOM_ENGINE
	return false;
#else
	return bAdaptiveResolutionEnabled;
#endif
}

#endif //PICO_HMD_SUPPORTED_PLATFORMS
