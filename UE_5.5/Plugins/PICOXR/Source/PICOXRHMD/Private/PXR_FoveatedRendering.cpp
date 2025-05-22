// Copyright Epic Games, Inc. All Rights Reserved.

#include "PXR_FoveatedRendering.h"

#include "RenderGraphBuilder.h"
#include "PXR_HMD.h"

FPICOXRFoveatedRenderingImageGenerator::FPICOXRFoveatedRenderingImageGenerator(const FXRSwapChainPtr& Swapchain)
	: FoveationSwapchain(Swapchain)
{
	GVRSImageManager.RegisterExternalImageGenerator(this);
}

FPICOXRFoveatedRenderingImageGenerator::~FPICOXRFoveatedRenderingImageGenerator()
{
	GVRSImageManager.UnregisterExternalImageGenerator(this);
}

FRDGTextureRef FPICOXRFoveatedRenderingImageGenerator::GetImage(FRDGBuilder& GraphBuilder, const FViewInfo& ViewInfo, FVariableRateShadingImageManager::EVRSImageType ImageType, bool bGetSoftwareImage)
{
	if (!FoveationSwapchain.IsValid()|| bGetSoftwareImage)
	{
		return nullptr;
	}

	FTextureRHIRef SwapchainTexture = FoveationSwapchain->GetTexture2DArray() ? FoveationSwapchain->GetTexture2DArray() : FoveationSwapchain->GetTexture2D();
	FIntPoint TexSize = SwapchainTexture->GetSizeXY();
	// Only set texture and return true if we have a valid texture of compatible size
	if (SwapchainTexture->IsValid() && TexSize.X > 0 && TexSize.Y > 0)
	{
		TRefCountPtr<IPooledRenderTarget> PooledRenderTarget = CreateRenderTarget(SwapchainTexture, *SwapchainTexture->GetName().ToString());
		return GraphBuilder.RegisterExternalTexture(PooledRenderTarget, *SwapchainTexture->GetName().ToString(), ERDGTextureFlags::SkipTracking);
	}
	return nullptr;
}

void FPICOXRFoveatedRenderingImageGenerator::PrepareImages(FRDGBuilder& GraphBuilder, const FSceneViewFamily& ViewFamily, const FMinimalSceneTextures& SceneTextures, bool bPrepareHardwareImages, bool bPrepareSoftwareImages)
{
	//Not implemented as images are updated in UpdateFoveationImages only
	//when foveation parameters change or when the color swapchain is reallocated.
	return;
}

bool FPICOXRFoveatedRenderingImageGenerator::IsEnabled() const
{
	return true;
}

bool FPICOXRFoveatedRenderingImageGenerator::IsSupportedByView(const FSceneView& View) const
{
	// Only used for XR views
	return IStereoRendering::IsStereoEyeView(View);
}

FRDGTextureRef FPICOXRFoveatedRenderingImageGenerator::GetDebugImage(FRDGBuilder& GraphBuilder, const FViewInfo& ViewInfo, FVariableRateShadingImageManager::EVRSImageType ImageType, bool bGetSoftwareImage)
{
	return nullptr;
}