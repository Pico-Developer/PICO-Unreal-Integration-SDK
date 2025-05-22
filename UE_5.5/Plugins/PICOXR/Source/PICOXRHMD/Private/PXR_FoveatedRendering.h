// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "VariableRateShadingImageManager.h"
#include "XRSwapchain.h"

class FPICOXRHMD;

class FPICOXRFoveatedRenderingImageGenerator : public IVariableRateShadingImageGenerator
{
public:
	FPICOXRFoveatedRenderingImageGenerator(const FXRSwapChainPtr& Swapchain);
	virtual ~FPICOXRFoveatedRenderingImageGenerator() override;

	// IVariableRateShadingImageGenerator interface
	// Returns cached VRS image.
	virtual FRDGTextureRef GetImage(FRDGBuilder& GraphBuilder, const FViewInfo& ViewInfo, FVariableRateShadingImageManager::EVRSImageType ImageType, bool bGetSoftwareImage = false) override;

	// Generates image(s) and saves to generator cache. Should only be run once per view per frame, in Render().
	virtual void PrepareImages(FRDGBuilder& GraphBuilder, const FSceneViewFamily& ViewFamily, const FMinimalSceneTextures& SceneTextures, bool bPrepareHardwareImages, bool bPrepareSoftwareImages) override;

	// Returns whether or not generator is enabled - can change at runtime
	virtual bool IsEnabled() const override;

	// Returns whether or not the given view supports this generator
	virtual bool IsSupportedByView(const FSceneView& View) const override;

	// Get VRS image to be used w/ debug overlay
	virtual FRDGTextureRef GetDebugImage(FRDGBuilder& GraphBuilder, const FViewInfo& ViewInfo, FVariableRateShadingImageManager::EVRSImageType ImageType, bool bGetSoftwareImage = false) override;

	virtual FVariableRateShadingImageManager::EVRSSourceType GetType() const override
	{
		return FVariableRateShadingImageManager::EVRSSourceType::FixedFoveation;
	}

private:
	const FXRSwapChainPtr& FoveationSwapchain;
};
