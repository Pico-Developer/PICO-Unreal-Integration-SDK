// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ARTextures.h"
#include "TextureResource.h"
#include "RHICommandList.h"
#include "PXR_Plugin_Types.h"

#if PLATFORM_ANDROID
#include "VulkanRHIPrivate.h"
#include "VulkanResources.h"
#include "VulkanPendingState.h"
#include "VulkanContext.h"
#endif
#include "PXR_MRTextures.generated.h"

#define VULKAN_CUBEMAP_DEBUG 0

UCLASS(BlueprintType)
class UPICOLightEstimationTexture:
	public UTextureCube
{
	GENERATED_BODY()

public:
	UPICOLightEstimationTexture(const FObjectInitializer& ObjectInitializer);
	// UTexture interface implementation
	virtual void BeginDestroy() override;
	virtual FTextureResource* CreateResource() override;
	virtual EMaterialValueType GetMaterialType() const override { return MCT_TextureCube; }
	// End UTexture interface
	FTextureResource* NewResource=nullptr;
	FIntPoint GetSize() const { return Size; }
	bool UpdateVulkanTextureByLightEstimateInfo(const FIntPoint& InSize, const PxrLightEstimationInfoEnvironmentTextureVulkan& InEstimationTexture);
#if PLATFORM_ANDROID
	VkImage GetVulkanTexture() const { return VulkanTexture; }
private:
	/** The Apple specific representation of the ar environment texture */
	VkImage VulkanTexture = nullptr;
#endif
private:
	float SourceCubemapAngle;
	FIntPoint Size;
};

class FMRVulkanResource :
	public FTextureResource
{
public:
	FMRVulkanResource(UPICOLightEstimationTexture* InOwner);
	virtual void InitRHI(FRHICommandListBase& RHICmdList) override;
	/**
	 * Called when the resource is initialized. This is only called by the rendering thread.
	 */
	//virtual void InitRHI(FRHICommandListBase& RHICmdList) override;
	virtual void ReleaseRHI() override
	{
		RHIUpdateTextureReference(Owner->TextureReference.TextureReferenceRHI, nullptr);
		EnvCubemapTextureRHIRef.SafeRelease();
		FTextureResource::ReleaseRHI();
	}
	
	/** Returns the width of the texture in pixels. */
	virtual uint32 GetSizeX() const override
	{
		return Size.X;
	}
	
	/** Returns the height of the texture in pixels. */
	virtual uint32 GetSizeY() const override
	{
		return Size.Y;
	}
private:
	FIntPoint Size;
	const UPICOLightEstimationTexture* Owner;
	FTextureRHIRef EnvCubemapTextureRHIRef;

	void CopyTextureCube_RenderThread(FRHICommandListImmediate& RHICmdList, FRHITexture* DstTexture, FRHITexture* SrcTexture, FIntRect DstRect, FIntRect SrcRect) const;
};