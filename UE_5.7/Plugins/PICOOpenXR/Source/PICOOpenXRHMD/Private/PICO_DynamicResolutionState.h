// @lint-ignore-every LICENSELINT
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "Misc/EngineVersionComparison.h"
#include "DynamicResolutionState.h"

//-------------------------------------------------------------------------------------------------
// FDynamicResolutionStatePICO
//-------------------------------------------------------------------------------------------------

class FDynamicResolutionStatePICO : public IDynamicResolutionState
{
public:
	FDynamicResolutionStatePICO(class FHMDPICO* HMDPICO);

	// ISceneViewFamilyScreenPercentage
	virtual void ResetHistory() override;
	virtual bool IsSupported() const override;
	virtual void SetupMainViewFamily(class FSceneViewFamily& ViewFamily) override;
#if !UE_VERSION_OLDER_THAN(5, 4, 0)
	virtual void SetTemporalUpscaler(const UE::Renderer::Private::ITemporalUpscaler* InTemporalUpscaler) override;
#endif // !UE_VERSION_OLDER_THAN(5, 4, 0)

protected:
	virtual DynamicRenderScaling::TMap<float> GetResolutionFractionsApproximation() const override;
	virtual DynamicRenderScaling::TMap<float> GetResolutionFractionsUpperBound() const override;
	virtual void SetEnabled(bool bEnable) override;
	virtual bool IsEnabled() const override;
	virtual void ProcessEvent(EDynamicResolutionStateEvent Event) override;

private:
	class FHMDPICO* PICOHMD = nullptr;
	float ResolutionFraction;
	float ResolutionFractionUpperBound;
};
