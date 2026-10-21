// @lint-ignore-every LICENSELINT
// Copyright Epic Games, Inc. All Rights Reserved.

#include "PICO_DynamicResolutionState.h"
#include "LegacyScreenPercentageDriver.h"
#include "SceneView.h"
#include "PICO_HMD.h"

//-------------------------------------------------------------------------------------------------
// FDynamicResolutionStatePICO implementation
//-------------------------------------------------------------------------------------------------

FDynamicResolutionStatePICO::FDynamicResolutionStatePICO(class FHMDPICO* HMDPICO)
	: PICOHMD(HMDPICO)
	, ResolutionFraction(-1.0f)
	, ResolutionFractionUpperBound(-1.0f)
{
	check(PICOHMD != nullptr);
}

void FDynamicResolutionStatePICO::ResetHistory() {
	// Empty - Oculus drives resolution fraction externally
};

bool FDynamicResolutionStatePICO::IsSupported() const
{
	return true;
}

void FDynamicResolutionStatePICO::SetupMainViewFamily(class FSceneViewFamily& ViewFamily)
{
	check(IsInGameThread());
	check(ViewFamily.EngineShowFlags.ScreenPercentage == true);

	if (IsEnabled())
	{
		ResolutionFraction = PICOHMD->CurrentDynamicPixelDensity;
		ResolutionFractionUpperBound = 1.0f;
		ViewFamily.SetScreenPercentageInterface(new FLegacyScreenPercentageDriver(ViewFamily, ResolutionFraction, ResolutionFractionUpperBound));
	}
}
#if !UE_VERSION_OLDER_THAN(5, 4, 0)
void FDynamicResolutionStatePICO::SetTemporalUpscaler(const UE::Renderer::Private::ITemporalUpscaler* InTemporalUpscaler)
{
	// Not supported
	return;
}
#endif // !UE_VERSION_OLDER_THAN(5, 4, 0)

DynamicRenderScaling::TMap<float> FDynamicResolutionStatePICO::GetResolutionFractionsApproximation() const
{
	DynamicRenderScaling::TMap<float> ResolutionFractions;
	ResolutionFractions.SetAll(1.0f);
	ResolutionFractions[GDynamicPrimaryResolutionFraction] = ResolutionFraction;
	return ResolutionFractions;
}

DynamicRenderScaling::TMap<float> FDynamicResolutionStatePICO::GetResolutionFractionsUpperBound() const
{
	DynamicRenderScaling::TMap<float> ResolutionFractions;
	ResolutionFractions.SetAll(1.0f);
	ResolutionFractions[GDynamicPrimaryResolutionFraction] = ResolutionFractionUpperBound;
	return ResolutionFractionUpperBound;
}

void FDynamicResolutionStatePICO::SetEnabled(bool bEnable)
{
	check(IsInGameThread());
	PICOHMD->bDynamicResolution = bEnable;
}

bool FDynamicResolutionStatePICO::IsEnabled() const
{
	check(IsInGameThread());
	return PICOHMD->bDynamicResolution;
}

void FDynamicResolutionStatePICO::ProcessEvent(EDynamicResolutionStateEvent Event) {
	// Empty - Oculus drives resolution fraction externally
};