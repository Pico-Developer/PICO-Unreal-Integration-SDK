// Copyright® 2015-2023 PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Unreal® Engine, Copyright 1998 – 2023, Epic Games, Inc. All rights reserved. 

#include "PXR_DynamicResolutionState.h"
#include "SceneView.h"
#include "PXR_Log.h"


//-------------------------------------------------------------------------------------------------
// FPXR_ScreenPercentage
//-------------------------------------------------------------------------------------------------

class FPXR_ScreenPercentage : public ISceneViewFamilyScreenPercentage
{
public:
	FPXR_ScreenPercentage(const FSceneViewFamily& InViewFamily, float InResolutionFraction, float InResolutionFractionUpperBound);

protected:
	// ISceneViewFamilyScreenPercentage
	virtual float GetPrimaryResolutionFractionUpperBound() const override;
	virtual ISceneViewFamilyScreenPercentage* Fork_GameThread(const class FSceneViewFamily& ForkedViewFamily) const override;
	virtual void ComputePrimaryResolutionFractions_RenderThread(TArray<FSceneViewScreenPercentageConfig>& OutViewScreenPercentageConfigs) const override;

private:
	const FSceneViewFamily& ViewFamily;
	const float ResolutionFraction;
	const float ResolutionFractionUpperBound;
};


//-------------------------------------------------------------------------------------------------
// FPXR_ScreenPercentage implementation
//-------------------------------------------------------------------------------------------------

FPXR_ScreenPercentage::FPXR_ScreenPercentage(const FSceneViewFamily& InViewFamily, float InResolutionFraction, float InResolutionFractionUpperBound)
	: ViewFamily(InViewFamily)
	, ResolutionFraction(InResolutionFraction)
	, ResolutionFractionUpperBound(InResolutionFractionUpperBound)
{
}

float FPXR_ScreenPercentage::GetPrimaryResolutionFractionUpperBound() const
{
	return ResolutionFractionUpperBound;
}

ISceneViewFamilyScreenPercentage* FPXR_ScreenPercentage::Fork_GameThread(const class FSceneViewFamily& ForkedViewFamily) const
{
	return new FPXR_ScreenPercentage(ForkedViewFamily, ResolutionFraction, ResolutionFractionUpperBound);
}

void FPXR_ScreenPercentage::ComputePrimaryResolutionFractions_RenderThread(TArray<FSceneViewScreenPercentageConfig>& OutViewScreenPercentageConfigs) const
{
	// CheckInRenderThread();

	for (int32 ConfigIter = 0; ConfigIter < OutViewScreenPercentageConfigs.Num(); ++ConfigIter)
	{
		OutViewScreenPercentageConfigs[ConfigIter].PrimaryResolutionFraction = ResolutionFraction;
	}
}


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
#endif

	return true;
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

	ViewFamily.SetScreenPercentageInterface(new FPXR_ScreenPercentage(ViewFamily, ResolutionFraction, ResolutionFractionUpperBound));
}

float FPXR_DynamicResolutionState::GetResolutionFractionApproximation() const
{
	return ResolutionFraction;
}

float FPXR_DynamicResolutionState::GetResolutionFractionUpperBound() const
{
	return ResolutionFractionUpperBound;
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
