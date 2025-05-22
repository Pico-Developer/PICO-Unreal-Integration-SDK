// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#include "PXR_DynamicResolutionState.h"
#include "LegacyScreenPercentageDriver.h"
#include "SceneView.h"
#include "PXR_Log.h"

//-------------------------------------------------------------------------------------------------
// FPXR_DynamicResolutionState implementation
//-------------------------------------------------------------------------------------------------

FPXR_DynamicResolutionState::FPXR_DynamicResolutionState(const FSettingsPtr InSettings, FPICOXRHMD* InHMD)
	: Settings(InSettings)
	, HMD(InHMD)
	, ResolutionFraction(1.0f)
	, ResolutionFractionUpperBound(1.0f)
{
}

FPXR_DynamicResolutionState::~FPXR_DynamicResolutionState()
{
}

void FPXR_DynamicResolutionState::ResetHistory()
{
};

bool FPXR_DynamicResolutionState::IsSupported() const
{
#ifndef PICO_CUSTOM_ENGINE
	PXR_LOGE(PxrUnreal, "Adaptive resolution is currently only supported on the PICO fork of Unreal Engine.");
	return false;
#else
	return true;
#endif

	
}

void FPXR_DynamicResolutionState::SetupMainViewFamily(class FSceneViewFamily& ViewFamily)
{
	CheckInGameThread();

	// Check whether resolution fraction is supposed to be displayed
	if (!ViewFamily.EngineShowFlags.ScreenPercentage || !IsEnabled())
	{
		PXR_LOGE(PxrUnreal, "Cannot display screen resolution. Adaptive resolution not enabled.");
		return;
	}

	// Copy resolution fraction (i.e. pixel density) and upper bound from settings
	ResolutionFraction = Settings->PixelDensity;
	ResolutionFractionUpperBound = Settings->PixelDensityMax;

	ViewFamily.SetScreenPercentageInterface(new FLegacyScreenPercentageDriver(ViewFamily, ResolutionFraction, ResolutionFractionUpperBound));
}

DynamicRenderScaling::TMap<float> FPXR_DynamicResolutionState::GetResolutionFractionsApproximation() const
{
	DynamicRenderScaling::TMap<float> ResolutionFractions;
	ResolutionFractions.SetAll(1.0f);
	ResolutionFractions[GDynamicPrimaryResolutionFraction] = ResolutionFraction;
	return ResolutionFractions;
}

DynamicRenderScaling::TMap<float> FPXR_DynamicResolutionState::GetResolutionFractionsUpperBound() const
{
	DynamicRenderScaling::TMap<float> ResolutionFractions;
	ResolutionFractions.SetAll(1.0f);
	ResolutionFractions[GDynamicPrimaryResolutionFraction] = ResolutionFractionUpperBound;
	return ResolutionFractions;
}

void FPXR_DynamicResolutionState::SetEnabled(bool bEnable)
{
	CheckInGameThread();

	if (!IsSupported())
	{
		PXR_LOGE(PxrUnreal, "Cannot enable adaptive resolution. This feature is currently only supported on the PICO fork of Unreal Engine.");
		return;
	}

	Settings->EnableAdaptiveResolution(bEnable);
}

bool FPXR_DynamicResolutionState::IsEnabled() const
{
	CheckInGameThread();
	return Settings->IsAdaptiveResolutionEnabled();
}

void FPXR_DynamicResolutionState::ProcessEvent(EDynamicResolutionStateEvent Event)
{
	// Copy resolution fraction (i.e. pixel density) and upper bound from settings
	ResolutionFraction = Settings->PixelDensity;
	ResolutionFractionUpperBound = Settings->PixelDensityMax;
};