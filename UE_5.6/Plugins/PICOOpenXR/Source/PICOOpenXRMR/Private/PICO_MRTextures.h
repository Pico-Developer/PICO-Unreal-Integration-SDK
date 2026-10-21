// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ARTextures.h"
#include "TextureResource.h"
#include "RHICommandList.h"
#include "openxr/pico_light_estimation.h"

#include "PICO_MRTextures.generated.h"

UCLASS(BlueprintType)
class ULightEstimationTexturePICO:
	public UTextureCube
{
	GENERATED_BODY()

public:
	ULightEstimationTexturePICO(const FObjectInitializer& ObjectInitializer);
	// UTexture interface implementation
	virtual void BeginDestroy() override;
	virtual FTextureResource* CreateResource() override;
	virtual EMaterialValueType GetMaterialType() const override { return MCT_TextureCube; }
	// End UTexture interface
	FTextureResource* NewResource=nullptr;
	FIntPoint GetSize() const { return Size; }
#if PLATFORM_ANDROID
	bool UpdateVulkanTextureByLightEstimateInfo(const FIntPoint& InSize, const XrLightEstimationInfoEnvironmentTextureVulkanPICO& InEstimationTexture);
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
	FMRVulkanResource(ULightEstimationTexturePICO* InOwner);
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
	const ULightEstimationTexturePICO* Owner;
	FTextureRHIRef EnvCubemapTextureRHIRef;

	void CopyTextureCube_RenderThread(FRHICommandListImmediate& RHICmdList, FRHITexture* DstTexture, FRHITexture* SrcTexture, FIntRect DstRect, FIntRect SrcRect) const;
};