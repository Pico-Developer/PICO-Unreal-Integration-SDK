// Fill out your copyright notice in the Description page of Project Settings.


#include "PICO_MRTextures.h"
#include "CommonRenderResources.h"
#include "PICO_MRTypes.h"
#include "ScreenRendering.h"
#include "RHIStaticStates.h"
#include "Modules/ModuleManager.h"
#include "RendererInterface.h"
#include "PICOOpenXRHMD/Private/PICO_Shaders.h"

#define VULKAN_CUBEMAP_POSITIVE_Y 2
#define VULKAN_CUBEMAP_NEGATIVE_Y 3

ULightEstimationTexturePICO::ULightEstimationTexturePICO(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer), SourceCubemapAngle(0), Size(128, 128)
{
	SRGB = true;
}

void ULightEstimationTexturePICO::BeginDestroy()
{
	Super::BeginDestroy();
}

FTextureResource* ULightEstimationTexturePICO::CreateResource()
{
	return new FMRVulkanResource(this);
}

#if PLATFORM_ANDROID
bool ULightEstimationTexturePICO::UpdateVulkanTextureByLightEstimateInfo(const FIntPoint& InSize, const XrLightEstimationInfoEnvironmentTextureVulkanPICO& InEstimationTexture)
{
	VkImage TempTexture = reinterpret_cast<VkImage>(InEstimationTexture.image);
	if (TempTexture != VK_NULL_HANDLE)
	{
		VulkanTexture = TempTexture;
		Size.X = InSize.X;
		Size.Y = InSize.Y;

		if (GetResource() == nullptr)
		{
			UpdateResource();
		}

		FlushRenderingCommands();
		UE_LOG(LogMRPICO, Error, TEXT("UpdateVulkanTextureByLightEstimateInfo FlushRenderingCommands!"));
		return true;
	}
	return false;
}
#endif

FMRVulkanResource::FMRVulkanResource(ULightEstimationTexturePICO* InOwner): Size(), Owner(InOwner)
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
			const FRHITextureCreateDesc Desc =FRHITextureCreateDesc::CreateCube(TEXT("FMRVulkanResource"), Size.X, PF_FloatRGBA)
			.SetFlags(ETextureCreateFlags::SRGB
			|ETextureCreateFlags::RenderTargetable
			|ETextureCreateFlags::ShaderResource
			|ETextureCreateFlags::UAV
			).SetClearValue(FClearValueBinding::Green);
			EnvCubemapTextureRHIRef = RHICreateTexture(Desc);
		}
		
		//Runtime vkImage Copy to Unreal TextureCube
		CopyTextureCube_RenderThread(RHICmdList.GetAsImmediate(), EnvCubemapTextureRHIRef, NewEnvironmentTexture, FIntRect(), FIntRect());
		UE_LOG(LogMRPICO, Error, TEXT("CopyTextureCube_RenderThread!"));

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

				TShaderMapRef<FPICOCubemapPS> PixelShader(ShaderMap);
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