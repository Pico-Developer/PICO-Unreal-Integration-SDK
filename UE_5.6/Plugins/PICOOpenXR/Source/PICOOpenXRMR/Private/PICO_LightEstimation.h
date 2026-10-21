// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "PICO_MR.h"

// Forward declaration
class FPICOOpenXRMRModule;

class FLightEstimationExtensionPICO : public FMixedRealityPICO
{
public:
	FLightEstimationExtensionPICO();

	virtual ~FLightEstimationExtensionPICO(){}

	static FLightEstimationExtensionPICO* GetInstance();

	virtual bool CreateProvider(const FSenseDataProviderCreateInfoBasePICO& CreateInfo, EResultPICO& OutResult) override;
	
	void GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions);
	virtual void PostGetSystem(XrInstance InInstance, XrSystemId InSystem);
	virtual void PostCreateSession(XrSession InSession) override;

	bool RequestLightEstimation(const FPICOPollFutureDelegate& Delegate, EResultPICO& OutResult);
	bool GetLightEstimationInfo(const XrFutureEXT& FutureHandle, ULightEstimationTexturePICO* TextureCube, float& SourceCubemapAngle, EResultPICO& OutResult);
	ESpatialLightEstimationResolutionPICO GetCurrentLightEstimationResolution();
	FIntPoint GetLightEstimationSize() const { return LightEstimationSize; }

private:
	bool bSupportLightEstimateEXT = false;
	bool bSupportLightEstimation = false;
	ESpatialLightEstimationResolutionPICO CurrentResolution;
	FIntPoint LightEstimationSize;
};
