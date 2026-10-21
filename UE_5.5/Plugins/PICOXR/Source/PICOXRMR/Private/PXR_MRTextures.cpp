// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#include "PXR_MRTextures.h"
#include "RHIStaticStates.h"
#include "CommonRenderResources.h"
#include "PXR_MRTypes.h"
#include "PXR_Plugin_Types.h"
#include "ScreenRendering.h"
#include "PXR_Shaders.h"
#include "PXR_HMDModule.h"
#include "PXR_Log.h"

#define VULKAN_CUBEMAP_POSITIVE_Y 2
#define VULKAN_CUBEMAP_NEGATIVE_Y 3


UPICOLightEstimationTexture::UPICOLightEstimationTexture(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer), SourceCubemapAngle(0), Size(128, 128)
{
	SRGB = true;
}

void UPICOLightEstimationTexture::BeginDestroy()
{
	Super::BeginDestroy();
}

FTextureResource* UPICOLightEstimationTexture::CreateResource()
{
	return new FMRVulkanResource(this);
}

bool UPICOLightEstimationTexture::UpdateVulkanTextureByLightEstimateInfo(const FIntPoint& InSize, const PxrLightEstimationInfoEnvironmentTextureVulkan& InEstimationTexture)
{
	PXR_LOGV(PxrMR, "UpdateVulkanTextureByLightEstimateInfo Start!");
	
	{
#if PLATFORM_ANDROID
		VkImage TempTexture = reinterpret_cast<VkImage>(InEstimationTexture.image);
		
		if (TempTexture != VK_NULL_HANDLE)
		{
			VulkanTexture = TempTexture;
			Size.X = InSize.X;
			Size.Y = InSize.Y;
			PXR_LOGV(PxrMR, "UpdateVulkanTextureByLightEstimateInfo Success!");

			if (GetResource() == nullptr)
			{
				UpdateResource();
				PXR_LOGV(PxrMR, "UpdateResource Trigger!");
			}

			FlushRenderingCommands();
			PXR_LOGV(PxrMR, "UpdateVulkanTextureByLightEstimateInfo End!");

			return true;
		}
#endif
	}

	return false;
}

FMRVulkanResource::FMRVulkanResource(UPICOLightEstimationTexture* InOwner): Size(), Owner(InOwner)
{
	bGreyScaleFormat = false;
	bSRGB = InOwner->SRGB;
	EnvCubemapTextureRHIRef =nullptr;
}

void FMRVulkanResource::InitRHI(FRHICommandListBase& RHICmdList)
{
	Size.X = Size.Y = Owner->GetSize().X;
	FTextureRHIRef NewEnvironmentTexture = nullptr;
#if PLATFORM_ANDROID
	IVulkanDynamicRHI* VulkanRHI = GetIVulkanDynamicRHI();
	VkImage VulkanTexture = Owner->GetVulkanTexture();

	if (VulkanTexture != VK_NULL_HANDLE)
	{
		PXR_LOGV(PxrMR, "NewEnvironmentTexture RHICreateTexture!");

		ETextureCreateFlags LightEstimationTexCreateFlags = TexCreate_UAV|TexCreate_SRGB;
		NewEnvironmentTexture=VulkanRHI->RHICreateTextureCubeFromResource(PF_FloatRGBA,Size.X,false,1,1,VulkanTexture,LightEstimationTexCreateFlags,FClearValueBinding::None);
	}
	else
	{
		return;
	}
#endif

	if (NewEnvironmentTexture.IsValid())
	{
		if (!EnvCubemapTextureRHIRef.IsValid())
		{
			PXR_LOGV(PxrMR, "EnvCubemapTextureRHIRef RHICreateTexture!");
			const FRHITextureCreateDesc Desc =FRHITextureCreateDesc::CreateCube(TEXT("FMRVulkanResource"), Size.X, PF_FloatRGBA)
			.SetFlags(ETextureCreateFlags::SRGB
			|ETextureCreateFlags::RenderTargetable
			|ETextureCreateFlags::ShaderResource
			|ETextureCreateFlags::UAV
			).SetClearValue(FClearValueBinding::Green);
			EnvCubemapTextureRHIRef = RHICreateTexture(Desc);
		}

		PXR_LOGV(PxrMR, "CopyTextureCube_RenderThread Start!");
		
		//Runtime vkImage Copy to Unreal TextureCube
		CopyTextureCube_RenderThread(RHICmdList.GetAsImmediate(), EnvCubemapTextureRHIRef, NewEnvironmentTexture, FIntRect(), FIntRect());
		
		TextureRHI = EnvCubemapTextureRHIRef;
		TextureRHI->SetName(Owner->GetFName());
		RHIBindDebugLabelName(TextureRHI, *Owner->GetName());
		RHIUpdateTextureReference(Owner->TextureReference.TextureReferenceRHI, TextureRHI);

		// Create the sampler state RHI resource.
		FSamplerStateInitializerRHI SamplerStateInitializer(SF_Point, AM_Wrap, AM_Wrap, AM_Wrap);
		SamplerStateRHI = GetOrCreateSamplerState(SamplerStateInitializer);
	}
}

