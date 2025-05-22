// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "PXR_HMD.h"
#include "PXR_HMDSettings.h"
#include "DynamicResolutionState.h"
#include "DynamicResolutionProxy.h"

//-------------------------------------------------------------------------------------------------
// FPXR_DynamicResolutionState
//-------------------------------------------------------------------------------------------------

class FPXR_DynamicResolutionState : public IDynamicResolutionState
{
public:
	FPXR_DynamicResolutionState(const FSettingsPtr InSettings, FPICOXRHMD* InHMD);
	~FPXR_DynamicResolutionState() override;

	// ISceneViewFamilyScreenPercentage
	virtual void ResetHistory() override;
	virtual bool IsSupported() const override;
	virtual void SetupMainViewFamily(class FSceneViewFamily& ViewFamily) override;

protected:
	virtual DynamicRenderScaling::TMap<float> GetResolutionFractionsApproximation() const override;
	virtual DynamicRenderScaling::TMap<float> GetResolutionFractionsUpperBound() const override;
	virtual void SetEnabled(bool bEnable) override;
	virtual bool IsEnabled() const override;
	virtual void ProcessEvent(EDynamicResolutionStateEvent Event) override;
	/** Apply the minimum/maximum resolution fraction for a third-party temporal upscaler. */
	virtual void SetTemporalUpscaler(const UE::Renderer::Private::ITemporalUpscaler* InTemporalUpscaler)
	{
	}

private:
	const FSettingsPtr Settings;
	FPICOXRHMD* HMD = nullptr;

	float ResolutionFraction;
	float ResolutionFractionUpperBound;
};