void FMRVulkanResource::CopyTextureCube_RenderThread(FRHICommandListImmediate& RHICmdList, FRHITexture* DstTexture, FRHITexture* SrcTexture, FIntRect DstRect, FIntRect SrcRect) const
{
	check(IsInRenderingThread());
	IRendererModule* RendererModule = &FModuleManager::GetModuleChecked<IRendererModule>("Renderer");

	FRHITexture* DstTextureCube = DstTexture->GetTextureCube();
	FRHITexture* SrcTextureCube = SrcTexture->GetTextureCube();
	FIntPoint DstSize;
	FIntPoint SrcSize;
	if (DstTextureCube && SrcTextureCube)
	{
		DstSize = FIntPoint(DstTextureCube->GetSize(), DstTextureCube->GetSize());
		SrcSize = FIntPoint(SrcTextureCube->GetSize(), SrcTextureCube->GetSize());
	}
	else
	{
		return;
	}

	if (DstRect.IsEmpty())
	{
		DstRect = FIntRect(FIntPoint::ZeroValue, DstSize);
	}

	if (SrcRect.IsEmpty())
	{
		SrcRect = FIntRect(FIntPoint::ZeroValue, SrcSize);
	}

	const uint32 ViewportWidth = DstRect.Width();
	const uint32 ViewportHeight = DstRect.Height();
	const FIntPoint TargetSize(ViewportWidth, ViewportHeight);
	float U = SrcRect.Min.X / (float)SrcSize.X;
	float V = SrcRect.Min.Y / (float)SrcSize.Y;
	float USize = SrcRect.Width() / (float)SrcSize.X;
	float VSize = SrcRect.Height() / (float)SrcSize.Y;

	FRHITexture* SrcTextureRHI = SrcTexture;
	RHICmdList.Transition(FRHITransitionInfo(DstTexture, ERHIAccess::Unknown, ERHIAccess::RTV));
	RHICmdList.Transition(FRHITransitionInfo(SrcTexture, ERHIAccess::Unknown, ERHIAccess::CopySrc));

    FGraphicsPipelineStateInitializer GraphicsPSOInit;
	
	GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGBA, BO_Add, BF_SourceAlpha, BF_Zero, BO_Add, BF_One, BF_Zero>::GetRHI();

	GraphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
	GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
	GraphicsPSOInit.PrimitiveType = PT_TriangleList;

	const auto cFeatureLevel = GMaxRHIFeatureLevel;
	auto ShaderMap = GetGlobalShaderMap(cFeatureLevel);
	TShaderMapRef<FScreenVS> VertexShader(ShaderMap);
	GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GFilterVertexDeclaration.VertexDeclarationRHI;
    GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
	
	if (DstTextureCube)
	{
		for (int FaceIndex = 0; FaceIndex < 6; FaceIndex++)
		{
			FRHIRenderPassInfo RPInfo(DstTexture, ERenderTargetActions::Load_Store);

			float FaceU = U;
			float FaceUSize = USize;
			float FaceV = V;
			float FaceVSize = VSize;
			
			// On Vulkan the positive and negative Y faces of the cubemap need to be flipped
			int NewFaceIndex = 0;
			
			if (FaceIndex == VULKAN_CUBEMAP_POSITIVE_Y)
				NewFaceIndex = VULKAN_CUBEMAP_NEGATIVE_Y;
			else if (FaceIndex == VULKAN_CUBEMAP_NEGATIVE_Y)
				NewFaceIndex = VULKAN_CUBEMAP_POSITIVE_Y;
			else
				NewFaceIndex = FaceIndex;
			
			int NewFaceOrder[6] = {2, 3, 4, 5,0, 1};
			NewFaceIndex = NewFaceOrder[FaceIndex];
			RPInfo.ColorRenderTargets[0].ArraySlice = NewFaceIndex;
			
			RHICmdList.BeginRenderPass(RPInfo, TEXT("CopyTextureFace"));
			{
				RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);

				TShaderMapRef<FPICOLightCubemapPS> PixelShader(ShaderMap);
				GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
				SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0);
				FRHISamplerState* SamplerState = DstRect.Size() == SrcRect.Size() ? TStaticSamplerState<SF_Point>::GetRHI() : TStaticSamplerState<SF_Bilinear>::GetRHI();

				FRHIBatchedShaderParameters& BatchedParameters = RHICmdList.GetScratchShaderParameters();
				PixelShader->SetParameters(BatchedParameters, SamplerState, SrcTextureRHI, FaceIndex);
				RHICmdList.SetBatchedShaderParameters(RHICmdList.GetBoundPixelShader(), BatchedParameters);

				RHICmdList.SetViewport(DstRect.Min.X, DstRect.Min.Y, 0.0f, DstRect.Max.X, DstRect.Max.Y, 1.0f);

				RendererModule->DrawRectangle(
				RHICmdList,
				0, 0, ViewportWidth, ViewportHeight,
				FaceU, FaceV, FaceUSize, FaceVSize,
				TargetSize,
				FIntPoint(1, 1),
				VertexShader,
				EDRF_Default);
			}
			RHICmdList.EndRenderPass();
		}
	}
	RHICmdList.Transition(FRHITransitionInfo(DstTexture, ERHIAccess::RTV, ERHIAccess::SRVMask));
}